#include "pch.h"
#include "framework.h"
#include "SettingsManager.h"

const SimulationSettings CSettingsManager::DEFAULT_SETTINGS;

static CSettingsManager* g_pSettingsManager = nullptr;

CSettingsManager* GetSettingsManager()
{
    if (!g_pSettingsManager)
    {
        g_pSettingsManager = new CSettingsManager();
        g_pSettingsManager->Initialize();
    }
    return g_pSettingsManager;
}

void DestroySettingsManager()
{
    if (g_pSettingsManager)
    {
        g_pSettingsManager->Shutdown();
        delete g_pSettingsManager;
        g_pSettingsManager = nullptr;
    }
}

CSettingsManager::CSettingsManager()
    : m_initialized(FALSE)
{
    m_currentSettings = DEFAULT_SETTINGS;
}

CSettingsManager::~CSettingsManager()
{
}

BOOL CSettingsManager::Initialize()
{
    if (m_initialized)
        return TRUE;

    LoadSettings();

    m_initialized = TRUE;
    return TRUE;
}

void CSettingsManager::Shutdown()
{
    if (m_initialized)
    {
        SaveSettings();
        m_initialized = FALSE;
    }
}

void CSettingsManager::LoadSettings()
{
    CWinApp* pApp = AfxGetApp();
    if (!pApp)
        return;

    // 전투 설정
    m_currentSettings.fightStyle = pApp->GetProfileString(_T("Simulation"), _T("FightStyle"), DEFAULT_SETTINGS.fightStyle);
    m_currentSettings.duration = pApp->GetProfileInt(_T("Simulation"), _T("Duration"), DEFAULT_SETTINGS.duration);
    m_currentSettings.iterations = pApp->GetProfileInt(_T("Simulation"), _T("Iterations"), DEFAULT_SETTINGS.iterations);
    m_currentSettings.targetCount = pApp->GetProfileInt(_T("Simulation"), _T("TargetCount"), DEFAULT_SETTINGS.targetCount);

    // 버프 설정
    m_currentSettings.useFlask = pApp->GetProfileInt(_T("Buffs"), _T("Flask"), DEFAULT_SETTINGS.useFlask);
    m_currentSettings.useFood = pApp->GetProfileInt(_T("Buffs"), _T("Food"), DEFAULT_SETTINGS.useFood);
    m_currentSettings.useRune = pApp->GetProfileInt(_T("Buffs"), _T("Rune"), DEFAULT_SETTINGS.useRune);
    m_currentSettings.useAugment = pApp->GetProfileInt(_T("Buffs"), _T("Augment"), DEFAULT_SETTINGS.useAugment);
    m_currentSettings.useBloodlust = pApp->GetProfileInt(_T("Buffs"), _T("Bloodlust"), DEFAULT_SETTINGS.useBloodlust);
    m_currentSettings.usePotion = pApp->GetProfileInt(_T("Buffs"), _T("Potion"), DEFAULT_SETTINGS.usePotion);

    // 외부 버프
    m_currentSettings.useArcaneIntellect = pApp->GetProfileInt(_T("ExternalBuffs"), _T("ArcaneIntellect"), DEFAULT_SETTINGS.useArcaneIntellect);
    m_currentSettings.useBattleShout = pApp->GetProfileInt(_T("ExternalBuffs"), _T("BattleShout"), DEFAULT_SETTINGS.useBattleShout);
    m_currentSettings.usePowerWordFortitude = pApp->GetProfileInt(_T("ExternalBuffs"), _T("PowerWordFortitude"), DEFAULT_SETTINGS.usePowerWordFortitude);
    m_currentSettings.useMarkOfTheWild = pApp->GetProfileInt(_T("ExternalBuffs"), _T("MarkOfTheWild"), DEFAULT_SETTINGS.useMarkOfTheWild);
    m_currentSettings.useMysticTouch = pApp->GetProfileInt(_T("ExternalBuffs"), _T("MysticTouch"), DEFAULT_SETTINGS.useMysticTouch);
    m_currentSettings.useChaosBrand = pApp->GetProfileInt(_T("ExternalBuffs"), _T("ChaosBrand"), DEFAULT_SETTINGS.useChaosBrand);
    m_currentSettings.useBleeding = pApp->GetProfileInt(_T("ExternalBuffs"), _T("Bleeding"), DEFAULT_SETTINGS.useBleeding);

    // 고급 설정
    m_currentSettings.reportDetails = pApp->GetProfileInt(_T("Advanced"), _T("ReportDetails"), DEFAULT_SETTINGS.reportDetails);
    m_currentSettings.calculateScaleFactors = pApp->GetProfileInt(_T("Advanced"), _T("ScaleFactors"), DEFAULT_SETTINGS.calculateScaleFactors);
    m_currentSettings.threads = pApp->GetProfileInt(_T("Advanced"), _T("Threads"), DEFAULT_SETTINGS.threads);

    // 프리셋 목록 불러오기
    CString presetList = pApp->GetProfileString(_T("Presets"), _T("List"), _T(""));
    if (!presetList.IsEmpty())
    {
        // 프리셋 이름 목록 파싱 (세미콜론으로 구분)
        int pos = 0;
        CString token = presetList.Tokenize(_T(";"), pos);
        while (!token.IsEmpty())
        {
            SettingsPreset preset;
            if (GetPresetInfo(token, preset))
            {
                m_presets.push_back(preset);
            }
            token = presetList.Tokenize(_T(";"), pos);
        }
    }

    // 유효성 검사
    CString errorMsg;
    if (!ValidateSettings(errorMsg))
    {
        // 기본값 복원
        RestoreDefaults();
    }
}

