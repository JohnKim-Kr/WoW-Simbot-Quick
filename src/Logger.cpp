#include "pch.h"
#include "framework.h"
#include "Logger.h"

static CLogger* g_pLogger = nullptr;

CLogger* GetLogger()
{
    if (!g_pLogger)
    {
        g_pLogger = new CLogger();
        g_pLogger->Initialize();
    }
    return g_pLogger;
}

void DestroyLogger()
{
    if (g_pLogger)
    {
        g_pLogger->Shutdown();
        delete g_pLogger;
        g_pLogger = nullptr;
    }
}

// LogEntry 구현
CString LogEntry::ToString() const
{
    CString str;
    str.Format(_T("[%s] [%s] [%s] %s"),
        timestamp.Format(_T("%Y-%m-%d %H:%M:%S")),
        GetLevelString(),
        GetCategoryString(),
        message);

    if (!sourceFile.IsEmpty())
    {
        CString sourceInfo;
        sourceInfo.Format(_T(" (%s:%d)"), sourceFile, lineNumber);
        str += sourceInfo;
    }

    return str;
}

CString LogEntry::GetLevelString() const
{
    switch (level)
    {
    case LogLevel::Debug:    return _T("DEBUG");
    case LogLevel::Info:     return _T("INFO");
    case LogLevel::Warning:  return _T("WARN");
    case LogLevel::Error:    return _T("ERROR");
    case LogLevel::Critical: return _T("CRIT");
    default:                 return _T("UNKNOWN");
    }
}

CString LogEntry::GetCategoryString() const
{
    switch (category)
    {
    case LogCategory::General:     return _T("GENERAL");
    case LogCategory::Api:         return _T("API");
    case LogCategory::Simc:        return _T("SIMC");
    case LogCategory::Simulation:  return _T("SIM");
    case LogCategory::Ui:          return _T("UI");
    case LogCategory::Settings:    return _T("CONFIG");
    case LogCategory::Network:     return _T("NET");
    default:                       return _T("UNKNOWN");
    }
}

// CLogger 구현
CLogger::CLogger()
    : m_minLevel(LogLevel::Info)
    , m_maxLogFiles(DEFAULT_MAX_LOG_FILES)
    , m_maxLogSize(DEFAULT_MAX_LOG_SIZE)
    , m_bConsoleOutput(FALSE)
    , m_bFileOutput(TRUE)
    , m_bDebugOutput(TRUE)
    , m_pLogFile(nullptr)
    , m_totalLogCount(0)
    , m_initialized(FALSE)
{
}

CLogger::~CLogger()
{
    Shutdown();
}

BOOL CLogger::Initialize(const CString& logDirectory)
{
    if (m_initialized)
        return TRUE;

    // 로그 디렉토리 설정
    if (logDirectory.IsEmpty())
    {
        m_logDirectory = GetLogDirectory();
    }
    else
    {
        m_logDirectory = logDirectory;
    }

    // 디렉토리 생성
    if (!CreateDirectory(m_logDirectory, NULL) && GetLastError() != ERROR_ALREADY_EXISTS)
    {
        // 실패핏도 계속 진행 (파일 출력 비활성화)
        m_bFileOutput = FALSE;
    }

    // 로그 파일 열기
    if (m_bFileOutput)
    {
        OpenLogFile();
    }

    // 기본 카테고리 레벨 설정
    m_categoryLevels[LogCategory::General] = LogLevel::Info;
    m_categoryLevels[LogCategory::Api] = LogLevel::Info;
    m_categoryLevels[LogCategory::Simc] = LogLevel::Info;
    m_categoryLevels[LogCategory::Simulation] = LogLevel::Info;
    m_categoryLevels[LogCategory::Ui] = LogLevel::Warning;
    m_categoryLevels[LogCategory::Settings] = LogLevel::Info;
    m_categoryLevels[LogCategory::Network] = LogLevel::Info;

    m_initialized = TRUE;

    Info(LogCategory::General, _T("Logger initialized"));

    return TRUE;
}

void CLogger::Shutdown()
{
    if (!m_initialized)
        return;

    Info(LogCategory::General, _T("Logger shutting down"));

    Flush();

    CSingleLock lock(&m_fileLock, TRUE);
    if (m_pLogFile)
    {
        delete m_pLogFile;
        m_pLogFile = nullptr;
    }

    m_initialized = FALSE;
}

