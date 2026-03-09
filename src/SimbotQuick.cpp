// SimbotQuick.cpp : Defines the class behaviors for the application.
//

#include "pch.h"
#include "framework.h"
#include "SimbotQuick.h"
#include "MainFrame.h"
#include "SimOptionsDlg.h"
#include "PresetManagerDlg.h"
#include "ResultHistoryDlg.h"
#include "LogViewerDlg.h"
#include "SettingsManager.h"
#include "ResultHistoryManager.h"
#include "Logger.h"
#include "DebugTools.h"

// 외부 선언 (소멸용)
extern void DestroySettingsManager();
extern void DestroyResultHistoryManager();
extern void DestroyLogger();

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CWoWSimbotQuickApp

BEGIN_MESSAGE_MAP(CWoWSimbotQuickApp, CWinApp)
    ON_COMMAND(ID_APP_ABOUT, &CWoWSimbotQuickApp::OnAppAbout)
    ON_COMMAND(ID_TOOLS_SETTINGS, &CWoWSimbotQuickApp::OnFileSettings)
    ON_COMMAND(ID_PRESETS_MANAGE, &CWoWSimbotQuickApp::OnPresetsManage)
    ON_COMMAND(ID_HISTORY_SHOW, &CWoWSimbotQuickApp::OnShowHistory)
    ON_COMMAND(ID_BUTTON_RUN_SIM, &CWoWSimbotQuickApp::OnToolsRunSim)
    ON_COMMAND(ID_BUTTON_STOP_SIM, &CWoWSimbotQuickApp::OnToolsStopSim)
    ON_UPDATE_COMMAND_UI(ID_BUTTON_STOP_SIM, &CWoWSimbotQuickApp::OnUpdateToolsStopSim)
    ON_COMMAND(ID_DEBUG_CONSOLE, &CWoWSimbotQuickApp::OnDebugConsole)
    ON_COMMAND(ID_DEBUG_LOG_VIEWER, &CWoWSimbotQuickApp::OnDebugLogViewer)
    ON_COMMAND(ID_DEBUG_DIAGNOSTICS, &CWoWSimbotQuickApp::OnDebugDiagnostics)
END_MESSAGE_MAP()

// CWoWSimbotQuickApp construction

CWoWSimbotQuickApp::CWoWSimbotQuickApp() noexcept
    : m_bSimRunning(FALSE)
{
    // support Restart Manager
    m_dwRestartManagerSupportFlags = AFX_RESTART_MANAGER_SUPPORT_ALL_ASPECTS;

    // TODO: replace application ID string below with unique ID string; recommended
    // format for string is CompanyName.ProductName.SubProduct.VersionInformation
    SetAppID(_T("WoWSimbot.WoWSimbotQuick.1.0"));

    // TODO: add construction code here,
    // Place all significant initialization in InitInstance
}

// The one and only CWoWSimbotQuickApp object

CWoWSimbotQuickApp theApp;

// CWoWSimbotQuickApp initialization

BOOL CWoWSimbotQuickApp::InitInstance()
{
    // InitCommonControlsEx() is required on Windows XP if an application
    // manifest specifies use of ComCtl32.dll version 6 or later to enable
    // visual styles.  Otherwise, any window creation will fail.
    INITCOMMONCONTROLSEX InitCtrls;
    InitCtrls.dwSize = sizeof(InitCtrls);
    // Set this to include all the common control classes you want to use
    // in your application.
    InitCtrls.dwICC = ICC_WIN95_CLASSES;
    InitCommonControlsEx(&InitCtrls);

    CWinApp::InitInstance();

    // Initialize OLE libraries
    if (!AfxOleInit())
    {
        AfxMessageBox(_T("OLE initialization failed. Make sure that the OLE libraries are the correct version."));
        return FALSE;
    }

    AfxEnableControlContainer();

    EnableTaskbarInteraction(FALSE);

    // AfxInitRichEdit2() is required to use RichEdit control
    AfxInitRichEdit2();

    // Initialize logger first (for startup logging)
    GetLogger();
    LOG_INFO(LogCategory::General, _T("Application starting..."));

    // Initialize other managers
    GetSettingsManager();
    GetResultHistoryManager();
    GetDebugTools();

    // Load settings from registry
    LoadSettings();

    // Standard initialization
    // If you are not using these features and wish to reduce the size
    // of your final executable, you should remove from the following
    // the specific initialization routines you do not need
    // Change the registry key under which our settings are stored
    // TODO: You should modify this string to be something appropriate
    // such as the name of your company or organization
    SetRegistryKey(_T("WoWSimbot"));

    // To create the main window, this code creates a new frame window
    // object and then sets it as the application's main window object

    CFrameWnd* pFrame = new CMainFrame;
    if (!pFrame)
        return FALSE;
    m_pMainWnd = pFrame;
    // create and load the frame with its resources

    pFrame->LoadFrame(IDR_MAINFRAME,
        WS_OVERLAPPEDWINDOW | FWS_ADDTOTITLE, nullptr,
        nullptr);

    // The one and only window has been initialized, so show and update it
    pFrame->ShowWindow(SW_SHOW);
    pFrame->UpdateWindow();

    m_hAccelTable = LoadAccelerators(IDR_MAINFRAME);

    return TRUE;
}

