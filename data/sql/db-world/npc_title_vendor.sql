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

-- Создаём шаблон NPC
INSERT INTO `creature_template` 
(`entry`, `difficulty_entry_1`, `difficulty_entry_2`, `difficulty_entry_3`, 
 `KillCredit1`, `KillCredit2`, `modelid1`, `modelid2`, `modelid3`, `modelid4`, 
 `name`, `subname`, `IconName`, `gossip_menu_id`, `minlevel`, `maxlevel`, 
 `exp`, `faction`, `npcflag`, `speed_walk`, `speed_run`, `scale`, 
 `rank`, `dmgschool`, `BaseAttackTime`, `RangeAttackTime`, `BaseVariance`, `RangeVariance`, 
 `unit_class`, `unit_flags`, `unit_flags2`, `dynamicflags`, `family`, `trainer_type`, 
 `trainer_spell`, `trainer_class`, `trainer_race`, `type`, `type_flags`, `lootid`, 
 `pickpocketloot`, `skinloot`, `resistance1`, `resistance2`, `resistance3`, `resistance4`, 
 `resistance5`, `resistance6`, `spell1`, `spell2`, `spell3`, `spell4`, `spell5`, `spell6`, 
 `spell7`, `spell8`, `PetSpellDataId`, `VehicleId`, `mingold`, `maxgold`, `AIName`, 
 `MovementType`, `InhabitType`, `HoverHeight`, `HealthModifier`, `ManaModifier`, 
 `ArmorModifier`, `DamageModifier`, `ExperienceModifier`, `RacialLeader`, `movementId`, 
 `RegenHealth`, `mechanic_immune_mask`, `flags_extra`, `ScriptName`, `VerifiedBuild`) 
VALUES 
(@NPC_ENTRY, 0, 0, 0, 
 0, 0, 28213, 0, 0, 0,  -- modelid1 = 28213 (Human Male Noble)
 'Торговец Титулами', 'Кастомные Звания', NULL, 0, 80, 80, 
 2, 35, 1, 1, 1.14286, 1, 
 0, 0, 2000, 2000, 1, 1, 
 1, 0, 2048, 0, 0, 0, 
 0, 0, 0, 7, 0, 0, 
 0, 0, 0, 0, 0, 0, 
 0, 0, 0, 0, 0, 0, 0, 0, 
 0, 0, 0, 0, 0, 0, '', 
 0, 3, 1, 1, 1, 
 1, 1, 1, 0, 0, 
 1, 0, 0, 'npc_title_vendor', 12340);

-- Спавним NPC в Даларане (Альянс) - Крашус Приземление
-- Координаты: рядом с магазином эмблем
INSERT INTO `creature` 
(`guid`, `id`, `map`, `zoneId`, `areaId`, `spawnMask`, `phaseMask`, 
 `modelid`, `equipment_id`, `position_x`, `position_y`, `position_z`, `orientation`, 
 `spawntimesecs`, `wander_distance`, `currentwaypoint`, `curhealth`, `curmana`, 
 `MovementType`, `npcflag`, `unit_flags`, `dynamicflags`, `ScriptName`, `VerifiedBuild`) 
VALUES 
(@NPC_GUID, @NPC_ENTRY, 571, 4395, 4560, 1, 1, 
 0, 0, 5822.15, 589.19, 660.94, 1.88, 
 300, 0, 0, 12600, 0, 
 0, 1, 0, 0, '', 12340);

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
JOIN creature c ON c.id = ct.entry
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
