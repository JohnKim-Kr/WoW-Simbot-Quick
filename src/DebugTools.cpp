// Suppress deprecated API warnings for networking
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include "pch.h"
#include "framework.h"

#include "DebugTools.h"
#include "Logger.h"
#include "SimbotQuick.h"

#include <psapi.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>

#pragma comment(lib, "psapi.lib")
#pragma comment(lib, "ws2_32.lib")

CDebugTools* CDebugTools::s_instance = nullptr;

CDebugTools* CDebugTools::GetInstance()
{
    if (!s_instance)
    {
        s_instance = new CDebugTools();
    }
    return s_instance;
}

void CDebugTools::DestroyInstance()
{
    if (s_instance)
    {
        delete s_instance;
        s_instance = nullptr;
    }
}

CDebugTools::CDebugTools()
    : m_bApiTracing(FALSE)
    , m_bConsoleVisible(FALSE)
    , m_hConsoleOutput(INVALID_HANDLE_VALUE)
{
}

CDebugTools::~CDebugTools()
{
    if (m_bConsoleVisible)
    {
        HideDebugConsole();
    }
}

CString CDebugTools::GetMemoryInfo() const
{
    CString info;

    MEMORYSTATUSEX memStatus;
    memStatus.dwLength = sizeof(memStatus);
    GlobalMemoryStatusEx(&memStatus);

    info += _T("=== 시스템 메모리 ===\r\n");
    info.AppendFormat(_T("전체 물리 메모리: %.1f GB\r\n"),
        memStatus.ullTotalPhys / (1024.0 * 1024.0 * 1024.0));
    info.AppendFormat(_T("사용 가능 물리 메모리: %.1f GB\r\n"),
        memStatus.ullAvailPhys / (1024.0 * 1024.0 * 1024.0));
    info.AppendFormat(_T("메모리 사용률: %lu%%\r\n\r\n"),
        memStatus.dwMemoryLoad);

    // 프로세스 메모리
    HANDLE hProcess = GetCurrentProcess();
    PROCESS_MEMORY_COUNTERS pmc;
    if (GetProcessMemoryInfo(hProcess, &pmc, sizeof(pmc)))
    {
        info += _T("=== 프로세스 메모리 ===\r\n");
        info.AppendFormat(_T("작업 집합: %.2f MB\r\n"),
            pmc.WorkingSetSize / (1024.0 * 1024.0));
        info.AppendFormat(_T("최대 작업 집합: %.2f MB\r\n"),
            pmc.PeakWorkingSetSize / (1024.0 * 1024.0));
        info.AppendFormat(_T("페이지 파일 사용: %.2f MB\r\n"),
            pmc.PagefileUsage / (1024.0 * 1024.0));
    }

    return info;
}

void CDebugTools::LogMemoryInfo()
{
    CString info = GetMemoryInfo();
    LOG_INFO(LogCategory::General, info);
}

CString CDebugTools::GetGdiInfo() const
{
    CString info;
    info.Format(_T("GDI 객체: %d\r\n사용자 객체: %d"),
        GetGuiResources(GetCurrentProcess(), GR_GDIOBJECTS),
        GetGuiResources(GetCurrentProcess(), GR_USEROBJECTS));
    return info;
}

void CDebugTools::LogGdiInfo()
{
    LOG_INFO(LogCategory::Ui, GetGdiInfo());
}

int CDebugTools::GetHandleCount() const
{
    DWORD handleCount = 0;
    GetProcessHandleCount(GetCurrentProcess(), &handleCount);
    return static_cast<int>(handleCount);
}

void CDebugTools::StartPerformanceCounter(const CString& name)
{
    CSingleLock lock(&m_perfLock, TRUE);

    auto& data = m_performanceCounters[name];
    data.start = std::chrono::high_resolution_clock::now();
}

void CDebugTools::StopPerformanceCounter(const CString& name)
{
    CSingleLock lock(&m_perfLock, TRUE);

    auto it = m_performanceCounters.find(name);
    if (it == m_performanceCounters.end())
        return;

    auto end = std::chrono::high_resolution_clock::now();
    it->second.total += end - it->second.start;
    it->second.count++;
}

CString CDebugTools::GetPerformanceReport() const
{
    CSingleLock lock(&m_perfLock, TRUE);

    CString report = _T("=== 성능 보고서 ===\r\n\r\n");

    for (const auto& pair : m_performanceCounters)
    {
        const auto& name = pair.first;
        const auto& data = pair.second;

        double avgMs = 0;
        if (data.count > 0)
        {
            avgMs = std::chrono::duration_cast<std::chrono::microseconds>(data.total).count() / 1000.0 / data.count;
        }

        CString line;
        line.Format(_T("%s: %.2f ms (호출 %d회)\r\n"), name, avgMs, data.count);
        report += line;
    }

    return report;
}

