/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>
 * Released under GNU AGPL v3 license
 */

#ifndef CUSTOM_TITLES_H
#define CUSTOM_TITLES_H

#include <string>

// Структура для хранения кастомного звания
struct CustomTitle
{
    uint32 id;
    uint32 conditionId;
    std::string nameMale;      // Название для мужчин
    std::string nameFemale;    // Название для женщин
    uint32 maskId;             // Битовый индекс в PLAYER__FIELD_KNOWN_TITLES
    uint32 requiredLevel;
    uint32 requiredAchievement;
    uint32 requiredItem;
    uint32 cost;
};

#endif // CUSTOM_TITLES_H
