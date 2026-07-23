# Задача 3. Клиент-серверная архитектура (Boost.Asio)

**Цель.** Сервис принимает задание на расчёт по сети от клиента, считает и отправляет
результат обратно. Сетевой слой — на **Boost.Asio**. В базовой части — **один клиент
за раз**. Нужен тестовый клиент, проверяющий именно результат расчёта.

Здесь появляется «поток данных» из требования ТЗ: `io_context.run()` крутится в главном
(рабочем) потоке, а поток сигналов из [Задачи 1](task1-signal-handling.md) останавливает
его через `io_context.stop()`.

---

## 1. Протокол

Простой строчный протокол поверх TCP (переиспользует существующий JSON-формат парсера):

- **Запрос:** одна строка JSON, завершённая `\n`:
  `{"first":2,"second":3,"operation":"add"}\n`
- **Ответ:** одна строка результата, завершённая `\n` — тот же текст, что сейчас печатает
  `StdoutPrinter`, например `Success : 5\n` или `Error: Divide by zero\n`.

Строчное кадрирование (`\n`) снимает проблему частичных чтений: на приёме используем
`asio::async_read_until(sock, buf, '\n')`. (Альтернатива — JSON-ответ `{"status","result"}`,
см. §7, D1.)

---

## 2. Переиспользование доменной логики

Расчёт уже реализован (`Parser`, `Calculator`, `Repository`, `Cache`). Вынесем сценарий
«строка запроса → строка ответа» в переиспользуемый обработчик, чтобы им пользовались и
сервер, и тесты.

- Новый **`RequestHandler`** (`include/app/request_handler.hpp`), собирается из тех же
  интерфейсов, что и сегодняшний `Application`:

  ```cpp
  class RequestHandler
  {
    public:
      RequestHandler(std::unique_ptr<IRepository>, std::unique_ptr<IParser>,
                     std::unique_ptr<ICalculator>);
      std::string handle(std::string_view requestJson);  // возвращает строку ответа
  };
  ```

  Внутри — текущая логика `Application::run` без ввода/вывода: `parse` → `repository.get`
  (cache hit) → иначе `calculator.calculate` + `repository.save` → форматирование ответа.
- `handle()` **ловит исключения** парсера/расчёта и возвращает строку-ошибку — битый
  запрос клиента даёт ответ об ошибке, а не роняет сессию/сервер.
- Форматирование `Task → std::string` выносим из `StdoutPrinter` в свободную функцию
  `std::string toResponse(const Task&)` (переиспользуют и printer, и `RequestHandler`).
- Переименование: `app/application.{hpp,cpp}` → `app/request_handler.{hpp,cpp}`,
  `test/unit/application.cpp` → `test/unit/request_handler.cpp` (CLI-обёртка `run(argc,argv)`
  сервису не нужна). Моки в тестах те же.

---

## 3. Сетевой слой (Asio)

Новые файлы `include/net/server.hpp` + `src/net/server.cpp` (+ `session`).

- **`Server`**: работает с **внешним** `io_context` (владеет `main`/тест), держит
  `tcp::acceptor` (bind+listen в конструкторе на `CALC_HOST:CALC_PORT`) и ссылку на
  `RequestHandler`.
  - `Server(asio::io_context&, uint16_t port, RequestHandler&)` — bind+listen и запуск
    цепочки `async_accept` в конструкторе.
  - `port()` — фактический порт (для тестов с эфемерным портом `0`).
  - Жизненным циклом управляет владелец `io_context`: `io.run()` / `io.stop()`
    (у `Server` своих `run/stop` нет — так проще и однозначнее).
- **Приём соединений** — последовательный (один клиент за раз): `async_accept` →
  обслужить сессию → снова `async_accept`.
- **`Session`** (`std::enable_shared_from_this`): цикл `async_read_until('\n')` →
  `handler.handle(line)` → `async_write(response)` → снова чтение, пока клиент не
  закроет соединение (EOF).

Скелет:

```cpp
void Server::doAccept()
{
    acceptor_.async_accept([this](auto ec, tcp::socket sock) {
        if (ec) { return; }                       // stop()/close → выходим
        std::make_shared<Session>(std::move(sock), handler_)->start();
        doAccept();                               // следующий клиент — после текущего
    });
}
```

---

## 4. Интеграция с сигналами и потоками (итоговая модель)

```cpp
int main()
{
    try
    {
        SignalHandler signals;                     // (1) блокирует сигналы ПЕРВЫМ

        const Config config = getConfig();
        auto repository = std::make_unique<Repository>(
            ConnectionPool(1, config), Cache{});
        RequestHandler handler(std::move(repository),
                               std::make_unique<Parser>(),
                               std::make_unique<Calculator>());

        boost::asio::io_context io;
        Server server(io, serverPort(), handler);  // bind + listen

        signals.start([&io] { io.stop(); });       // (2) поток №1 — сигналы
        Logger::getInstance().info("Listening...");
        io.run();                                   // поток №2 — данные; блок до stop()

        Logger::getInstance().info("Shutting down gracefully");
    }
    catch (const std::exception& ec)
    {
        calculator::Logger::getInstance().error(ec.what());
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
```

- **Потоков ≥2:** сигналы (`sigwait`) + данные (`io.run()`) — требование ТЗ выполнено.
- **ThreadSanitizer (итоговая проверка):** обработчики Asio исполняются в единственном
  потоке `io.run()`, поэтому `Repository`/`Cache` доступны только из него — гонок нет.
  Поток сигналов лишь вызывает `io.stop()` (потокобезопасно). Прогон сборки с
  `-fsanitize=thread` на сетевом тесте (§5) — ожидается чисто, без гонок и deadlock.

