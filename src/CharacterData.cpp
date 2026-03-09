#include "pch.h"
#include "framework.h"
#include "CharacterData.h"

CCharacterData::CCharacterData()
    : m_nLevel(0)
    , m_fItemLevel(0.0)
{
}

CCharacterData::~CCharacterData()
{
}

void CCharacterData::Clear()
{
    m_strName.clear();
    m_strRealm.clear();
    m_strRegion.clear();
    m_nLevel = 0;
    m_fItemLevel = 0.0;
    m_strClassName.clear();
    m_strActiveSpec.clear();
    m_equipment.clear();
    m_talentSpellIds.clear();
    m_talents.clear();
}

CString CCharacterData::ToSimcProfile() const
{
    CString profile;

    // Character header
    std::string simcClass = MapClassToSimc(m_strClassName);
    std::string simcSpec = MapSpecToSimc(m_strActiveSpec);

    profile += _T("# Character: ");
    profile += CA2T(m_strName.c_str());
    profile += _T(" - ");
    profile += CA2T(m_strRealm.c_str());
    profile += _T("\n");

    profile += _T("# Region: ");
    profile += CA2T(m_strRegion.c_str());
    profile += _T("\n");

    profile += _T("# Item Level: ");
    CString ilvl;
    ilvl.Format(_T("%.1f\n"), m_fItemLevel);
    profile += ilvl;

    profile += _T("\n");

    // Class and spec
    profile += CA2T(simcClass.c_str());
    profile += _T("=\"");
    profile += CA2T(m_strName.c_str());
    profile += _T("\"\n");

    profile += _T("  spec=");
    profile += CA2T(simcSpec.c_str());
    profile += _T("\n");

    profile += _T("  level=");
    CString level;
    level.Format(_T("%d\n"), m_nLevel);
    profile += level;

    profile += _T("  race=human\n");  // Default race - should be populated from API
    profile += _T("  role=");
    // Determine role based on spec (simplified)
    if (simcSpec == "tank")
        profile += _T("tank\n");
    else if (simcSpec == "healing" || simcSpec == "restoration")
        profile += _T("heal\n");
    else
        profile += _T("attack\n");

    profile += _T("\n");

    // Equipment
    for (const auto& slot : m_equipment)
    {
        std::string simcSlot = MapSlotToSimc(slot.slotType);
        if (simcSlot.empty())
            continue;

        profile += CA2T(simcSlot.c_str());
        profile += _T("=\"");
        profile += CA2T(slot.itemId.c_str());

        // Add bonus IDs
        if (!slot.bonusIds.empty())
        {
            profile += _T("", bonus_id="");
            for (size_t i = 0; i < slot.bonusIds.size(); ++i)
            {
                if (i > 0) profile += _T("/");
                profile += CA2T(slot.bonusIds[i].c_str());
            }
            profile += _T(""");
        }

        // Add gems
        if (!slot.gemIds.empty())
        {
            profile += _T(", gem_id="");
            for (size_t i = 0; i < slot.gemIds.size(); ++i)
            {
                if (i > 0) profile += _T("/");
                profile += CA2T(slot.gemIds[i].c_str());
            }
            profile += _T(""");
        }

        // Add enchant
        if (slot.enchantId > 0)
        {
            CString enchant;
            enchant.Format(_T(", enchant=%d"), slot.enchantId);
            profile += enchant;
        }

        profile += _T("\n");
    }

    profile += _T("\n");

    // Talents (new system - using talent codes or selected spell IDs)
    if (!m_talentSpellIds.empty())
    {
        profile += _T("# Talents\n");
        // In newer simc versions, talents are specified differently
        // This is a placeholder for the actual talent string format
        profile += _T("# talent_spells=");
        for (size_t i = 0; i < m_talentSpellIds.size(); ++i)
        {
            CString talent;
            if (i > 0) profile += _T(",");
            talent.Format(_T("%d"), m_talentSpellIds[i]);
            profile += talent;
        }
        profile += _T("\n");
    }

    return profile;
}

