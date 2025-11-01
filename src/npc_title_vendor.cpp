/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>
 * Released under GNU AGPL v3 license
 */

#include "ScriptMgr.h"
#include "Player.h"
#include "ScriptedCreature.h"
#include "ScriptedGossip.h"
#include "Chat/Chat.h"
#include "CustomTitles.h"
#include "Config.h"
#include "DatabaseEnv.h"
#include <unordered_map>

class npc_title_vendor : public CreatureScript
{
public:
    npc_title_vendor() : CreatureScript("npc_title_vendor") { }

    bool OnGossipHello(Player* player, Creature* creature) override
    {
        if (!sConfigMgr->GetOption<bool>("CustomTitles.Enable", true))
        {
            AddGossipItemFor(player, GOSSIP_ICON_CHAT, "Система кастомных титулов отключена.", GOSSIP_SENDER_MAIN, 0);
            SendGossipMenuFor(player, DEFAULT_GOSSIP_MESSAGE, creature->GetGUID());
            return true;
        }

        AddGossipItemFor(player, GOSSIP_ICON_VENDOR, "|TInterface\\Icons\\Achievement_Boss_LichKing:24:24:-15:0|t [Купить титул за Эмблемы Льда]", GOSSIP_SENDER_MAIN, 1000);
        AddGossipItemFor(player, GOSSIP_ICON_VENDOR, "|TInterface\\Icons\\INV_Misc_Coin_01:24:24:-15:0|t [Купить титул за золото]", GOSSIP_SENDER_MAIN, 2000);
        AddGossipItemFor(player, GOSSIP_ICON_TALK, "|TInterface\\Icons\\INV_Misc_Book_09:24:24:-15:0|t [Мои титулы]", GOSSIP_SENDER_MAIN, 3000);

        SendGossipMenuFor(player, DEFAULT_GOSSIP_MESSAGE, creature->GetGUID());
        return true;
    }

