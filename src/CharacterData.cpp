#include "pch.h"
#include "framework.h"
#include "CharacterData.h"
#include <map>
#include <cctype>

namespace
{
    void AppendItemLine(CString& profile, const CString& slotName, const CString& baseToken,
        uint32_t itemId, const std::vector<uint32_t>& bonusIds, const std::vector<uint32_t>& gemIds,
        uint32_t enchantId, uint32_t contextId, double itemLevel)
    {
        profile += slotName;
        profile += _T("=");
        profile += baseToken;

        if (itemId > 0)
        {
            CString itemIdText;
            itemIdText.Format(_T(",id=%u"), itemId);
            profile += itemIdText;
        }

        if (!bonusIds.empty())
        {
            profile += _T(",bonus_id=");
            for (size_t i = 0; i < bonusIds.size(); ++i)
            {
                if (i > 0) profile += _T("/");
                CString bonusId;
                bonusId.Format(_T("%u"), bonusIds[i]);
                profile += bonusId;
            }
        }

        if (!gemIds.empty())
        {
            profile += _T(",gem_id=");
            for (size_t i = 0; i < gemIds.size(); ++i)
            {
                if (i > 0) profile += _T("/");
                CString gemId;
                gemId.Format(_T("%u"), gemIds[i]);
                profile += gemId;
            }
        }

        if (enchantId > 0)
        {
            CString enchant;
            enchant.Format(_T(",enchant_id=%u"), enchantId);
            profile += enchant;
        }

        if (contextId > 0)
        {
            CString context;
            context.Format(_T(",context=%u"), contextId);
            profile += context;
        }

        if (itemLevel > 0.0)
        {
            CString itemLevelText;
            itemLevelText.Format(_T(",ilevel=%.0f"), itemLevel);
            profile += itemLevelText;
        }

        profile += _T("\n");
    }

    CString JoinBonusIds(const std::vector<uint32_t>& bonusIds)
    {
        CString joined;
        for (size_t i = 0; i < bonusIds.size(); ++i)
        {
            if (i > 0)
            {
                joined += _T("/");
            }

            CString value;
            value.Format(_T("%u"), bonusIds[i]);
            joined += value;
        }

        return joined;
    }
}

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
    m_strRace.clear();
    m_strSoulbind.clear();
    m_equipment.clear();
    m_items.clear();
    m_trinket1Override = TrinketOverrideSelection();
    m_trinket2Override = TrinketOverrideSelection();
    m_talentSpellIds.clear();
    m_talents.clear();
    m_talentData.clear();
    m_strTalentString.clear();
    m_settings = SimulationSettings();
}

