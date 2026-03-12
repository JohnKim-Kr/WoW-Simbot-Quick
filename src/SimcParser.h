#pragma once

class CCharacterData;

// Parser for SimulationCraft addon profile strings
class CSimcParser
{
public:
    CSimcParser();
    ~CSimcParser();

    // Parse simc profile string and populate character data
    BOOL Parse(const std::string& profileStr, CCharacterData* pOutData);

private:
    // Parse individual profile lines
    BOOL ParseCharacterLine(const std::string& line, CCharacterData* pOutData);
    BOOL ParseGearLine(const std::string& line, CCharacterData* pOutData);
    BOOL ParseTalentLine(const std::string& line, CCharacterData* pOutData);
    BOOL ParseSoulbindLine(const std::string& line, CCharacterData* pOutData);
    BOOL ParseRegionLine(const std::string& line, CCharacterData* pOutData);
    
    // Parse simulation and expansion options
    BOOL ParseSimulationOption(const std::string& line, CCharacterData* pOutData);
    BOOL ParseExpansionOption(const std::string& line, CCharacterData* pOutData);
    BOOL ParseBuffOverride(const std::string& line, CCharacterData* pOutData);

    // Helper functions
    std::string Trim(const std::string& str);
    std::vector<std::string> Split(const std::string& str, char delimiter);
    int ParseItemLevel(const std::string& bonusStr);

    // Class definition parsing (e.g., mage="Name")
    BOOL IsClassLine(const std::string& line);
    BOOL ParseClassDefinitionLine(const std::string& line, CCharacterData* pOutData);
};