void CSettingsManager::SaveSettings() const
{
    CWinApp* pApp = AfxGetApp();
    if (!pApp)
        return;

    // 전투 설정
    pApp->WriteProfileString(_T("Simulation"), _T("FightStyle"), m_currentSettings.fightStyle);
    pApp->WriteProfileInt(_T("Simulation"), _T("Duration"), m_currentSettings.duration);
    pApp->WriteProfileInt(_T("Simulation"), _T("Iterations"), m_currentSettings.iterations);
    pApp->WriteProfileInt(_T("Simulation"), _T("TargetCount"), m_currentSettings.targetCount);

    // 버프 설정
    pApp->WriteProfileInt(_T("Buffs"), _T("Flask"), m_currentSettings.useFlask);
    pApp->WriteProfileInt(_T("Buffs"), _T("Food"), m_currentSettings.useFood);
    pApp->WriteProfileInt(_T("Buffs"), _T("Rune"), m_currentSettings.useRune);
    pApp->WriteProfileInt(_T("Buffs"), _T("Augment"), m_currentSettings.useAugment);
    pApp->WriteProfileInt(_T("Buffs"), _T("Bloodlust"), m_currentSettings.useBloodlust);
    pApp->WriteProfileInt(_T("Buffs"), _T("Potion"), m_currentSettings.usePotion);

    // 외부 버프
    pApp->WriteProfileInt(_T("ExternalBuffs"), _T("ArcaneIntellect"), m_currentSettings.useArcaneIntellect);
    pApp->WriteProfileInt(_T("ExternalBuffs"), _T("BattleShout"), m_currentSettings.useBattleShout);
    pApp->WriteProfileInt(_T("ExternalBuffs"), _T("PowerWordFortitude"), m_currentSettings.usePowerWordFortitude);
    pApp->WriteProfileInt(_T("ExternalBuffs"), _T("MarkOfTheWild"), m_currentSettings.useMarkOfTheWild);
    pApp->WriteProfileInt(_T("ExternalBuffs"), _T("MysticTouch"), m_currentSettings.useMysticTouch);
    pApp->WriteProfileInt(_T("ExternalBuffs"), _T("ChaosBrand"), m_currentSettings.useChaosBrand);
    pApp->WriteProfileInt(_T("ExternalBuffs"), _T("Bleeding"), m_currentSettings.useBleeding);

    // 고급 설정
    pApp->WriteProfileInt(_T("Advanced"), _T("ReportDetails"), m_currentSettings.reportDetails);
    pApp->WriteProfileInt(_T("Advanced"), _T("ScaleFactors"), m_currentSettings.calculateScaleFactors);
    pApp->WriteProfileInt(_T("Advanced"), _T("Threads"), m_currentSettings.threads);

    // 프리셋 목록 저장
    CString presetList;
    for (const auto& preset : m_presets)
    {
        if (!presetList.IsEmpty())
            presetList += _T(";");
        presetList += preset.name;
    }
    pApp->WriteProfileString(_T("Presets"), _T("List"), presetList);
}

