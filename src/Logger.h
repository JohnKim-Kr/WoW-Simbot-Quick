#pragma once

// 로그 레벨 정의
enum class LogLevel
{
    Debug = 0,      // 상세 디버그 정보
    Info = 1,       // 일반 정보
    Warning = 2,    // 경고
    Error = 3,      // 오류
    Critical = 4    // 치명적 오류
};

// 로그 카테고리
enum class LogCategory
{
    General,        // 일반
    Api,            // Battle.net API
    Simc,           // Simc 실행
    Simulation,     // 시뮬레이션
    Ui,             // UI
    Settings,       // 설정
    Network         // 네트워크
};

// 로그 항목 구조체
struct LogEntry
{
    CTime       timestamp;
    LogLevel    level;
    LogCategory category;
    CString     message;
    CString     sourceFile;
    int         lineNumber;
    CString     functionName;

    LogEntry()
        : level(LogLevel::Info)
        , category(LogCategory::General)
        , lineNumber(0)
    {
    }

    CString ToString() const;
    CString GetLevelString() const;
    CString GetCategoryString() const;
};

// 로그 콜백 타입
using LogCallback = std::function<void(const LogEntry&)>;

// 로거 클래스
class CLogger
{
public:
    CLogger();
    virtual ~CLogger();

    // 초기화 및 종료
    BOOL Initialize(const CString& logDirectory = CString());
    void Shutdown();

    // 설정
    void SetLogLevel(LogLevel level) { m_minLevel = level; }
    LogLevel GetLogLevel() const { return m_minLevel; }

    void SetMaxLogFiles(int maxFiles) { m_maxLogFiles = maxFiles; }
    void SetMaxLogSize(DWORD maxSizeBytes) { m_maxLogSize = maxSizeBytes; }

    void EnableConsoleOutput(BOOL enable) { m_bConsoleOutput = enable; }
    void EnableFileOutput(BOOL enable) { m_bFileOutput = enable; }
    void EnableDebugOutput(BOOL enable) { m_bDebugOutput = enable; }

    // 카테고리별 로그 레벨 설정
    void SetCategoryLogLevel(LogCategory category, LogLevel level);
    LogLevel GetCategoryLogLevel(LogCategory category) const;

    // 로그 콜백 등록
    void SetCallback(LogCallback callback) { m_callback = callback; }

    // 로그 작성
    void Log(LogLevel level, LogCategory category, const CString& message,
             const char* file = nullptr, int line = 0, const char* func = nullptr);

    // 편의용 메서드
    void Debug(LogCategory category, const CString& message,
               const char* file = nullptr, int line = 0, const char* func = nullptr);
    void Info(LogCategory category, const CString& message,
              const char* file = nullptr, int line = 0, const char* func = nullptr);
    void Warning(LogCategory category, const CString& message,
                 const char* file = nullptr, int line = 0, const char* func = nullptr);
    void Error(LogCategory category, const CString& message,
               const char* file = nullptr, int line = 0, const char* func = nullptr);
    void Critical(LogCategory category, const CString& message,
                  const char* file = nullptr, int line = 0, const char* func = nullptr);

    // 로그 파일 관리
    void Flush();
    CString GetCurrentLogFile() const { return m_currentLogFile; }
    std::vector<CString> GetLogFiles() const;
    BOOL ClearAllLogs();
    BOOL ArchiveLogs(const CString& archivePath);

    // 로그 내용 읽기
    CString GetRecentLogs(int count = 100) const;
    BOOL ExportLogs(const CString& filePath, int maxEntries = 0) const;

    // 통계
    int GetTotalLogCount() const { return m_totalLogCount; }
    int GetLogCountByLevel(LogLevel level) const;

    // 성능 측정
    class ScopeTimer
    {
    public:
        ScopeTimer(CLogger* logger, LogCategory category, const CString& operation);
        ~ScopeTimer();

    private:
        CLogger*    m_logger;
        LogCategory m_category;
        CString     m_operation;
        std::chrono::high_resolution_clock::time_point m_start;
    };

private:
    // 내부 메서드
    CString GetLogDirectory() const;
    CString GenerateLogFileName() const;
    void OpenLogFile();
    void RotateLogFile();
    void CleanupOldLogs();
    void WriteToFile(const CString& text);
    BOOL ShouldLog(LogLevel level, LogCategory category) const;

    // 로그 레벨 문자열
    static CString LevelToString(LogLevel level);
    static CString CategoryToString(LogCategory category);

private:
    // 설정
    LogLevel    m_minLevel;
    int         m_maxLogFiles;
    DWORD       m_maxLogSize;
    BOOL        m_bConsoleOutput;
    BOOL        m_bFileOutput;
    BOOL        m_bDebugOutput;

    // 카테고리별 로그 레벨
    std::map<LogCategory, LogLevel> m_categoryLevels;

    // 파일 관련
    CString     m_logDirectory;
    CString     m_currentLogFile;
    CStdioFile* m_pLogFile;
    CCriticalSection m_fileLock;

    // 콜백
    LogCallback m_callback;

    // 통계
    int         m_totalLogCount;
    std::map<LogLevel, int> m_levelCounts;

    // 초기화 상태
    BOOL        m_initialized;

    // 상수
    static const int DEFAULT_MAX_LOG_FILES = 10;
    static const DWORD DEFAULT_MAX_LOG_SIZE = 10 * 1024 * 1024; // 10MB
};

// 전역 로거 접근자
CLogger* GetLogger();
void DestroyLogger();

// 편의용 매크로
#define LOG_DEBUG(cat, msg)     GetLogger()->Debug(cat, msg, __FILE__, __LINE__, __FUNCTION__)
#define LOG_INFO(cat, msg)      GetLogger()->Info(cat, msg, __FILE__, __LINE__, __FUNCTION__)
#define LOG_WARNING(cat, msg)   GetLogger()->Warning(cat, msg, __FILE__, __LINE__, __FUNCTION__)
#define LOG_ERROR(cat, msg)     GetLogger()->Error(cat, msg, __FILE__, __LINE__, __FUNCTION__)
#define LOG_CRITICAL(cat, msg)  GetLogger()->Critical(cat, msg, __FILE__, __LINE__, __FUNCTION__)

// 성능 측정 매크로
#define LOG_SCOPE_TIMER(cat, op)    CLogger::ScopeTimer _timer(GetLogger(), cat, op)