void CDebugTools::EnableApiTracing(BOOL enable)
{
    m_bApiTracing = enable;
    LOG_INFO(LogCategory::General, enable ? _T("API 추적 활성화") : _T("API 추적 비활성화"));
}

BOOL CDebugTools::TestSimcInstallation(const CString& simcPath, CString& outVersion, CString& outError)
{
    if (simcPath.IsEmpty())
    {
        outError = _T("simc 경로가 설정되지 않았습니다.");
        return FALSE;
    }

    if (!PathFileExists(simcPath))
    {
        outError.Format(_T("simc.exe를 찾을 수 없습니다: %s"), simcPath);
        return FALSE;
    }

    // 버전 확인 실행
    CString cmdLine;
    cmdLine.Format(_T("\"%%s\" --version"), simcPath);

    SECURITY_ATTRIBUTES sa;
    sa.nLength = sizeof(sa);
    sa.bInheritHandle = TRUE;
    sa.lpSecurityDescriptor = NULL;

    HANDLE hRead = NULL, hWrite = NULL;
    if (!CreatePipe(&hRead, &hWrite, &sa, 0))
    {
        outError = _T("파이프 생성 실패");
        return FALSE;
    }

    SetHandleInformation(hRead, HANDLE_FLAG_INHERIT, 0);

    STARTUPINFO si;
    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    si.hStdError = hWrite;
    si.hStdOutput = hWrite;
    si.dwFlags = STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW;
    si.wShowWindow = SW_HIDE;

    PROCESS_INFORMATION pi;
    ZeroMemory(&pi, sizeof(pi));

    if (!CreateProcess(NULL, cmdLine.GetBuffer(), NULL, NULL, TRUE,
                       CREATE_NO_WINDOW, NULL, NULL, &si, &pi))
    {
        cmdLine.ReleaseBuffer();
        CloseHandle(hRead);
        CloseHandle(hWrite);
        outError.Format(_T("프로세스 생성 실패: %d"), GetLastError());
        return FALSE;
    }

    cmdLine.ReleaseBuffer();
    CloseHandle(hWrite);

    // 출력 읽기
    char buffer[4096];
    DWORD bytesRead = 0;
    std::string output;

    while (ReadFile(hRead, buffer, sizeof(buffer) - 1, &bytesRead, NULL) && bytesRead > 0)
    {
        buffer[bytesRead] = '\0';
        output += buffer;
    }

    CloseHandle(hRead);
    WaitForSingleObject(pi.hProcess, 5000);
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);

    outVersion = CA2T(output.c_str());
    outVersion.Trim();

    if (outVersion.IsEmpty())
    {
        outError = _T("버전 정보를 가져올 수 없습니다.");
        return FALSE;
    }

    return TRUE;
}

BOOL CDebugTools::TestNetworkConnection(const CString& host, int port, int timeoutMs, CString& outError)
{
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
    {
        outError = _T("Winsock 초기화 실패");
        return FALSE;
    }

    SOCKET sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sock == INVALID_SOCKET)
    {
        outError = _T("소켓 생성 실패");
        WSACleanup();
        return FALSE;
    }

    // 타임아웃 설정
    DWORD timeout = timeoutMs;
    setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (const char*)&timeout, sizeof(timeout));
    setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, (const char*)&timeout, sizeof(timeout));

    // 주소 변환
    USES_CONVERSION;
    const char* hostA = T2A(host);

    hostent* he = gethostbyname(hostA);
    if (!he)
    {
        outError.Format(_T("호스트 이름 해석 실패: %s"), host);
        closesocket(sock);
        WSACleanup();
        return FALSE;
    }

    sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons((u_short)port);
    addr.sin_addr.s_addr = *(u_long*)he->h_addr;

    // 연결 시도
    if (connect(sock, (sockaddr*)&addr, sizeof(addr)) == SOCKET_ERROR)
    {
        outError.Format(_T("연결 실패: %d"), WSAGetLastError());
        closesocket(sock);
        WSACleanup();
        return FALSE;
    }

    closesocket(sock);
    WSACleanup();

    return TRUE;
}

