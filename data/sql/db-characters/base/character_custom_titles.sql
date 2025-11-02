-- --------------------------------------------------------
-- Custom Titles Module - Characters Database
-- --------------------------------------------------------

-- Таблица для хранения выбранных кастомных званий персонажей
DROP TABLE IF EXISTS `character_custom_titles`;
CREATE TABLE `character_custom_titles` (
  `guid` INT UNSIGNED NOT NULL COMMENT 'GUID персонажа',
  `title_id` INT UNSIGNED NOT NULL COMMENT 'ID кастомного звания из custom_titles',
  `mask_id` INT UNSIGNED NOT NULL COMMENT 'Битовый индекс звания',
  `is_active` TINYINT(1) NOT NULL DEFAULT 1 COMMENT 'Активно ли звание (1 = да, 0 = нет)',
  `purchased_date` TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP COMMENT 'Дата получения звания',
  PRIMARY KEY (`guid`, `title_id`),
  UNIQUE KEY `idx_active` (`guid`, `is_active`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COMMENT='Таблица выбранных кастомных званий персонажей';
