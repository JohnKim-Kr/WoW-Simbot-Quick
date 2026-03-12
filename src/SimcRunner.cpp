#include "pch.h"
#include "framework.h"
#include "SimcRunner.h"
#include <filesystem>
namespace fs = std::filesystem;

CSimcRunner::CSimcRunner()
    : m_hProcess(nullptr)
    , m_hThread(nullptr)
    , m_bRunning(FALSE)
    , m_bCancelRequested(FALSE)
{
}

CSimcRunner::~CSimcRunner()
{
    Cancel();
}

BOOL CSimcRunner::RunSimulation(const CString& simcPath,
                                 const CString& profile,
                                 const CString& outputFile,
                                 ProgressCallback progressCallback)
{
    if (m_bRunning)
    {
        m_strLastError = _T("Simulation already running");
        return FALSE;
    }

    // Verify simc.exe exists
    if (!fs::exists(std::filesystem::path(std::wstring(simcPath))))
    {
        m_strLastError.Format(_T("simc.exe not found: %s"), simcPath);
        return FALSE;
    }

    // Create temporary profile file
    CString profileFile = CreateProfileFile(profile);
    if (profileFile.IsEmpty())
    {
        m_strLastError = _T("Failed to create profile file");
        return FALSE;
    }

    m_bRunning = TRUE;
    m_bCancelRequested = FALSE;

    // Build command line
    CString cmdLine;
    cmdLine.Format(_T("\"%s\" \"%s\""),
        (LPCTSTR)simcPath, (LPCTSTR)profileFile);

    // Set up security attributes for pipe
    SECURITY_ATTRIBUTES sa;
    sa.nLength = sizeof(SECURITY_ATTRIBUTES);
    sa.bInheritHandle = TRUE;
    sa.lpSecurityDescriptor = NULL;

    // Create stdout pipe
    HANDLE hStdoutRead = NULL;
    HANDLE hStdoutWrite = NULL;
    if (!CreatePipe(&hStdoutRead, &hStdoutWrite, &sa, 0))
    {
        m_strLastError = _T("Failed to create pipe");
        DeleteFile(profileFile);
        m_bRunning = FALSE;
        return FALSE;
    }

    // Ensure read handle is not inherited
    SetHandleInformation(hStdoutRead, HANDLE_FLAG_INHERIT, 0);

    // Set up process startup info
    STARTUPINFO si;
    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    si.hStdError = hStdoutWrite;
    si.hStdOutput = hStdoutWrite;
    si.dwFlags |= STARTF_USESTDHANDLES;

    PROCESS_INFORMATION pi;
    ZeroMemory(&pi, sizeof(pi));

    // Create process
    BOOL success = CreateProcess(
        NULL,                   // Application name (NULL = use command line)
        cmdLine.GetBuffer(),    // Command line
        NULL,                   // Process security attributes
        NULL,                   // Thread security attributes
        TRUE,                   // Inherit handles
        CREATE_NO_WINDOW,       // Creation flags (no console window)
        NULL,                   // Environment
        NULL,                   // Current directory
        &si,                    // Startup info
        &pi);                   // Process information

    cmdLine.ReleaseBuffer();
    CloseHandle(hStdoutWrite);

    if (!success)
    {
        m_strLastError.Format(_T("Failed to start simc process: %d"), GetLastError());
        CloseHandle(hStdoutRead);
        DeleteFile(profileFile);
        m_bRunning = FALSE;
        return FALSE;
    }

    m_hProcess = pi.hProcess;
    m_hThread = pi.hThread;

    // Immediately notify simulation has started (0%)
    if (progressCallback)
    {
        progressCallback(0);
    }

    // Read output and update progress
    CString output;
    int lastProgress = 0;

    while (!m_bCancelRequested)
    {
        DWORD exitCode = 0;
        GetExitCodeProcess(m_hProcess, &exitCode);

        // Read available output
        CString newOutput;
        if (ReadProcessOutput(hStdoutRead, newOutput))
        {
            output += newOutput;

            // Parse progress
            int progress = ParseProgress(output);
            if (progress > lastProgress)
            {
                lastProgress = progress;
                if (progressCallback)
                {
                    progressCallback(progress);
                }
            }
        }

        if (exitCode != STILL_ACTIVE)
        {
            break;
        }

        // Small delay to prevent busy-waiting
        Sleep(100);
    }

    // Clean up handles
    CloseHandle(hStdoutRead);

    // Check if cancelled
    if (m_bCancelRequested)
    {
        TerminateProcess(m_hProcess, 1);
        m_strLastError = _T("Simulation cancelled");
    }
    else
    {
        DWORD exitCode = 0;
        GetExitCodeProcess(m_hProcess, &exitCode);

        if (exitCode != 0)
        {
            m_strLastError.Format(_T("simc exited with code %d\nOutput: %s"), exitCode, output);
        }
    }

    // Clean up process handles
    CloseHandle(m_hProcess);
    CloseHandle(m_hThread);
    m_hProcess = nullptr;
    m_hThread = nullptr;

    // DEBUG: Don't delete temporary profile file for debugging
    // DeleteFile(profileFile);
    TRACE(_T("Profile file for debugging: %s\n"), profileFile);

    m_bRunning = FALSE;

    return !m_bCancelRequested && m_strLastError.IsEmpty();
}

