/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>
 * Released under GNU AGPL v3 license
 */

// This file is auto-loaded by the module system
// Add your module scripts here

void AddCustomTitlesScripts();
void AddSC_npc_title_vendor();

// Module loader entry point
void Addmod_custom_titlesScripts()
{
    AddCustomTitlesScripts();
    AddSC_npc_title_vendor();
}