    bool OnGossipSelect(Player* player, Creature* creature, uint32 sender, uint32 action) override
    {
        ClearGossipMenuFor(player);

        // Меню покупки за Эмблемы Льда
        if (action == 1000)
        {
            AddGossipItemFor(player, GOSSIP_ICON_CHAT, "<< Назад", GOSSIP_SENDER_MAIN, 0);
            
            for (auto const& [titleId, title] : GetCustomTitles())
            {
                // Рассчитываем стоимость в эмблемах (например, 1 золото = 1 эмблема)
                uint32 emblemCost = title.cost > 0 ? title.cost : 10; // Минимум 10 эмблем
                
                std::string titleName = player->getGender() == GENDER_MALE ? title.nameMale : title.nameFemale;
                std::string gossipText = "|TInterface\\Icons\\Achievement_Boss_LichKing:24:24:-15:0|t " + titleName;
                
                // Проверяем, есть ли титул у игрока
                QueryResult result = CharacterDatabase.Query(
                    "SELECT 1 FROM character_custom_titles WHERE guid = {} AND title_id = {}",
                    player->GetGUID().GetCounter(), titleId
                );

                if (result)
                {
                    gossipText += " |cFF00FF00[Уже куплено]|r";
                }
                else
                {
                    gossipText += " |cFFFFD700[" + std::to_string(emblemCost) + " Эмблем Льда]|r";
                }

                AddGossipItemFor(player, GOSSIP_ICON_VENDOR, gossipText, GOSSIP_SENDER_MAIN, 1000 + titleId);
            }

            SendGossipMenuFor(player, DEFAULT_GOSSIP_MESSAGE, creature->GetGUID());
            return true;
        }

        // Меню покупки за золото
        if (action == 2000)
        {
            AddGossipItemFor(player, GOSSIP_ICON_CHAT, "<< Назад", GOSSIP_SENDER_MAIN, 0);
            
            for (auto const& [titleId, title] : GetCustomTitles())
            {
                std::string titleName = player->getGender() == GENDER_MALE ? title.nameMale : title.nameFemale;
                std::string gossipText = "|TInterface\\Icons\\INV_Misc_Coin_01:24:24:-15:0|t " + titleName;
                
                QueryResult result = CharacterDatabase.Query(
                    "SELECT 1 FROM character_custom_titles WHERE guid = {} AND title_id = {}",
                    player->GetGUID().GetCounter(), titleId
                );

                if (result)
                {
                    gossipText += " |cFF00FF00[Уже куплено]|r";
                }
                else if (title.cost > 0)
                {
                    gossipText += " |cFFFFD700[" + std::to_string(title.cost) + " золота]|r";
                }
                else
                {
                    gossipText += " |cFF00FF00[Бесплатно]|r";
                }

                AddGossipItemFor(player, GOSSIP_ICON_VENDOR, gossipText, GOSSIP_SENDER_MAIN, 2000 + titleId);
            }

            SendGossipMenuFor(player, DEFAULT_GOSSIP_MESSAGE, creature->GetGUID());
            return true;
        }

        // Меню "Мои титулы"
        if (action == 3000)
        {
            AddGossipItemFor(player, GOSSIP_ICON_CHAT, "<< Назад", GOSSIP_SENDER_MAIN, 0);

            QueryResult result = CharacterDatabase.Query(
                "SELECT title_id, is_active FROM character_custom_titles WHERE guid = {}",
                player->GetGUID().GetCounter()
            );

            if (result)
            {
                do
                {
                    Field* fields = result->Fetch();
                    uint32 titleId = fields[0].Get<uint32>();
                    bool isActive = fields[1].Get<bool>();

                    auto it = GetCustomTitles().find(titleId);
                    if (it != GetCustomTitles().end())
                    {
                        std::string titleName = player->getGender() == GENDER_MALE ? it->second.nameMale : it->second.nameFemale;
                        std::string status = isActive ? " |cFF00FF00[Активен]|r" : "";
                        std::string gossipText = titleName + status;

                        // Если не активен - предлагаем активировать
                        if (!isActive)
                        {
                            AddGossipItemFor(player, GOSSIP_ICON_CHAT, gossipText + " |cFFFFD700[Активировать]|r", 
                                GOSSIP_SENDER_MAIN, 3000 + titleId);
                        }
                        else
                        {
                            AddGossipItemFor(player, GOSSIP_ICON_CHAT, gossipText, GOSSIP_SENDER_MAIN, 3000);
                        }
                    }
                } while (result->NextRow());
            }
            else
            {
                AddGossipItemFor(player, GOSSIP_ICON_CHAT, "У вас нет купленных титулов", GOSSIP_SENDER_MAIN, 0);
            }

            SendGossipMenuFor(player, DEFAULT_GOSSIP_MESSAGE, creature->GetGUID());
            return true;
        }

        // Покупка титула за Эмблемы Льда (action 1001+)
        if (action > 1000 && action < 2000)
        {
            uint32 titleId = action - 1000;
            PurchaseTitleWithEmblems(player, creature, titleId);
            OnGossipSelect(player, creature, sender, 1000); // Вернуться в меню эмблем
            return true;
        }

        // Покупка титула за золото (action 2001+)
        if (action > 2000 && action < 3000)
        {
            uint32 titleId = action - 2000;
            PurchaseTitleWithGold(player, creature, titleId);
            OnGossipSelect(player, creature, sender, 2000); // Вернуться в меню золота
            return true;
        }

        // Активация титула (action 3001+)
        if (action > 3000)
        {
            uint32 titleId = action - 3000;
            ActivateTitle(player, creature, titleId);
            OnGossipSelect(player, creature, sender, 3000); // Вернуться в меню титулов
            return true;
        }

        // Назад в главное меню
        if (action == 0)
        {
            OnGossipHello(player, creature);
            return true;
        }

        CloseGossipMenuFor(player);
        return true;
    }

private:
    void PurchaseTitleWithEmblems(Player* player, Creature* creature, uint32 titleId)
    {
        auto it = GetCustomTitles().find(titleId);
        if (it == GetCustomTitles().end())
        {
            ChatHandler(player->GetSession()).PSendSysMessage("Титул не найден.");
            return;
        }

        CustomTitle const& title = it->second;

        // Проверка, куплен ли уже титул
        QueryResult check = CharacterDatabase.Query(
            "SELECT 1 FROM character_custom_titles WHERE guid = {} AND title_id = {}",
            player->GetGUID().GetCounter(), titleId
        );

        if (check)
        {
            ChatHandler(player->GetSession()).PSendSysMessage("У вас уже есть этот титул!");
            return;
        }

        // Рассчитываем стоимость в эмблемах
        uint32 emblemCost = title.cost > 0 ? title.cost : 10;
        const uint32 EMBLEM_OF_FROST = 49426;

        // Проверяем наличие эмблем
        if (!player->HasItemCount(EMBLEM_OF_FROST, emblemCost))
        {
            ChatHandler(player->GetSession()).PSendSysMessage("Недостаточно Эмблем Льда! Требуется: {}", emblemCost);
            return;
        }

        // Забираем эмблемы
        player->DestroyItemCount(EMBLEM_OF_FROST, emblemCost, true);

        // Выдаём титул
        CharacterDatabase.Execute(
            "INSERT INTO character_custom_titles (guid, title_id, mask_id, is_active) VALUES ({}, {}, {}, 0)",
            player->GetGUID().GetCounter(), titleId, title.maskId
        );

        std::string titleName = player->getGender() == GENDER_MALE ? title.nameMale : title.nameFemale;
        ChatHandler(player->GetSession()).PSendSysMessage("|cFF00FF00Вы приобрели титул:|r |cFFFFD700{}|r", titleName);
        ChatHandler(player->GetSession()).PSendSysMessage("Используйте меню 'Мои титулы' для активации.");
    }