void CSimcRunner::Cancel()
{
    m_bCancelRequested = TRUE;

    if (m_hProcess)
    {
        // Give it a moment to terminate gracefully
        Sleep(500);

        DWORD exitCode = 0;
        GetExitCodeProcess(m_hProcess, &exitCode);

        if (exitCode == STILL_ACTIVE)
        {
            TerminateProcess(m_hProcess, 1);
        }
    }
}

CString CSimcRunner::GetSimcVersion(const CString& simcPath)
{
    CString version;

    // Run simc.exe --version
    CString cmdLine;
    cmdLine.Format(_T("\"%s\" --version"), (LPCTSTR)simcPath);

    SECURITY_ATTRIBUTES sa;
    sa.nLength = sizeof(SECURITY_ATTRIBUTES);
    sa.bInheritHandle = TRUE;
    sa.lpSecurityDescriptor = NULL;

    HANDLE hStdoutRead = NULL;
    HANDLE hStdoutWrite = NULL;

    if (!CreatePipe(&hStdoutRead, &hStdoutWrite, &sa, 0))
    {
        return version;
    }

    SetHandleInformation(hStdoutRead, HANDLE_FLAG_INHERIT, 0);

    STARTUPINFO si;
    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    si.hStdError = hStdoutWrite;
    si.hStdOutput = hStdoutWrite;
    si.dwFlags |= STARTF_USESTDHANDLES;

    PROCESS_INFORMATION pi;
    ZeroMemory(&pi, sizeof(pi));

    if (CreateProcess(NULL, cmdLine.GetBuffer(), NULL, NULL, TRUE,
                      CREATE_NO_WINDOW, NULL, NULL, &si, &pi))
    {
        cmdLine.ReleaseBuffer();
        CloseHandle(hStdoutWrite);

        // Read output
        char buffer[4096];
        DWORD bytesRead = 0;
        std::string output;

        while (ReadFile(hStdoutRead, buffer, sizeof(buffer) - 1, &bytesRead, NULL) && bytesRead > 0)
        {
            buffer[bytesRead] = '\0';
            output += buffer;
        }

        CloseHandle(hStdoutRead);
        WaitForSingleObject(pi.hProcess, 5000);
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);

        // Parse version from output (format: "simc version X.Y.Z")
        version = CA2T(output.c_str());
        version.Trim();
    }
    else
    {
        CloseHandle(hStdoutRead);
        CloseHandle(hStdoutWrite);
    }

    return version;
}

CString CSimcRunner::CreateProfileFile(const CString& profile)
{
    // Create temporary file in system temp directory
    TCHAR tempPath[MAX_PATH];
    GetTempPath(MAX_PATH, tempPath);

    TCHAR tempFile[MAX_PATH];
    GetTempFileName(tempPath, _T("SIM"), 0, tempFile);

    // Change extension to .simc
    CString profileFile = tempFile;
    profileFile.Replace(_T(".tmp"), _T(".simc"));

    // Write profile to file (binary mode for UTF-8 compatibility)
    CFile file;
    if (file.Open(profileFile, CFile::modeCreate | CFile::modeWrite))
    {
        // Convert CString to UTF-8
        CT2A utf8Profile(profile, CP_UTF8);
        file.Write(utf8Profile, strlen(utf8Profile));
        file.Close();
        TRACE(_T("Profile file created: %s (%d bytes)\n"), profileFile, strlen(utf8Profile));
        return profileFile;
    }
    TRACE(_T("Failed to create profile file: %s\n"), profileFile);

    return CString();
}

int CSimcRunner::ParseProgress(const CString& output)
{
    // Look for progress indicators in simc output
    // Common formats: "Generating baseline...", "X%", etc.

    // Try to find percentage
    std::regex pctRegex(R"((\d+)%)");
    std::string str = std::string(CT2A(output));

    std::sregex_iterator iter(str.begin(), str.end(), pctRegex);
    std::sregex_iterator end;

    int maxProgress = 0;
    for (; iter != end; ++iter)
    {
        int pct = std::stoi((*iter)[1].str());
        if (pct > maxProgress)
            maxProgress = pct;
    }

    if (maxProgress > 0)
        return maxProgress;

    // Check for specific phases
    if (str.find("Generating baseline") != std::string::npos)
        return 10;
    if (str.find("Analyzing") != std::string::npos)
        return 50;
    if (str.find("Generating reports") != std::string::npos)
        return 90;
    if (str.find("Done") != std::string::npos)
        return 100;

    return 0;
}

BOOL CSimcRunner::ReadProcessOutput(HANDLE hStdoutRead, CString& output)
{
    DWORD bytesAvailable = 0;
    if (!PeekNamedPipe(hStdoutRead, NULL, 0, NULL, &bytesAvailable, NULL))
    {
        return FALSE;
    }

    if (bytesAvailable == 0)
    {
        return FALSE;
    }

    std::vector<char> buffer(bytesAvailable + 1);
    DWORD bytesRead = 0;

    if (ReadFile(hStdoutRead, buffer.data(), bytesAvailable, &bytesRead, NULL))
    {
        buffer[bytesRead] = '\0';
        output = CA2T(buffer.data());
        return TRUE;
    }

    return FALSE;
}
