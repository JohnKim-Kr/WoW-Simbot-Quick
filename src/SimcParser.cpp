#include "pch.h"
#include "framework.h"
#include "SimcParser.h"
#include "CharacterData.h"
#include <sstream>
#include <algorithm>

CSimcParser::CSimcParser()
{
}

CSimcParser::~CSimcParser()
{
}

BOOL CSimcParser::Parse(const std::string& profileStr, CCharacterData* pOutData)
{
    if (!pOutData || profileStr.empty())
        return FALSE;

    std::istringstream stream(profileStr);
    std::string line;

    while (std::getline(stream, line))
    {
        // Skip empty lines and comments
        line = Trim(line);
        if (line.empty() || line[0] == '#')
            continue;

        // Parse based on line type
        if (line.find("char") == 0 || line.find("character") == 0 || line.find("name=") == 0)
        {
            ParseCharacterLine(line, pOutData);
        }
        else if (line.find("class=") == 0 || line.find("spec=") == 0 ||
                 line.find("level=") == 0 || line.find("race=") == 0 ||
                 line.find("server=") == 0 || line.find("realm=") == 0)
        {
            ParseCharacterLine(line, pOutData);
        }
        else if (IsClassLine(line))
        {
            // Parse class definition line like: mage="CharacterName"
            ParseClassDefinitionLine(line, pOutData);
        }
        else if (line.find("head=") == 0 || line.find("neck=") == 0 ||
                 line.find("shoulder=") == 0 || line.find("chest=") == 0 ||
                 line.find("waist=") == 0 || line.find("legs=") == 0 ||
                 line.find("feet=") == 0 || line.find("wrist=") == 0 ||
                 line.find("hands=") == 0 || line.find("finger") == 0 ||
                 line.find("trinket") == 0 || line.find("back=") == 0 ||
                 line.find("main_hand=") == 0 || line.find("off_hand=") == 0)
        {
            ParseGearLine(line, pOutData);
        }
        else if (line.find("talents=") == 0)
        {
            ParseTalentLine(line, pOutData);
        }
        else if (line.find("region=") == 0)
        {
            ParseRegionLine(line, pOutData);
        }
        else if (line.find("role=") == 0)
        {
            // Role is parsed but not stored currently
            continue;
        }
        else if (line.find("soulbind=") == 0 || line.find("conduit=") == 0)
        {
            ParseSoulbindLine(line, pOutData);
        }
        else if (line.find("iterations=") == 0 || line.find("desired_targets=") == 0 ||
                 line.find("max_time=") == 0 || line.find("calculate_scale_factors=") == 0 ||
                 line.find("scale_only=") == 0 || line.find("report_details=") == 0 ||
                 line.find("single_actor_batch=") == 0 || line.find("optimize_expressions=") == 0 ||
                 line.find("target_error=") == 0)
        {
            ParseSimulationOption(line, pOutData);
        }
        else if (line.find("temporary_enchant=") == 0 || line.find("midnight.") == 0)
        {
            ParseExpansionOption(line, pOutData);
        }
        else if (line.find("override.") == 0)
        {
            ParseBuffOverride(line, pOutData);
        }
    }

    // Set default region if not parsed
    if (pOutData->GetRegion().empty())
    {
        pOutData->SetRegion("us");
    }

    // Calculate average item level from parsed items
    pOutData->CalculateItemLevelFromItems();

    return pOutData->IsValid();
}

