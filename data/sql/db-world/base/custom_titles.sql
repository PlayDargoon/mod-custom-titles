-- --------------------------------------------------------
-- Custom Titles Module - World Database
-- --------------------------------------------------------

-- Таблица для хранения кастомных званий (DBC-подобная структура)
DROP TABLE IF EXISTS `chartitles_dbc`;
CREATE TABLE `chartitles_dbc` (
  `ID` INT UNSIGNED NOT NULL AUTO_INCREMENT COMMENT 'Уникальный ID звания',
  `condition_ID` INT UNSIGNED DEFAULT 0 COMMENT 'ID условия',
  `name1` VARCHAR(255) NOT NULL DEFAULT '' COMMENT 'Название (язык 1)',
  `name2` VARCHAR(255) NOT NULL DEFAULT '' COMMENT 'Название (язык 2)',
  `name3` VARCHAR(255) NOT NULL DEFAULT '' COMMENT 'Название (язык 3)',
  `name4` VARCHAR(255) NOT NULL DEFAULT '' COMMENT 'Название (язык 4)',
  `name5` VARCHAR(255) NOT NULL DEFAULT '' COMMENT 'Название (язык 5)',
  `name6` VARCHAR(255) NOT NULL DEFAULT '' COMMENT 'Название (язык 6)',
  `name7` VARCHAR(255) NOT NULL DEFAULT '' COMMENT 'Название (язык 7)',
  `name8` VARCHAR(255) NOT NULL DEFAULT '' COMMENT 'Название (язык 8)',
  `name9` VARCHAR(255) NOT NULL DEFAULT '' COMMENT 'Название (язык 9)',
  `name10` VARCHAR(255) NOT NULL DEFAULT '' COMMENT 'Название (язык 10)',
  `name11` VARCHAR(255) NOT NULL DEFAULT '' COMMENT 'Название (язык 11)',
  `name12` VARCHAR(255) NOT NULL DEFAULT '' COMMENT 'Название (язык 12)',
  `name13` VARCHAR(255) NOT NULL DEFAULT '' COMMENT 'Название (язык 13) - ruRU',
  `name14` VARCHAR(255) NOT NULL DEFAULT '' COMMENT 'Название (язык 14)',
  `name15` VARCHAR(255) NOT NULL DEFAULT '' COMMENT 'Название (язык 15)',
  `name16` VARCHAR(255) NOT NULL DEFAULT '' COMMENT 'Название (язык 16)',
  `name_mask` INT UNSIGNED NOT NULL DEFAULT 16712190 COMMENT 'Языковая маска для name',
  `name_female1` VARCHAR(255) NOT NULL DEFAULT '' COMMENT 'Название женское (язык 1)',
  `name_female2` VARCHAR(255) NOT NULL DEFAULT '' COMMENT 'Название женское (язык 2)',
  `name_female3` VARCHAR(255) NOT NULL DEFAULT '' COMMENT 'Название женское (язык 3)',
  `name_female4` VARCHAR(255) NOT NULL DEFAULT '' COMMENT 'Название женское (язык 4)',
  `name_female5` VARCHAR(255) NOT NULL DEFAULT '' COMMENT 'Название женское (язык 5)',
  `name_female6` VARCHAR(255) NOT NULL DEFAULT '' COMMENT 'Название женское (язык 6)',
  `name_female7` VARCHAR(255) NOT NULL DEFAULT '' COMMENT 'Название женское (язык 7)',
  `name_female8` VARCHAR(255) NOT NULL DEFAULT '' COMMENT 'Название женское (язык 8)',
  `name_female9` VARCHAR(255) NOT NULL DEFAULT '' COMMENT 'Название женское (язык 9)',
  `name_female10` VARCHAR(255) NOT NULL DEFAULT '' COMMENT 'Название женское (язык 10)',
  `name_female11` VARCHAR(255) NOT NULL DEFAULT '' COMMENT 'Название женское (язык 11)',
  `name_female12` VARCHAR(255) NOT NULL DEFAULT '' COMMENT 'Название женское (язык 12)',
  `name_female13` VARCHAR(255) NOT NULL DEFAULT '' COMMENT 'Название женское (язык 13) - ruRU',
  `name_female14` VARCHAR(255) NOT NULL DEFAULT '' COMMENT 'Название женское (язык 14)',
  `name_female15` VARCHAR(255) NOT NULL DEFAULT '' COMMENT 'Название женское (язык 15)',
  `name_female16` VARCHAR(255) NOT NULL DEFAULT '' COMMENT 'Название женское (язык 16)',
  `name_female_mask` INT UNSIGNED NOT NULL DEFAULT 16712490 COMMENT 'Языковая маска для name_female',
  `mask_ID` INT UNSIGNED NOT NULL DEFAULT 144 COMMENT 'Битовый индекс в PLAYER__FIELD_KNOWN_TITLES',
  `required_level` INT UNSIGNED DEFAULT 0 COMMENT 'Требуемый уровень (кастомное поле)',
  `required_achievement` INT UNSIGNED DEFAULT 0 COMMENT 'Требуемое достижение (кастомное поле)',
  `required_item` INT UNSIGNED DEFAULT 0 COMMENT 'Требуемый предмет (кастомное поле)',
  `cost` INT UNSIGNED DEFAULT 0 COMMENT 'Стоимость в золоте (кастомное поле)',
  PRIMARY KEY (`ID`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COMMENT='Таблица кастомных званий (DBC-подобная структура)';

-- Примеры кастомных званий
INSERT INTO `chartitles_dbc` VALUES 
(179, 9139, 'Test Title', '', '', '', '', '', '', '', '', '', '', 'Тестовое звание', '', '', '', '', 16712190, 'Test Title', '', '', '', '', '', '', '', '', '', '', 'Тестовое звание', '', '', '', '', 16712490, 144, 0, 0, 0, 0),
(180, 9140, 'Legendary Warrior', '', '', '', '', '', '', '', '', '', '', 'Легендарный Воин', '', '', '', '', 16712190, 'Legendary Warrior', '', '', '', '', '', '', '', '', '', '', 'Легендарная Воительница', '', '', '', '', 16712490, 145, 80, 0, 0, 100),
(181, 9141, 'Dragon Lord', '', '', '', '', '', '', '', '', '', '', 'Повелитель Драконов', '', '', '', '', 16712190, 'Dragon Lady', '', '', '', '', '', '', '', '', '', '', 'Повелительница Драконов', '', '', '', '', 16712490, 146, 70, 0, 0, 500),
(182, 9142, 'Arena Master', '', '', '', '', '', '', '', '', '', '', 'Мастер Арены', '', '', '', '', 16712190, 'Arena Master', '', '', '', '', '', '', '', '', '', '', 'Мастер Арены', '', '', '', '', 16712490, 147, 60, 0, 0, 250),
(183, 9143, 'Champion of Azeroth', '', '', '', '', '', '', '', '', '', '', 'Чемпион Азерота', '', '', '', '', 16712190, 'Champion of Azeroth', '', '', '', '', '', '', '', '', '', '', 'Чемпионка Азерота', '', '', '', '', 16712490, 148, 80, 0, 0, 1000),
(184, 9144, 'Unstoppable', '', '', '', '', '', '', '', '', '', '', 'Неудержимый', '', '', '', '', 16712190, 'Unstoppable', '', '', '', '', '', '', '', '', '', '', 'Неудержимая', '', '', '', '', 16712490, 149, 50, 0, 0, 50),
(185, 9145, 'Raid Conqueror', '', '', '', '', '', '', '', '', '', '', 'Покоритель Рейдов', '', '', '', '', 16712190, 'Raid Conqueror', '', '', '', '', '', '', '', '', '', '', 'Покорительница Рейдов', '', '', '', '', 16712490, 150, 80, 0, 0, 750),
(186, 9146, 'Grand Magus', '', '', '', '', '', '', '', '', '', '', 'Великий Маг', '', '', '', '', 16712190, 'Grand Magus', '', '', '', '', '', '', '', '', '', '', 'Великая Волшебница', '', '', '', '', 16712490, 151, 70, 0, 0, 300),
(187, 9147, 'Godslayer', '', '', '', '', '', '', '', '', '', '', 'Убийца Богов', '', '', '', '', 16712190, 'Godslayer', '', '', '', '', '', '', '', '', '', '', 'Убийца Богов', '', '', '', '', 16712490, 152, 80, 0, 0, 2000),
(188, 9148, 'Lorekeeper', '', '', '', '', '', '', '', '', '', '', 'Хранитель Знаний', '', '', '', '', 16712190, 'Lorekeeper', '', '', '', '', '', '', '', '', '', '', 'Хранительница Знаний', '', '', '', '', 16712490, 153, 60, 0, 0, 150),
(189, 9149, 'Dark Lord', '', '', '', '', '', '', '', '', '', '', 'Темный Лорд', '', '', '', '', 16712190, 'Dark Lady', '', '', '', '', '', '', '', '', '', '', 'Темная Леди', '', '', '', '', 16712490, 154, 75, 0, 0, 800);