CString CCharacterData::ToSimcProfile() const
{
    CString profile;
    const SimulationSettings& settings = m_settings;

    // Character header
    std::string simcClass = MapClassToSimc(m_strClassName);
    std::string simcSpec = MapSpecToSimc(m_strActiveSpec);

    profile += _T("# Base Actor\n");
    profile += _T("# Raidbots-generated SimC input\n\n");

    // Class and spec
    profile += CA2T(simcClass.c_str());
    profile += _T("=\"");
    profile += CA2T(m_strName.c_str());
    profile += _T("\"\n");

    profile += _T("level=");
    CString level;
    level.Format(_T("%d\n"), m_nLevel);
    profile += level;

    profile += _T("race=");
    profile += CA2T(m_strRace.empty() ? "human" : m_strRace.c_str());
    profile += _T("\n");

    profile += _T("region=");
    profile += CA2T(m_strRegion.c_str());
    profile += _T("\n");

    profile += _T("server=");
    profile += CA2T(m_strRealm.c_str());
    profile += _T("\n");

    profile += _T("role=");
    if (simcSpec == "tank") profile += _T("tank\n");
    else if (simcSpec == "healing" || simcSpec == "restoration") profile += _T("heal\n");
    else profile += _T("attack\n");

    profile += _T("spec=");
    profile += CA2T(simcSpec.c_str());
    profile += _T("\n");

    profile += _T("talents=");
    profile += CA2T(m_strTalentString.c_str());
    profile += _T("\n");

    profile += _T("\n");

    // Equipment
    for (const auto& item : m_items)
    {
        std::string simcSlot = ItemSlotToSimc(item.slot);
        if (simcSlot.empty()) continue;
        if ((item.slot == ItemSlot::TRINKET1 && m_trinket1Override.enabled) ||
            (item.slot == ItemSlot::TRINKET2 && m_trinket2Override.enabled))
        {
            continue;
        }

        const CString slotName(static_cast<LPCWSTR>(CA2W(simcSlot.c_str())));
        AppendItemLine(profile, slotName, _T(""), item.id, item.bonusIds, item.gemIds, item.enchantId, item.contextId, item.itemLevel);
    }

    const TrinketOverrideSelection* overrides[] = { &m_trinket1Override, &m_trinket2Override };
    for (const TrinketOverrideSelection* overrideSelection : overrides)
    {
        if (!overrideSelection->enabled || overrideSelection->slot == ItemSlot::NONE)
        {
            continue;
        }

        const std::string simcSlot = ItemSlotToSimc(overrideSelection->slot);
        if (simcSlot.empty())
        {
            continue;
        }

        double overrideItemLevel = overrideSelection->itemLevel;
        if (overrideItemLevel <= 0.0)
        {
            const ItemData* currentSlotItem = FindItem(overrideSelection->slot);
            if (currentSlotItem != nullptr)
            {
                overrideItemLevel = currentSlotItem->itemLevel;
            }
        }

        const CString slotName(static_cast<LPCWSTR>(CA2W(simcSlot.c_str())));
        const CString tokenName(static_cast<LPCWSTR>(CA2W(overrideSelection->simcToken.c_str())));
        AppendItemLine(profile, slotName, tokenName,
            overrideSelection->itemId, overrideSelection->bonusIds, {}, 0, overrideSelection->contextId, overrideItemLevel);
    }

    profile += _T("name=");
    profile += CA2T(m_strName.c_str());
    profile += _T("\n\n");

    profile += _T("# Consumables\n\n");
    if (!settings.usePotion) profile += _T("potion=disabled\n");
    if (!settings.useFood) profile += _T("food=disabled\n");
    if (!settings.useFlask) profile += _T("flask=disabled\n");
    if (!settings.useRune || !settings.useAugment) profile += _T("augmentation=disabled\n");
    if (!settings.usePotion || !settings.useFood || !settings.useFlask || !settings.useRune || !settings.useAugment)
    {
        profile += _T("\n");
    }

    // Expansion Options
    profile += _T("# Expansion Options\n");

    // Only emit temporary_enchant when a real simc token is provided.
    const CString temporaryEnchant = settings.temporaryEnchant;
    if (!temporaryEnchant.IsEmpty() &&
        temporaryEnchant.CompareNoCase(_T("None")) != 0 &&
        temporaryEnchant.Find(_T(":")) != -1)
    {
        profile += _T("temporary_enchant=");
        profile += temporaryEnchant;
        profile += _T("\n");
    }

    // Emit midnight options only when enabled to avoid parser errors on older simc builds.
    if (settings.useCrucibleViolence || settings.useCrucibleSustenance || settings.useCruciblePredation)
    {
        profile.AppendFormat(_T("midnight.crucible_of_erratic_energies_violence=%d\n"), settings.useCrucibleViolence ? 1 : 0);
        profile.AppendFormat(_T("midnight.crucible_of_erratic_energies_sustenance=%d\n"), settings.useCrucibleSustenance ? 1 : 0);
        profile.AppendFormat(_T("midnight.crucible_of_erratic_energies_predation=%d\n"), settings.useCruciblePredation ? 1 : 0);
    }

    profile += _T("\n# Actors\n\n");

    // Simulation Options
    profile += _T("# Simulation Options\n");
    profile.AppendFormat(_T("iterations=%d\n"), settings.iterations);
    profile.AppendFormat(_T("desired_targets=%d\n"), settings.targetCount);
    profile.AppendFormat(_T("max_time=%d\n"), settings.duration);
    profile.AppendFormat(_T("calculate_scale_factors=%d\n"), settings.calculateScaleFactors ? 1 : 0);
    
    if (settings.calculateScaleFactors && !settings.scaleOnly.IsEmpty()) {
        profile += _T("scale_only=");
        profile += settings.scaleOnly;
        profile += _T("\n");
    }

    // Buff Overrides
    profile.AppendFormat(_T("override.bloodlust=%d\n"), settings.useBloodlust ? 1 : 0);
    profile.AppendFormat(_T("override.arcane_intellect=%d\n"), settings.useArcaneIntellect ? 1 : 0);
    profile.AppendFormat(_T("override.power_word_fortitude=%d\n"), settings.usePowerWordFortitude ? 1 : 0);
    profile.AppendFormat(_T("override.battle_shout=%d\n"), settings.useBattleShout ? 1 : 0);
    profile.AppendFormat(_T("override.mystic_touch=%d\n"), settings.useMysticTouch ? 1 : 0);
    profile.AppendFormat(_T("override.chaos_brand=%d\n"), settings.useChaosBrand ? 1 : 0);
    profile.AppendFormat(_T("override.skyfury=%d\n"), settings.useSkyfury ? 1 : 0);
    profile.AppendFormat(_T("override.mark_of_the_wild=%d\n"), settings.useMarkOfTheWild ? 1 : 0);
    profile.AppendFormat(_T("override.hunters_mark=%d\n"), settings.useHuntersMark ? 1 : 0);
    profile.AppendFormat(_T("override.bleeding=%d\n"), settings.useBleeding ? 1 : 0);

    profile.AppendFormat(_T("report_details=%d\n"), settings.reportDetails ? 1 : 0);
    profile.AppendFormat(_T("single_actor_batch=%d\n"), settings.singleActorBatch ? 1 : 0);
    profile.AppendFormat(_T("optimize_expressions=%d\n"), settings.optimizeExpressions ? 1 : 0);
    
    CString targetError;
    targetError.Format(_T("target_error=%.2f\n"), settings.targetError);
    profile += targetError;

    return profile;
}

