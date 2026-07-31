# Задача 2. systemd-сервис

**Цель.** Сделать из проекта systemd-сервис, управляемый командами:

```bash
sudo systemctl start   calc
sudo systemctl restart calc
sudo systemctl stop    calc
```

Опирается на graceful-завершение из [Задачи 1](task1-signal-handling.md): `systemctl stop`
по умолчанию шлёт `SIGTERM`, который наш поток сигналов обрабатывает штатно.

---

## 1. Unit-файл

Новый файл `packaging/calc.service`:

```ini
[Unit]
Description=calc calculation service
After=network-online.target postgresql.service
Wants=network-online.target

[Service]
Type=simple
EnvironmentFile=/etc/calc/calc.env
ExecStart=/usr/bin/calc
Restart=on-failure
RestartSec=2
KillSignal=SIGTERM        # (по умолчанию, указано явно) → graceful из Задачи 1
TimeoutStopSec=10         # сколько ждать перед SIGKILL
User=calc
Group=calc

[Install]
WantedBy=multi-user.target
```

**Пояснения**

- `Type=simple` — процесс не форкается и не демонизируется; systemd считает его
  запущенным сразу. (Альтернатива — `Type=notify` + `sd_notify(READY=1)` после
  привязки сокета; требует `libsystemd`. См. §5, D1.)
- `EnvironmentFile` — конфигурация сервиса (в т.ч. `DB_*`, которые `getConfig()` уже
  читает из окружения). Файл поставляется отдельно (см. §2).
- `KillSignal=SIGTERM` + `TimeoutStopSec` — корректная остановка; если сервис не успел
  за таймаут, systemd добьёт `SIGKILL`.
- `User/Group=calc` — непривилегированный системный пользователь (создаётся в postinst,
  см. [Задачу 4](task4-deb-package-cpack.md)).
- `restart` в systemd = `stop`+`start`, отдельной настройки не требует.

---

## 2. Файл конфигурации

Новый файл `packaging/calc.env` (устанавливается в `/etc/calc/calc.env`):

```ini
# --- Database ---
DB_HOST=127.0.0.1
DB_PORT=5432
DB_NAME=calc
DB_USER=calc
DB_PASSWORD=change_me

# --- Server (используется начиная с Задачи 3) ---
CALC_HOST=0.0.0.0
CALC_PORT=5555
```

> Пароль в примере — плейсхолдер. Файл кладётся в `/etc`, при упаковке помечается как
> `conffile` (Задача 4), чтобы обновление пакета не затирало правки администратора.

---

## 3. Установка через CMake

В `CMakeLists.txt` (эти же правила позже питают CPack в Задаче 4):

```cmake
include(GNUInstallDirs)

install(TARGETS ${PROJECT_NAME}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})          # /usr/bin/calc

install(FILES packaging/calc.service
    DESTINATION /lib/systemd/system)

install(FILES packaging/calc.env
    DESTINATION ${CMAKE_INSTALL_SYSCONFDIR}/calc)         # /etc/calc/
```

После установки вручную (без пакета) нужен `sudo systemctl daemon-reload`
(при установке из deb это делает postinst — Задача 4).

---

## 4. Проверка

> Для ручной проверки (без deb) заранее создай системного пользователя, иначе `User=calc`
> не даст сервису стартовать (в deb это делает postinst — [Задача 4](task4-deb-package-cpack.md)):
> `sudo useradd --system --no-create-home --shell /usr/sbin/nologin calc`

```bash
sudo cmake --build build --target install
sudo systemctl daemon-reload
sudo systemctl start  calc && systemctl status calc     # active (running)
sudo systemctl restart calc
sudo systemctl stop   calc                              # SIGTERM → graceful
journalctl -u calc -n 50                                # логи spdlog (stderr → journal)
```

Ожидаемо: в `journalctl` видно «Service started» при старте и запись о graceful-остановке
при stop.

---

## 5. Точки принятия решений

- **D1. `Type=simple` vs `notify`.** Рекомендация — `simple` (проще, без доп. зависимостей).
  `notify` точнее сообщает systemd о готовности после `bind()`, но требует `libsystemd`
  и вызова `sd_notify`.
- **D2. Автозапуск.** Оставляем ручное управление (`start/stop`), как в ТЗ. При желании —
  `systemctl enable calc` (или `[Install] WantedBy` + enable в postinst).
- **D3. Каталог unit-файла.** `/lib/systemd/system` (для пакетов) vs `/etc/systemd/system`
  (локально). Для deb — `/lib/systemd/system`.

---

## 6. Definition of Done

- [ ] `packaging/calc.service` и `packaging/calc.env` добавлены.
- [ ] `install()`-правила для бинарника, unit-файла и конфига.
- [ ] `systemctl start/stop/restart calc` работают; `stop` завершает сервис gracefully.
- [ ] Логи видны в `journalctl -u calc`.

**Соседние документы:** [Задача 1](task1-signal-handling.md) ·
[Задача 3](task3-network-boost-asio.md) · [Задача 4](task4-deb-package-cpack.md).
