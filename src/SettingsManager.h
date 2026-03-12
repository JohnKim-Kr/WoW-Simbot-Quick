#pragma once

#include <vector>
#include <string>
#include "SimulationSettings.h"

// Preset info structure
struct SettingsPreset
{
    CString name;
    SimulationSettings settings;
    CString description;
    CTime   createdTime;

    SettingsPreset() {}
    SettingsPreset(const CString& n, const SimulationSettings& s, const CString& d)
        : name(n), settings(s), description(d), createdTime(CTime::GetCurrentTime())
    {
    }
};

// Settings manager class
class CSettingsManager
{
public:
    CSettingsManager();
    virtual ~CSettingsManager();

    BOOL Initialize();
    void Shutdown();

    SimulationSettings& GetCurrentSettings() { return m_currentSettings; }
    const SimulationSettings& GetCurrentSettings() const { return m_currentSettings; }

    void LoadSettings();
    void SaveSettings() const;

    void RestoreDefaults();

    BOOL SavePreset(const CString& name, const CString& description = _T(""));
    BOOL LoadPreset(const CString& name);
    BOOL DeletePreset(const CString& name);
    std::vector<CString> GetPresetNames() const;
    BOOL PresetExists(const CString& name) const;

    BOOL GetPresetInfo(const CString& name, SettingsPreset& preset) const;

    static std::vector<CString> GetFightStyles();
    static std::vector<CString> GetTempEnchants();
    static CString GetFightStyleDisplayName(const CString& style);

    CString GenerateSimcOptions() const;
    BOOL ValidateSettings(CString& errorMessage) const;

    static const SimulationSettings DEFAULT_SETTINGS;

private:
    SimulationSettings m_currentSettings;
    std::vector<SettingsPreset> m_presets;
    BOOL m_initialized;

    void LoadPresets();
    void SavePresets() const;
    
    CString SettingsToJson(const SimulationSettings& s) const;
    void JsonToSettings(const CString& json, SimulationSettings& s) const;
};

CSettingsManager* GetSettingsManager();
