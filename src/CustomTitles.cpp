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
            { "list",    HandleCustomTitleListCommand,      SEC_GAMEMASTER,     Console::Yes },
            { "add",     HandleCustomTitleAddCommand,       SEC_GAMEMASTER,     Console::No },
            { "remove",  HandleCustomTitleRemoveCommand,    SEC_GAMEMASTER,     Console::No },
            { "reload",  HandleCustomTitleReloadCommand,    SEC_ADMINISTRATOR,  Console::Yes }
        };

        static ChatCommandTable commandTable =
        {
            { "ctitle", customTitleCommandTable }
        };

        return commandTable;
    }

    // Команда для просмотра всех кастомных званий (только GM)
    static bool HandleCustomTitleListCommand(ChatHandler* handler, char const* /*args*/)
    {
        if (customTitles.empty())
        {
            handler->PSendSysMessage("Нет доступных кастомных званий.");
            return true;
        }

        handler->PSendSysMessage("=== Список всех кастомных званий ===");

        for (auto const& [id, title] : customTitles)
        {
            handler->PSendSysMessage("ID: {} | Муж: {} | Жен: {} | Mask: {}", 
                id, title.nameMale, title.nameFemale, title.maskId);
        }

        handler->PSendSysMessage("Используйте: .ctitle add <имя_игрока> <id> для выдачи звания");
        handler->PSendSysMessage("Используйте: .ctitle remove <имя_игрока> для снятия звания");
        return true;
    }

    // Команда для выдачи кастомного звания игроку (только GM)
    static bool HandleCustomTitleAddCommand(ChatHandler* handler, char const* args)
    {
        if (!*args)
        {
            handler->PSendSysMessage("Использование: .ctitle add <имя_игрока> <id>");
            return false;
        }

        char* nameStr = strtok((char*)args, " ");
        char* idStr = strtok(nullptr, " ");

        if (!nameStr || !idStr)
        {
            handler->PSendSysMessage("Использование: .ctitle add <имя_игрока> <id>");
            return false;
        }

        std::string playerName = nameStr;
        uint32 titleId = atoi(idStr);

        // Находим титул
        auto it = customTitles.find(titleId);
        if (it == customTitles.end())
        {
            handler->PSendSysMessage("Звание с ID {} не найдено.", titleId);
            return false;
        }

        CustomTitle const& title = it->second;

        // Ищем игрока
        Player* target = ObjectAccessor::FindPlayerByName(playerName);
        if (!target)
        {
            handler->PSendSysMessage("Игрок '{}' не найден или не в сети.", playerName);
            return false;
        }

        // Устанавливаем битовую маску в PLAYER__FIELD_KNOWN_TITLES
        if (title.maskId > 0)
        {
            uint64 mask = uint64(1) << title.maskId;
            uint64 oldMask = target->GetUInt64Value(PLAYER__FIELD_KNOWN_TITLES + (title.maskId / 64));
            target->SetUInt64Value(PLAYER__FIELD_KNOWN_TITLES + (title.maskId / 64), oldMask | mask);
        }

        // Снимаем активный статус со всех других титулов
        CharacterDatabase.Execute("UPDATE character_custom_titles SET is_active = 0 WHERE guid = {}", 
            target->GetGUID().GetCounter());

        // Сохранение кастомного звания в БД персонажа и установка активным
        CharacterDatabase.Execute(
            "INSERT INTO character_custom_titles (guid, title_id, mask_id, is_active) "
            "VALUES ({}, {}, {}, 1) "
            "ON DUPLICATE KEY UPDATE is_active = 1", 
            target->GetGUID().GetCounter(), titleId, title.maskId
        );

        std::string titleName = target->getGender() == GENDER_MALE ? title.nameMale : title.nameFemale;
        
        handler->PSendSysMessage("Звание '{}' (ID: {}) выдано игроку '{}'.", titleName, titleId, playerName);
        ChatHandler(target->GetSession()).PSendSysMessage("|cFFFFD700GM выдал вам кастомное звание:|r |cFF00FF00{}|r", titleName);

        return true;
    }

    // Команда для снятия кастомного звания у игрока (только GM)
    static bool HandleCustomTitleRemoveCommand(ChatHandler* handler, char const* args)
    {
        if (!*args)
        {
            handler->PSendSysMessage("Использование: .ctitle remove <имя_игрока>");
            return false;
        }

        std::string playerName = args;

        // Ищем игрока
        Player* target = ObjectAccessor::FindPlayerByName(playerName);
        if (!target)
        {
            handler->PSendSysMessage("Игрок '{}' не найден или не в сети.", playerName);
            return false;
        }

        // Удаляем все кастомные звания игрока
        CharacterDatabase.Execute("DELETE FROM character_custom_titles WHERE guid = {}", 
            target->GetGUID().GetCounter());

        handler->PSendSysMessage("Кастомное звание игрока '{}' снято.", playerName);
        ChatHandler(target->GetSession()).PSendSysMessage("|cFFFF0000GM снял ваше кастомное звание.|r");

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
