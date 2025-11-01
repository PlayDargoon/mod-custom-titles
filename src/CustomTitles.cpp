/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>
 * Released under GNU AGPL v3 license
 */

#include "ScriptMgr.h"
#include "Player.h"
#include "Config.h"
#include "Chat.h"
#include "DatabaseEnv.h"
#include "ObjectAccessor.h"
#include "CharacterCache.h"
#include "CustomTitles.h"
#include <unordered_map>
#include <string>

using namespace Acore::ChatCommands;

// Глобальное хранилище кастомных званий
std::unordered_map<uint32, CustomTitle> customTitles;

// Загрузка кастомных званий из базы данных
void LoadCustomTitles()
{
    customTitles.clear();

    // Загружаем данные из таблицы custom_titles
    // name13 = русское название для мужчин
    // name_female13 = русское название для женщин
    QueryResult result = WorldDatabase.Query(
        "SELECT ID, condition_ID, name13, name_female13, mask_ID, "
        "required_level, required_achievement, required_item, cost "
        "FROM custom_titles"
    );

    if (!result)
    {
        LOG_INFO("module", ">> Loaded 0 custom titles. Table `custom_titles` is empty or doesn't exist.");
        return;
    }

    uint32 count = 0;
    do
    {
        Field* fields = result->Fetch();
        CustomTitle title;

        title.id = fields[0].Get<uint32>();
        title.conditionId = fields[1].Get<uint32>();
        title.nameMale = fields[2].Get<std::string>();
        title.nameFemale = fields[3].Get<std::string>();
        title.maskId = fields[4].Get<uint32>();
        title.requiredLevel = fields[5].Get<uint32>();
        title.requiredAchievement = fields[6].Get<uint32>();
        title.requiredItem = fields[7].Get<uint32>();
        title.cost = fields[8].Get<uint32>();

        customTitles[title.id] = title;
        count++;

    } while (result->NextRow());

    LOG_INFO("module", ">> Loaded {} custom titles from custom_titles table.", count);
}

// Функция-геттер для доступа к титулам из других файлов
std::unordered_map<uint32, CustomTitle>& GetCustomTitles()
{
    return customTitles;
}

// Класс для управления кастомными званиями
class CustomTitleManager : public WorldScript
{
public:
    CustomTitleManager() : WorldScript("CustomTitleManager") { }

    void OnStartup() override
    {
        if (!sConfigMgr->GetOption<bool>("CustomTitles.Enable", true))
        {
            LOG_INFO("module", ">> mod-custom-titles is disabled.");
            return;
        }

        LOG_INFO("module", ">> Loading custom titles...");
        LoadCustomTitles();
    }
};

// Класс для обработки команд игрока
class CustomTitleCommands : public CommandScript
{
public:
    CustomTitleCommands() : CommandScript("CustomTitleCommands") { }

    ChatCommandTable GetCommands() const override
    {
        static ChatCommandTable customTitleCommandTable =
        {
            { "list",    HandleCustomTitleListCommand,      SEC_PLAYER,         Console::No },
            { "set",     HandleCustomTitleSetCommand,       SEC_PLAYER,         Console::No },
            { "remove",  HandleCustomTitleRemoveCommand,    SEC_PLAYER,         Console::No },
            { "add",     HandleCustomTitleAddCommand,       SEC_GAMEMASTER,     Console::No },
            { "revoke",  HandleCustomTitleRevokeCommand,    SEC_GAMEMASTER,     Console::No },
            { "reload",  HandleCustomTitleReloadCommand,    SEC_ADMINISTRATOR,  Console::Yes }
        };

        static ChatCommandTable commandTable =
        {
            { "ctitle", customTitleCommandTable }
        };

        return commandTable;
    }

    // Команда для просмотра доступных кастомных званий
    static bool HandleCustomTitleListCommand(ChatHandler* handler, char const* /*args*/)
    {
        Player* player = handler->GetSession()->GetPlayer();

        if (customTitles.empty())
        {
            handler->PSendSysMessage("Нет доступных кастомных званий.");
            return true;
        }

        handler->PSendSysMessage("=== Доступные кастомные звания ===");

        for (auto const& [id, title] : customTitles)
        {
            std::string titleName = player->getGender() == GENDER_MALE ? title.nameMale : title.nameFemale;
            
            bool canUse = true;
            std::string requirements = "";

            if (title.requiredLevel > 0 && player->GetLevel() < title.requiredLevel)
            {
                canUse = false;
                requirements += " [Требуется уровень " + std::to_string(title.requiredLevel) + "]";
            }

            if (title.requiredAchievement > 0)
            {
                // Проверка достижения (можно расширить)
                requirements += " [Требуется достижение ID: " + std::to_string(title.requiredAchievement) + "]";
            }

            if (title.requiredItem > 0 && !player->HasItemCount(title.requiredItem, 1))
            {
                canUse = false;
                requirements += " [Требуется предмет ID: " + std::to_string(title.requiredItem) + "]";
            }

            if (title.cost > 0)
            {
                uint32 playerGold = player->GetMoney() / GOLD;
                if (playerGold < title.cost)
                {
                    canUse = false;
                }
                requirements += " [Стоимость: " + std::to_string(title.cost) + " золота]";
            }

            std::string status = canUse ? "|cff00ff00[Доступно]|r" : "|cffff0000[Недоступно]|r";
            handler->PSendSysMessage("ID: {} - {} {} {}", id, titleName, status, requirements);
        }

        handler->PSendSysMessage("Используйте: .ctitle set <id> для установки звания");
        return true;
    }

