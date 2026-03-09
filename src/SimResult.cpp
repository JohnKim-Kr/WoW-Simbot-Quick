#include "pch.h"
#include "framework.h"
#include "SimResult.h"

CSimResult::CSimResult()
    : m_timestamp(CTime::GetCurrentTime())
    , m_duration(300)
    , m_iterations(10000)
    , m_simTime(0)
{
    GenerateId();
}

CSimResult::~CSimResult()
{
}

void CSimResult::GenerateId()
{
    m_strId = GenerateUuid();
}

CString CSimResult::GetTimestampString() const
{
    return m_timestamp.Format(_T("%Y-%m-%d %H:%M:%S"));
}

CString CSimResult::GenerateUuid()
{
    GUID guid;
    HRESULT hr = CoCreateGuid(&guid);
    if (SUCCEEDED(hr))
    {
        WCHAR szGuid[40] = {0};
        StringFromGUID2(guid, szGuid, 40);
        return CString(szGuid);
    }
    // Fallback: timestamp based
    return CTime::GetCurrentTime().Format(_T("%Y%m%d%H%M%S%f"));
}

json CSimResult::ToJson() const
{
    json j;
    j["id"] = CT2A(m_strId);
    j["timestamp"] = CT2A(GetTimestampString());

    // Build info
    j["build"]["character"] = CT2A(m_buildInfo.characterName);
    j["build"]["server"] = CT2A(m_buildInfo.server);
    j["build"]["region"] = CT2A(m_buildInfo.region);
    j["build"]["class"] = CT2A(m_buildInfo.className);
    j["build"]["spec"] = CT2A(m_buildInfo.specName);
    j["build"]["item_level"] = m_buildInfo.itemLevel;
    j["build"]["talents"] = CT2A(m_buildInfo.talents);

    // Simulation parameters
    j["simulation"]["fight_style"] = CT2A(m_fightStyle);
    j["simulation"]["duration"] = m_duration;
    j["simulation"]["iterations"] = m_iterations;
    j["simulation"]["sim_time"] = m_simTime;

    // DPS stats
    j["dps"]["mean"] = m_dpsStats.mean;
    j["dps"]["min"] = m_dpsStats.min;
    j["dps"]["max"] = m_dpsStats.max;
    j["dps"]["std_dev"] = m_dpsStats.stdDev;
    j["dps"]["error"] = m_dpsStats.error;
    j["dps"]["confidence"] = m_dpsStats.confidence;

    // HPS stats
    j["hps"]["mean"] = m_hpsStats.mean;
    j["hps"]["min"] = m_hpsStats.min;
    j["hps"]["max"] = m_hpsStats.max;
    j["hps"]["std_dev"] = m_hpsStats.stdDev;
    j["hps"]["error"] = m_hpsStats.error;
    j["hps"]["confidence"] = m_hpsStats.confidence;

    // Stat weights
    j["stat_weights"] = json::array();
    for (const auto& sw : m_statWeights)
    {
        json weight;
        weight["stat"] = CT2A(sw.statName);
        weight["value"] = sw.weight;
        weight["error"] = sw.error;
        weight["confidence"] = sw.confidence;
        j["stat_weights"].push_back(weight);
    }

    // Abilities
    j["abilities"] = json::array();
    for (const auto& ab : m_abilities)
    {
        json ability;
        ability["name"] = CT2A(ab.name);
        ability["dps"] = ab.dps;
        ability["pct"] = ab.pct;
        ability["count"] = ab.count;
        ability["interval"] = ab.interval;
        j["abilities"].push_back(ability);
    }

    j["json_path"] = CT2A(m_strJsonPath);

    return j;
}