CString CLogger::GetLogDirectory() const
{
    TCHAR szPath[MAX_PATH];
    if (SUCCEEDED(SHGetFolderPath(NULL, CSIDL_LOCAL_APPDATA, NULL, 0, szPath)))
    {
        CString path(szPath);
        path += _T("\\WoWSimbotQuick\\Logs");
        return path;
    }

    return _T(".\\Logs");
}

CString CLogger::GenerateLogFileName() const
{
    CTime now = CTime::GetCurrentTime();
    CString fileName;
    fileName.Format(_T("WoWSimbotQuick_%s.log"),
        now.Format(_T("%Y%m%d_%H%M%S")));

    CString fullPath = m_logDirectory;
    fullPath += _T("\\");
    fullPath += fileName;

    return fullPath;
}

void CLogger::OpenLogFile()
{
    CSingleLock lock(&m_fileLock, TRUE);

    if (m_pLogFile)
    {
        delete m_pLogFile;
        m_pLogFile = nullptr;
    }

    m_currentLogFile = GenerateLogFileName();

    m_pLogFile = new CStdioFile();
    if (!m_pLogFile->Open(m_currentLogFile,
                          CFile::modeCreate | CFile::modeWrite | CFile::typeText | CFile::shareDenyWrite))
    {
        delete m_pLogFile;
        m_pLogFile = nullptr;
        m_currentLogFile.Empty();
    }
    else
    {
        // BOM 작성 (UTF-8)
        const char bom[] = "\xEF\xBB\xBF";
        m_pLogFile->Write(bom, 3);

        // 헤더 작성
        CString header;
        header.Format(_T("=== WoW Simbot Quick Log ===\r\n"));
        header += _T("Started: ");
        header += CTime::GetCurrentTime().Format(_T("%Y-%m-%d %H:%M:%S"));
        header += _T("\r\n");
        header += _T("Version: 1.0.0\r\n");
        header += _T("============================\r\n\r\n");

        m_pLogFile->WriteString(header);
    }
}

void CLogger::RotateLogFile()
{
    if (!m_pLogFile)
        return;

    CFileStatus status;
    if (!CFile::GetStatus(m_currentLogFile, status))
        return;

    if (status.m_size < (LONG)m_maxLogSize)
        return;

    // 파일 크기 초과, 새 파일 생성
    Info(LogCategory::General, _T("Log file size limit reached, rotating"));

    Flush();

    {
        CSingleLock lock(&m_fileLock, TRUE);
        delete m_pLogFile;
        m_pLogFile = nullptr;
    }

    OpenLogFile();
    CleanupOldLogs();
}

void CLogger::CleanupOldLogs()
{
    CFileFind finder;
    CString pattern = m_logDirectory + _T("\\WoWSimbotQuick_*.log");

    std::vector<std::pair<CTime, CString>> logFiles;

    BOOL bWorking = finder.FindFile(pattern);
    while (bWorking)
    {
        bWorking = finder.FindNextFile();

        if (finder.IsDots() || finder.IsDirectory())
            continue;

        CFileStatus status;
        if (CFile::GetStatus(finder.GetFilePath(), status))
        {
            logFiles.push_back(std::make_pair(status.m_mtime, finder.GetFilePath()));
        }
    }
    finder.Close();

    // 시간 순으로 정렬 (오래된 순)
    std::sort(logFiles.begin(), logFiles.end());

    // 오래된 파일 삭제
    while ((int)logFiles.size() >= m_maxLogFiles)
    {
        DeleteFile(logFiles.front().second);
        logFiles.erase(logFiles.begin());
    }
}

void CLogger::WriteToFile(const CString& text)
{
    if (!m_pLogFile)
        return;

    CSingleLock lock(&m_fileLock, TRUE);

    if (m_pLogFile)
    {
        m_pLogFile->WriteString(text);
        m_pLogFile->WriteString(_T("\r\n"));
    }
}

BOOL CLogger::ShouldLog(LogLevel level, LogCategory category) const
{
    // 전역 레벨 체크
    if (level < m_minLevel)
        return FALSE;

    // 카테고리 레벨 체크
    auto it = m_categoryLevels.find(category);
    if (it != m_categoryLevels.end())
    {
        if (level < it->second)
            return FALSE;
    }

    return TRUE;
}

