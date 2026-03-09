#pragma once

// 시뮬레이션 설정 구조체
struct SimulationSettings
{
    // 전투 설정
    CString fightStyle;
    int     duration;
    int     iterations;
    int     targetCount;

    // 버프 설정
    BOOL    useFlask;
    BOOL    useFood;
    BOOL    useRune;
    BOOL    useAugment;
    BOOL    useBloodlust;
    BOOL    usePotion;

    // 플레이어 설정
    BOOL    useArcaneIntellect;
    BOOL    useBattleShout;
    BOOL    usePowerWordFortitude;
    BOOL    useMarkOfTheWild;
    BOOL    useMysticTouch;
    BOOL    useChaosBrand;
    BOOL    useBleeding;

    // 고급 설정
    BOOL    reportDetails;
    BOOL    calculateScaleFactors;
    int     threads;

    SimulationSettings()
        : fightStyle(_T("patchwerk"))
        , duration(300)
        , iterations(10000)
        , targetCount(1)
        , useFlask(TRUE)
        , useFood(TRUE)
        , useRune(TRUE)
        , useAugment(TRUE)
        , useBloodlust(TRUE)
        , usePotion(TRUE)
        , useArcaneIntellect(TRUE)
        , useBattleShout(TRUE)
        , usePowerWordFortitude(TRUE)
        , useMarkOfTheWild(TRUE)
        , useMysticTouch(TRUE)
        , useChaosBrand(TRUE)
        , useBleeding(TRUE)
        , reportDetails(TRUE)
        , calculateScaleFactors(FALSE)
        , threads(0)  // 0 = auto
    {
    }
};

// 프리셋 정보 구조체
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

// 설정 관리자 클래스
class CSettingsManager
{
public:
    CSettingsManager();
    virtual ~CSettingsManager();

    // 초기화
    BOOL Initialize();
    void Shutdown();

    // 현재 설정 접근자
    SimulationSettings& GetCurrentSettings() { return m_currentSettings; }
    const SimulationSettings& GetCurrentSettings() const { return m_currentSettings; }

    // 설정 저장/불러오기
    void LoadSettings();
    void SaveSettings() const;

    // 기본값 복원
    void RestoreDefaults();

    // 프리셋 관리
    BOOL SavePreset(const CString& name, const CString& description = _T(""));
    BOOL LoadPreset(const CString& name);
    BOOL DeletePreset(const CString& name);
    std::vector<CString> GetPresetNames() const;
    BOOL PresetExists(const CString& name) const;

    // 특정 프리셋 정보 가져오기
    BOOL GetPresetInfo(const CString& name, SettingsPreset& preset) const;

    // 전투 유형 목록
    static std::vector<CString> GetFightStyles();
    static CString GetFightStyleDisplayName(const CString& style);

    // 설정 유효성 검사
    BOOL ValidateSettings(CString& errorMessage) const;

    // 설정을 simc 형식 옵션으로 변환
    CString GenerateSimcOptions() const;

private:
    // 레지스트리 키 경로
    CString GetSettingsRegistryKey() const;
    CString GetPresetsRegistryKey() const;
    CString GetPresetRegistryKey(const CString& presetName) const;

    // 개별 설정 저장/불러오기
    void LoadSettingFromRegistry(const CString& keyName, int& value, int defaultValue);
    void LoadSettingFromRegistry(const CString& keyName, BOOL& value, BOOL defaultValue);
    void LoadSettingFromRegistry(const CString& keyName, CString& value, const CString& defaultValue);

    void SaveSettingToRegistry(const CString& keyName, int value) const;
    void SaveSettingToRegistry(const CString& keyName, BOOL value) const;
    void SaveSettingToRegistry(const CString& keyName, const CString& value) const;

    // JSON 변환 (프리셋용)
    json SettingsToJson(const SimulationSettings& settings) const;
    BOOL JsonToSettings(const json& j, SimulationSettings& settings) const;

private:
    SimulationSettings m_currentSettings;
    std::vector<SettingsPreset> m_presets;
    BOOL m_initialized;

    // 기본값
    static const SimulationSettings DEFAULT_SETTINGS;
};

// 전역 접근자
CSettingsManager* GetSettingsManager();