BOOL CSimcParser::ParseCharacterLine(const std::string& line, CCharacterData* pOutData)
{
    // Parse character name: char="CharacterName" or name="CharacterName"
    size_t pos = line.find("char");
    if (pos == std::string::npos) pos = line.find("name");

    if (pos != std::string::npos)
    {
        size_t eqPos = line.find('=', pos);
        if (eqPos != std::string::npos)
        {
            std::string name = line.substr(eqPos + 1);
            // Handle both quoted and unquoted names
            if (name.length() >= 2 && name.front() == '"' && name.back() == '"')
                name = name.substr(1, name.length() - 2);
            
            pOutData->SetName(name);
            return TRUE;
        }
    }

    // Parse class: class=mage
    pos = line.find("class=");
    if (pos == 0)
    {
        size_t eqPos = line.find('=', pos);
        if (eqPos != std::string::npos)
        {
            pOutData->SetClassName(line.substr(eqPos + 1));
            return TRUE;
        }
    }

    // Parse spec: spec=frost
    pos = line.find("spec=");
    if (pos == 0)
    {
        size_t eqPos = line.find('=', pos);
        if (eqPos != std::string::npos)
        {
            pOutData->SetActiveSpecName(line.substr(eqPos + 1));
            return TRUE;
        }
    }

    // Parse level: level=80
    pos = line.find("level=");
    if (pos == 0)
    {
        size_t eqPos = line.find('=', pos);
        if (eqPos != std::string::npos)
        {
            try {
                pOutData->SetLevel(std::stoi(line.substr(eqPos + 1)));
            } catch (...) {}
            return TRUE;
        }
    }

    // Parse race: race=human
    pos = line.find("race=");
    if (pos == 0)
    {
        size_t eqPos = line.find('=', pos);
        if (eqPos != std::string::npos)
        {
            pOutData->SetRace(line.substr(eqPos + 1));
            return TRUE;
        }
    }

    // Parse server/realm: server=realmname or realm=realmname
    pos = line.find("server=");
    if (pos == std::string::npos) pos = line.find("realm=");

    if (pos != std::string::npos)
    {
        size_t eqPos = line.find('=', pos);
        if (eqPos != std::string::npos)
        {
            std::string realm = line.substr(eqPos + 1);
            // Remove quotes if present
            if (realm.length() >= 2 && realm.front() == '"' && realm.back() == '"')
                realm = realm.substr(1, realm.length() - 2);
            pOutData->SetRealm(realm);
            return TRUE;
        }
    }

    return FALSE;
}

BOOL CSimcParser::ParseGearLine(const std::string& line, CCharacterData* pOutData)
{
    CCharacterData::ItemSlot slot = CCharacterData::ItemSlot::NONE;

    if (line.find("head=") == 0) slot = CCharacterData::ItemSlot::HEAD;
    else if (line.find("neck=") == 0) slot = CCharacterData::ItemSlot::NECK;
    else if (line.find("shoulder=") == 0) slot = CCharacterData::ItemSlot::SHOULDER;
    else if (line.find("back=") == 0) slot = CCharacterData::ItemSlot::BACK;
    else if (line.find("chest=") == 0) slot = CCharacterData::ItemSlot::CHEST;
    else if (line.find("wrist=") == 0) slot = CCharacterData::ItemSlot::WRIST;
    else if (line.find("hands=") == 0) slot = CCharacterData::ItemSlot::HANDS;
    else if (line.find("waist=") == 0) slot = CCharacterData::ItemSlot::WAIST;
    else if (line.find("legs=") == 0) slot = CCharacterData::ItemSlot::LEGS;
    else if (line.find("feet=") == 0) slot = CCharacterData::ItemSlot::FEET;
    else if (line.find("finger1=") == 0) slot = CCharacterData::ItemSlot::FINGER1;
    else if (line.find("finger2=") == 0) slot = CCharacterData::ItemSlot::FINGER2;
    else if (line.find("trinket1=") == 0) slot = CCharacterData::ItemSlot::TRINKET1;
    else if (line.find("trinket2=") == 0) slot = CCharacterData::ItemSlot::TRINKET2;
    else if (line.find("main_hand=") == 0) slot = CCharacterData::ItemSlot::MAIN_HAND;
    else if (line.find("off_hand=") == 0) slot = CCharacterData::ItemSlot::OFF_HAND;

    if (slot == CCharacterData::ItemSlot::NONE)
        return FALSE;

    // Parse item ID: id=12345
    size_t idPos = line.find("id=");
    if (idPos != std::string::npos)
    {
        CCharacterData::ItemData item;
        item.slot = slot;

        size_t eqPos = line.find('=', idPos);
        if (eqPos != std::string::npos)
        {
            size_t endPos = line.find(',', eqPos);
            if (endPos == std::string::npos)
                endPos = line.length();
            try {
                item.id = std::stoul(line.substr(eqPos + 1, endPos - eqPos - 1));
            } catch (...) {}
        }

        // Parse bonus IDs
        size_t bonusPos = line.find("bonus_id=");
        if (bonusPos != std::string::npos)
        {
            size_t eqPos = line.find('=', bonusPos);
            if (eqPos != std::string::npos)
            {
                size_t endPos = line.find(',', eqPos);
                if (endPos == std::string::npos)
                    endPos = line.length();
                std::string bonusStr = line.substr(eqPos + 1, endPos - eqPos - 1);
                std::vector<std::string> bonuses = Split(bonusStr, '/');
                for (const auto& b : bonuses) {
                    try { item.bonusIds.push_back(std::stoul(b)); } catch (...) {}
                }
            }
        }

        // Parse enchant
        size_t enchantPos = line.find("enchant_id=");
        if (enchantPos != std::string::npos)
        {
            size_t eqPos = line.find('=', enchantPos);
            if (eqPos != std::string::npos)
            {
                size_t endPos = line.find(',', eqPos);
                if (endPos == std::string::npos)
                    endPos = line.length();
                try { item.enchantId = std::stoul(line.substr(eqPos + 1, endPos - eqPos - 1)); } catch (...) {}
            }
        }

        // Parse gems
        size_t gemPos = line.find("gem_id=");
        if (gemPos != std::string::npos)
        {
            size_t eqPos = line.find('=', gemPos);
            if (eqPos != std::string::npos)
            {
                size_t endPos = line.find(',', eqPos);
                if (endPos == std::string::npos)
                    endPos = line.length();
                std::string gemStr = line.substr(eqPos + 1, endPos - eqPos - 1);
                std::vector<std::string> gems = Split(gemStr, '/');
                for (const auto& g : gems) {
                    try { item.gemIds.push_back(std::stoul(g)); } catch (...) {}
                }
            }
        }

        // Parse item level: ilevel=636
        size_t ilvlPos = line.find("ilevel=");
        if (ilvlPos != std::string::npos)
        {
            size_t eqPos = line.find('=', ilvlPos);
            if (eqPos != std::string::npos)
            {
                size_t endPos = line.find(',', eqPos);
                if (endPos == std::string::npos)
                    endPos = line.length();
                try { item.itemLevel = std::stod(line.substr(eqPos + 1, endPos - eqPos - 1)); } catch (...) {}
            }
        }

        pOutData->AddItem(item);
        return TRUE;
    }

    return FALSE;
}

