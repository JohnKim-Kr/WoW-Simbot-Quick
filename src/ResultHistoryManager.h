#pragma once

#include "SimResult.h"

// 시뮬레이션 결과 히스토리 관리자
class CResultHistoryManager
{
public:
    CResultHistoryManager();
    virtual ~CResultHistoryManager();

    // 초기화 및 종료
    BOOL Initialize();
    void Shutdown();

    // 결과 추가
    BOOL AddResult(const CSimResult& result);

    // 결과 조회
    CSimResult* GetResult(const CString& id);
    CSimResult* GetResultByIndex(int index);
    int GetResultCount() const { return static_cast<int>(m_results.size()); }

    // 결과 목록
    const std::vector<CSimResult>& GetAllResults() const { return m_results; }
    std::vector<CSimResult> GetResultsForCharacter(const CString& name, const CString& server);

    // 결과 삭제
    BOOL DeleteResult(const CString& id);
    BOOL DeleteResultByIndex(int index);
    void ClearAllResults();

    // 결과 비교
    CString CompareResults(const CString& id1, const CString& id2) const;
    CString CompareResultsByIndex(int index1, int index2) const;

    // 결과 낵스포트
    BOOL ExportResultToJson(const CString& id, const CString& filePath) const;
    BOOL ExportResultToCsv(const CString& id, const CString& filePath) const;
    BOOL ExportAllToCsv(const CString& filePath) const;

    // 통계
    double GetAverageDpsForCharacter(const CString& name, const CString& server) const;
    double GetBestDpsForCharacter(const CString& name, const CString& server) const;

    // simc JSON에서 결과 파싱
    BOOL ParseSimcJson(const CString& jsonFile, CSimResult& outResult) const;

    // 최대 저장 개수 설정
    void SetMaxResults(int maxResults) { m_nMaxResults = maxResults; }
    int GetMaxResults() const { return m_nMaxResults; }

    // 결과 데이터베이스 파일 경로
    CString GetHistoryFilePath() const;

private:
    // 저장/로드
    void LoadHistory();
    void SaveHistory() const;
    void SaveResultToFile(const CSimResult& result) const;
    void DeleteResultFile(const CString& id) const;

    // 히스토리 디렉토리
    CString GetHistoryDirectory() const;
    CString GetResultFilePath(const CString& id) const;

    // 오래된 결과 정리
    void TrimOldResults();

    // ID로 인덱스 찾기
    int FindResultIndex(const CString& id) const;

private:
    std::vector<CSimResult> m_results;
    std::map<CString, int> m_idToIndex;
    int m_nMaxResults;
    BOOL m_initialized;

    static const int DEFAULT_MAX_RESULTS = 100;
};

// 전역 접근자
CResultHistoryManager* GetResultHistoryManager();
void DestroyResultHistoryManager();