void CLogger::Log(LogLevel level, LogCategory category, const CString& message,
                  const char* file, int line, const char* func)
{
    if (!ShouldLog(level, category))
        return;

    LogEntry entry;
    entry.timestamp = CTime::GetCurrentTime();
    entry.level = level;
    entry.category = category;
    entry.message = message;

    if (file)
    {
        // 파일명만 추출
        CString fullPath(file);
        int pos = fullPath.ReverseFind('\\');
        if (pos > 0)
            entry.sourceFile = fullPath.Mid(pos + 1);
        else
            entry.sourceFile = fullPath;
    }

    entry.lineNumber = line;

    if (func)
    {
        entry.functionName = func;
    }

    CString logText = entry.ToString();

    // 파일 출력
    if (m_bFileOutput)
    {
        RotateLogFile();
        WriteToFile(logText);
    }

    // 디버그 출력
    if (m_bDebugOutput)
    {
        OutputDebugString(logText + _T("\n"));
    }

    // 콘솔 출력 (디버그 빌드에서만)
    if (m_bConsoleOutput)
    {
        _tprintf(_T("%s\n"), logText);
    }

    // 콜백 호출
    if (m_callback)
    {
        m_callback(entry);
    }

    // 통계 업데이트
    m_totalLogCount++;
    m_levelCounts[level]++;
}

void CLogger::Debug(LogCategory category, const CString& message,
                    const char* file, int line, const char* func)
{
    Log(LogLevel::Debug, category, message, file, line, func);
}

void CLogger::Info(LogCategory category, const CString& message,
                   const char* file, int line, const char* func)
{
    Log(LogLevel::Info, category, message, file, line, func);
}

void CLogger::Warning(LogCategory category, const CString& message,
                      const char* file, int line, const char* func)
{
    Log(LogLevel::Warning, category, message, file, line, func);
}

void CLogger::Error(LogCategory category, const CString& message,
                    const char* file, int line, const char* func)
{
    Log(LogLevel::Error, category, message, file, line, func);
}

void CLogger::Critical(LogCategory category, const CString& message,
                       const char* file, int line, const char* func)
{
    Log(LogLevel::Critical, category, message, file, line, func);
}

void CLogger::SetCategoryLogLevel(LogCategory category, LogLevel level)
{
    m_categoryLevels[category] = level;
}

LogLevel CLogger::GetCategoryLogLevel(LogCategory category) const
{
    auto it = m_categoryLevels.find(category);
    if (it != m_categoryLevels.end())
        return it->second;
    return m_minLevel;
}

void CLogger::Flush()
{
    CSingleLock lock(&m_fileLock, TRUE);
    if (m_pLogFile)
    {
        m_pLogFile->Flush();
    }
}

std::vector<CString> CLogger::GetLogFiles() const
{
    std::vector<CString> files;

    CFileFind finder;
    CString pattern = m_logDirectory + _T("\\WoWSimbotQuick_*.log");

    BOOL bWorking = finder.FindFile(pattern);
    while (bWorking)
    {
        bWorking = finder.FindNextFile();

        if (!finder.IsDots() && !finder.IsDirectory())
        {
            files.push_back(finder.GetFilePath());
        }
    }
    finder.Close();

    return files;
}

BOOL CLogger::ClearAllLogs()
{
    Flush();

    auto files = GetLogFiles();
    for (const auto& file : files)
    {
        DeleteFile(file);
    }

    // 현재 파일 재생성
    if (m_bFileOutput)
    {
        CSingleLock lock(&m_fileLock, TRUE);
        delete m_pLogFile;
        m_pLogFile = nullptr;
        OpenLogFile();
    }

    Info(LogCategory::General, _T("All logs cleared"));

    return TRUE;
}

BOOL CLogger::ArchiveLogs(const CString& archivePath)
{
    // ZIP 파일로 로그 아카이브 생성
    // Windows shell API를 사용한 압축

    auto logFiles = GetLogFiles();
    if (logFiles.empty())
    {
        return FALSE;
    }

    // 임시 폴터 생성
    CString tempDir = m_logDirectory + _T("\\TempArchive");
    CreateDirectory(tempDir, NULL);

    // 파일들을 임시 폴터로 복사
    for (const auto& file : logFiles)
    {
        CString fileName = PathFindFileName(file);
        CString destPath = tempDir + _T("\\") + fileName;
        CopyFile(file, destPath, FALSE);
    }

    // PowerShell을 사용하여 ZIP 생성
    CString psCmd;
    psCmd.Format(_T("powershell -Command \"Compress-Archive -Path '%s\\*' -DestinationPath '%s' -Force\""),
        tempDir, archivePath);

    _tsystem(psCmd);

    // 임시 폴터 정리
    for (const auto& file : logFiles)
    {
        CString fileName = PathFindFileName(file);
        CString destPath = tempDir + _T("\\") + fileName;
        DeleteFile(destPath);
    }
    RemoveDirectory(tempDir);

    Info(LogCategory::General, _T("Logs archived to: ") + archivePath);

    return PathFileExists(archivePath);
}