void CSettingsManager::RestoreDefaults()
{
    m_currentSettings = DEFAULT_SETTINGS;
}

BOOL CSettingsManager::SavePreset(const CString& name, const CString& description)
{
    if (name.IsEmpty())
        return FALSE;

    // 이미 존재하는지 확인
    auto it = std::find_if(m_presets.begin(), m_presets.end(),
        [&name](const SettingsPreset& p) { return p.name == name; });

    if (it != m_presets.end())
    {
        // 덮어쓰기
        it->settings = m_currentSettings;
        it->description = description;
        it->createdTime = CTime::GetCurrentTime();
    }
    else
    {
        // 새 프리셋 추가
        m_presets.emplace_back(name, m_currentSettings, description);
    }

    // JSON으로 저장
    CWinApp* pApp = AfxGetApp();
    if (pApp)
    {
        json j = SettingsToJson(m_currentSettings);
        j["description"] = CT2A(description);
        j["created"] = CT2A(CTime::GetCurrentTime().Format(_T("%Y-%m-%d %H:%M:%S")));

        CString presetKey;
        presetKey.Format(_T("Preset_%s"), name);
        pApp->WriteProfileString(_T("Presets"), presetKey, CA2T(j.dump().c_str()));
    }

    return TRUE;
}

BOOL CSettingsManager::LoadPreset(const CString& name)
{
    SettingsPreset preset;
    if (!GetPresetInfo(name, preset))
        return FALSE;

    m_currentSettings = preset.settings;
    return TRUE;
}

BOOL CSettingsManager::DeletePreset(const CString& name)
{
    auto it = std::remove_if(m_presets.begin(), m_presets.end(),
        [&name](const SettingsPreset& p) { return p.name == name; });

    if (it == m_presets.end())
        return FALSE;

    m_presets.erase(it, m_presets.end());

    // 레지스트리에서도 삭제
    CWinApp* pApp = AfxGetApp();
    if (pApp)
    {
        CString presetKey;
        presetKey.Format(_T("Preset_%s"), name);
        pApp->WriteProfileString(_T("Presets"), presetKey, NULL);
    }

    return TRUE;
}

std::vector<CString> CSettingsManager::GetPresetNames() const
{
    std::vector<CString> names;
    for (const auto& preset : m_presets)
    {
        names.push_back(preset.name);
    }
    return names;
}

BOOL CSettingsManager::PresetExists(const CString& name) const
{
    return std::any_of(m_presets.begin(), m_presets.end(),
        [&name](const SettingsPreset& p) { return p.name == name; });
}

BOOL CSettingsManager::GetPresetInfo(const CString& name, SettingsPreset& preset) const
{
    CWinApp* pApp = AfxGetApp();
    if (!pApp)
        return FALSE;

    CString presetKey;
    presetKey.Format(_T("Preset_%s"), name);

    CString presetData = pApp->GetProfileString(_T("Presets"), presetKey, _T(""));
    if (presetData.IsEmpty())
        return FALSE;

    try
    {
        auto j = json::parse(CT2A(presetData));

        preset.name = name;
        JsonToSettings(j, preset.settings);

        if (j.contains("description"))
            preset.description = CA2T(j["description"].get<std::string>().c_str());

        if (j.contains("created"))
        {
            CString createdStr = CA2T(j["created"].get<std::string>().c_str());
            COleDateTime date;
            if (date.ParseDateTime(createdStr))
                preset.createdTime = CTime(date.GetYear(), date.GetMonth(), date.GetDay(),
                    date.GetHour(), date.GetMinute(), date.GetSecond());
        }

        return TRUE;
    }
    catch (const std::exception&)
    {
        return FALSE;
    }
}

std::vector<CString> CSettingsManager::GetFightStyles()
{
    return {
        _T("patchwerk"),
        _T("light_movement"),
        _T("heavy_movement"),
        _T("dungeon"),
        _T("casting_patchwerk"),
        _T("hectic_add_cleave")
    };
}

