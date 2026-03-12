#include "pch.h"
#include "framework.h"
#include "SettingsManager.h"
#include "SimbotQuick.h"
#include <algorithm>

const SimulationSettings CSettingsManager::DEFAULT_SETTINGS;

CSettingsManager::CSettingsManager()
    : m_initialized(FALSE)
{
}

CSettingsManager::~CSettingsManager()
{
    if (m_initialized) Shutdown();
}

BOOL CSettingsManager::Initialize()
{
    if (m_initialized) return TRUE;
    LoadSettings();
    LoadPresets();
    m_initialized = TRUE;
    return TRUE;
}

void CSettingsManager::Shutdown()
{
    if (!m_initialized) return;
    SaveSettings();
    SavePresets();
    m_initialized = FALSE;
}

void CSettingsManager::LoadSettings()
{
    CWinApp* pApp = AfxGetApp();
    m_currentSettings.fightStyle = pApp->GetProfileString(_T("Settings"), _T("FightStyle"), _T("patchwerk"));
    m_currentSettings.duration = pApp->GetProfileInt(_T("Settings"), _T("Duration"), 300);
    m_currentSettings.iterations = pApp->GetProfileInt(_T("Settings"), _T("Iterations"), 10000);
    m_currentSettings.targetCount = pApp->GetProfileInt(_T("Settings"), _T("TargetCount"), 1);
    m_currentSettings.useFlask = pApp->GetProfileInt(_T("Settings"), _T("UseFlask"), 1);
    m_currentSettings.useFood = pApp->GetProfileInt(_T("Settings"), _T("UseFood"), 1);
    m_currentSettings.useRune = pApp->GetProfileInt(_T("Settings"), _T("UseRune"), 1);
    m_currentSettings.useAugment = pApp->GetProfileInt(_T("Settings"), _T("UseAugment"), 1);
    m_currentSettings.useBloodlust = pApp->GetProfileInt(_T("Settings"), _T("UseBloodlust"), 1);
    m_currentSettings.usePotion = pApp->GetProfileInt(_T("Settings"), _T("UsePotion"), 1);
    
    m_currentSettings.useArcaneIntellect = pApp->GetProfileInt(_T("Settings"), _T("UseArcaneIntellect"), 1);
    m_currentSettings.useBattleShout = pApp->GetProfileInt(_T("Settings"), _T("UseBattleShout"), 1);
    m_currentSettings.usePowerWordFortitude = pApp->GetProfileInt(_T("Settings"), _T("UsePowerWordFortitude"), 1);
    m_currentSettings.useMarkOfTheWild = pApp->GetProfileInt(_T("Settings"), _T("UseMarkOfTheWild"), 1);
    m_currentSettings.useMysticTouch = pApp->GetProfileInt(_T("Settings"), _T("UseMysticTouch"), 1);
    m_currentSettings.useChaosBrand = pApp->GetProfileInt(_T("Settings"), _T("UseChaosBrand"), 1);
    m_currentSettings.useSkyfury = pApp->GetProfileInt(_T("Settings"), _T("UseSkyfury"), 1);
    m_currentSettings.useHuntersMark = pApp->GetProfileInt(_T("Settings"), _T("UseHuntersMark"), 1);
    m_currentSettings.useBleeding = pApp->GetProfileInt(_T("Settings"), _T("UseBleeding"), 1);
    
    m_currentSettings.reportDetails = pApp->GetProfileInt(_T("Settings"), _T("ReportDetails"), 1);
    m_currentSettings.calculateScaleFactors = pApp->GetProfileInt(_T("Settings"), _T("CalculateScaleFactors"), 0);
    m_currentSettings.scaleOnly = pApp->GetProfileString(_T("Settings"), _T("ScaleOnly"), m_currentSettings.scaleOnly);
    m_currentSettings.singleActorBatch = pApp->GetProfileInt(_T("Settings"), _T("SingleActorBatch"), 1);
    m_currentSettings.optimizeExpressions = pApp->GetProfileInt(_T("Settings"), _T("OptimizeExpressions"), 1);
    m_currentSettings.targetError = _tstof(pApp->GetProfileString(_T("Settings"), _T("TargetError"), _T("0.05")));
    m_currentSettings.threads = pApp->GetProfileInt(_T("Settings"), _T("Threads"), 0);
}