std::string CCharacterData::MapClassToSimc(const std::string& className)
{
    static const std::map<std::string, std::string> classMap = {
        {"Warrior", "warrior"}, {"Paladin", "paladin"}, {"Hunter", "hunter"},
        {"Rogue", "rogue"}, {"Priest", "priest"}, {"Deathknight", "deathknight"},
        {"Shaman", "shaman"}, {"Mage", "mage"}, {"Warlock", "warlock"},
        {"Monk", "monk"}, {"Druid", "druid"}, {"Demonhunter", "demonhunter"},
        {"Evoker", "evoker"}
    };
    auto it = classMap.find(className);
    if (it != classMap.end()) return it->second;
    
    std::string lowerClass = className;
    std::transform(lowerClass.begin(), lowerClass.end(), lowerClass.begin(), ::tolower);
    for (const auto& pair : classMap) {
        std::string lowerKey = pair.first;
        std::transform(lowerKey.begin(), lowerKey.end(), lowerKey.begin(), ::tolower);
        if (lowerKey == lowerClass) return pair.second;
    }
    return "unknown";
}

std::string CCharacterData::MapSpecToSimc(const std::string& specName)
{
    static const std::map<std::string, std::string> specMap = {
        {"Arms", "arms"}, {"Fury", "fury"}, {"Protection", "protection"},
        {"Holy", "holy"}, {"Retribution", "retribution"},
        {"Beast Mastery", "beast_mastery"}, {"Marksmanship", "marksmanship"}, {"Survival", "survival"},
        {"Assassination", "assassination"}, {"Outlaw", "outlaw"}, {"Subtlety", "subtlety"},
        {"Discipline", "discipline"}, {"Shadow", "shadow"},
        {"Blood", "blood"}, {"Frost", "frost"}, {"Unholy", "unholy"},
        {"Elemental", "elemental"}, {"Enhancement", "enhancement"}, {"Restoration", "restoration"},
        {"Arcane", "arcane"}, {"Fire", "fire"},
        {"Affliction", "affliction"}, {"Demonology", "demonology"}, {"Destruction", "destruction"},
        {"Brewmaster", "brewmaster"}, {"Windwalker", "windwalker"}, {"Mistweaver", "mistweaver"},
        {"Balance", "balance"}, {"Feral", "feral"}, {"Guardian", "guardian"},
        {"Havoc", "havoc"}, {"Vengeance", "vengeance"},
        {"Devastation", "devastation"}, {"Preservation", "preservation"}, {"Augmentation", "augmentation"}
    };
    auto it = specMap.find(specName);
    if (it != specMap.end()) return it->second;

    std::string lowerSpec = specName;
    std::transform(lowerSpec.begin(), lowerSpec.end(), lowerSpec.begin(), ::tolower);
    for (const auto& pair : specMap) {
        std::string lowerKey = pair.first;
        std::transform(lowerKey.begin(), lowerKey.end(), lowerKey.begin(), ::tolower);
        if (lowerKey == lowerSpec) return pair.second;
    }
    return "unknown";
}