CString CSettingsManager::GetFightStyleDisplayName(const CString& style)
{
    static const std::map<CString, CString> displayNames = {
        {_T("patchwerk"), _T("Patchwerk (단일 대상)")},
        {_T("light_movement"), _T("Light Movement")},
        {_T("heavy_movement"), _T("Heavy Movement")},
        {_T("dungeon"), _T("Dungeon (Mythic+)")},
        {_T("casting_patchwerk"), _T("Casting Patchwerk")},
        {_T("hectic_add_cleave"), _T("Hectic Add Cleave")}
    };

    auto it = displayNames.find(style);
    if (it != displayNames.end())
        return it->second;

    return style;
}

BOOL CSettingsManager::ValidateSettings(CString& errorMessage) const
{
    // 전투 유형 검사
    auto fightStyles = GetFightStyles();
    if (std::find(fightStyles.begin(), fightStyles.end(), m_currentSettings.fightStyle) == fightStyles.end())
    {
        errorMessage = _T("잘못된 전투 유형입니다.");
        return FALSE;
    }

    // 지속 시간 검사
    if (m_currentSettings.duration < 10 || m_currentSettings.duration > 1800)
    {
        errorMessage = _T("지속 시간은 10초에서 1800초 사이여야 합니다.");
        return FALSE;
    }

    // 반복 횟수 검사
    if (m_currentSettings.iterations < 100 || m_currentSettings.iterations > 10000000)
    {
        errorMessage = _T("반복 횟수는 100회에서 10,000,000회 사이여야 합니다.");
        return FALSE;
    }

    // 대상 수 검사
    if (m_currentSettings.targetCount < 1 || m_currentSettings.targetCount > 100)
    {
        errorMessage = _T("대상 수는 1에서 100 사이여야 합니다.");
        return FALSE;
    }

    return TRUE;
}

CString CSettingsManager::GenerateSimcOptions() const
{
    CString options;

    // 전투 설정
    options += _T("# 전투 설정\n");
    options.AppendFormat(_T("fight_style=%s\n"), m_currentSettings.fightStyle);
    options.AppendFormat(_T("max_time=%d\n"), m_currentSettings.duration);
    options.AppendFormat(_T("iterations=%d\n"), m_currentSettings.iterations);
    options.AppendFormat(_T("desired_targets=%d\n"), m_currentSettings.targetCount);

    // 버프 설정
    options += _T("\n# 개인 버프\n");
    options.AppendFormat(_T("override.flask=%d\n"), m_currentSettings.useFlask ? 1 : 0);
    options.AppendFormat(_T("override.food=%d\n"), m_currentSettings.useFood ? 1 : 0);
    options.AppendFormat(_T("override.rune=%d\n"), m_currentSettings.useRune ? 1 : 0);
    options.AppendFormat(_T("override.augment=%d\n"), m_currentSettings.useAugment ? 1 : 0);
    options.AppendFormat(_T("override.potion=%d\n"), m_currentSettings.usePotion ? 1 : 0);

    // 외부 버프
    options += _T("\n# 외부 버프\n");
    options.AppendFormat(_T("override.bloodlust=%d\n"), m_currentSettings.useBloodlust ? 1 : 0);
    options.AppendFormat(_T("override.arcane_intellect=%d\n"), m_currentSettings.useArcaneIntellect ? 1 : 0);
    options.AppendFormat(_T("override.battle_shout=%d\n"), m_currentSettings.useBattleShout ? 1 : 0);
    options.AppendFormat(_T("override.power_word_fortitude=%d\n"), m_currentSettings.usePowerWordFortitude ? 1 : 0);
    options.AppendFormat(_T("override.mark_of_the_wild=%d\n"), m_currentSettings.useMarkOfTheWild ? 1 : 0);
    options.AppendFormat(_T("override.mystic_touch=%d\n"), m_currentSettings.useMysticTouch ? 1 : 0);
    options.AppendFormat(_T("override.chaos_brand=%d\n"), m_currentSettings.useChaosBrand ? 1 : 0);
    options.AppendFormat(_T("override.bleeding=%d\n"), m_currentSettings.useBleeding ? 1 : 0);

    // 고급 설정
    options += _T("\n# 고급 설정\n");
    options.AppendFormat(_T("report_details=%d\n"), m_currentSettings.reportDetails ? 1 : 0);
    options.AppendFormat(_T("calculate_scale_factors=%d\n"), m_currentSettings.calculateScaleFactors ? 1 : 0);

    if (m_currentSettings.threads > 0)
    {
        options.AppendFormat(_T("threads=%d\n"), m_currentSettings.threads);
    }

    return options;
}

