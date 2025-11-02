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
#include "Mail.h"
#include <unordered_map>
#include <string>
#include <sstream>
#include <cstdlib>

using namespace Acore::ChatCommands;

// Глобальное хранилище кастомных званий
std::unordered_map<uint32, CustomTitle> customTitles;

// Вспомогательная функция: безопасно установить бит известного звания у игрока
static void SetKnownTitleBit(Player* player, uint32 maskId)
{
    if (!player || maskId == 0)
        return;

    // В 3.3.5 у игрока 3 64-битных слота для званий (0..191)
    uint32 index = maskId / 64;   // слот 0..2
    uint32 bitPos = maskId % 64;  // позиция 0..63
    if (index >= 3)
    {
        LOG_WARN("module", "CustomTitles: maskId {} is out of range (index {}). Skipping bit set.", maskId, index);
        return;
    }

    uint64 mask = uint64(1) << bitPos;
    uint64 oldMask = player->GetUInt64Value(PLAYER__FIELD_KNOWN_TITLES + index);
    player->SetUInt64Value(PLAYER__FIELD_KNOWN_TITLES + index, oldMask | mask);
}

// Загрузка кастомных званий из базы данных
void LoadCustomTitles()
{
    customTitles.clear();
    
    LOG_INFO("module", "CustomTitles: Starting to load from database...");

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
        LOG_ERROR("module", ">> CustomTitles: Failed to load! Table `custom_titles` is empty or doesn't exist!");
        LOG_ERROR("module", ">> Please apply SQL: modules/mod-custom-titles/data/sql/db-world/base/custom_titles.sql");
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
        
        // Логируем первые 3 звания для диагностики
        if (count <= 3)
        {
            LOG_INFO("module", "  - Loaded title ID {}: '{}' / '{}' (mask_ID: {})", 
                     title.id, title.nameMale, title.nameFemale, title.maskId);
        }

    } while (result->NextRow());

    LOG_INFO("module", ">> Successfully loaded {} custom titles from custom_titles table.", count);
}

// Функция-геттер для доступа к титулам из других файлов
std::unordered_map<uint32, CustomTitle>& GetCustomTitles()
{
    return customTitles;
}

