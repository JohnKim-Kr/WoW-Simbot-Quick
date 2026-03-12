#include "pch.h"
#include "framework.h"
#include "ResultHistoryManager.h"

static CResultHistoryManager* g_pResultHistoryManager = nullptr;

CResultHistoryManager* GetResultHistoryManager()
{
    if (!g_pResultHistoryManager)
    {
        g_pResultHistoryManager = new CResultHistoryManager();
        g_pResultHistoryManager->Initialize();
    }
    return g_pResultHistoryManager;
}

void DestroyResultHistoryManager()
{
    if (g_pResultHistoryManager)
    {
        g_pResultHistoryManager->Shutdown();
        delete g_pResultHistoryManager;
        g_pResultHistoryManager = nullptr;
    }
}

CResultHistoryManager::CResultHistoryManager()
    : m_nMaxResults(DEFAULT_MAX_RESULTS)
    , m_initialized(FALSE)
{
}

CResultHistoryManager::~CResultHistoryManager()
{
}

BOOL CResultHistoryManager::Initialize()
{
    if (m_initialized)
        return TRUE;

    // 히스토리 디렉토리 생성
    CString historyDir = GetHistoryDirectory();
    if (!CreateDirectory(historyDir, NULL) && GetLastError() != ERROR_ALREADY_EXISTS)
    {
        TRACE1("Failed to create history directory: %s\n", CT2A(historyDir));
    }

    LoadHistory();

    m_initialized = TRUE;
    return TRUE;
}

void CResultHistoryManager::Shutdown()
{
    if (m_initialized)
    {
        SaveHistory();
        m_initialized = FALSE;
    }
}

CString CResultHistoryManager::GetHistoryDirectory() const
{
    TCHAR szPath[MAX_PATH];
    if (SUCCEEDED(SHGetFolderPath(NULL, CSIDL_LOCAL_APPDATA, NULL, 0, szPath)))
    {
        CString path(szPath);
        path += _T("\\WoWSimbotQuick\\History");
        return path;
    }

    // Fallback to app directory
    return _T(".\\History");
}

CString CResultHistoryManager::GetResultFilePath(const CString& id) const
{
    CString filePath = GetHistoryDirectory();
    filePath += _T("\\");
    filePath += id;
    filePath += _T(".json");
    return filePath;
}

CString CResultHistoryManager::GetHistoryFilePath() const
{
    CString filePath = GetHistoryDirectory();
    filePath += _T("\\history.json");
    return filePath;
}

void CResultHistoryManager::LoadHistory()
{
    CString historyFile = GetHistoryFilePath();

    std::string histFile = std::string(CT2A(historyFile));
    std::ifstream file(histFile);
    if (!file.is_open())
        return;

    try
    {
        json j;
        file >> j;

        if (j.contains("results") && j["results"].is_array())
        {
            for (const auto& resultJson : j["results"])
            {
                CSimResult result;
                if (result.FromJson(resultJson))
                {
                    // 결과 파일이 실제로 존재하는지 확인
                    CString resultFile = GetResultFilePath(result.GetId());
                    if (PathFileExists(resultFile))
                    {
                        m_idToIndex[result.GetId()] = static_cast<int>(m_results.size());
                        m_results.push_back(result);
                    }
                }
            }
        }

        if (j.contains("max_results"))
            m_nMaxResults = j["max_results"].get<int>();
    }
    catch (const std::exception& e)
    {
        TRACE1("Failed to load history: %s\n", e.what());
    }
}

void CResultHistoryManager::SaveHistory() const
{
    CString historyFile = GetHistoryFilePath();

    // 디렉토리 생성
    CString historyDir = GetHistoryDirectory();
    CreateDirectory(historyDir, NULL);

    try
    {
        json j;
        j["version"] = 1;
        j["max_results"] = m_nMaxResults;
        j["results"] = json::array();

        for (const auto& result : m_results)
        {
            j["results"].push_back(result.ToJson());
            SaveResultToFile(result);
        }

        std::string histPath = std::string(CT2A(historyFile));
        std::ofstream file(histPath);
        if (file.is_open())
        {
            file << j.dump(4);
        }
    }
    catch (const std::exception& e)
    {
        TRACE1("Failed to save history: %s\n", e.what());
    }
}

void CResultHistoryManager::SaveResultToFile(const CSimResult& result) const
{
    CString filePath = GetResultFilePath(result.GetId());
    result.SaveToFile(filePath);
}

