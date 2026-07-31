# Задача 1. Обработка системных сигналов (SIGTERM)

**Цель.** Приложение перестаёт быть одноразовой CLI-утилитой и превращается в
долгоживущий процесс, который работает до получения `SIGTERM`, после чего
корректно (graceful) завершается.

**Контекст всего ДЗ.** Проект становится Linux-сервисом. По требованию ТЗ в нём
должно быть **минимум 2 потока**: поток обработки сигналов ОС и поток
получения/обработки данных. Эта задача закладывает потоковую модель и механизм
завершения; сетевой цикл обработки данных добавляется в
[Задаче 3](task3-network-boost-asio.md).

---

## 1. Модель сигналов и потоков

Идиоматичный для Linux-сервиса подход — **не** асинхронный обработчик, а
**выделенный поток, ожидающий сигнал через `sigwait`**:

1. В главном потоке **до создания любых потоков** блокируем `SIGTERM` (и `SIGINT`)
   через `pthread_sigmask(SIG_BLOCK, …)`. Блокировка наследуется всеми потоками,
   поэтому сигнал не будет доставлен асинхронно.
2. Выделенный **поток сигналов** вызывает `sigwait()` и блокируется до прихода сигнала.
3. Получив сигнал, поток инициирует завершение: выставляет атомарный флаг и вызывает
   колбэк остановки (в Задаче 3 — `io_context.stop()`).

```
 main (рабочий поток / поток данных)      поток сигналов
 ────────────────────────────────         ─────────────────
 pthread_sigmask(BLOCK, {SIGTERM,SIGINT})
 start signal thread ───────────────────▶ sigwait(&set) ──(ждёт)
 <работа: ждёт/обслуживает запросы>                │  ← SIGTERM
        ▲                                          ▼
        └──── onShutdown() / stopRequested ◀── выставить флаг, notify
 graceful cleanup (RAII) → join signal thread → exit
```

**Почему `sigwait`-поток лучше async-обработчика:** обработчик сигнала исполняется
в контексте прерывания и async-signal-safe — из него нельзя логировать (spdlog),
брать мьютексы, аллоцировать. Поток же после `sigwait` работает в обычном контексте:
может писать в лог, останавливать `io_context`, корректно освобождать ресурсы.

**Почему не `boost::asio::signal_set`:** он выполнял бы обработчик в том же потоке
`io_context.run()`, где идёт обработка данных, — получился бы один поток и на сигналы,
и на данные, что нарушает требование ТЗ о двух потоках. Выделенный `sigwait`-поток даёт
честное разделение «поток 1 — сигналы, поток 2 — данные».

---

## 2. Компонент `SignalHandler`

Новый класс инкапсулирует всю работу с сигналами.

**`include/utils/signal_handler.hpp`**

```cpp
#ifndef SIGNAL_HANDLER_HPP_
#define SIGNAL_HANDLER_HPP_

#include <atomic>
#include <functional>
#include <thread>

namespace calculator
{
class SignalHandler
{
  public:
    // Блокирует SIGTERM/SIGINT в вызывающем (главном) потоке.
    // ВАЖНО: создать ПЕРВЫМ, до запуска остальных потоков.
    SignalHandler();
    SignalHandler(const SignalHandler&) = delete;
    SignalHandler& operator=(const SignalHandler&) = delete;
    SignalHandler(SignalHandler&&) = delete;
    SignalHandler& operator=(SignalHandler&&) = delete;
    ~SignalHandler();                  // будит и join'ит поток сигналов

    // Запускает поток с sigwait. onShutdown вызывается при приходе сигнала
    // (напр. io_context.stop()); задаётся здесь, а не в конструкторе, т.к.
    // io_context создаётся уже после блокировки сигналов.
    void start(std::function<void()> onShutdown = {});
    void waitForShutdown();            // блокирует вызывающий поток до сигнала
    bool stopRequested() const noexcept;

  private:
    std::function<void()> onShutdown_;
    std::thread thread_;
    std::atomic<bool> stopRequested_{false};
};
} // namespace calculator

#endif
```

**`src/utils/signal_handler.cpp`** (ключевые моменты):

```cpp
#define _POSIX_C_SOURCE 200809L   // ДО include: открыть sigwait/pthread_sigmask

#include "utils/signal_handler.hpp"
#include <csignal>
#include <pthread.h>
#include <unistd.h>

namespace calculator
{
namespace
{
sigset_t terminationSet()
{
    sigset_t set;
    sigemptyset(&set);
    sigaddset(&set, SIGTERM);
    sigaddset(&set, SIGINT);
    return set;
}
} // namespace

SignalHandler::SignalHandler()
{
    sigset_t set = terminationSet();
    pthread_sigmask(SIG_BLOCK, &set, nullptr);   // блок во всех будущих потоках
}

void SignalHandler::start(std::function<void()> onShutdown)
{
    onShutdown_ = std::move(onShutdown);         // до создания потока → без гонки
    thread_ = std::thread([this] {
        sigset_t set = terminationSet();
        int sig = 0;
        sigwait(&set, &sig);                     // ждём SIGTERM/SIGINT
        stopRequested_.store(true, std::memory_order_relaxed);
        if (onShutdown_) { onShutdown_(); }      // напр. io_context.stop()
    });
}

void SignalHandler::waitForShutdown()
{
    if (thread_.joinable()) { thread_.join(); }  // блок до обработки сигнала, без spin
}

bool SignalHandler::stopRequested() const noexcept
{
    return stopRequested_.load(std::memory_order_relaxed);
}

SignalHandler::~SignalHandler()
{
    if (thread_.joinable())
    {
        // Сигнала не было (напр. исключение при старте) — будим sigwait.
        if (!stopRequested_.load()) { ::kill(::getpid(), SIGTERM); }
        thread_.join();
    }
}
} // namespace calculator
```

