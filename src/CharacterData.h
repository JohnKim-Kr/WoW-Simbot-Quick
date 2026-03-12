#pragma once

#include <vector>
#include <string>
#include <cstdint>
#include <memory>
#include "SimulationSettings.h"

// Character data model class
// Stores all character information from Battle.net API or simc addon
// and converts to simc profile format

class CCharacterData
{
public:
    // Item slot enumeration
    enum class ItemSlot
    {
        NONE,
        HEAD,
        NECK,
        SHOULDER,
        BACK,
        CHEST,
        WRIST,
        HANDS,
        WAIST,
        LEGS,
        FEET,
        FINGER1,
        FINGER2,
        TRINKET1,
        TRINKET2,
        MAIN_HAND,
        OFF_HAND
    };

    // Item data structure
    struct ItemData
    {
        ItemSlot slot;
        uint32_t id;
        double itemLevel;
        uint32_t enchantId;
        std::vector<uint32_t> gemIds;
        std::vector<uint32_t> bonusIds;

        ItemData() : slot(ItemSlot::NONE), id(0), itemLevel(0.0), enchantId(0) {}
    };

    // Equipment slot structure (for API data)
    struct EquipmentSlot
    {
        std::string slotType;
        std::string itemId;
        int itemLevel;
        std::string quality;
        std::string name;
        std::vector<std::string> bonusIds;
        std::vector<std::string> gemIds;
        int enchantId;

        EquipmentSlot() : itemLevel(0), enchantId(0) {}
    };

    // Talent structure
    struct TalentData
    {
        int tier;
        int choice;
        int talentId;
        int spellId;
        std::string name;

        TalentData() : tier(0), choice(0), talentId(0), spellId(0) {}
    };

    struct Talent
    {
        int talentId;
        int spellId;
        std::string name;
        int rank;

        Talent() : talentId(0), spellId(0), rank(0) {}
    };

public:
    CCharacterData();
    virtual ~CCharacterData();

    // Basic info
    void SetName(const std::string& name) { m_strName = name; }
    void SetRealm(const std::string& realm) { m_strRealm = realm; }
    void SetRegion(const std::string& region) { m_strRegion = region; }
    void SetLevel(int level) { m_nLevel = level; }
    void SetItemLevel(double ilvl) { m_fItemLevel = ilvl; }
    void SetClassName(const std::string& className) { m_strClassName = className; }
    void SetActiveSpecName(const std::string& specName) { m_strActiveSpec = specName; }
    void SetRace(const std::string& race) { m_strRace = race; }
    void SetSoulbind(const std::string& soulbind) { m_strSoulbind = soulbind; }

    // Getters
    std::string GetName() const { return m_strName; }
    std::string GetRealm() const { return m_strRealm; }
    std::string GetRegion() const { return m_strRegion; }
    int GetLevel() const { return m_nLevel; }
    double GetItemLevel() const { return m_fItemLevel; }
    std::string GetClassName() const { return m_strClassName; }
    std::string GetActiveSpecName() const { return m_strActiveSpec; }
    std::string GetRace() const { return m_strRace; }
    std::string GetSoulbind() const { return m_strSoulbind; }

    // Equipment (API format)
    void AddEquipment(const EquipmentSlot& slot) { m_equipment.push_back(slot); }
    const std::vector<EquipmentSlot>& GetEquipment() const { return m_equipment; }
    void ClearEquipment() { m_equipment.clear(); }

    // Items (simc parser format)
    void AddItem(const ItemData& item) { m_items.push_back(item); }
    const std::vector<ItemData>& GetItems() const { return m_items; }
    void ClearItems() { m_items.clear(); }

    // Talents
    void AddTalent(int spellId) { m_talentSpellIds.push_back(spellId); }
    void AddTalent(const TalentData& talent) { m_talentData.push_back(talent); }
    void AddTalent(const Talent& talent) { m_talents.push_back(talent); }
    const std::vector<int>& GetTalentSpellIds() const { return m_talentSpellIds; }
    const std::vector<TalentData>& GetTalentData() const { return m_talentData; }

    // Dragonflight talent string
    void SetTalentString(const std::string& talentStr) { m_strTalentString = talentStr; }
    std::string GetTalentString() const { return m_strTalentString; }

    // Simulation Settings
    SimulationSettings& GetSettings() { return m_settings; }
    const SimulationSettings& GetSettings() const { return m_settings; }

    // Validation
    BOOL IsValid() const { return !m_strName.empty() && m_nLevel > 0; }
    void Clear();

    // Convert to simc profile format
    CString ToSimcProfile() const;

    // Map class name to simc class name
    static std::string MapClassToSimc(const std::string& className);

    // Map spec name to simc spec
    static std::string MapSpecToSimc(const std::string& specName);

    // Map slot type to simc slot name
    static std::string MapSlotToSimc(const std::string& slotType);

    // Map ItemSlot to simc slot name
    static std::string ItemSlotToSimc(ItemSlot slot);

    // Calculate average item level from equipped items
    void CalculateItemLevelFromItems();

private:
    // Basic info
    std::string m_strName;
    std::string m_strRealm;
    std::string m_strRegion;
    int         m_nLevel;
    double      m_fItemLevel;
    std::string m_strClassName;
    std::string m_strActiveSpec;
    std::string m_strRace;
    std::string m_strSoulbind;

    // Equipment
    std::vector<EquipmentSlot> m_equipment;

    // Items (from simc parser)
    std::vector<ItemData> m_items;

    // Talents
    std::vector<int> m_talentSpellIds;
    std::vector<Talent> m_talents;
    std::vector<TalentData> m_talentData;
    std::string m_strTalentString;

    // Parsed simulation settings
    SimulationSettings m_settings;
};