---

## 5. Тесты

Основная тестовая нагрузка — здесь. Два уровня.

### 5.1. Юнит-тест `RequestHandler` (gmock, без сети и БД)

Переносим `test/unit/application.cpp` → `request_handler.cpp`; моки
`MockRepository/MockParser/MockCalculator` те же, проверяем возвращённую строку:

- **cache hit:** `get` вернул `Task` → строка результата, `calculate` **не вызван** (`.Times(0)`);
- **cache miss:** `calculate` + `save`, затем строка;
- **битый JSON:** `parse` бросает → `handle` возвращает строку-ошибку;
- **формат** (`toResponse`) — табличный тест по всем `Status`.

Логику кэша проверяет именно этот уровень: по сети её не видно — ответ одинаков.

### 5.2. Сетевой тест с тестовым клиентом (end-to-end, без БД)

Отдельная цель `network_tests` (нужен Boost), CTest-label `network`. БД не поднимаем:
в `RequestHandler` внедряем **`FakeRepository`** — простую in-memory реализацию
`IRepository` (`get`/`save` поверх `std::vector<Task>` + `operator==`). Тест герметичен,
годится для CI и прогона под TSan.

Схема — детерминированная, без `sleep`:

1. `Server` на порту `0`; `bind()` синхронен → порт доступен сразу (`server.port()`).
2. `io.run()` — в фоновом `std::thread`.
3. Клиент Boost.Asio: `connect(port)` → `write(json+"\n")` → `read_until('\n')` → сверить.
4. **Дедлайн на каждую операцию:** оборачиваем в async + `steady_timer` и `io.run_for(...)`
   (или ставим таймаут сокета) — зависший тест падает по времени, а не висит. Ожидание
   готовности сервера — не `sleep`, а синхронный `bind` из п.1.
5. **Teardown в фикстуре** (`TearDown`): `io.stop()` + `join()` — срабатывает даже при
   провале `ASSERT_*`, иначе фоновый поток утечёт и сломает процесс тестов/TSan.

Синхронный клиент-хелпер (для наглядности; в тесте — с дедлайном, см. п.4):

```cpp
using tcp = boost::asio::ip::tcp;
std::string request(uint16_t port, std::string_view json)
{
    boost::asio::io_context io;
    tcp::socket sock(io);
    sock.connect({boost::asio::ip::make_address("127.0.0.1"), port});
    boost::asio::write(sock, boost::asio::buffer(std::string(json) + "\n"));
    boost::asio::streambuf buf;
    boost::asio::read_until(sock, buf, '\n');
    return {std::istreambuf_iterator<char>(&buf), {}};
}
```

Проверяемые случаи (сверяем результат расчёта):

| Запрос | Ответ |
|---|---|
| `{"first":2,"second":3,"operation":"add"}` | `Success : 5\n` |
| `{"first":10,"second":0,"operation":"div"}` | `Error: Divide by zero\n` |
| повтор запроса | тот же ответ (идемпотентность; сам cache-hit проверяет 5.1) |

---

## 6. Сборка

- **Boost:** `find_package(Boost REQUIRED COMPONENTS system)` (CONFIG-режим на новых
  CMake); линковать `Boost::system` (Asio header-only) и `Threads::Threads`. Системный
  пакет — `libboost-dev` (runtime-зависимость deb — [Задача 4](task4-deb-package-cpack.md)).
- **Исходники (`src/CMakeLists.txt`):** в `SOURCES` заменить `app/application.cpp` на
  `app/request_handler.cpp`, добавить `net/server.cpp` (+ функция `toResponse`).
- **Тесты (`test/CMakeLists.txt`):** `unit/application.cpp` → `unit/request_handler.cpp`
  (в `unit_tests`); новая цель `network_tests` (`integration/network.cpp` +
  `FakeRepository`), линкуется с `Boost::system`/`Threads::Threads`, отдельный
  CTest-label `network`.
- **Порт из конфигурации:** читать `CALC_PORT`/`CALC_HOST` из окружения (как `DB_*`),
  с дефолтами; переменные заведены в `calc.env` ([Задача 2](task2-systemd-service.md)).

---

## 7. Точки принятия решений

- **D1. Формат ответа.** Рекомендация — текст (переиспользуем формат `StdoutPrinter`,
  минимум кода). Альтернатива — JSON `{"status","result"}` (строже парсится в тестах).
- **D2. Сессия.** Одно соединение = поток запросов до EOF (рекомендуется) vs один
  запрос/ответ и закрытие. ТЗ допускает оба (один клиент за раз).
- **D3. Многопоточность Asio.** База — один поток `io.run()` (проще, TSan-чисто).
  Несколько клиентов/потоков — вне базовой части ТЗ.

---

## 8. Definition of Done

- [ ] Логика расчёта вынесена в `RequestHandler` (`строка → строка`), формат ответа общий.
- [ ] `Server`/`Session` на Boost.Asio; последовательный приём, строчный протокол.
- [ ] `main`: `io.run()` в рабочем потоке, поток сигналов вызывает `io.stop()`.
- [ ] Юнит-тесты `RequestHandler` (cache hit/miss, битый JSON) переехали с `Application`.
- [ ] Сетевой `network_tests` с `FakeRepository` проверяет результаты расчёта; без `sleep`
      (дедлайны), teardown в фикстуре.
- [ ] `find_package(Boost)`, новые исходники и цели подключены в CMake.
- [ ] **ThreadSanitizer** на итоговой сборке (сетевой тест): без гонок и deadlock.

**Соседние документы:** [Задача 1](task1-signal-handling.md) ·
[Задача 2](task2-systemd-service.md) · [Задача 4](task4-deb-package-cpack.md).
