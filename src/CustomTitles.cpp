/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>
 * Released under GNU AGPL v3 license
 */

#include "ScriptMgr.h"
#include "Player.h"
#include "Config.h"
#include "Chat.h"
#include "DatabaseEnv.h"
#include <unordered_map>
#include <string>

// Структура для хранения кастомного звания (DBC-подобная)
struct CustomTitle
{
    uint32 id;
    uint32 conditionId;
    std::string nameMale;      // name13 (ruRU для мужчин)
    std::string nameFemale;    // name_female13 (ruRU для женщин)
    uint32 maskId;             // битовый индекс в PLAYER__FIELD_KNOWN_TITLES
    uint32 requiredLevel;
    uint32 requiredAchievement;
    uint32 requiredItem;
    uint32 cost;
};

// Глобальное хранилище кастомных званий
std::unordered_map<uint32, CustomTitle> customTitles;

// Загрузка кастомных званий из базы данных
void LoadCustomTitles()
{
    customTitles.clear();

    // Загружаем данные из таблицы chartitles_dbc
    // name13 = русское название для мужчин
    // name_female13 = русское название для женщин
    QueryResult result = WorldDatabase.Query(
        "SELECT ID, condition_ID, name13, name_female13, mask_ID, "
        "required_level, required_achievement, required_item, cost "
        "FROM chartitles_dbc"
    );

    if (!result)
    {
        LOG_INFO("module", ">> Loaded 0 custom titles. Table `chartitles_dbc` is empty or doesn't exist.");
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

    LOG_INFO("module", ">> Loaded {} custom titles from chartitles_dbc.", count);
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

    std::vector<ChatCommand> GetCommands() override
    {
        static std::vector<ChatCommand> customTitleCommandTable =
        {
            { "list",    SEC_PLAYER, false, &HandleCustomTitleListCommand,    "" },
            { "set",     SEC_PLAYER, false, &HandleCustomTitleSetCommand,     "" },
            { "remove",  SEC_PLAYER, false, &HandleCustomTitleRemoveCommand,  "" },
            { "reload",  SEC_ADMINISTRATOR, true, &HandleCustomTitleReloadCommand, "" }
        };

        static std::vector<ChatCommand> commandTable =
        {
            { "ctitle", SEC_PLAYER, false, nullptr, "", customTitleCommandTable }
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

        // Устанавливаем звание через битовую маску (как в стандартных званиях)
        // Используем mask_ID для установки бита в PLAYER__FIELD_KNOWN_TITLES
        if (title.maskId > 0)
        {
            player->SetTitle(title.maskId);
        }

        // Сохранение кастомного звания в БД персонажа
        CharacterDatabase.Execute("REPLACE INTO character_custom_titles (guid, title_id, mask_id) VALUES ({}, {}, {})", 
            player->GetGUID().GetCounter(), titleId, title.maskId);

        std::string titleName = player->getGender() == GENDER_MALE ? title.nameMale : title.nameFemale;
        handler->PSendSysMessage("Вы получили звание: {}", titleName);
        handler->PSendSysMessage("Используйте стандартное меню выбора званий для его активации.");

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
                        player->SetTitle(maskId, false); // false = не удалять, а добавить
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