    void PurchaseTitleWithGold(Player* player, Creature* creature, uint32 titleId)
    {
        auto it = GetCustomTitles().find(titleId);
        if (it == GetCustomTitles().end())
        {
            ChatHandler(player->GetSession()).PSendSysMessage("Титул не найден.");
            return;
        }

        CustomTitle const& title = it->second;

        // Проверка, куплен ли уже титул
        QueryResult check = CharacterDatabase.Query(
            "SELECT 1 FROM character_custom_titles WHERE guid = {} AND title_id = {}",
            player->GetGUID().GetCounter(), titleId
        );

        if (check)
        {
            ChatHandler(player->GetSession()).PSendSysMessage("У вас уже есть этот титул!");
            return;
        }

        // Проверяем золото
        if (title.cost > 0)
        {
            uint32 playerGold = player->GetMoney() / GOLD;
            if (playerGold < title.cost)
            {
                ChatHandler(player->GetSession()).PSendSysMessage("Недостаточно золота! Требуется: {} золота", title.cost);
                return;
            }

            player->ModifyMoney(-int64(title.cost * GOLD));
        }

        // Выдаём титул
        CharacterDatabase.Execute(
            "INSERT INTO character_custom_titles (guid, title_id, mask_id, is_active) VALUES ({}, {}, {}, 0)",
            player->GetGUID().GetCounter(), titleId, title.maskId
        );

        std::string titleName = player->getGender() == GENDER_MALE ? title.nameMale : title.nameFemale;
        ChatHandler(player->GetSession()).PSendSysMessage("|cFF00FF00Вы приобрели титул:|r |cFFFFD700{}|r", titleName);
        ChatHandler(player->GetSession()).PSendSysMessage("Используйте меню 'Мои титулы' для активации.");
    }

    void ActivateTitle(Player* player, Creature* creature, uint32 titleId)
    {
        // Снимаем активный статус со всех титулов
        CharacterDatabase.Execute(
            "UPDATE character_custom_titles SET is_active = 0 WHERE guid = {}",
            player->GetGUID().GetCounter()
        );

        // Активируем выбранный титул
        CharacterDatabase.Execute(
            "UPDATE character_custom_titles SET is_active = 1 WHERE guid = {} AND title_id = {}",
            player->GetGUID().GetCounter(), titleId
        );

        auto it = GetCustomTitles().find(titleId);
        if (it != GetCustomTitles().end())
        {
            std::string titleName = player->getGender() == GENDER_MALE ? it->second.nameMale : it->second.nameFemale;
            ChatHandler(player->GetSession()).PSendSysMessage("|cFFFFD700Титул активирован:|r |cFF00FF00{}|r", titleName);
        }
    }
};

void AddSC_npc_title_vendor()
{
    new npc_title_vendor();
}