    // Команда для установки кастомного звания
    static bool HandleCustomTitleSetCommand(ChatHandler* handler, char const* args)
    {
        if (!*args)
        {
            handler->PSendSysMessage("Использование: .ctitle set <id>");
            return false;
        }

        uint32 titleId = atoi(args);
        Player* player = handler->GetSession()->GetPlayer();

        auto it = customTitles.find(titleId);
        if (it == customTitles.end())
        {
            handler->PSendSysMessage("Звание с ID {} не найдено.", titleId);
            return false;
        }

        CustomTitle const& title = it->second;

        // Проверка требований
        if (title.requiredLevel > 0 && player->GetLevel() < title.requiredLevel)
        {
            handler->PSendSysMessage("Требуется уровень {}.", title.requiredLevel);
            return false;
        }

        if (title.requiredItem > 0 && !player->HasItemCount(title.requiredItem, 1))
        {
            handler->PSendSysMessage("У вас нет необходимого предмета (ID: {}).", title.requiredItem);
            return false;
        }

        if (title.cost > 0)
        {
            uint32 playerGold = player->GetMoney() / GOLD;
            if (playerGold < title.cost)
            {
                handler->PSendSysMessage("Недостаточно золота. Требуется: {} золота.", title.cost);
                return false;
            }

            player->ModifyMoney(-int64(title.cost * GOLD));
        }

        // Устанавливаем битовую маску вручную в PLAYER__FIELD_KNOWN_TITLES
        // maskId - это битовый индекс (bit_index из CharTitles.dbc)
        if (title.maskId > 0)
        {
            // Устанавливаем бит в known titles
            uint64 mask = uint64(1) << title.maskId;
            uint64 oldMask = player->GetUInt64Value(PLAYER__FIELD_KNOWN_TITLES + (title.maskId / 64));
            player->SetUInt64Value(PLAYER__FIELD_KNOWN_TITLES + (title.maskId / 64), oldMask | mask);
        }

        // Снимаем активный статус со всех других титулов
        CharacterDatabase.Execute("UPDATE character_custom_titles SET is_active = 0 WHERE guid = {}", 
            player->GetGUID().GetCounter());

        // Сохранение кастомного звания в БД персонажа и установка активным
        CharacterDatabase.Execute(
            "INSERT INTO character_custom_titles (guid, title_id, mask_id, is_active) "
            "VALUES ({}, {}, {}, 1) "
            "ON DUPLICATE KEY UPDATE is_active = 1", 
            player->GetGUID().GetCounter(), titleId, title.maskId
        );

        std::string titleName = player->getGender() == GENDER_MALE ? title.nameMale : title.nameFemale;
        handler->PSendSysMessage("|cFFFFD700Кастомное звание установлено:|r |cFF00FF00{}|r", titleName);
        handler->PSendSysMessage("Звание отображается в чате и при входе в игру.");

        return true;
    }

    // Команда для снятия кастомного звания
    static bool HandleCustomTitleRemoveCommand(ChatHandler* handler, char const* /*args*/)
    {
        Player* player = handler->GetSession()->GetPlayer();

        CharacterDatabase.Execute("DELETE FROM character_custom_titles WHERE guid = {}", 
            player->GetGUID().GetCounter());

        handler->PSendSysMessage("Ваше кастомное звание снято.");
        return true;
    }