void CSettingsManager::SaveSettings() const
{
    CWinApp* pApp = AfxGetApp();
    pApp->WriteProfileString(_T("Settings"), _T("FightStyle"), m_currentSettings.fightStyle);
    pApp->WriteProfileInt(_T("Settings"), _T("Duration"), m_currentSettings.duration);
    pApp->WriteProfileInt(_T("Settings"), _T("Iterations"), m_currentSettings.iterations);
    pApp->WriteProfileInt(_T("Settings"), _T("TargetCount"), m_currentSettings.targetCount);
    pApp->WriteProfileInt(_T("Settings"), _T("UseFlask"), m_currentSettings.useFlask);
    pApp->WriteProfileInt(_T("Settings"), _T("UseFood"), m_currentSettings.useFood);
    pApp->WriteProfileInt(_T("Settings"), _T("UseRune"), m_currentSettings.useRune);
    pApp->WriteProfileInt(_T("Settings"), _T("UseAugment"), m_currentSettings.useAugment);
    pApp->WriteProfileInt(_T("Settings"), _T("UseBloodlust"), m_currentSettings.useBloodlust);
    pApp->WriteProfileInt(_T("Settings"), _T("UsePotion"), m_currentSettings.usePotion);
    
    pApp->WriteProfileInt(_T("Settings"), _T("UseArcaneIntellect"), m_currentSettings.useArcaneIntellect);
    pApp->WriteProfileInt(_T("Settings"), _T("UseBattleShout"), m_currentSettings.useBattleShout);
    pApp->WriteProfileInt(_T("Settings"), _T("UsePowerWordFortitude"), m_currentSettings.usePowerWordFortitude);
    pApp->WriteProfileInt(_T("Settings"), _T("UseMarkOfTheWild"), m_currentSettings.useMarkOfTheWild);
    pApp->WriteProfileInt(_T("Settings"), _T("UseMysticTouch"), m_currentSettings.useMysticTouch);
    pApp->WriteProfileInt(_T("Settings"), _T("UseChaosBrand"), m_currentSettings.useChaosBrand);
    pApp->WriteProfileInt(_T("Settings"), _T("UseSkyfury"), m_currentSettings.useSkyfury);
    pApp->WriteProfileInt(_T("Settings"), _T("UseHuntersMark"), m_currentSettings.useHuntersMark);
    pApp->WriteProfileInt(_T("Settings"), _T("UseBleeding"), m_currentSettings.useBleeding);
    
    pApp->WriteProfileInt(_T("Settings"), _T("ReportDetails"), m_currentSettings.reportDetails);
    pApp->WriteProfileInt(_T("Settings"), _T("CalculateScaleFactors"), m_currentSettings.calculateScaleFactors);
    pApp->WriteProfileString(_T("Settings"), _T("ScaleOnly"), m_currentSettings.scaleOnly);
    pApp->WriteProfileInt(_T("Settings"), _T("SingleActorBatch"), m_currentSettings.singleActorBatch);
    pApp->WriteProfileInt(_T("Settings"), _T("OptimizeExpressions"), m_currentSettings.optimizeExpressions);
    CString targetError;
    targetError.Format(_T("%.2f"), m_currentSettings.targetError);
    pApp->WriteProfileString(_T("Settings"), _T("TargetError"), targetError);
    pApp->WriteProfileInt(_T("Settings"), _T("Threads"), m_currentSettings.threads);
}

void CSettingsManager::RestoreDefaults() { m_currentSettings = DEFAULT_SETTINGS; }

void CSettingsManager::LoadPresets()
{
    CWinApp* pApp = AfxGetApp();
    CString presetList = pApp->GetProfileString(_T("Presets"), _T("List"), _T(""));
    if (!presetList.IsEmpty()) {
        int pos = 0;
        CString token = presetList.Tokenize(_T(";"), pos);
        while (!token.IsEmpty()) {
            CString json = pApp->GetProfileString(_T("Presets"), token, _T(""));
            if (!json.IsEmpty()) {
                SettingsPreset p; p.name = token; JsonToSettings(json, p.settings); m_presets.push_back(p);
            }
            token = presetList.Tokenize(_T(";"), pos);
        }
    }
}

