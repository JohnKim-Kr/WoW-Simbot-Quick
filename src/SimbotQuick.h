// SimbotQuick.h : main header file for the WoWSimbotQuick application
#pragma once

#ifndef __AFXWIN_H__
    #error "include 'pch.h' before including this file for PCH"
#endif

#include "resource.h"       // main symbols

// Application version
#define APP_VERSION_MAJOR   1
#define APP_VERSION_MINOR   0
#define APP_VERSION_PATCH   0

// Registry keys
#define REG_KEY_COMPANY     _T("WoWSimbot")
#define REG_KEY_APP         _T("WoWSimbotQuick")
#define REG_KEY_SETTINGS    _T("Settings")
#define REG_KEY_OAUTH       _T("OAuth")

// Forward declarations
class CMainFrame;
class CBnetApiClient;
class CSimcRunner;

// CWoWSimbotQuickApp:
// See SimbotQuick.cpp for the implementation of this class
//
class CWoWSimbotQuickApp : public CWinApp
{
public:
    CWoWSimbotQuickApp() noexcept;

    // Overrides
public:
    virtual BOOL InitInstance();
    virtual int ExitInstance();

    // Implementation
    afx_msg void OnAppAbout();
    afx_msg void OnFileSettings();
    afx_msg void OnToolsRunSim();
    afx_msg void OnToolsStopSim();
    afx_msg void OnUpdateToolsStopSim(CCmdUI* pCmdUI);
    afx_msg void OnPresetsManage();
    afx_msg void OnShowHistory();
    afx_msg void OnDebugConsole();
    afx_msg void OnDebugLogViewer();
    afx_msg void OnDebugDiagnostics();

    DECLARE_MESSAGE_MAP()

    // Application-wide accessors
    CString GetSimcPath() const { return m_strSimcPath; }
    void SetSimcPath(const CString& path) { m_strSimcPath = path; }

    CString GetBnetClientId() const { return m_strClientId; }
    void SetBnetClientId(const CString& id) { m_strClientId = id; }

    CString GetBnetClientSecret() const { return m_strClientSecret; }
    void SetBnetClientSecret(const CString& secret) { m_strClientSecret = secret; }

    // Settings persistence
    void LoadSettings();
    void SaveSettings() const;

    // Simulation state
    BOOL IsSimRunning() const { return m_bSimRunning; }
    void SetSimRunning(BOOL running) { m_bSimRunning = running; }

private:
    CString m_strSimcPath;
    CString m_strClientId;
    CString m_strClientSecret;
    BOOL    m_bSimRunning;

    HCURSOR m_hAccelTable;
};

extern CWoWSimbotQuickApp theApp;