std::string CCharacterData::MapSlotToSimc(const std::string& slotType)
{
    static const std::map<std::string, std::string> slotMap = {
        {"HEAD", "head"}, {"NECK", "neck"}, {"SHOULDER", "shoulders"},
        {"BACK", "back"}, {"CHEST", "chest"}, {"SHIRT", "shirt"},
        {"TABARD", "tabard"}, {"WRIST", "wrist"}, {"HANDS", "hands"},
        {"WAIST", "waist"}, {"LEGS", "legs"}, {"FEET", "feet"},
        {"FINGER_1", "finger1"}, {"FINGER_2", "finger2"},
        {"TRINKET_1", "trinket1"}, {"TRINKET_2", "trinket2"},
        {"MAIN_HAND", "main_hand"}, {"OFF_HAND", "off_hand"}, {"RANGED", "ranged"}
    };
    std::string upperSlot = slotType;
    std::transform(upperSlot.begin(), upperSlot.end(), upperSlot.begin(), ::toupper);
    auto it = slotMap.find(upperSlot);
    if (it != slotMap.end()) return it->second;
    return "";
}

std::string CCharacterData::ItemSlotToSimc(ItemSlot slot)
{
    switch (slot) {
    case ItemSlot::HEAD: return "head"; case ItemSlot::NECK: return "neck";
    case ItemSlot::SHOULDER: return "shoulders"; case ItemSlot::BACK: return "back";
    case ItemSlot::CHEST: return "chest"; case ItemSlot::WRIST: return "wrist";
    case ItemSlot::HANDS: return "hands"; case ItemSlot::WAIST: return "waist";
    case ItemSlot::LEGS: return "legs"; case ItemSlot::FEET: return "feet";
    case ItemSlot::FINGER1: return "finger1"; case ItemSlot::FINGER2: return "finger2";
    case ItemSlot::TRINKET1: return "trinket1"; case ItemSlot::TRINKET2: return "trinket2";
    case ItemSlot::MAIN_HAND: return "main_hand"; case ItemSlot::OFF_HAND: return "off_hand";
    default: return "";
    }
}

void CCharacterData::CalculateItemLevelFromItems()
{
    if (m_items.empty()) return;
    double totalIlvl = 0; int count = 0;
    double twoHandIlvl = 0;
    for (const auto& item : m_items) {
        if (item.itemLevel > 0) {
            if (item.slot == ItemSlot::MAIN_HAND) twoHandIlvl = item.itemLevel;
            else if (item.slot != ItemSlot::OFF_HAND) { totalIlvl += item.itemLevel; count++; }
        }
    }
    bool hasOffHand = false;
    for (const auto& item : m_items) { if (item.slot == ItemSlot::OFF_HAND && item.itemLevel > 0) { hasOffHand = true; break; } }
    if (!hasOffHand && twoHandIlvl > 0) { totalIlvl += twoHandIlvl * 2; count += 2; }
    else { if (twoHandIlvl > 0) { totalIlvl += twoHandIlvl; count++; }
        for (const auto& item : m_items) { if (item.slot == ItemSlot::OFF_HAND && item.itemLevel > 0) { totalIlvl += item.itemLevel; count++; break; } }
    }
    if (count > 0) m_fItemLevel = totalIlvl / count;
}