void CSettingsManager::SavePresets() const
{
    CWinApp* pApp = AfxGetApp();
    CString names;
    for (const auto& p : m_presets) {
        names += p.name; names += _T(";");
        pApp->WriteProfileString(_T("Presets"), p.name, SettingsToJson(p.settings));
    }
    pApp->WriteProfileString(_T("Presets"), _T("List"), names);
}

BOOL CSettingsManager::SavePreset(const CString& name, const CString& description)
{
    auto it = std::find_if(m_presets.begin(), m_presets.end(), [&](const SettingsPreset& p) { return p.name == name; });
    if (it != m_presets.end()) { it->settings = m_currentSettings; it->description = description; }
    else m_presets.push_back(SettingsPreset(name, m_currentSettings, description));
    return TRUE;
}

BOOL CSettingsManager::LoadPreset(const CString& name)
{
    auto it = std::find_if(m_presets.begin(), m_presets.end(), [&](const SettingsPreset& p) { return p.name == name; });
    if (it != m_presets.end()) { m_currentSettings = it->settings; return TRUE; }
    return FALSE;
}

BOOL CSettingsManager::DeletePreset(const CString& name)
{
    auto it = std::remove_if(m_presets.begin(), m_presets.end(), [&](const SettingsPreset& p) { return p.name == name; });
    if (it != m_presets.end()) { m_presets.erase(it, m_presets.end()); return TRUE; }
    return FALSE;
}

std::vector<CString> CSettingsManager::GetPresetNames() const { std::vector<CString> names; for (const auto& p : m_presets) names.push_back(p.name); return names; }
BOOL CSettingsManager::PresetExists(const CString& name) const { return std::any_of(m_presets.begin(), m_presets.end(), [&](const SettingsPreset& p) { return p.name == name; }); }
BOOL CSettingsManager::GetPresetInfo(const CString& name, SettingsPreset& preset) const {
    auto it = std::find_if(m_presets.begin(), m_presets.end(), [&](const SettingsPreset& p) { return p.name == name; });
    if (it != m_presets.end()) { preset = *it; return TRUE; }
    return FALSE;
}

std::vector<CString> CSettingsManager::GetFightStyles()
{
    std::vector<CString> styles;
    styles.push_back(_T("patchwerk"));
    styles.push_back(_T("dungeon_slice"));
    styles.push_back(_T("dungeon_route"));
    styles.push_back(_T("hectic_add_cleave"));
    styles.push_back(_T("casting_patchwerk"));
    return styles;
}

std::vector<CString> CSettingsManager::GetTempEnchants()
{
    std::vector<CString> enchants;
    enchants.push_back(_T("None"));
    enchants.push_back(_T("Shadow Oil"));
    enchants.push_back(_T("Frost Oil"));
    return enchants;
}

CString CSettingsManager::GetFightStyleDisplayName(const CString& style) { return style; }

CString CSettingsManager::GenerateSimcOptions() const
{
    CString opt;
    opt.Format(_T("fight_style=%s\niterations=%d\nmax_time=%d\ndesired_targets=%d\n"),
        (LPCTSTR)m_currentSettings.fightStyle, m_currentSettings.iterations, m_currentSettings.duration, m_currentSettings.targetCount);

    // Do not emit legacy consumable directives (flask/food/augmentation_rune=none).
    // Recent simc builds may reject these values/options depending on profile context.
    
    if (m_currentSettings.calculateScaleFactors) {
        opt += _T("calculate_scale_factors=1\n");
        if (!m_currentSettings.scaleOnly.IsEmpty()) {
            opt.AppendFormat(_T("scale_only=%s\n"), (LPCTSTR)m_currentSettings.scaleOnly);
        }
    }
    
    if (m_currentSettings.threads > 0) opt.AppendFormat(_T("threads=%d\n"), m_currentSettings.threads);
    
    return opt;
}

BOOL CSettingsManager::ValidateSettings(CString& err) const { return TRUE; }

CString CSettingsManager::SettingsToJson(const SimulationSettings& s) const { return _T("{}"); }
void CSettingsManager::JsonToSettings(const CString& json, SimulationSettings& s) const {}

static CSettingsManager g_settingsManager;
CSettingsManager* GetSettingsManager() { return &g_settingsManager; }