BOOL CSimResult::FromJson(const json& j)
{
    try
    {
        if (j.contains("id"))
            m_strId = CA2T(j["id"].get<std::string>().c_str());
        if (j.contains("timestamp"))
        {
            CString timeStr = CA2T(j["timestamp"].get<std::string>().c_str());
            COleDateTime date;
            if (date.ParseDateTime(timeStr))
            {
                m_timestamp = CTime(date.GetYear(), date.GetMonth(), date.GetDay(),
                                   date.GetHour(), date.GetMinute(), date.GetSecond());
            }
        }

        // Build info
        if (j.contains("build"))
        {
            auto& build = j["build"];
            if (build.contains("character")) m_buildInfo.characterName = CA2T(build["character"].get<std::string>().c_str());
            if (build.contains("server")) m_buildInfo.server = CA2T(build["server"].get<std::string>().c_str());
            if (build.contains("region")) m_buildInfo.region = CA2T(build["region"].get<std::string>().c_str());
            if (build.contains("class")) m_buildInfo.className = CA2T(build["class"].get<std::string>().c_str());
            if (build.contains("spec")) m_buildInfo.specName = CA2T(build["spec"].get<std::string>().c_str());
            if (build.contains("item_level")) m_buildInfo.itemLevel = build["item_level"].get<double>();
            if (build.contains("talents")) m_buildInfo.talents = CA2T(build["talents"].get<std::string>().c_str());
        }

        // Simulation parameters
        if (j.contains("simulation"))
        {
            auto& sim = j["simulation"];
            if (sim.contains("fight_style")) m_fightStyle = CA2T(sim["fight_style"].get<std::string>().c_str());
            if (sim.contains("duration")) m_duration = sim["duration"].get<int>();
            if (sim.contains("iterations")) m_iterations = sim["iterations"].get<int>();
            if (sim.contains("sim_time")) m_simTime = sim["sim_time"].get<double>();
        }

        // DPS stats
        if (j.contains("dps"))
        {
            auto& dps = j["dps"];
            if (dps.contains("mean")) m_dpsStats.mean = dps["mean"].get<double>();
            if (dps.contains("min")) m_dpsStats.min = dps["min"].get<double>();
            if (dps.contains("max")) m_dpsStats.max = dps["max"].get<double>();
            if (dps.contains("std_dev")) m_dpsStats.stdDev = dps["std_dev"].get<double>();
            if (dps.contains("error")) m_dpsStats.error = dps["error"].get<double>();
            if (dps.contains("confidence")) m_dpsStats.confidence = dps["confidence"].get<double>();
        }

        // HPS stats
        if (j.contains("hps"))
        {
            auto& hps = j["hps"];
            if (hps.contains("mean")) m_hpsStats.mean = hps["mean"].get<double>();
            if (hps.contains("min")) m_hpsStats.min = hps["min"].get<double>();
            if (hps.contains("max")) m_hpsStats.max = hps["max"].get<double>();
            if (hps.contains("std_dev")) m_hpsStats.stdDev = hps["std_dev"].get<double>();
            if (hps.contains("error")) m_hpsStats.error = hps["error"].get<double>();
            if (hps.contains("confidence")) m_hpsStats.confidence = hps["confidence"].get<double>();
        }

        // Stat weights
        if (j.contains("stat_weights") && j["stat_weights"].is_array())
        {
            m_statWeights.clear();
            for (const auto& sw : j["stat_weights"])
            {
                StatWeight weight;
                if (sw.contains("stat")) weight.statName = CA2T(sw["stat"].get<std::string>().c_str());
                if (sw.contains("value")) weight.weight = sw["value"].get<double>();
                if (sw.contains("error")) weight.error = sw["error"].get<double>();
                if (sw.contains("confidence")) weight.confidence = sw["confidence"].get<double>();
                m_statWeights.push_back(weight);
            }
        }

        // Abilities
        if (j.contains("abilities") && j["abilities"].is_array())
        {
            m_abilities.clear();
            for (const auto& ab : j["abilities"])
            {
                AbilityInfo ability;
                if (ab.contains("name")) ability.name = CA2T(ab["name"].get<std::string>().c_str());
                if (ab.contains("dps")) ability.dps = ab["dps"].get<double>();
                if (ab.contains("pct")) ability.pct = ab["pct"].get<double>();
                if (ab.contains("count")) ability.count = ab["count"].get<int>();
                if (ab.contains("interval")) ability.interval = ab["interval"].get<double>();
                m_abilities.push_back(ability);
            }
        }

        if (j.contains("json_path"))
            m_strJsonPath = CA2T(j["json_path"].get<std::string>().c_str());

        return TRUE;
    }
    catch (const std::exception&)
    {
        return FALSE;
    }
}

BOOL CSimResult::SaveToFile(const CString& filePath) const
{
    try
    {
        json j = ToJson();
        std::ofstream file(CT2A(filePath));
        if (!file.is_open())
            return FALSE;

        file << j.dump(4);
        return TRUE;
    }
    catch (const std::exception&)
    {
        return FALSE;
    }
}

BOOL CSimResult::LoadFromFile(const CString& filePath)
{
    try
    {
        std::ifstream file(CT2A(filePath));
        if (!file.is_open())
            return FALSE;

        json j;
        file >> j;
        return FromJson(j);
    }
    catch (const std::exception&)
    {
        return FALSE;
    }
}

CString CSimResult::CompareWith(const CSimResult& other) const
{
    CString result;
    result += _T("=== 결과 비교 ===\n\n");

    // 빌드 정보
    result += _T("[빌드 정보]\n");
    result.AppendFormat(_T("이전: %s - %s (%s) IL %.1f\n"),
        m_buildInfo.characterName, m_buildInfo.specName, m_buildInfo.className, m_buildInfo.itemLevel);
    result.AppendFormat(_T("현재: %s - %s (%s) IL %.1f\n\n"),
        other.m_buildInfo.characterName, other.m_buildInfo.specName, other.m_buildInfo.className, other.m_buildInfo.itemLevel);

    // DPS 비교
    double dpsDiff = other.m_dpsStats.mean - m_dpsStats.mean;
    double dpsPct = (m_dpsStats.mean > 0) ? (dpsDiff / m_dpsStats.mean * 100.0) : 0;

    result += _T("[DPS 비교]\n");
    result.AppendFormat(_T("이전: %.1f (±%.1f)\n"), m_dpsStats.mean, m_dpsStats.error);
    result.AppendFormat(_T("현재: %.1f (±%.1f)\n"), other.m_dpsStats.mean, other.m_dpsStats.error);
    result.AppendFormat(_T("차이: %+.1f (%+.2f%%)\n\n"), dpsDiff, dpsPct);

    // HPS 비교 (힐러인 경우)
    if (m_hpsStats.mean > 0 || other.m_hpsStats.mean > 0)
    {
        double hpsDiff = other.m_hpsStats.mean - m_hpsStats.mean;
        double hpsPct = (m_hpsStats.mean > 0) ? (hpsDiff / m_hpsStats.mean * 100.0) : 0;

        result += _T("[HPS 비교]\n");
        result.AppendFormat(_T("이전: %.1f (±%.1f)\n"), m_hpsStats.mean, m_hpsStats.error);
        result.AppendFormat(_T("현재: %.1f (±%.1f)\n"), other.m_hpsStats.mean, other.m_hpsStats.error);
        result.AppendFormat(_T("차이: %+.1f (%+.2f%%)\n\n"), hpsDiff, hpsPct);
    }

    return result;
}

CString CSimResult::GetDpsString() const
{
    CString str;
    str.Format(_T("%.1f"), m_dpsStats.mean);
    return str;
}

CString CSimResult::GetHpsString() const
{
    CString str;
    str.Format(_T("%.1f"), m_hpsStats.mean);
    return str;
}