    // GM команда для выдачи звания другому игроку
    static bool HandleCustomTitleAddCommand(ChatHandler* handler, char const* args)
    {
        if (!*args)
        {
            handler->PSendSysMessage("Использование: .ctitle add <имя_игрока> <id_звания>");
            return false;
        }

        char* nameStr = strtok((char*)args, " ");
        char* idStr = strtok(nullptr, " ");

        if (!nameStr || !idStr)
        {
            handler->PSendSysMessage("Использование: .ctitle add <имя_игрока> <id_звания>");
            return false;
        }

        std::string playerName = nameStr;
        uint32 titleId = atoi(idStr);

        // Нормализуем имя игрока
        if (!normalizePlayerName(playerName))
        {
            handler->SendSysMessage(LANG_PLAYER_NOT_FOUND);
            return false;
        }

        // Получаем GUID игрока по имени
        ObjectGuid guid = sCharacterCache->GetCharacterGuidByName(playerName);
        if (!guid)
        {
            handler->SendSysMessage(LANG_PLAYER_NOT_FOUND);
            return false;
        }

        // Проверяем существование звания
        auto it = customTitles.find(titleId);
        if (it == customTitles.end())
        {
            handler->PSendSysMessage("Звание с ID {} не найдено.", titleId);
            return false;
        }

        CustomTitle const& title = it->second;

        // Выдаём звание игроку (без проверки требований и оплаты)
        CharacterDatabase.Execute("REPLACE INTO character_custom_titles (guid, title_id, mask_id) VALUES ({}, {}, {})", 
            guid.GetCounter(), titleId, title.maskId);

        // Если игрок онлайн - устанавливаем битовую маску немедленно
        if (Player* targetPlayer = ObjectAccessor::FindPlayerByName(playerName))
        {
            if (title.maskId > 0)
            {
                uint64 mask = uint64(1) << title.maskId;
                uint64 oldMask = targetPlayer->GetUInt64Value(PLAYER__FIELD_KNOWN_TITLES + (title.maskId / 64));
                targetPlayer->SetUInt64Value(PLAYER__FIELD_KNOWN_TITLES + (title.maskId / 64), oldMask | mask);
            }

            std::string titleName = targetPlayer->getGender() == GENDER_MALE ? title.nameMale : title.nameFemale;
            ChatHandler(targetPlayer->GetSession()).PSendSysMessage("Вы получили звание: {}", titleName);
        }

        handler->PSendSysMessage("Звание {} (ID: {}) выдано игроку {}.", title.nameMale, titleId, playerName);
        return true;
    }

    // GM команда для отзыва звания у игрока
    static bool HandleCustomTitleRevokeCommand(ChatHandler* handler, char const* args)
    {
        if (!*args)
        {
            handler->PSendSysMessage("Использование: .ctitle revoke <имя_игрока>");
            return false;
        }

        std::string playerName = args;

        // Нормализуем имя игрока
        if (!normalizePlayerName(playerName))
        {
            handler->SendSysMessage(LANG_PLAYER_NOT_FOUND);
            return false;
        }

        // Получаем GUID игрока по имени
        ObjectGuid guid = sCharacterCache->GetCharacterGuidByName(playerName);
        if (!guid)
        {
            handler->SendSysMessage(LANG_PLAYER_NOT_FOUND);
            return false;
        }

        // Удаляем звание из БД
        CharacterDatabase.Execute("DELETE FROM character_custom_titles WHERE guid = {}", guid.GetCounter());

        // Если игрок онлайн - уведомляем
        if (Player* targetPlayer = ObjectAccessor::FindPlayerByName(playerName))
        {
            ChatHandler(targetPlayer->GetSession()).PSendSysMessage("Ваше кастомное звание отозвано администратором.");
        }

        handler->PSendSysMessage("Кастомное звание отозвано у игрока {}.", playerName);
        return true;
    }

    // Команда для перезагрузки званий (только для администраторов)
    static bool HandleCustomTitleReloadCommand(ChatHandler* handler, char const* /*args*/)
    {
        LoadCustomTitles();
        handler->PSendSysMessage("Кастомные звания перезагружены. Всего загружено: {}", customTitles.size());
        return true;
    }
};

// Класс для отображения кастомного звания над персонажем
class CustomTitlePlayer : public PlayerScript
{
public:
    CustomTitlePlayer() : PlayerScript("CustomTitlePlayer") { }

    void OnPlayerLogin(Player* player) override
    {
        if (!sConfigMgr->GetOption<bool>("CustomTitles.Enable", true))
            return;

        // Загрузка кастомных званий игрока из БД
        QueryResult result = CharacterDatabase.Query(
            "SELECT title_id, mask_id FROM character_custom_titles WHERE guid = {}", 
            player->GetGUID().GetCounter()
        );

        if (result)
        {
            do
            {
                Field* fields = result->Fetch();
                uint32 titleId = fields[0].Get<uint32>();
                uint32 maskId = fields[1].Get<uint32>();

                auto it = customTitles.find(titleId);
                if (it != customTitles.end())
                {
                    CustomTitle const& title = it->second;
                    
                    // Устанавливаем битовую маску, чтобы звание было доступно в меню
                    if (maskId > 0)
                    {
                        // Устанавливаем бит в known titles при логине
                        uint64 mask = uint64(1) << maskId;
                        uint64 oldMask = player->GetUInt64Value(PLAYER__FIELD_KNOWN_TITLES + (maskId / 64));
                        player->SetUInt64Value(PLAYER__FIELD_KNOWN_TITLES + (maskId / 64), oldMask | mask);
                    }

                    // Опционально: уведомляем игрока
                    if (sConfigMgr->GetOption<bool>("CustomTitles.AnnounceOnLogin", true))
                    {
                        std::string titleName = player->getGender() == GENDER_MALE ? title.nameMale : title.nameFemale;
                        ChatHandler(player->GetSession()).PSendSysMessage("У вас есть кастомное звание: {}", titleName);
                    }
                }
            } while (result->NextRow());
        }
    }
};

// Регистрация всех скриптов модуля
void AddCustomTitlesScripts()
{
    new CustomTitleManager();
    new CustomTitleCommands();
    new CustomTitlePlayer();
}
