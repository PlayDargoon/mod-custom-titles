-- --------------------------------------------------------
-- Custom Titles Module - World Database
-- --------------------------------------------------------

-- Таблица для хранения кастомных званий (DBC-подобная структура)
DROP TABLE IF EXISTS `custom_titles`;
CREATE TABLE `custom_titles` (
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

-- Кастомные звания для сервера
INSERT INTO `custom_titles` VALUES 
-- Базовое кастомное звание (ID 178)
(178, 9138, 'Azeroth Pioneer', '', '', '', '', '', '', '', '', '', '', 'Пионер Азерота', '', '', '', '', 16712190, 'Azeroth Pioneer', '', '', '', '', '', '', '', '', '', '', 'Пионерка Азерота', '', '', '', '', 16712490, 143, 0, 0, 0, 0),
-- Звания команды Azeroth и особые звания
(179, 9139, '%s from Azeroth Team', '', '', '', '', '', '', '', '', '', '', '%s из Команды Azeroth', '', '', '', '', 16712190, '%s from Azeroth Team', '', '', '', '', '', '', '', '', '', '', '%s из Команды Azeroth', '', '', '', '', 16712490, 144, 0, 0, 0, 0),
(180, 9140, 'Tester', '', '', '', '', '', '', '', '', '', '', 'Тестировщик', '', '', '', '', 16712190, 'Tester', '', '', '', '', '', '', '', '', '', '', 'Тестировщик', '', '', '', '', 16712490, 145, 0, 0, 0, 0),
(181, 9141, 'Honorary Hero', '', '', '', '', '', '', '', '', '', '', 'Почетный Герой', '', '', '', '', 16712190, 'Honorary Hero', '', '', '', '', '', '', '', '', '', '', 'Почетная Героиня', '', '', '', '', 16712490, 146, 0, 0, 0, 0),
(182, 9142, 'First on Server', '', '', '', '', '', '', '', '', '', '', 'Первый на Сервере', '', '', '', '', 16712190, 'First on Server', '', '', '', '', '', '', '', '', '', '', 'Первая на Сервере', '', '', '', '', 16712490, 147, 0, 0, 0, 0),
(183, 9143, 'Who Passed Hell', '', '', '', '', '', '', '', '', '', '', 'Прошедший Ад', '', '', '', '', 16712190, 'Who Passed Hell', '', '', '', '', '', '', '', '', '', '', 'Прошедшая Ад', '', '', '', '', 16712490, 148, 0, 0, 0, 0),
(184, 9144, 'Godslayer', '', '', '', '', '', '', '', '', '', '', 'Убийца Богов', '', '', '', '', 16712190, 'Godslayer', '', '', '', '', '', '', '', '', '', '', 'Убийца Богов', '', '', '', '', 16712490, 149, 0, 0, 0, 0),
(185, 9145, 'Creator', '', '', '', '', '', '', '', '', '', '', 'Создатель', '', '', '', '', 16712190, 'Creator', '', '', '', '', '', '', '', '', '', '', 'Создательница', '', '', '', '', 16712490, 150, 0, 0, 0, 0),

-- Звания за Challenge Mode
(186, 9146, 'Hardcore Survivor', '', '', '', '', '', '', '', '', '', '', 'Хардкорный Выживший', '', '', '', '', 16712190, 'Hardcore Survivor', '', '', '', '', '', '', '', '', '', '', 'Хардкорная Выжившая', '', '', '', '', 16712490, 151, 0, 0, 0, 0),
(187, 9147, 'Semi-Hardcore Warrior', '', '', '', '', '', '', '', '', '', '', 'Полу-Хардкорный Воин', '', '', '', '', 16712190, 'Semi-Hardcore Warrior', '', '', '', '', '', '', '', '', '', '', 'Полу-Хардкорная Воительница', '', '', '', '', 16712490, 152, 0, 0, 0, 0),
(188, 9148, 'Master Crafter', '', '', '', '', '', '', '', '', '', '', 'Мастер-Ремесленник', '', '', '', '', 16712190, 'Master Crafter', '', '', '', '', '', '', '', '', '', '', 'Мастер-Ремесленница', '', '', '', '', 16712490, 153, 0, 0, 0, 0),
(189, 9149, 'Ascetic', '', '', '', '', '', '', '', '', '', '', 'Аскет', '', '', '', '', 16712190, 'Ascetic', '', '', '', '', '', '', '', '', '', '', 'Аскетка', '', '', '', '', 16712490, 154, 0, 0, 0, 0),
(190, 9150, 'Slow Learner', '', '', '', '', '', '', '', '', '', '', 'Медленный Ученик', '', '', '', '', 16712190, 'Slow Learner', '', '', '', '', '', '', '', '', '', '', 'Медленная Ученица', '', '', '', '', 16712490, 155, 0, 0, 0, 0),
(191, 9151, 'Patient Master', '', '', '', '', '', '', '', '', '', '', 'Терпеливый Мастер', '', '', '', '', 16712190, 'Patient Master', '', '', '', '', '', '', '', '', '', '', 'Терпеливая Мастер', '', '', '', '', 16712490, 156, 0, 0, 0, 0),
(192, 9152, 'Quest Devotee', '', '', '', '', '', '', '', '', '', '', 'Адепт Квестов', '', '', '', '', 16712190, 'Quest Devotee', '', '', '', '', '', '', '', '', '', '', 'Адептка Квестов', '', '', '', '', 16712490, 157, 0, 0, 0, 0),
(193, 9153, 'Iron Man', '', '', '', '', '', '', '', '', '', '', 'Железный Человек', '', '', '', '', 16712190, 'Iron Woman', '', '', '', '', '', '', '', '', '', '', 'Железная Женщина', '', '', '', '', 16712490, 158, 0, 0, 0, 0);
