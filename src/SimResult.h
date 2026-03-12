#pragma once

// 시뮬레이션 결과 데이터 클래스
class CSimResult
{
public:
    // DPS 통계
    struct DpsStats
    {
        double mean;
        double minVal;
        double maxVal;
        double stdDev;
        double median;
        double error;
        double confidence;

        DpsStats() : mean(0), minVal(0), maxVal(0), stdDev(0), median(0), error(0), confidence(0) {}
    };

    // 능력치 기여도
    struct StatWeight
    {
        CString statName;
        double weight;
        double error;
        double confidence;

        StatWeight() : weight(0), error(0), confidence(0) {}
    };

    // 스킬 기여도
    struct AbilityInfo
    {
        CString name;
        double dps;
        double pct;
        int count;
        double interval;

        AbilityInfo() : dps(0), pct(0), count(0), interval(0) {}
    };

    // 빌드 정보
    struct BuildInfo
    {
        CString characterName;
        CString server;
        CString region;
        CString className;
        CString specName;
        double itemLevel;
        CString talents;

        BuildInfo() : itemLevel(0) {}
    };

public:
    CSimResult();
    virtual ~CSimResult();

    // 고유 ID
    CString GetId() const { return m_strId; }
    void GenerateId();

    // 타임스탬프
    CTime GetTimestamp() const { return m_timestamp; }
    void SetTimestamp(const CTime& time) { m_timestamp = time; }
    CString GetTimestampString() const;

    // 기본 정보
    BuildInfo& GetBuildInfo() { return m_buildInfo; }
    const BuildInfo& GetBuildInfo() const { return m_buildInfo; }

    // DPS 통계
    DpsStats& GetDpsStats() { return m_dpsStats; }
    const DpsStats& GetDpsStats() const { return m_dpsStats; }

    // HPS 통계 (힐러용)
    DpsStats& GetHpsStats() { return m_hpsStats; }
    const DpsStats& GetHpsStats() const { return m_hpsStats; }

    // 시뮬레이션 정보
    CString GetFightStyle() const { return m_fightStyle; }
    void SetFightStyle(const CString& style) { m_fightStyle = style; }

    int GetDuration() const { return m_duration; }
    void SetDuration(int duration) { m_duration = duration; }

    int GetIterations() const { return m_iterations; }
    void SetIterations(int iterations) { m_iterations = iterations; }

    double GetSimTime() const { return m_simTime; }
    void SetSimTime(double time) { m_simTime = time; }

    // 능력치 기여도
    std::vector<StatWeight>& GetStatWeights() { return m_statWeights; }
    const std::vector<StatWeight>& GetStatWeights() const { return m_statWeights; }
    void AddStatWeight(const StatWeight& weight) { m_statWeights.push_back(weight); }

    // 스킬 기여도
    std::vector<AbilityInfo>& GetAbilities() { return m_abilities; }
    const std::vector<AbilityInfo>& GetAbilities() const { return m_abilities; }
    void AddAbility(const AbilityInfo& ability) { m_abilities.push_back(ability); }

    // JSON 파일 경로
    CString GetJsonPath() const { return m_strJsonPath; }
    void SetJsonPath(const CString& path) { m_strJsonPath = path; }

    // 유효성 검사
    BOOL IsValid() const { return m_dpsStats.mean > 0; }

    // 직렬화
    json ToJson() const;
    BOOL FromJson(const json& j);

    // 파일 저장/로드
    BOOL SaveToFile(const CString& filePath) const;
    BOOL LoadFromFile(const CString& filePath);

    // 간단한 비교
    CString CompareWith(const CSimResult& other) const;

    // 핵심 수치 문자열
    CString GetDpsString() const;
    CString GetHpsString() const;

private:
    CString     m_strId;           // 고유 ID
    CTime       m_timestamp;       // 시뮬레이션 시간
    BuildInfo   m_buildInfo;       // 빌드 정보

    // DPS/HPS 통계
    DpsStats    m_dpsStats;
    DpsStats    m_hpsStats;

    // 시뮬레이션 파라미터
    CString     m_fightStyle;
    int         m_duration;
    int         m_iterations;
    double      m_simTime;         // 시뮬레이션 실행 시간 (초)

    // 결과 데이터
    std::vector<StatWeight> m_statWeights;
    std::vector<AbilityInfo> m_abilities;

    // 원본 JSON 경로
    CString     m_strJsonPath;

    // UUID 생성
    static CString GenerateUuid();
};