CString CLogger::GetRecentLogs(int count)
{
    if (!m_pLogFile || m_currentLogFile.IsEmpty())
        return CString();

    Flush();

    CString content;
    CStdioFile file;
    if (file.Open(m_currentLogFile, CFile::modeRead | CFile::typeText | CFile::shareDenyNone))
    {
        CString line;
        std::vector<CString> lines;

        while (file.ReadString(line))
        {
            lines.push_back(line);
        }

        file.Close();

        // 최근 count개만 추출
        int start = (int)lines.size() - count;
        if (start < 0) start = 0;

        for (int i = start; i < (int)lines.size(); i++)
        {
            content += lines[i];
            content += _T("\r\n");
        }
    }

    return content;
}

BOOL CLogger::ExportLogs(const CString& filePath, int maxEntries)
{
    // 로그 낵스포트 구현
    CString ext = PathFindExtension(filePath);
    ext.MakeLower();

    if (ext == _T(".csv"))
    {
        return ExportLogsToCsv(filePath, maxEntries);
    }
    else if (ext == _T(".json"))
    {
        return ExportLogsToJson(filePath, maxEntries);
    }
    else
    {
        // 기본 텍스트 형식
        return ExportLogsToText(filePath, maxEntries);
    }
}

BOOL CLogger::ExportLogsToCsv(const CString& filePath, int maxEntries)
{
    CStdioFile file;
    if (!file.Open(filePath, CFile::modeCreate | CFile::modeWrite | CFile::typeText))
    {
        return FALSE;
    }

    // CSV 헤더
    file.WriteString(_T("Timestamp,Level,Category,Message,Source File,Line,Function\n"));

    // 현재 로그 파일 읽기
    if (!m_currentLogFile.IsEmpty() && PathFileExists(m_currentLogFile))
    {
        CString content = GetRecentLogs(maxEntries);

        // 각 라인을 CSV로 변환
        int pos = 0;
        CString line;
        while (AfxExtractSubString(line, content, pos++, '\n'))
        {
            line.Trim();
            if (line.IsEmpty())
                continue;

            // CSV 형식으로 변환
            CString timestamp, level, category, message, source;

            int p1 = line.Find('[');
            int p2 = line.Find(']', p1);
            if (p1 >= 0 && p2 > p1)
            {
                timestamp = line.Mid(p1 + 1, p2 - p1 - 1);
            }

            p1 = line.Find('[', p2);
            p2 = line.Find(']', p1);
            if (p1 >= 0 && p2 > p1)
            {
                level = line.Mid(p1 + 1, p2 - p1 - 1);
            }

            p1 = line.Find('[', p2);
            p2 = line.Find(']', p1);
            if (p1 >= 0 && p2 > p1)
            {
                category = line.Mid(p1 + 1, p2 - p1 - 1);
            }

            int msgStart = p2 + 2;
            int srcPos = line.Find(_T(" ("), msgStart);
            if (srcPos > 0)
            {
                message = line.Mid(msgStart, srcPos - msgStart);
                source = line.Mid(srcPos + 2, line.GetLength() - srcPos - 3);
            }
            else
            {
                message = line.Mid(msgStart);
            }

            // CSV 특수문자 처리
            message.Replace(_T("\""), _T("\"\""));
            if (message.Find(',') >= 0 || message.Find('"') >= 0 || message.Find('\n') >= 0)
            {
                message = _T("\"") + message + _T("\"");
            }

            CString csvLine;
            csvLine.Format(_T("%s,%s,%s,%s,%s\n"),
                timestamp, level, category, message, source);
            file.WriteString(csvLine);
        }
    }

    file.Close();
    return TRUE;
}