> `waitForShutdown()` просто join'ит поток сигналов (тот завершается сразу после обработки
> сигнала) — никакого busy-spin, `main` спит в ядре. В Задаче 3 `waitForShutdown()` не
> нужен: главный поток крутит `io_context.run()`, а `onShutdown_` его останавливает.

---

## 3. Изменения в `main()` (скелет сервиса)

`main` перестаёт обрабатывать `argv` и превращается в жизненный цикл сервиса. В этой
задаче тело сервиса — заглушка (ожидание сигнала); реальный сетевой цикл придёт в
Задаче 3.

```cpp
int main()
{
    try
    {
        SignalHandler signals;   // (1) блокирует сигналы в главном потоке (ПЕРВЫМ)
        signals.start();         // (2) поток сигналов (поток №1)

        const Config config = getConfig();
        Cache cache;
        ConnectionPool pool(1, config);
        auto repository =
            std::make_unique<Repository>(std::move(pool), std::move(cache));

        Logger::getInstance().info("Service started");

        // ЗАГЛУШКА до Задачи 3: главный (рабочий) поток №2 ждёт сигнала.
        signals.waitForShutdown();

        Logger::getInstance().info("SIGTERM received, shutting down gracefully");
        // RAII: соединения с БД закрываются здесь; ~SignalHandler join'ит поток.
    }
    catch (const std::exception& ec)
    {
        calculator::Logger::getInstance().error(ec.what());
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;   // код возврата — см. §6, D1
}
```

Потоков ровно два: поток сигналов и главный рабочий поток — требование ТЗ выполнено.

---

## 4. Сборка

- **pthread:** в корневом `CMakeLists.txt` — `find_package(Threads REQUIRED)`, линковать
  `Threads::Threads` к `calc_core` (и/или к `calc`).
- **Новый файл:** добавить `utils/signal_handler.cpp` в `SOURCES` (`src/CMakeLists.txt`).
- **Feature-макрос:** проект собирается с `-std=c++17` и `CMAKE_CXX_EXTENSIONS OFF`,
  поэтому POSIX-символы (`sigwait`, `pthread_sigmask`) скрыты без
  `#define _POSIX_C_SOURCE 200809L` в начале `.cpp` (до include) — уже учтено в §2.

---

## 5. Тестирование и ThreadSanitizer

- По ТЗ **тестировать обработку сигналов не нужно** — юнит-тесты для `SignalHandler`
  не пишем.
- **ThreadSanitizer** запускается на **итоговой** версии (после Задачи 3) — проверяет
  многопоточность на гонки и deadlock. Потоковая модель уже здесь спроектирована
  TSan-совместимой: общее состояние между потоками — только `std::atomic<bool>` и
  потокобезопасный колбэк (в Задаче 3 — `io_context.stop()`); никаких общих
  мьютексов/данных без синхронизации. Конкретный запуск — на сетевом тесте
  ([Задача 3](task3-network-boost-asio.md), §5), т.к. он реально задействует оба потока.
- Профиль TSan — отдельная опция и **отдельная папка сборки** (`-fsanitize=thread`
  несовместим с ASan): по аналогии с текущим `USE_SANITIZERS` добавить `USE_TSAN`,
  взаимоисключающую с ASan/Valgrind.

---

## 6. Точки принятия решений

- **D1. Код возврата при SIGTERM.** Рекомендация — `EXIT_SUCCESS` (штатная остановка
  сервиса; systemd так и ожидает при `KillSignal=SIGTERM`). Альтернатива — `128+SIGTERM=143`.
- **D2. Набор сигналов.** ТЗ требует `SIGTERM`; заодно ловим `SIGINT` (удобно при запуске
  из терминала). Оба ведут к graceful-остановке.
- **D3. Ожидание в `main`.** Реализуем через `join` потока сигналов (main спит в ядре),
  без busy-spin — бесконечный busy-цикл жёг бы ядро CPU. Требование ТЗ про «busy-циклы
  вместо sleep» относится к **тестам** (ожидание ответа сервера), а не к бесконечному
  ожиданию в сервисе.

---

## 7. Definition of Done

- [ ] `SignalHandler` реализован: блокировка сигналов + поток `sigwait` + `stopRequested()`.
- [ ] `main()` перестроен в жизненный цикл сервиса; работает до `SIGTERM`, затем graceful.
- [ ] Ровно ≥2 потока (сигналы + рабочий), поток сигналов корректно join'ится.
- [ ] `CMakeLists`: `Threads::Threads`, новый исходник, профиль TSan.
- [ ] Собирается в Debug/Release; `clang-tidy`/`clang-format` без замечаний.
- [ ] Ручная проверка: процесс не завершается сам; `kill -TERM <pid>` → лог о graceful-остановке и выход.

**Далее:** [Задача 2 — systemd](task2-systemd-service.md) ·
[Задача 3 — сеть/Asio](task3-network-boost-asio.md) ·
[Задача 4 — deb/CPack](task4-deb-package-cpack.md).
