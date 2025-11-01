# 📋 УСТАНОВКА МОДУЛЯ mod-custom-titles

## ШАГ 1: Применить SQL в базы данных

### 1.1 База данных WORLD (acore_world_test)

```powershell
# Создание таблицы custom_titles (кастомные титулы)
Get-Content "modules\mod-custom-titles\data\sql\db-world\base\custom_titles.sql" | mysql -h 185.135.81.201 -u azeroth_server -pJa1YDYNCMdQifZB --default-character-set=utf8mb4 acore_world_test

# Добавление NPC продавца в Даларане
Get-Content "modules\mod-custom-titles\data\sql\db-world\npc_title_vendor.sql" | mysql -h 185.135.81.201 -u azeroth_server -pJa1YDYNCMdQifZB --default-character-set=utf8mb4 acore_world_test

# (Опционально) Тестовый титул ID 178
Get-Content "test_custom_title.sql" | mysql -h 185.135.81.201 -u azeroth_server -pJa1YDYNCMdQifZB --default-character-set=utf8mb4 acore_world_test
```

### 1.2 База данных CHARACTERS (acore_characters)

```powershell
# Создание таблицы character_custom_titles (титулы персонажей)
Get-Content "modules\mod-custom-titles\data\sql\db-characters\base\character_custom_titles.sql" | mysql -h 185.135.81.201 -u azeroth_server -pJa1YDYNCMdQifZB --default-character-set=utf8mb4 acore_characters
```

---

## ШАГ 2: Компиляция сервера

```bash
./acore.sh compiler build
```

Или напрямую через cmake:
```bash
cd build
cmake --build . --target all -j $(nproc)
```

---

## ШАГ 3: Копирование конфига

```powershell
# Скопировать конфиг в папку сервера
Copy-Item "modules\mod-custom-titles\conf\mod_custom_titles.conf.dist" -Destination "путь_к_серверу\configs\modules\mod_custom_titles.conf"
```

Или просто создайте файл `mod_custom_titles.conf` в папке configs с содержимым:
```properties
CustomTitles.Enable = 1
CustomTitles.AnnounceOnLogin = 1
CustomTitles.ShowInChat = 1
CustomTitles.ShowAboveHead = 1
```

---

## ШАГ 4: Перезапуск сервера

```bash
# Остановить
pkill worldserver
pkill authserver

# Запустить
./acore.sh run-authserver
./acore.sh run-worldserver
```

---

## 📊 СОЗДАННЫЕ ТАБЛИЦЫ

### Таблица: `custom_titles` (acore_world_test)
Хранит все кастомные титулы с переводами на разные языки.

**Поля:**
- `ID` - уникальный ID титула
- `name13` - название на русском (мужское)
- `name_female13` - название на русском (женское)
- `mask_ID` - битовый индекс
- `required_level` - требуемый уровень
- `required_achievement` - требуемое достижение
- `required_item` - требуемый предмет
- `cost` - стоимость (в золоте / эмблемах)

### Таблица: `character_custom_titles` (acore_characters)
Хранит купленные титулы персонажей.

**Поля:**
- `guid` - GUID персонажа
- `title_id` - ID титула из custom_titles
- `mask_id` - битовый индекс
- `is_active` - активен ли титул (1 = да, 0 = нет)
- `purchased_date` - дата покупки

---

## 🎮 ИСПОЛЬЗОВАНИЕ В ИГРЕ

### NPC Продавец
**Локация:** Даларан, Крашус Приземление (координаты: 5822, 589, 660)
**Имя:** Торговец Титулами

**Меню NPC:**
1. **Купить титул за Эмблемы Льда** (ID 49426)
   - Стоимость = поле `cost` из таблицы
2. **Купить титул за золото**
   - Стоимость = поле `cost` * 10000 copper
3. **Мои титулы**
   - Просмотр купленных титулов
   - Активация выбранного титула

### Команды в чате

**Для игроков:**
```
.ctitle list              - Показать все кастомные титулы
.ctitle set <ID>          - Установить и активировать титул
.ctitle remove            - Снять кастомный титул
```

**Для GM:**
```
.ctitle add <игрок> <ID>     - Выдать титул игроку
.ctitle revoke <игрок> <ID>  - Забрать титул у игрока
.ctitle reload               - Перезагрузить титулы из БД
```

---

## ✅ ПРОВЕРКА УСТАНОВКИ

### В логе worldserver должно быть:
```
>> Loading custom titles...
>> Loaded X custom titles from custom_titles table.
```

### Проверка в игре:
1. Команда `.ctitle list` должна показать доступные титулы
2. NPC "Торговец Титулами" должен быть в Даларане
3. Купите тестовый титул (ID 178) за эмблемы или золото
4. Активируйте через меню "Мои титулы"

---

## 🔧 УСТРАНЕНИЕ ПРОБЛЕМ

### Ошибка: "Table custom_titles doesn't exist"
- Примените SQL: `custom_titles.sql`

### Ошибка: "Table character_custom_titles doesn't exist"
- Примените SQL: `character_custom_titles.sql`

### NPC не появляется в игре
- Проверьте что SQL применен: `npc_title_vendor.sql`
- Перезагрузите сервер
- Используйте команду: `.npc info` возле координат NPC

### Титулы не загружаются
- Проверьте лог worldserver на ошибки
- Убедитесь что `CustomTitles.Enable = 1` в конфиге
- Используйте команду `.ctitle reload`

---

## 📦 СПИСОК SQL ФАЙЛОВ

1. ✅ `modules/mod-custom-titles/data/sql/db-world/base/custom_titles.sql`
2. ✅ `modules/mod-custom-titles/data/sql/db-world/npc_title_vendor.sql`
3. ✅ `modules/mod-custom-titles/data/sql/db-characters/base/character_custom_titles.sql`
4. ✅ `test_custom_title.sql` (опционально - тестовый титул)

Всё готово! 🚀