int CWoWSimbotQuickApp::ExitInstance()
{
    LOG_INFO(LogCategory::General, _T("Application shutting down..."));

    // Save settings before exiting
    SaveSettings();

    // Cleanup managers
    DestroySettingsManager();
    DestroyResultHistoryManager();
    DestroyDebugTools();
    DestroyLogger();

    //TODO: handle additional resources you may have added
    AfxOleTerm(FALSE);

    return CWinApp::ExitInstance();
}

// App command to run the dialog
void CWoWSimbotQuickApp::OnAppAbout()
{
    CString strAbout;
    strAbout.Format(_T("WoW Simbot Quick v%d.%d.%d\n\n")
        _T("A Windows native application for World of Warcraft DPS/HPS simulation.\n\n")
        _T("Powered by SimulationCraft (simc)\n")
        _T("Battle.net API integration for character data retrieval.\n\n")
        _T("2026 WoWSimbot Team"),
        APP_VERSION_MAJOR, APP_VERSION_MINOR, APP_VERSION_PATCH);

    AfxMessageBox(strAbout, MB_ICONINFORMATION | MB_OK);
}

void CWoWSimbotQuickApp::OnFileSettings()
{
    CSimOptionsDlg dlg;
    dlg.DoModal();
}

void CWoWSimbotQuickApp::OnToolsRunSim()
{
    CMainFrame* pFrame = static_cast<CMainFrame*>(m_pMainWnd);
    if (pFrame)
    {
        pFrame->StartSimulation();
    }
}

void CWoWSimbotQuickApp::OnToolsStopSim()
{
    CMainFrame* pFrame = static_cast<CMainFrame*>(m_pMainWnd);
    if (pFrame)
    {
        pFrame->StopSimulation();
    }
}

void CWoWSimbotQuickApp::OnUpdateToolsStopSim(CCmdUI* pCmdUI)
{
    pCmdUI->Enable(m_bSimRunning);
}

void CWoWSimbotQuickApp::LoadSettings()
{
    CWinApp* pApp = AfxGetApp();

    m_strSimcPath = pApp->GetProfileString(REG_KEY_SETTINGS, _T("SimcPath"), _T(""));
    m_strClientId = pApp->GetProfileString(REG_KEY_SETTINGS, _T("ClientId"), _T(""));

    // Client secret is encrypted (simple obfuscation for now)
    m_strClientSecret = pApp->GetProfileString(REG_KEY_SETTINGS, _T("ClientSecret"), _T(""));
}

void CWoWSimbotQuickApp::OnDebugConsole()
{
    CDebugTools* pDebug = GetDebugTools();
    if (pDebug)
    {
        if (pDebug->IsDebugConsoleVisible())
        {
            pDebug->HideDebugConsole();
        }
        else
        {
            pDebug->ShowDebugConsole();
        }
    }
}

void CWoWSimbotQuickApp::OnDebugLogViewer()
{
    CLogViewerDlg dlg;
    dlg.DoModal();
}

void CWoWSimbotQuickApp::OnDebugDiagnostics()
{
    CDebugTools* pDebug = GetDebugTools();
    if (pDebug)
    {
        CString report = pDebug->GenerateDiagnosticReport();

        // 파일로 저장
        CString filePath;
        TCHAR szPath[MAX_PATH];
        if (SUCCEEDED(SHGetFolderPath(NULL, CSIDL_DESKTOP, NULL, 0, szPath)))
        {
            filePath.Format(_T("%s\\WoWSimbotQuick_Diagnostic_%s.txt"),
                szPath, CTime::GetCurrentTime().Format(_T("%Y%m%d_%H%M%S")));

            if (pDebug->SaveDiagnosticReport(filePath))
            {
                CString msg;
                msg.Format(_T("진단 보고서가 저장되었습니다:\n%s\n\n열어보시겠습니까?"), filePath);
                if (AfxMessageBox(msg, MB_YESNO | MB_ICONINFORMATION) == IDYES)
                {
                    ShellExecute(NULL, _T("open"), filePath, NULL, NULL, SW_SHOWNORMAL);
                }
            }
        }
    }
}

void CWoWSimbotQuickApp::SaveSettings() const
{
    CWinApp* pApp = AfxGetApp();

    pApp->WriteProfileString(REG_KEY_SETTINGS, _T("SimcPath"), m_strSimcPath);
    pApp->WriteProfileString(REG_KEY_SETTINGS, _T("ClientId"), m_strClientId);
    pApp->WriteProfileString(REG_KEY_SETTINGS, _T("ClientSecret"), m_strClientSecret);

    // Save settings manager settings
    CSettingsManager* pMgr = GetSettingsManager();
    if (pMgr)
    {
        pMgr->SaveSettings();
    }
}

void CWoWSimbotQuickApp::OnPresetsManage()
{
    CPresetManagerDlg dlg;
    if (dlg.DoModal() == IDOK)
    {
        // 메인 프레임에 설정 변경 알림
        CMainFrame* pFrame = static_cast<CMainFrame*>(m_pMainWnd);
        if (pFrame)
        {
            pFrame->PostMessage(WM_COMMAND, MAKEWPARAM(ID_SETTINGS_CHANGED, 0), 0);
        }
    }
}

void CWoWSimbotQuickApp::OnShowHistory()
{
    CResultHistoryDlg dlg;
    dlg.DoModal();
}