// Функция отправки письма при получении звания
void SendTitleMail(Player* player, uint32 titleId, std::string const& titleName)
{
    // Загружаем шаблон письма из базы данных
    QueryResult result = WorldDatabase.Query("SELECT mail_subject, mail_body FROM custom_title_mails WHERE title_id = {}", titleId);
    
    std::string subject = "Поздравляем с получением звания!";
    std::string body = "";
    
    if (result)
    {
        Field* fields = result->Fetch();
        subject = fields[0].Get<std::string>();
        body = fields[1].Get<std::string>();
        
        // Заменяем плейсхолдеры на реальные значения
        size_t pos = 0;
        while ((pos = body.find("{PLAYER_NAME}", pos)) != std::string::npos)
        {
            body.replace(pos, 13, player->GetName());
            pos += player->GetName().length();
        }
        
        pos = 0;
        while ((pos = body.find("{TITLE_NAME}", pos)) != std::string::npos)
        {
            body.replace(pos, 12, titleName);
            pos += titleName.length();
        }
        
        // Заменяем \n на реальные переносы строк
        pos = 0;
        while ((pos = body.find("\\n", pos)) != std::string::npos)
        {
            body.replace(pos, 2, "\n");
            pos += 1;
        }
    }
    else
    {
        // Если шаблон не найден, используем стандартное сообщение
        body = "Поздравляем, " + player->GetName() + "!\n\n"
               "Тебе присвоено почетное звание " + titleName + ".\n\n"
               "Твои заслуги навсегда останутся в памяти этого мира!\n\n"
               "С уважением,\nКоманда Azeroth";
        
        LOG_WARN("module", "Mail template not found for title ID {}. Using default template.", titleId);
    }
    
    // Отправляем письмо
    MailSender sender(MAIL_NORMAL, 0, MAIL_STATIONERY_GM);
    MailDraft draft(subject, body);
    CharacterDatabaseTransaction trans = CharacterDatabase.BeginTransaction();
    draft.SendMailTo(trans, MailReceiver(player), sender, MAIL_CHECK_MASK_NONE, 0, 0, false, true);
    CharacterDatabase.CommitTransaction(trans);
    
    LOG_INFO("module", "Title mail sent to player {} for title ID {}", player->GetName(), titleId);
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

    // Команда для выдачи кастомного звания игроку (только GM)
    static bool HandleCustomTitleAddCommand(ChatHandler* handler, char const* args)
    {
        Player* target = nullptr;
        std::string playerName;
        uint32 titleId = 0;

        // Если нет аргументов - пытаемся взять таргет
        if (!args || !*args)
        {
            target = handler->getSelectedPlayerOrSelf();
            if (!target)
            {
                handler->PSendSysMessage("Использование: .ctitle add <имя_игрока> <id> или возьмите игрока в таргет и используйте .ctitle add <id>");
                return false;
            }
            handler->PSendSysMessage("Использование: .ctitle add <id> для выбранного игрока или .ctitle add <имя_игрока> <id>");
            return false;
        }

        // Безопасный парсинг аргументов без модификации исходной строки
        std::string input(args);
        std::istringstream iss(input);
        std::string firstToken;
        std::string secondToken;
        iss >> firstToken;
        if (!firstToken.empty())
            iss >> secondToken;

        // Если только один аргумент - это ID для таргета
        if (secondToken.empty())
        {
            // Формат: .ctitle add <id> (для выбранного таргета)
            titleId = static_cast<uint32>(strtoul(firstToken.c_str(), nullptr, 10));
            target = handler->getSelectedPlayerOrSelf();
            
            if (!target)
            {
                handler->PSendSysMessage("Возьмите игрока в таргет или укажите имя: .ctitle add <имя_игрока> <id>");
                return false;
            }
            
            playerName = target->GetName();
        }
        else
        {
            // Два аргумента - имя и ID
            playerName = firstToken;
            titleId = static_cast<uint32>(strtoul(secondToken.c_str(), nullptr, 10));
            target = ObjectAccessor::FindPlayerByName(playerName);
            
            if (!target)
            {
                handler->PSendSysMessage("Игрок '{}' не найден или не в сети.", playerName);
                return false;
            }
        }

        // Проверяем, что звания загружены
        if (customTitles.empty())
        {
            handler->PSendSysMessage("Ошибка: звания не загружены. Всего в кэше: 0. Используйте .ctitle reload");
            return false;
        }

        // Находим титул
        auto it = customTitles.find(titleId);
        if (it == customTitles.end())
        {
            handler->PSendSysMessage("Ошибка: звание ID {} не найдено. Всего загружено: {}. Используйте .ctitle reload", 
                                     titleId, customTitles.size());
            return false;
        }

        CustomTitle const& title = it->second;

        // Получаем название заранее, до любых операций с БД
        std::string titleName = target->getGender() == GENDER_MALE ? title.nameMale : title.nameFemale;
        
        // Если для текущего пола название пустое, используем альтернативное
        if (titleName.empty())
            titleName = !title.nameMale.empty() ? title.nameMale : title.nameFemale;
        
        // Если всё ещё пустое, используем ID как fallback
        if (titleName.empty())
            titleName = "Custom Title #" + std::to_string(titleId);

        // Устанавливаем битовую маску в PLAYER__FIELD_KNOWN_TITLES
        if (title.maskId > 0)
            SetKnownTitleBit(target, title.maskId);

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
        
        // Сообщение GM
        handler->PSendSysMessage("Звание '{}' (ID: {}) выдано игроку '{}'.", titleName, titleId, playerName);
        
        // Сообщение игроку (проверяем сессию)
        if (WorldSession* session = target->GetSession())
        {
            ChatHandler(session).PSendSysMessage("|cFFFFD700Поздравляем, {}!|r", target->GetName());
            ChatHandler(session).PSendSysMessage("|cFFFFD700Вы заслужили звание:|r |cFFFF8000{}|r", titleName);
        }
        
        // Анонс на весь сервер (если включено в конфиге)
        if (sConfigMgr->GetOption<bool>("CustomTitles.ServerAnnouncement", true))
        {
            std::string announcement = "|cFFFF0000[Серверное объявление]|r |cFFFFD700" + std::string(target->GetName()) + 
                                       " заслужил(а) звание:|r |cFFFF8000" + titleName + "|r";
            ChatHandler(nullptr).SendWorldText(announcement);
        }

        // Отправка поздравительного письма игроку (если включено в конфиге)
        if (sConfigMgr->GetOption<bool>("CustomTitles.SendMail", true))
        {
            SendTitleMail(target, titleId, titleName);
        }

        return true;
    }

    // Команда для снятия кастомного звания у игрока (только GM)
    static bool HandleCustomTitleRemoveCommand(ChatHandler* handler, char const* args)
    {
        Player* target = nullptr;
        std::string playerName;

        // Если нет аргументов - берём таргет
        if (!*args)
        {
            target = handler->getSelectedPlayerOrSelf();
            if (!target)
            {
                handler->PSendSysMessage("Использование: .ctitle remove <имя_игрока> или возьмите игрока в таргет");
                return false;
            }
            playerName = target->GetName();
        }
        else
        {
            // Аргумент указан - ищем по имени
            playerName = args;
            target = ObjectAccessor::FindPlayerByName(playerName);
            
            if (!target)
            {
                handler->PSendSysMessage("Игрок '{}' не найден или не в сети.", playerName);
                return false;
            }
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
                        // Устанавливаем бит в known titles при логине (безопасно)
                        SetKnownTitleBit(player, maskId);
                    }

                }
            } while (result->NextRow());
        }
        
        // Опционально: уведомляем игрока о модуле (один раз при входе)
        if (sConfigMgr->GetOption<bool>("CustomTitles.AnnounceModuleOnLogin", true))
        {
            ChatHandler(player->GetSession()).PSendSysMessage("|cFF00FF00[Custom Titles]|r Модуль кастомных званий активен!");
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