std::string CCharacterData::MapClassToSimc(const std::string& className)
{
    // Map class names to simc class identifiers
    static const std::map<std::string, std::string> classMap = {
        {"Warrior", "warrior"},
        {"Paladin", "paladin"},
        {"Hunter", "hunter"},
        {"Rogue", "rogue"},
        {"Priest", "priest"},
        {"Death Knight", "deathknight"},
        {"Shaman", "shaman"},
        {"Mage", "mage"},
        {"Warlock", "warlock"},
        {"Monk", "monk"},
        {"Druid", "druid"},
        {"Demon Hunter", "demonhunter"},
        {"Evoker", "evoker"}
    };

    auto it = classMap.find(className);
    if (it != classMap.end())
        return it->second;

    return "unknown";
}

std::string CCharacterData::MapSpecToSimc(const std::string& specName)
{
    // Map spec names to simc spec identifiers
    static const std::map<std::string, std::string> specMap = {
        // Warrior
        {"Arms", "arms"},
        {"Fury", "fury"},
        {"Protection", "protection"},
        // Paladin
        {"Holy", "holy"},
        {"Retribution", "retribution"},
        // Hunter
        {"Beast Mastery", "beast_mastery"},
        {"Marksmanship", "marksmanship"},
        {"Survival", "survival"},
        // Rogue
        {"Assassination", "assassination"},
        {"Outlaw", "outlaw"},
        {"Subtlety", "subtlety"},
        // Priest
        {"Discipline", "discipline"},
        {"Shadow", "shadow"},
        // Death Knight
        {"Blood", "blood"},
        {"Frost", "frost"},
        {"Unholy", "unholy"},
        // Shaman
        {"Elemental", "elemental"},
        {"Enhancement", "enhancement"},
        {"Restoration", "restoration"},
        // Mage
        {"Arcane", "arcane"},
        {"Fire", "fire"},
        {"Frost", "frost"},
        // Warlock
        {"Affliction", "affliction"},
        {"Demonology", "demonology"},
        {"Destruction", "destruction"},
        // Monk
        {"Brewmaster", "brewmaster"},
        {"Windwalker", "windwalker"},
        {"Mistweaver", "mistweaver"},
        // Druid
        {"Balance", "balance"},
        {"Feral", "feral"},
        {"Guardian", "guardian"},
        // Demon Hunter
        {"Havoc", "havoc"},
        {"Vengeance", "vengeance"},
        // Evoker
        {"Devastation", "devastation"},
        {"Preservation", "preservation"},
        {"Augmentation", "augmentation"}
    };

    auto it = specMap.find(specName);
    if (it != specMap.end())
        return it->second;

    // Try lowercase matching
    std::string lowerSpec = specName;
    std::transform(lowerSpec.begin(), lowerSpec.end(), lowerSpec.begin(), ::tolower);

    for (const auto& pair : specMap)
    {
        std::string lowerKey = pair.first;
        std::transform(lowerKey.begin(), lowerKey.end(), lowerKey.begin(), ::tolower);
        if (lowerKey == lowerSpec)
            return pair.second;
    }

    return "unknown";
}

std::string CCharacterData::MapSlotToSimc(const std::string& slotType)
{
    // Map equipment slot types to simc slot names
    static const std::map<std::string, std::string> slotMap = {
        {"HEAD", "head"},
        {"NECK", "neck"},
        {"SHOULDER", "shoulders"},
        {"BACK", "back"},
        {"CHEST", "chest"},
        {"SHIRT", "shirt"},
        {"TABARD", "tabard"},
        {"WRIST", "wrist"},
        {"HANDS", "hands"},
        {"WAIST", "waist"},
        {"LEGS", "legs"},
        {"FEET", "feet"},
        {"FINGER_1", "finger1"},
        {"FINGER_2", "finger2"},
        {"TRINKET_1", "trinket1"},
        {"TRINKET_2", "trinket2"},
        {"MAIN_HAND", "main_hand"},
        {"OFF_HAND", "off_hand"},
        {"RANGED", "ranged"}
    };

    // Convert input to uppercase for matching
    std::string upperSlot = slotType;
    std::transform(upperSlot.begin(), upperSlot.end(), upperSlot.begin(), ::toupper);

    auto it = slotMap.find(upperSlot);
    if (it != slotMap.end())
        return it->second;

    // Try direct lowercase
    std::string lowerSlot = slotType;
    std::transform(lowerSlot.begin(), lowerSlot.end(), lowerSlot.begin(), ::tolower);

    for (const auto& pair : slotMap)
    {
        if (pair.second == lowerSlot)
            return pair.second;
    }

    return "";
}