BOOL CDebugTools::ValidateSettings(CString& outReport) const
{
    BOOL allValid = TRUE;
    outReport = _T("=== 설정 유효성 검사 ===\r\n\r\n");

    CWoWSimbotQuickApp* pApp = static_cast<CWoWSimbotQuickApp*>(AfxGetApp());
    if (!pApp)
    {
        outReport += _T("[오류] 애플리케이션 인스턴스를 찾을 수 없습니다.\r\n");
        return FALSE;
    }

    // simc 경로 검사
    CString simcPath = pApp->m_strSimcPath;
    if (simcPath.IsEmpty())
    {
        outReport += _T("[경고] simc.exe 경로가 설정되지 않았습니다.\r\n");
        allValid = FALSE;
    }
    else if (!PathFileExists(simcPath))
    {
        outReport += _T("[오류] 설정된 simc.exe 경로가 존재하지 않습니다.\r\n");
        allValid = FALSE;
    }
    else
    {
        outReport += _T("[정상] simc.exe 경로\r\n");
    }

    outReport += _T("\r\n");
    outReport += allValid ? _T("모든 필수 설정이 유효합니다.") : _T("일부 설정에 문제가 있습니다.");

    return allValid;
}

CString CDebugTools::GetSystemInfo() const
{
    CString info = _T("=== 시스템 정보 ===\r\n\r\n");

    // Windows 버전
    OSVERSIONINFOEX osvi;
    ZeroMemory(&osvi, sizeof(OSVERSIONINFOEX));
    osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);

    typedef LONG(WINAPI* RtlGetVersionPtr)(PRTL_OSVERSIONINFOW);
    HMODULE hNtdll = GetModuleHandle(_T("ntdll.dll"));
    if (hNtdll)
    {
        RtlGetVersionPtr RtlGetVersion = (RtlGetVersionPtr)GetProcAddress(hNtdll, "RtlGetVersion");
        if (RtlGetVersion)
        {
            RtlGetVersion((PRTL_OSVERSIONINFOW)&osvi);
        }
    }

    info.AppendFormat(_T("Windows 버전: %lu.%lu (빌드 %lu)\r\n"),
        osvi.dwMajorVersion, osvi.dwMinorVersion, osvi.dwBuildNumber);

    // 프로세서 정보
    SYSTEM_INFO sysInfo;
    ::GetSystemInfo(&sysInfo);
    info.AppendFormat(_T("프로세서 수: %lu\r\n"), sysInfo.dwNumberOfProcessors);

    // 디스플레이 정보
    HDC hdc = GetDC(NULL);
    info.AppendFormat(_T("화면 해상도: %dx%d\r\n"),
        GetDeviceCaps(hdc, HORZRES), GetDeviceCaps(hdc, VERTRES));
    ReleaseDC(NULL, hdc);

    return info;
}

CString CDebugTools::GenerateDiagnosticReport() const
{
    CString report;

    report += GetSystemInfo();
    report += _T("\r\n");
    report += GetMemoryInfo();
    report += _T("\r\n");

    CString settingsReport;
    ValidateSettings(settingsReport);
    report += settingsReport;
    report += _T("\r\n");

    report += GetPerformanceReport();
    report += _T("\r\n");

    report += GetGdiInfo();

    return report;
}

BOOL CDebugTools::SaveDiagnosticReport(const CString& filePath) const
{
    CString report = GenerateDiagnosticReport();

    CStdioFile file;
    if (!file.Open(filePath, CFile::modeCreate | CFile::modeWrite | CFile::typeText))
        return FALSE;

    file.WriteString(report);
    file.Close();

    return TRUE;
}

void CDebugTools::ShowDebugConsole()
{
    if (m_bConsoleVisible)
        return;

    AllocConsole();

    // 표준 입출력 리다이렉션
    FILE* fp;
    freopen_s(&fp, "CONOUT$", "w", stdout);
    freopen_s(&fp, "CONIN$", "r", stdin);
    freopen_s(&fp, "CONOUT$", "w", stderr);

    // 콘솔 제목 설정
    SetConsoleTitle(_T("WoW Simbot Quick - Debug Console"));

    m_hConsoleOutput = GetStdHandle(STD_OUTPUT_HANDLE);
    m_bConsoleVisible = TRUE;

    WriteToDebugConsole(_T("Debug Console Opened\r\n"));
    WriteToDebugConsole(GetSystemInfo());
}

void CDebugTools::HideDebugConsole()
{
    if (!m_bConsoleVisible)
        return;

    FreeConsole();
    m_bConsoleVisible = FALSE;
    m_hConsoleOutput = INVALID_HANDLE_VALUE;
}

BOOL CDebugTools::IsDebugConsoleVisible() const
{
    return m_bConsoleVisible;
}

void CDebugTools::WriteToDebugConsole(const CString& text)
{
    if (!m_bConsoleVisible || m_hConsoleOutput == INVALID_HANDLE_VALUE)
        return;

    DWORD written;
    CT2A textA(text, CP_UTF8);
    WriteConsoleA(m_hConsoleOutput, textA, strlen(textA), &written, NULL);
}
