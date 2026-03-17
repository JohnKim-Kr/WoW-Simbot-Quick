#include "pch.h"
#include "TrinketData.h"

namespace
{
    const std::vector<CTrinketOption> kMidnightTrinkets = {
        { 1, _T("Heart of Wind"), _T("heart_of_wind"), _T("dungeon"), _T("Windrunner Spire"), 250256, 0, _T(""), _T(""), true, false },
        { 2, _T("Jelly Replicator"), _T("jelly_replicator"), _T("dungeon"), _T("Magisters' Terrace"), 250242, 0, _T(""), _T(""), true, false },
        { 3, _T("Latch's Crooked Hook"), _T("latchs_crooked_hook"), _T("dungeon"), _T("Windrunner Spire"), 250226, 0, _T(""), _T(""), true, true },
        { 4, _T("Kroluk's Warbanner"), _T("kroluks_warbanner"), _T("dungeon"), _T("Windrunner Spire"), 250227, 0, _T(""), _T(""), true, true },
        { 5, _T("Algeth'ar Puzzle Box"), _T("algethar_puzzle_box"), _T("dungeon"), _T("Algeth'ar Academy"), 193701, 0, _T(""), _T(""), true, true },
        { 6, _T("Void Stalker's Contract"), _T("void_stalkers_contract"), _T("dungeon"), _T("Seat of the Triumvirate"), 151307, 0, _T(""), _T(""), true, false },
        { 7, _T("Solarflare Prism"), _T("solarflare_prism"), _T("dungeon"), _T("Skyreach"), 252420, 0, _T(""), _T(""), true, true },
        { 8, _T("Rotting Globule"), _T("rotting_globule"), _T("dungeon"), _T("Pit of Saron"), 252421, 0, _T(""), _T(""), true, false },
        { 9, _T("Ampoule of Pure Void"), _T("ampoule_of_pure_void"), _T("dungeon"), _T("Seat of the Triumvirate"), 151312, 0, _T(""), _T(""), true, false },
        { 10, _T("Solar Core Igniter"), _T("solar_core_igniter"), _T("dungeon"), _T("Skyreach"), 252418, 0, _T(""), _T(""), true, true },
        { 11, _T("Eye of the Drowning Void"), _T("eye_of_the_drowning_void"), _T("dungeon"), _T("Magisters' Terrace"), 250257, 0, _T(""), _T(""), true, false },
        { 12, _T("Emberwing Feather"), _T("emberwing_feather"), _T("dungeon"), _T("Windrunner Spire"), 250144, 0, _T(""), _T(""), true, false },
        { 13, _T("Mark of Light"), _T("mark_of_light"), _T("dungeon"), _T("Nexus-Point Xenas"), 250241, 0, _T(""), _T(""), true, false },
        { 14, _T("Dragon Games Equipment"), _T("dragon_games_equipment"), _T("dungeon"), _T("Algeth'ar Academy"), 193719, 0, _T(""), _T(""), true, true },
        { 15, _T("Soulcatcher's Charm"), _T("soulcatchers_charm"), _T("dungeon"), _T("Maisara Caverns"), 250223, 0, _T(""), _T(""), true, false },
        { 16, _T("Emerald Coach's Whistle"), _T("emerald_coachs_whistle"), _T("dungeon"), _T("Algeth'ar Academy"), 193718, 0, _T(""), _T(""), true, true },
        { 17, _T("Nevermelting Ice Crystal"), _T("nevermelting_ice_crystal"), _T("dungeon"), _T("Pit of Saron"), 50259, 0, _T(""), _T(""), true, true },
        { 18, _T("Reality Breacher"), _T("reality_breacher"), _T("dungeon"), _T("Seat of the Triumvirate"), 151310, 0, _T(""), _T(""), true, false },
        { 19, _T("Refueling Orb"), _T("refueling_orb"), _T("dungeon"), _T("Magisters' Terrace"), 250246, 0, _T(""), _T(""), true, true },
        { 20, _T("Whisper of the Duskwraith"), _T("whisper_of_the_duskwraith"), _T("dungeon"), _T("Nexus-Point Xenas"), 250253, 0, _T(""), _T(""), true, false },
        { 21, _T("Echo of L'ura"), _T("echo_of_lura"), _T("dungeon"), _T("Seat of the Triumvirate"), 151340, 0, _T(""), _T(""), true, true },
        { 22, _T("Radiant Sunstone"), _T("radiant_sunstone"), _T("dungeon"), _T("Skyreach"), 252411, 0, _T(""), _T(""), true, true },
        { 23, _T("Vessel of Tortured Souls"), _T("vessel_of_tortured_souls"), _T("dungeon"), _T("Maisara Caverns"), 250258, 0, _T(""), _T(""), true, false },
        { 101, _T("Light Company Guidon"), _T("light_company_guidon"), _T("raid"), _T("Voidspire - Imperator Averzian"), 249344, 0, _T(""), _T(""), true, true },
        { 102, _T("Heart of Ancient Hunger"), _T("heart_of_ancient_hunger"), _T("raid"), _T("Voidspire - Vorasius"), 249342, 0, _T(""), _T(""), true, false },
        { 103, _T("Volatile Void Suffuser"), _T("volatile_void_suffuser"), _T("raid"), _T("Voidspire - Fallen-King Salhadaar"), 249341, 0, _T(""), _T(""), true, false },
        { 104, _T("Wraps of Cosmic Madness"), _T("wraps_of_cosmic_madness"), _T("raid"), _T("Voidspire - Fallen-King Salhadaar"), 249340, 0, _T(""), _T(""), true, false },
        { 105, _T("Gloom-Spattered Dreadscale"), _T("gloom_spattered_dreadscale"), _T("raid"), _T("Voidspire - Vaelgor & Ezzorak"), 249339, 0, _T(""), _T(""), true, false },
        { 106, _T("Vaelgor's Final Stare"), _T("vaelgors_final_stare"), _T("raid"), _T("Voidspire - Vaelgor & Ezzorak"), 249346, 0, _T(""), _T(""), true, true },
        { 107, _T("Litany of Lightblind Wrath"), _T("litany_of_lightblind_wrath"), _T("raid"), _T("Voidspire - Lightblinded Vanguard"), 249808, 0, _T(""), _T(""), true, true },
        { 108, _T("Locus-Walker's Ribbon"), _T("locus_walkers_ribbon"), _T("raid"), _T("Voidspire - Crown of the Cosmos"), 249809, 0, _T(""), _T(""), true, false },
        { 109, _T("Ranger-Captain's Iridescent Insignia"), _T("ranger_captains_iridescent_insignia"), _T("raid"), _T("Voidspire - Crown of the Cosmos"), 249345, 0, _T(""), _T(""), true, true },
    };
}

const std::vector<CTrinketOption>& GetMidnightTrinketOptions()
{
    return kMidnightTrinkets;
}

const CTrinketOption* FindMidnightTrinketOption(int entryId)
{
    for (const auto& option : kMidnightTrinkets)
    {
        if (option.entryId == entryId)
        {
            return &option;
        }
    }

    return nullptr;
}