const CCharacterData::ItemData* CCharacterData::FindItem(ItemSlot slot) const
{
    for (const auto& item : m_items)
    {
        if (item.slot == slot)
        {
            return &item;
        }
    }

    return nullptr;
}

CString CCharacterData::DescribeItemSlot(ItemSlot slot) const
{
    const ItemData* item = FindItem(slot);
    if (item == nullptr || item->id == 0)
    {
        return _T("현재 장착 정보 없음");
    }

    CString description;
    if (item->itemLevel > 0.0)
    {
        description.Format(_T("현재 장착: id=%u, ilvl %.0f"), item->id, item->itemLevel);
    }
    else
    {
        description.Format(_T("현재 장착: id=%u"), item->id);
    }

    if (!item->bonusIds.empty())
    {
        description += _T(", bonus=");
        description += JoinBonusIds(item->bonusIds);
    }

    if (item->contextId > 0)
    {
        CString context;
        context.Format(_T(", context=%u"), item->contextId);
        description += context;
    }

    return description;
}

void CCharacterData::SetTrinketOverride(const TrinketOverrideSelection& selection)
{
    if (selection.slot == ItemSlot::TRINKET1)
    {
        m_trinket1Override = selection;
    }
    else if (selection.slot == ItemSlot::TRINKET2)
    {
        m_trinket2Override = selection;
    }
}

void CCharacterData::ClearTrinketOverride(ItemSlot slot)
{
    if (slot == ItemSlot::TRINKET1)
    {
        m_trinket1Override = TrinketOverrideSelection();
        m_trinket1Override.slot = ItemSlot::TRINKET1;
    }
    else if (slot == ItemSlot::TRINKET2)
    {
        m_trinket2Override = TrinketOverrideSelection();
        m_trinket2Override.slot = ItemSlot::TRINKET2;
    }
}

const CCharacterData::TrinketOverrideSelection* CCharacterData::GetTrinketOverride(ItemSlot slot) const
{
    if (slot == ItemSlot::TRINKET1)
    {
        return &m_trinket1Override;
    }

    if (slot == ItemSlot::TRINKET2)
    {
        return &m_trinket2Override;
    }

    return nullptr;
}

BOOL CCharacterData::ValidateTrinketOverrides(CString& errorMessage) const
{
    const TrinketOverrideSelection* overrides[] = { &m_trinket1Override, &m_trinket2Override };
    for (const TrinketOverrideSelection* overrideSelection : overrides)
    {
        if (!overrideSelection->enabled)
        {
            continue;
        }

        if (overrideSelection->itemId == 0 && overrideSelection->simcToken.empty())
        {
            errorMessage = _T("선택한 장신구의 SIMC 식별자가 없습니다.");
            return FALSE;
        }
    }

    if (m_trinket1Override.enabled && m_trinket2Override.enabled)
    {
        const bool sameId = m_trinket1Override.itemId != 0 &&
            m_trinket1Override.itemId == m_trinket2Override.itemId;
        const bool sameToken = m_trinket1Override.itemId == 0 && m_trinket2Override.itemId == 0 &&
            !m_trinket1Override.simcToken.empty() &&
            m_trinket1Override.simcToken == m_trinket2Override.simcToken;

        if ((sameId || sameToken) && (m_trinket1Override.uniqueEquipped || m_trinket2Override.uniqueEquipped))
        {
            errorMessage = _T("중복 착용이 불가능한 장신구를 두 슬롯에 동시에 선택했습니다.");
            return FALSE;
        }
    }

    return TRUE;
}