json CSettingsManager::SettingsToJson(const SimulationSettings& settings) const
{
    json j;
    j["fight_style"] = CT2A(settings.fightStyle);
    j["duration"] = settings.duration;
    j["iterations"] = settings.iterations;
    j["target_count"] = settings.targetCount;

    j["buffs"]["flask"] = settings.useFlask ? true : false;
    j["buffs"]["food"] = settings.useFood ? true : false;
    j["buffs"]["rune"] = settings.useRune ? true : false;
    j["buffs"]["augment"] = settings.useAugment ? true : false;
    j["buffs"]["bloodlust"] = settings.useBloodlust ? true : false;
    j["buffs"]["potion"] = settings.usePotion ? true : false;

    j["external_buffs"]["arcane_intellect"] = settings.useArcaneIntellect ? true : false;
    j["external_buffs"]["battle_shout"] = settings.useBattleShout ? true : false;
    j["external_buffs"]["power_word_fortitude"] = settings.usePowerWordFortitude ? true : false;
    j["external_buffs"]["mark_of_the_wild"] = settings.useMarkOfTheWild ? true : false;
    j["external_buffs"]["mystic_touch"] = settings.useMysticTouch ? true : false;
    j["external_buffs"]["chaos_brand"] = settings.useChaosBrand ? true : false;
    j["external_buffs"]["bleeding"] = settings.useBleeding ? true : false;

    j["advanced"]["report_details"] = settings.reportDetails ? true : false;
    j["advanced"]["scale_factors"] = settings.calculateScaleFactors ? true : false;
    j["advanced"]["threads"] = settings.threads;

    return j;
}

BOOL CSettingsManager::JsonToSettings(const json& j, SimulationSettings& settings) const
{
    try
    {
        if (j.contains("fight_style"))
            settings.fightStyle = CA2T(j["fight_style"].get<std::string>().c_str());
        if (j.contains("duration"))
            settings.duration = j["duration"].get<int>();
        if (j.contains("iterations"))
            settings.iterations = j["iterations"].get<int>();
        if (j.contains("target_count"))
            settings.targetCount = j["target_count"].get<int>();

        if (j.contains("buffs"))
        {
            auto& buffs = j["buffs"];
            if (buffs.contains("flask")) settings.useFlask = buffs["flask"].get<bool>();
            if (buffs.contains("food")) settings.useFood = buffs["food"].get<bool>();
            if (buffs.contains("rune")) settings.useRune = buffs["rune"].get<bool>();
            if (buffs.contains("augment")) settings.useAugment = buffs["augment"].get<bool>();
            if (buffs.contains("bloodlust")) settings.useBloodlust = buffs["bloodlust"].get<bool>();
            if (buffs.contains("potion")) settings.usePotion = buffs["potion"].get<bool>();
        }

        if (j.contains("external_buffs"))
        {
            auto& ext = j["external_buffs"];
            if (ext.contains("arcane_intellect")) settings.useArcaneIntellect = ext["arcane_intellect"].get<bool>();
            if (ext.contains("battle_shout")) settings.useBattleShout = ext["battle_shout"].get<bool>();
            if (ext.contains("power_word_fortitude")) settings.usePowerWordFortitude = ext["power_word_fortitude"].get<bool>();
            if (ext.contains("mark_of_the_wild")) settings.useMarkOfTheWild = ext["mark_of_the_wild"].get<bool>();
            if (ext.contains("mystic_touch")) settings.useMysticTouch = ext["mystic_touch"].get<bool>();
            if (ext.contains("chaos_brand")) settings.useChaosBrand = ext["chaos_brand"].get<bool>();
            if (ext.contains("bleeding")) settings.useBleeding = ext["bleeding"].get<bool>();
        }

        if (j.contains("advanced"))
        {
            auto& adv = j["advanced"];
            if (adv.contains("report_details")) settings.reportDetails = adv["report_details"].get<bool>();
            if (adv.contains("scale_factors")) settings.calculateScaleFactors = adv["scale_factors"].get<bool>();
            if (adv.contains("threads")) settings.threads = adv["threads"].get<int>();
        }

        return TRUE;
    }
    catch (const std::exception&)
    {
        return FALSE;
    }
}
