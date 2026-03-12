#pragma once
#include <atomic>
#include <mutex>

// Simc runner class - manages simc.exe process execution
class CSimcRunner
{
public:
    using ProgressCallback = std::function<void(int)>;

    CSimcRunner();
    virtual ~CSimcRunner();

    // Run simulation with given profile
    // Returns TRUE if simulation completed successfully
    BOOL RunSimulation(const CString& simcPath,
                       const CString& profile,
                       const CString& outputFile,
                       ProgressCallback progressCallback = nullptr);

    // Cancel running simulation
    void Cancel();

    // Check if simulation is running
    BOOL IsRunning() const { return m_bRunning.load() ? TRUE : FALSE; }

    // Get last error message
    CString GetLastError() const { return m_strLastError; }

    // Get simc version
    CString GetSimcVersion(const CString& simcPath);

private:
    // Create temporary profile file
    CString CreateProfileFile(const CString& profile);

    // Parse progress from simc output
    int ParseProgress(const CString& output);

    // Read process output
    BOOL ReadProcessOutput(HANDLE hStdoutRead, CString& output);

private:
    HANDLE  m_hProcess;
    HANDLE  m_hThread;
    std::atomic_bool m_bRunning;
    std::atomic_bool m_bCancelRequested;
    mutable std::mutex m_processMutex;
    CString m_strLastError;

    // Constants
    static constexpr DWORD PROCESS_TIMEOUT_MS = 600000;  // 10 minutes max
};
