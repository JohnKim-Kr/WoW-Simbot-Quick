#pragma once

// 디버깅 및 진단 도구 클래스
class CDebugTools
{
public:
    CDebugTools();
    ~CDebugTools();

    // 싱글톤 접근자
    static CDebugTools* GetInstance();
    static void DestroyInstance();

    // 메모리 정보
    CString GetMemoryInfo() const;
    void LogMemoryInfo();

    // GDI 리소스 정보
    CString GetGdiInfo() const;
    void LogGdiInfo();

    // 핸들 카운트
    int GetHandleCount() const;

    // 성능 카운터
    void StartPerformanceCounter(const CString& name);
    void StopPerformanceCounter(const CString& name);
    CString GetPerformanceReport() const;

    // API 호출 추적
    void EnableApiTracing(BOOL enable);
    BOOL IsApiTracingEnabled() const { return m_bApiTracing; }

    // Simc 명령줄 테스트
    BOOL TestSimcInstallation(const CString& simcPath, CString& outVersion, CString& outError);

    // 네트워크 연결 테스트
    BOOL TestNetworkConnection(const CString& host, int port, int timeoutMs, CString& outError);

    // Battle.net API 테스트
    BOOL TestBnetApi(const CString& clientId, const CString& clientSecret, CString& outError);

    // 설정 유효성 검사
    BOOL ValidateSettings(CString& outReport);

    // 시스템 정보
    CString GetSystemInfo() const;

    // 진단 보고서 생성
    CString GenerateDiagnosticReport() const;
    BOOL SaveDiagnosticReport(const CString& filePath) const;

    // 디버그 콘솔
    void ShowDebugConsole();
    void HideDebugConsole();
    BOOL IsDebugConsoleVisible() const;
    void WriteToDebugConsole(const CString& text);

private:
    static CDebugTools* s_instance;

    BOOL m_bApiTracing;
    BOOL m_bConsoleVisible;
    HANDLE m_hConsoleOutput;

    // 성능 카운터 데이터
    struct PerformanceData
    {
        std::chrono::high_resolution_clock::time_point start;
        std::chrono::high_resolution_clock::duration total;
        int count;

        PerformanceData()
            : count(0)
        {}
    };

    std::map<CString, PerformanceData> m_performanceCounters;
    mutable CCriticalSection m_perfLock;
};

// 편의용 매크로
#define DEBUG_TOOLS() CDebugTools::GetInstance()
