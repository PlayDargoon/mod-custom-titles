# Инструкция по установке модуля Custom Titles

## Шаг 1: Подготовка

Убедитесь, что у вас установлен и настроен AzerothCore.

## Шаг 2: Размещение модуля

Модуль уже находится в правильном месте: `modules/mod-custom-titles/`

## Шаг 3: Применение SQL скриптов

### Для базы данных World (acore_world):

```bash
# В PowerShell или через HeidiSQL/MySQL Workbench выполните:
mysql -u root -p acore_world < modules/mod-custom-titles/data/sql/db-world/base/custom_titles.sql
```

### Для базы данных Characters (acore_characters):

```bash
mysql -u root -p acore_characters < modules/mod-custom-titles/data/sql/db-characters/base/character_custom_titles.sql
```

## Шаг 4: Конфигурация

Скопируйте конфигурационный файл:

```bash
# В PowerShell:
Copy-Item modules\mod-custom-titles\conf\mod_custom_titles.conf.dist env\dist\etc\mod_custom_titles.conf
```

Или вручную создайте файл `env/dist/etc/mod_custom_titles.conf` со следующим содержимым:

```ini
CustomTitles.Enable = 1
CustomTitles.AnnounceOnLogin = 1
```

## Шаг 5: Компиляция

Перекомпилируйте сервер:

```bash
# Используйте встроенную задачу VS Code или выполните:
./acore.sh compiler build
```

Или используйте задачу в VS Code: `AzerothCore: Build`

## Шаг 6: Запуск сервера

После успешной компиляции запустите сервер:

```bash
./acore.sh run-worldserver
```

## Шаг 7: Проверка работы модуля

1. Войдите в игру на своем персонаже
2. Введите команду: `.ctitle list`
3. Вы должны увидеть список доступных кастомных званий

## Использование

### Команды для игроков:

- `.ctitle list` - Показать все доступные кастомные звания
- `.ctitle set <id>` - Установить звание (например: `.ctitle set 1`)
- `.ctitle remove` - Снять текущее звание

### Команды для администраторов:

- `.ctitle reload` - Перезагрузить звания из БД без перезапуска сервера

## Добавление новых званий

Чтобы добавить новые звания, выполните SQL запрос в базе `acore_world`:

```sql
INSERT INTO `custom_titles` 
(`id`, `name_male`, `name_female`, `required_level`, `required_achievement`, `required_item`, `cost`) 
VALUES 
(11, 'Мое Новое Звание', 'Мое Новое Звание', 70, 0, 0, 300);
```

Затем выполните в игре: `.ctitle reload`

## Устранение проблем

### Модуль не загружается

1. Проверьте логи worldserver на наличие ошибок
2. Убедитесь, что SQL скрипты применены правильно
3. Проверьте, что конфигурационный файл находится в нужной папке

### Команды не работают

1. Убедитесь, что в конфигурации `CustomTitles.Enable = 1`
2. Перезапустите worldserver
3. Проверьте логи на наличие ошибок загрузки модуля

### Звания не отображаются

1. Выполните `.ctitle reload` в игре
2. Проверьте, что таблица `custom_titles` заполнена данными
3. Проверьте логи на ошибки при загрузке званий

## Дополнительная настройка

Вы можете редактировать параметры в конфигурационном файле:

- `CustomTitles.Enable` - Включить/выключить модуль (1/0)
- `CustomTitles.AnnounceOnLogin` - Показывать звание при входе (1/0)

После изменения конфигурации необходимо перезапустить worldserver.
