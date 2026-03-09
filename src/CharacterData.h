#pragma once

// Character data model class
// Stores all character information from Battle.net API
// and converts to simc profile format

class CCharacterData
{
public:
    // Equipment slot structure
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

    // Getters
    std::string GetName() const { return m_strName; }
    std::string GetRealm() const { return m_strRealm; }
    std::string GetRegion() const { return m_strRegion; }
    int GetLevel() const { return m_nLevel; }
    double GetItemLevel() const { return m_fItemLevel; }
    std::string GetClassName() const { return m_strClassName; }
    std::string GetActiveSpecName() const { return m_strActiveSpec; }

    // Equipment
    void AddEquipment(const EquipmentSlot& slot) { m_equipment.push_back(slot); }
    const std::vector<EquipmentSlot>& GetEquipment() const { return m_equipment; }
    void ClearEquipment() { m_equipment.clear(); }

    // Talents
    void AddTalent(int spellId) { m_talentSpellIds.push_back(spellId); }
    void AddTalent(const Talent& talent) { m_talents.push_back(talent); }
    const std::vector<int>& GetTalentSpellIds() const { return m_talentSpellIds; }

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

private:
    // Basic info
    std::string m_strName;
    std::string m_strRealm;
    std::string m_strRegion;
    int         m_nLevel;
    double      m_fItemLevel;
    std::string m_strClassName;
    std::string m_strActiveSpec;

    // Equipment
    std::vector<EquipmentSlot> m_equipment;

    // Talents
    std::vector<int> m_talentSpellIds;
    std::vector<Talent> m_talents;
};