BOOL CSimcParser::ParseTalentLine(const std::string& line, CCharacterData* pOutData)
{
    size_t pos = line.find("talents=");
    if (pos == 0)
    {
        size_t eqPos = line.find('=', pos);
        if (eqPos != std::string::npos)
        {
            std::string talentStr = Trim(line.substr(eqPos + 1));

            // Check if it's Dragonflight format (contains letters)
            bool isDragonflightFormat = std::any_of(talentStr.begin(), talentStr.end(), ::isalpha);

            if (isDragonflightFormat)
            {
                pOutData->SetTalentString(talentStr);
            }
            else
            {
                // Old numeric format
                for (size_t i = 0; i < talentStr.length() && i < 7; ++i)
                {
                    int choice = talentStr[i] - '0';
                    if (choice >= 1 && choice <= 3)
                    {
                        CCharacterData::TalentData talent;
                        talent.tier = (int)i;
                        talent.choice = choice - 1;
                        pOutData->AddTalent(talent);
                    }
                }
            }
            return TRUE;
        }
    }
    return FALSE;
}

BOOL CSimcParser::ParseSimulationOption(const std::string& line, CCharacterData* pOutData)
{
    size_t eqPos = line.find('=');
    if (eqPos == std::string::npos) return FALSE;

    std::string key = Trim(line.substr(0, eqPos));
    std::string value = Trim(line.substr(eqPos + 1));
    SimulationSettings& settings = pOutData->GetSettings();

    if (key == "iterations") {
        try { settings.iterations = std::stoi(value); } catch (...) {}
    } else if (key == "desired_targets") {
        try { settings.targetCount = std::stoi(value); } catch (...) {}
    } else if (key == "max_time") {
        try { settings.duration = std::stoi(value); } catch (...) {}
    } else if (key == "calculate_scale_factors") {
        settings.calculateScaleFactors = (value == "1");
    } else if (key == "scale_only") {
        settings.scaleOnly = CA2T(value.c_str());
    } else if (key == "report_details") {
        settings.reportDetails = (value == "1");
    } else if (key == "single_actor_batch") {
        settings.singleActorBatch = (value == "1");
    } else if (key == "optimize_expressions") {
        settings.optimizeExpressions = (value == "1");
    } else if (key == "target_error") {
        try { settings.targetError = std::stod(value); } catch (...) {}
    }

    return TRUE;
}

BOOL CSimcParser::ParseExpansionOption(const std::string& line, CCharacterData* pOutData)
{
    size_t eqPos = line.find('=');
    if (eqPos == std::string::npos) return FALSE;

    std::string key = Trim(line.substr(0, eqPos));
    std::string value = Trim(line.substr(eqPos + 1));
    SimulationSettings& settings = pOutData->GetSettings();

    if (key == "temporary_enchant") {
        settings.temporaryEnchant = CA2T(value.c_str());
    } else if (key == "midnight.crucible_of_erratic_energies_violence") {
        settings.useCrucibleViolence = (value == "1");
    } else if (key == "midnight.crucible_of_erratic_energies_sustenance") {
        settings.useCrucibleSustenance = (value == "1");
    } else if (key == "midnight.crucible_of_erratic_energies_predation") {
        settings.useCruciblePredation = (value == "1");
    }

    return TRUE;
}