void CResultHistoryManager::DeleteResultFile(const CString& id) const
{
    CString filePath = GetResultFilePath(id);
    DeleteFile(filePath);
}

BOOL CResultHistoryManager::AddResult(const CSimResult& result)
{
    // 맨 앞에 추가 (최신순)
    m_results.insert(m_results.begin(), result);
    m_idToIndex[result.GetId()] = 0;

    // 인덱스 업데이트
    for (size_t i = 0; i < m_results.size(); ++i)
    {
        m_idToIndex[m_results[i].GetId()] = static_cast<int>(i);
    }

    // 오래된 결과 정리
    TrimOldResults();

    // 저장
    SaveHistory();

    return TRUE;
}

CSimResult* CResultHistoryManager::GetResult(const CString& id)
{
    auto it = m_idToIndex.find(id);
    if (it != m_idToIndex.end())
    {
        return &m_results[it->second];
    }
    return nullptr;
}

CSimResult* CResultHistoryManager::GetResultByIndex(int index)
{
    if (index >= 0 && index < static_cast<int>(m_results.size()))
    {
        return &m_results[index];
    }
    return nullptr;
}

std::vector<CSimResult> CResultHistoryManager::GetResultsForCharacter(const CString& name, const CString& server)
{
    std::vector<CSimResult> results;
    for (const auto& result : m_results)
    {
        if (result.GetBuildInfo().characterName == CT2A(name) &&
            result.GetBuildInfo().server == CT2A(server))
        {
            results.push_back(result);
        }
    }
    return results;
}

BOOL CResultHistoryManager::DeleteResult(const CString& id)
{
    auto it = m_idToIndex.find(id);
    if (it == m_idToIndex.end())
        return FALSE;

    int index = it->second;
    return DeleteResultByIndex(index);
}

BOOL CResultHistoryManager::DeleteResultByIndex(int index)
{
    if (index < 0 || index >= static_cast<int>(m_results.size()))
        return FALSE;

    CString id = m_results[index].GetId();

    // 파일 삭제
    DeleteResultFile(id);

    // 벡터에서 제거
    m_results.erase(m_results.begin() + index);

    // 인덱스 맵 재구성
    m_idToIndex.clear();
    for (size_t i = 0; i < m_results.size(); ++i)
    {
        m_idToIndex[m_results[i].GetId()] = static_cast<int>(i);
    }

    // 저장
    SaveHistory();

    return TRUE;
}

void CResultHistoryManager::ClearAllResults()
{
    // 모든 파일 삭제
    for (const auto& result : m_results)
    {
        DeleteResultFile(result.GetId());
    }

    m_results.clear();
    m_idToIndex.clear();

    SaveHistory();
}

void CResultHistoryManager::TrimOldResults()
{
    while (static_cast<int>(m_results.size()) > m_nMaxResults)
    {
        // 가장 오래된 결과 삭제 (맨 뒤)
        CString id = m_results.back().GetId();
        DeleteResultFile(id);
        m_idToIndex.erase(id);
        m_results.pop_back();
    }
}

int CResultHistoryManager::FindResultIndex(const CString& id) const
{
    auto it = m_idToIndex.find(id);
    if (it != m_idToIndex.end())
        return it->second;
    return -1;
}

CString CResultHistoryManager::CompareResults(const CString& id1, const CString& id2) const
{
    auto it1 = m_idToIndex.find(id1);
    auto it2 = m_idToIndex.find(id2);

    if (it1 == m_idToIndex.end() || it2 == m_idToIndex.end())
        return _T("결과를 찾을 수 없습니다.");

    return m_results[it1->second].CompareWith(m_results[it2->second]);
}

CString CResultHistoryManager::CompareResultsByIndex(int index1, int index2) const
{
    if (index1 < 0 || index1 >= static_cast<int>(m_results.size()) ||
        index2 < 0 || index2 >= static_cast<int>(m_results.size()))
    {
        return _T("잘못된 인덱스입니다.");
    }

    return m_results[index1].CompareWith(m_results[index2]);
}

BOOL CResultHistoryManager::ExportResultToJson(const CString& id, const CString& filePath) const
{
    auto it = m_idToIndex.find(id);
    if (it == m_idToIndex.end())
        return FALSE;

    return m_results[it->second].SaveToFile(filePath);
}