BOOL CLogger::ExportLogsToJson(const CString& filePath, int maxEntries)
{
    CStdioFile file;
    if (!file.Open(filePath, CFile::modeCreate | CFile::modeWrite | CFile::typeText))
    {
        return FALSE;
    }

    file.WriteString(_T("{\n"));
    file.WriteString(_T("  \"logs\": [\n"));

    CString content = GetRecentLogs(maxEntries);

    int pos = 0;
    CString line;
    int count = 0;
    while (AfxExtractSubString(line, content, pos++, '\n'))
    {
        line.Trim();
        if (line.IsEmpty())
            continue;

        if (count > 0)
        {
            file.WriteString(_T(",\n"));
        }

        // JSON 형식으로 변환
        CString timestamp, level, category, message, source;

        int p1 = line.Find('[');
        int p2 = line.Find(']', p1);
        if (p1 >= 0 && p2 > p1)
        {
            timestamp = line.Mid(p1 + 1, p2 - p1 - 1);
        }

        p1 = line.Find('[', p2);
        p2 = line.Find(']', p1);
        if (p1 >= 0 && p2 > p1)
        {
            level = line.Mid(p1 + 1, p2 - p1 - 1);
        }

        p1 = line.Find('[', p2);
        p2 = line.Find(']', p1);
        if (p1 >= 0 && p2 > p1)
        {
            category = line.Mid(p1 + 1, p2 - p1 - 1);
        }

        int msgStart = p2 + 2;
        int srcPos = line.Find(_T(" ("), msgStart);
        if (srcPos > 0)
        {
            message = line.Mid(msgStart, srcPos - msgStart);
            source = line.Mid(srcPos + 2, line.GetLength() - srcPos - 3);
        }
        else
        {
            message = line.Mid(msgStart);
        }

        // JSON 특수문자 이스케이프
        message.Replace(_T("\\"), _T("\\\\"));
        message.Replace(_T("\""), _T("\\\""));
        message.Replace(_T("\n"), _T("\\n"));
        message.Replace(_T("\r"), _T("\\r"));
        message.Replace(_T("\t"), _T("\\t"));

        CString jsonLine;
        jsonLine.Format(_T("    {\n"));
        jsonLine += _T("      \"timestamp\": \"") + timestamp + _T("\",\n");
        jsonLine += _T("      \"level\": \"") + level + _T("\",\n");
        jsonLine += _T("      \"category\": \"") + category + _T("\",\n");
        jsonLine += _T("      \"message\": \"") + message + _T("\"");
        if (!source.IsEmpty())
        {
            jsonLine += _T(",\n      \"source\": \"") + source + _T("\"");
        }
        jsonLine += _T("\n    }");

        file.WriteString(jsonLine);
        count++;
    }

    file.WriteString(_T("\n  ]\n}"));
    file.Close();
    return TRUE;
}

BOOL CLogger::ExportLogsToText(const CString& filePath, int maxEntries)
{
    CString content = GetRecentLogs(maxEntries);

    CStdioFile file;
    if (!file.Open(filePath, CFile::modeCreate | CFile::modeWrite | CFile::typeText))
    {
        return FALSE;
    }

    // BOM 작성
    const char bom[] = "\xEF\xBB\xBF";
    file.Write(bom, 3);

    file.WriteString(content);
    file.Close();
    return TRUE;
}

int CLogger::GetLogCountByLevel(LogLevel level) const
{
    auto it = m_levelCounts.find(level);
    if (it != m_levelCounts.end())
        return it->second;
    return 0;
}

CString CLogger::LevelToString(LogLevel level)
{
    switch (level)
    {
    case LogLevel::Debug:    return _T("DEBUG");
    case LogLevel::Info:     return _T("INFO");
    case LogLevel::Warning:  return _T("WARN");
    case LogLevel::Error:    return _T("ERROR");
    case LogLevel::Critical: return _T("CRITICAL");
    default:                 return _T("UNKNOWN");
    }
}

CString CLogger::CategoryToString(LogCategory category)
{
    switch (category)
    {
    case LogCategory::General:     return _T("GENERAL");
    case LogCategory::Api:         return _T("API");
    case LogCategory::Simc:        return _T("SIMC");
    case LogCategory::Simulation:  return _T("SIMULATION");
    case LogCategory::Ui:          return _T("UI");
    case LogCategory::Settings:    return _T("SETTINGS");
    case LogCategory::Network:     return _T("NETWORK");
    default:                       return _T("UNKNOWN");
    }
}

// ScopeTimer 구현
CLogger::ScopeTimer::ScopeTimer(CLogger* logger, LogCategory category, const CString& operation)
    : m_logger(logger)
    , m_category(category)
    , m_operation(operation)
    , m_start(std::chrono::high_resolution_clock::now())
{
}

CLogger::ScopeTimer::~ScopeTimer()
{
    if (!m_logger)
        return;

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - m_start);

    CString msg;
    msg.Format(_T("%s completed in %lld ms"), m_operation, duration.count());
    m_logger->Debug(m_category, msg);
}
