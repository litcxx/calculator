# Задача 4. deb-пакет через CPack

**Цель.** Собрать deb-пакет сервиса средствами CPack. В пакет входят бинарник и
конфигурационные файлы (unit-файл systemd и env-конфиг из
[Задачи 2](task2-systemd-service.md)).

---

## 1. Предпосылка: полные install-правила

CPack пакует то, что описано в `install()`. К этому моменту должны быть заданы
(частично добавлены в Задаче 2):

```cmake
include(GNUInstallDirs)
install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}) # /usr/bin/calc
install(FILES packaging/calc.service DESTINATION /lib/systemd/system)
install(FILES packaging/calc.env     DESTINATION ${CMAKE_INSTALL_SYSCONFDIR}/calc) # /etc/calc
```

---

## 2. Конфигурация CPack

В конец корневого `CMakeLists.txt`:

```cmake
set(CPACK_GENERATOR "DEB")
set(CPACK_PACKAGE_NAME "calc")
set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION})            # 2.0
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "calc calculation service")
set(CPACK_PACKAGE_CONTACT "maintainer@example.com")

# Архитектуру CPack определяет сам (dpkg --print-architecture) — вручную НЕ задаём
# (пустая строка сломала бы поле Architecture).
set(CPACK_DEBIAN_PACKAGE_SECTION "utils")
set(CPACK_DEBIAN_PACKAGE_PRIORITY "optional")

# Runtime-зависимости определяем автоматически по разделяемым библиотекам
# (подтянет libpq5, libboost-system, libstdc++6 нужных версий под дистрибутив сборки):
set(CPACK_DEBIAN_PACKAGE_SHLIBDEPS ON)                  # требует dpkg-shlibdeps
# Fallback при выключенном SHLIBDEPS (версия boost зависит от дистрибутива):
# set(CPACK_DEBIAN_PACKAGE_DEPENDS "libpq5, libboost-system1.83.0")

# conffiles + maintainer-скрипты (см. §3)
set(CPACK_DEBIAN_PACKAGE_CONTROL_EXTRA
    "${CMAKE_SOURCE_DIR}/packaging/deb/postinst"
    "${CMAKE_SOURCE_DIR}/packaging/deb/prerm"
    "${CMAKE_SOURCE_DIR}/packaging/deb/postrm"
    "${CMAKE_SOURCE_DIR}/packaging/deb/conffiles")

include(CPack)
```

> `SHLIBDEPS ON` требует `dpkg-shlibdeps` и корректно слинкованного бинарника; версия
> boost-пакета в fallback зависит от дистрибутива (1.74 на Ubuntu 22.04, 1.83 на 24.04),
> поэтому в общем случае надёжнее автоопределение.

---

## 3. Maintainer-скрипты и conffiles

Сервисному пакету нужны скрипты жизненного цикла (`packaging/deb/`):

**`postinst`** — после установки:

```bash
#!/bin/sh
set -e
# системный пользователь для сервиса
getent group calc >/dev/null || addgroup --system calc
getent passwd calc >/dev/null || adduser --system --ingroup calc --no-create-home calc
systemctl daemon-reload || true
# по желанию: systemctl enable --now calc
```

**`prerm`** — перед удалением: остановить сервис:

```bash
#!/bin/sh
set -e
systemctl stop calc || true
systemctl disable calc || true
```

**`postrm`** — после удаления: перечитать unit'ы:

```bash
#!/bin/sh
set -e
systemctl daemon-reload || true
```

**`conffiles`** — чтобы dpkg не затирал правки конфига при обновлении:

```
/etc/calc/calc.env
```

Все скрипты — исполняемые (`chmod +x`), с корректными shebang.

---

## 4. Сборка пакета

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
cd build && cpack -G DEB            # → calc-2.0-Linux.deb
```

Проверка содержимого и установка:

```bash
dpkg -c build/calc-*.deb           # список файлов: /usr/bin/calc, /lib/systemd/system, /etc/calc
sudo dpkg -i build/calc-*.deb      # postinst: пользователь + daemon-reload
systemctl start calc               # проверка запуска (см. Задача 2)
sudo dpkg -r calc                  # prerm: stop; чистое удаление
```

---

## 5. Точки принятия решений

- **D1. `SHLIBDEPS` vs ручной `DEPENDS`.** Рекомендация — включить `SHLIBDEPS` (точнее),
  оставив ручной список как страховку.
- **D2. Автозапуск после установки.** По умолчанию — не включать (соответствует ТЗ:
  запуск вручную). При желании — `systemctl enable --now calc` в `postinst`.
- **D3. Имя/версия файла пакета.** По умолчанию `calc-2.0-Linux.deb`; можно задать
  `CPACK_DEBIAN_FILE_NAME "DEB-DEFAULT"` для канонического `calc_2.0_amd64.deb`.

---

## 6. Definition of Done

- [ ] `install()` покрывает бинарник, unit-файл и конфиг.
- [ ] CPack настроен на `DEB`: имя, версия, архитектура, зависимости.
- [ ] `postinst`/`prerm`/`postrm`/`conffiles` добавлены и исполняемы.
- [ ] `cpack -G DEB` собирает пакет; `dpkg -i` ставит, сервис стартует; `dpkg -r` чисто удаляет.
- [ ] `/etc/calc/calc.env` помечен conffile (правки переживают обновление).

**Соседние документы:** [Задача 1](task1-signal-handling.md) ·
[Задача 2](task2-systemd-service.md) · [Задача 3](task3-network-boost-asio.md).