BOOL CResultHistoryManager::ExportResultToCsv(const CString& id, const CString& filePath) const
{
    auto it = m_idToIndex.find(id);
    if (it == m_idToIndex.end())
        return FALSE;

    const CSimResult& result = m_results[it->second];

    try
    {
        std::string exportPath = std::string(CT2A(filePath));
        std::ofstream file(exportPath);
        if (!file.is_open())
            return FALSE;

        // UTF-8 BOM
        file << "\xEF\xBB\xBF";

        // 헤더
        file << "항목,값\n";

        // 기본 정보
        file << "캐릭터," << CT2A(result.GetBuildInfo().characterName) << "\n";
        file << "서버," << CT2A(result.GetBuildInfo().server) << "\n";
        file << "직업," << CT2A(result.GetBuildInfo().className) << "\n";
        file << "전문화," << CT2A(result.GetBuildInfo().specName) << "\n";
        file << "아이템레벨," << result.GetBuildInfo().itemLevel << "\n";

        // DPS
        file << "\nDPS 통계\n";
        file << "평균 DPS," << result.GetDpsStats().mean << "\n";
        file << "최소 DPS," << result.GetDpsStats().minVal << "\n";
        file << "최대 DPS," << result.GetDpsStats().maxVal << "\n";
        file << "표준편차," << result.GetDpsStats().stdDev << "\n";
        file << "오차," << result.GetDpsStats().error << "\n";

        // 스킬 기여도
        file << "\n스킬 기여도\n";
        file << "스킬명,DPS,비율,사용횟수\n";
        for (const auto& ability : result.GetAbilities())
        {
            file << CT2A(ability.name) << ","
                 << ability.dps << ","
                 << ability.pct << ","
                 << ability.count << "\n";
        }

        return TRUE;
    }
    catch (const std::exception&)
    {
        return FALSE;
    }
}

BOOL CResultHistoryManager::ExportAllToCsv(const CString& filePath) const
{
    try
    {
        std::string exportPath = std::string(CT2A(filePath));
        std::ofstream file(exportPath);
        if (!file.is_open())
            return FALSE;

        // UTF-8 BOM
        file << "\xEF\xBB\xBF";

        // 헤더
        file << "날짜,캐릭터,서버,직업,전문화,IL,DPS,DPS 오차,HPS,전투유형,지속시간,반복횟수\n";

        // 데이터
        for (const auto& result : m_results)
        {
            file << CT2A(result.GetTimestampString()) << ","
                 << CT2A(result.GetBuildInfo().characterName) << ","
                 << CT2A(result.GetBuildInfo().server) << ","
                 << CT2A(result.GetBuildInfo().className) << ","
                 << CT2A(result.GetBuildInfo().specName) << ","
                 << result.GetBuildInfo().itemLevel << ","
                 << result.GetDpsStats().mean << ","
                 << result.GetDpsStats().error << ","
                 << result.GetHpsStats().mean << ","
                 << CT2A(result.GetFightStyle()) << ","
                 << result.GetDuration() << ","
                 << result.GetIterations() << "\n";
        }

        return TRUE;
    }
    catch (const std::exception&)
    {
        return FALSE;
    }
}

double CResultHistoryManager::GetAverageDpsForCharacter(const CString& name, const CString& server) const
{
    double totalDps = 0;
    int count = 0;

    for (const auto& result : m_results)
    {
        if (result.GetBuildInfo().characterName == CT2A(name) &&
            result.GetBuildInfo().server == CT2A(server))
        {
            totalDps += result.GetDpsStats().mean;
            count++;
        }
    }

    return count > 0 ? totalDps / count : 0;
}

double CResultHistoryManager::GetBestDpsForCharacter(const CString& name, const CString& server) const
{
    double bestDps = 0;

    for (const auto& result : m_results)
    {
        if (result.GetBuildInfo().characterName == CT2A(name) &&
            result.GetBuildInfo().server == CT2A(server))
        {
            if (result.GetDpsStats().mean > bestDps)
                bestDps = result.GetDpsStats().mean;
        }
    }

    return bestDps;
}