BOOL CSimcParser::ParseBuffOverride(const std::string& line, CCharacterData* pOutData)
{
    size_t eqPos = line.find('=');
    if (eqPos == std::string::npos) return FALSE;

    std::string key = Trim(line.substr(0, eqPos));
    std::string value = Trim(line.substr(eqPos + 1));
    SimulationSettings& settings = pOutData->GetSettings();
    BOOL bVal = (value == "1");

    if (key == "override.bloodlust") settings.useBloodlust = bVal;
    else if (key == "override.arcane_intellect") settings.useArcaneIntellect = bVal;
    else if (key == "override.power_word_fortitude") settings.usePowerWordFortitude = bVal;
    else if (key == "override.battle_shout") settings.useBattleShout = bVal;
    else if (key == "override.mystic_touch") settings.useMysticTouch = bVal;
    else if (key == "override.chaos_brand") settings.useChaosBrand = bVal;
    else if (key == "override.skyfury") settings.useSkyfury = bVal;
    else if (key == "override.mark_of_the_wild") settings.useMarkOfTheWild = bVal;
    else if (key == "override.hunters_mark") settings.useHuntersMark = bVal;
    else if (key == "override.bleeding") settings.useBleeding = bVal;

    return TRUE;
}

BOOL CSimcParser::ParseSoulbindLine(const std::string& line, CCharacterData* pOutData)
{
    if (line.find("soulbind=") == 0)
    {
        std::string soulbindName = Trim(line.substr(9));
        size_t commaPos = soulbindName.find(',');
        if (commaPos != std::string::npos) soulbindName = Trim(soulbindName.substr(0, commaPos));

        if (soulbindName.length() >= 2 && soulbindName.front() == '"' && soulbindName.back() == '"')
            soulbindName = soulbindName.substr(1, soulbindName.length() - 2);

        pOutData->SetSoulbind(soulbindName);
        return TRUE;
    }
    return FALSE;
}

BOOL CSimcParser::ParseRegionLine(const std::string& line, CCharacterData* pOutData)
{
    size_t pos = line.find("region=");
    if (pos == 0)
    {
        pOutData->SetRegion(Trim(line.substr(7)));
        return TRUE;
    }
    return FALSE;
}

std::string CSimcParser::Trim(const std::string& str)
{
    size_t first = str.find_first_not_of(" \t\r\n");
    if (first == std::string::npos) return "";
    size_t last = str.find_last_not_of(" \t\r\n");
    return str.substr(first, last - first + 1);
}

std::vector<std::string> CSimcParser::Split(const std::string& str, char delimiter)
{
    std::vector<std::string> tokens;
    std::stringstream ss(str);
    std::string token;
    while (std::getline(ss, token, delimiter)) tokens.push_back(Trim(token));
    return tokens;
}

int CSimcParser::ParseItemLevel(const std::string& bonusStr)
{
    return 0;
}

BOOL CSimcParser::IsClassLine(const std::string& line)
{
    static const char* classNames[] = {
        "warrior", "paladin", "hunter", "rogue", "priest",
        "death_knight", "deathknight", "shaman", "mage", "warlock", 
        "monk", "druid", "demon_hunter", "demonhunter", "evoker"
    };

    for (const char* className : classNames) {
        size_t len = strlen(className);
        if (line.length() >= len && line.substr(0, len) == className) {
            if (line.length() == len || line[len] == '=' || isspace(line[len])) return TRUE;
        }
    }
    return FALSE;
}

BOOL CSimcParser::ParseClassDefinitionLine(const std::string& line, CCharacterData* pOutData)
{
    size_t eqPos = line.find('=');
    if (eqPos == std::string::npos) return FALSE;

    std::string className = Trim(line.substr(0, eqPos));
    if (className == "death_knight") className = "deathknight";
    else if (className == "demon_hunter") className = "demonhunter";

    if (!className.empty()) {
        std::string displayClass = className;
        displayClass[0] = toupper(displayClass[0]);
        pOutData->SetClassName(displayClass);
    }

    std::string name = Trim(line.substr(eqPos + 1));
    if (name.length() >= 2 && name.front() == '"' && name.back() == '"')
        name = name.substr(1, name.length() - 2);

    if (!name.empty()) pOutData->SetName(name);
    return TRUE;
}
