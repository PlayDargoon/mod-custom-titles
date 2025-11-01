-- ========================================================
-- NPC ПРОДАВЕЦ КАСТОМНЫХ ТИТУЛОВ В ДАЛАРАНЕ
-- ========================================================
-- Применить в базу acore_world_test
-- ========================================================

SET @NPC_ENTRY := 190000;  -- Entry для NPC (уникальный ID)
SET @NPC_GUID := 3000001;  -- GUID для spawn (уникальный)

-- Удаляем старые записи если есть
DELETE FROM `creature_template` WHERE `entry` = @NPC_ENTRY;
DELETE FROM `creature` WHERE `guid` = @NPC_GUID;

-- Создаём шаблон NPC (минимальная версия - только обязательные поля)
INSERT INTO `creature_template` 
(`entry`, `name`, `subname`, `minlevel`, `maxlevel`, `faction`, `npcflag`, `ScriptName`) 
VALUES 
(@NPC_ENTRY, 'Торговец Титулами', 'Кастомные Звания', 80, 80, 35, 1, 'npc_title_vendor');

-- Добавляем модель для NPC (Human Male Noble)
DELETE FROM `creature_template_model` WHERE `CreatureID` = @NPC_ENTRY;
INSERT INTO `creature_template_model` (`CreatureID`, `Idx`, `CreatureDisplayID`, `DisplayScale`, `Probability`) VALUES
(@NPC_ENTRY, 0, 28213, 1, 1);

-- Спавним NPC в Даларане (Альянс) - Крашус Приземление
-- Координаты: рядом с магазином эмблем
INSERT INTO `creature` 
(`guid`, `id1`, `map`, `zoneId`, `areaId`, `spawnMask`, `phaseMask`, 
 `equipment_id`, `position_x`, `position_y`, `position_z`, `orientation`, 
 `spawntimesecs`, `wander_distance`, `currentwaypoint`, `curhealth`, `curmana`, 
 `MovementType`, `npcflag`, `unit_flags`, `dynamicflags`, `VerifiedBuild`) 
VALUES 
(@NPC_GUID, @NPC_ENTRY, 571, 4395, 4560, 1, 1, 
 0, 5822.15, 589.19, 660.94, 1.88, 
 300, 0, 0, 12600, 0, 
 0, 1, 0, 0, 12340);

-- Проверка
SELECT 
    ct.entry, 
    ct.name, 
    ct.subname,
    c.position_x,
    c.position_y, 
    c.position_z,
    c.map
FROM creature_template ct
LEFT JOIN creature c ON c.id1 = ct.entry
WHERE ct.entry = @NPC_ENTRY;

-- ========================================================
-- ИНСТРУКЦИИ:
-- ========================================================
-- 1. NPC находится в Даларане (Крашус Приземление)
-- 2. Координаты: 5822, 589, 660 (рядом с магазином эмблем)
-- 3. Entry: 190000 (измените если конфликтует)
-- 4. GUID: 3000001 (измените если конфликтует)
-- 
-- Меню NPC:
-- - [Купить титул за Эмблемы Льда] - 49426 (Emblem of Frost)
-- - [Купить титул за золото] - обычная валюта
-- - [Мои титулы] - активация купленных титулов
-- 
-- Стоимость:
-- - В эмблемах = значение поля cost из таблицы custom_titles
-- - В золоте = значение поля cost из таблицы custom_titles
-- ========================================================