BOOL CResultHistoryManager::ParseSimcJson(const CString& jsonFile, CSimResult& outResult) const
{
    try
    {
        std::string jsonPath = std::string(CT2A(jsonFile));
        std::ifstream file(jsonPath);
        if (!file.is_open())
        {
            TRACE(_T("ParseSimcJson: Failed to open file %s\n"), jsonFile);
            return FALSE;
        }

        json j;
        file >> j;

        if (!j.contains("sim") || !j["sim"].contains("players"))
        {
            TRACE(_T("ParseSimcJson: Missing 'sim' or 'players' in JSON\n"));
            return FALSE;
        }

        auto& players = j["sim"]["players"];
        if (players.empty())
        {
            TRACE(_T("ParseSimcJson: Players array is empty\n"));
            return FALSE;
        }

        auto& player = players[0];

        if (!player.contains("collected_data"))
        {
            TRACE(_T("ParseSimcJson: Missing 'collected_data' in player\n"));
            return FALSE;
        }

        auto& data = player["collected_data"];

        // DPS 통계
        if (data.contains("dps"))
        {
            auto& dps = data["dps"];

            if (dps.contains("mean"))
                outResult.GetDpsStats().mean = dps["mean"].get<double>();
            if (dps.contains("min"))
                outResult.GetDpsStats().minVal = dps["min"].get<double>();
            if (dps.contains("max"))
                outResult.GetDpsStats().maxVal = dps["max"].get<double>();
            if (dps.contains("std_dev"))
                outResult.GetDpsStats().stdDev = dps["std_dev"].get<double>();
            if (dps.contains("median"))
                outResult.GetDpsStats().median = dps["median"].get<double>();
            if (dps.contains("error"))
                outResult.GetDpsStats().error = dps["error"].get<double>();
            if (dps.contains("confidence"))
                outResult.GetDpsStats().confidence = dps["confidence"].get<double>();
        }
        else
        {
            TRACE(_T("ParseSimcJson: Missing 'dps' in collected_data\n"));
        }

        // HPS 통계
        if (data.contains("hps"))
        {
            auto& hps = data["hps"];
            if (hps.contains("mean"))
                outResult.GetHpsStats().mean = hps["mean"].get<double>();
            if (hps.contains("min"))
                outResult.GetHpsStats().minVal = hps["min"].get<double>();
            if (hps.contains("max"))
                outResult.GetHpsStats().maxVal = hps["max"].get<double>();
            if (hps.contains("std_dev"))
                outResult.GetHpsStats().stdDev = hps["std_dev"].get<double>();
            if (hps.contains("median"))
                outResult.GetHpsStats().median = hps["median"].get<double>();
            if (hps.contains("error"))
                outResult.GetHpsStats().error = hps["error"].get<double>();
            if (hps.contains("confidence"))
                outResult.GetHpsStats().confidence = hps["confidence"].get<double>();
        }

        // 빌드 정보
        if (player.contains("name"))
            outResult.GetBuildInfo().characterName = CA2T(player["name"].get<std::string>().c_str());

        // 시뮬레이션 파라미터
        if (j["sim"].contains("options"))
        {
            auto& opts = j["sim"]["options"];
            if (opts.contains("max_time"))
                outResult.SetDuration(static_cast<int>(opts["max_time"].get<double>()));
            if (opts.contains("iterations"))
                outResult.SetIterations(opts["iterations"].get<int>());
            if (opts.contains("fight_style"))
                outResult.SetFightStyle(CString(opts["fight_style"].get<std::string>().c_str()));
        }

        // 스킬 기여도
        if (player.contains("collected_data") && data.contains("actions"))
        {
            auto& actions = data["actions"];
            for (auto it = actions.begin(); it != actions.end(); ++it)
            {
                CSimResult::AbilityInfo ability;
                ability.name = CA2T(it.key().c_str());

                auto& action = it.value();
                if (action.contains("dps"))
                    ability.dps = action["dps"]["mean"].get<double>();
                if (action.contains("pct"))
                    ability.pct = action["pct"]["mean"].get<double>();
                if (action.contains("num_executes"))
                    ability.count = static_cast<int>(action["num_executes"]["mean"].get<double>());

                outResult.AddAbility(ability);
            }
        }

        // 실행 시간
        if (j["sim"].contains("statistics") && j["sim"]["statistics"].contains("elapsed_cpu_seconds"))
        {
            outResult.SetSimTime(j["sim"]["statistics"]["elapsed_cpu_seconds"].get<double>());
        }

        outResult.SetJsonPath(jsonFile);

        return TRUE;
    }
    catch (const std::exception& e)
    {
        CString errorMsg;
        errorMsg.Format(_T("ParseSimcJson error: %s"), CA2T(e.what()));
        AfxMessageBox(errorMsg, MB_ICONERROR);
        return FALSE;
    }
}
