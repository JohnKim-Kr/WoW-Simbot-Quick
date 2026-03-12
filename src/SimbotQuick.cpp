#include "pch.h"
#include "framework.h"
#include "SimbotQuick.h"
#include "MainFrame.h"
#include "SimOptionsDlg.h"
#include "SettingsManager.h"
#include <vector>
#include <string>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

BEGIN_MESSAGE_MAP(CWoWSimbotQuickApp, CWinApp)
    ON_COMMAND(ID_HELP, &CWinApp::OnHelp)
    ON_COMMAND(ID_BUTTON_RUN_SIM, &CWoWSimbotQuickApp::OnToolsRunSim)
    ON_COMMAND(ID_BUTTON_STOP_SIM, &CWoWSimbotQuickApp::OnToolsStopSim)
    ON_UPDATE_COMMAND_UI(ID_BUTTON_STOP_SIM, &CWoWSimbotQuickApp::OnUpdateToolsStopSim)
    ON_COMMAND(ID_TOOLS_SETTINGS, &CWoWSimbotQuickApp::OnFileSettings)
    ON_COMMAND(ID_PRESETS_MANAGE, &CWoWSimbotQuickApp::OnPresetsManage)
    ON_COMMAND(ID_HISTORY_SHOW, &CWoWSimbotQuickApp::OnShowHistory)
    ON_COMMAND(ID_DEBUG_CONSOLE, &CWoWSimbotQuickApp::OnDebugConsole)
    ON_COMMAND(ID_DEBUG_LOG_VIEWER, &CWoWSimbotQuickApp::OnDebugLogViewer)
    ON_COMMAND(ID_DEBUG_DIAGNOSTICS, &CWoWSimbotQuickApp::OnDebugDiagnostics)
    ON_COMMAND(ID_HELP_ABOUT, &CWoWSimbotQuickApp::OnAppAbout)
END_MESSAGE_MAP()

CWoWSimbotQuickApp::CWoWSimbotQuickApp() noexcept
{
    m_bSimRunning = FALSE;
}

CWoWSimbotQuickApp theApp;
BOOL CWoWSimbotQuickApp::InitInstance()
{
    INITCOMMONCONTROLSEX InitCtrls;
    InitCtrls.dwSize = sizeof(InitCtrls);
    InitCtrls.dwICC = ICC_WIN95_CLASSES;
    InitCommonControlsEx(&InitCtrls);

    CWinApp::InitInstance();

    if (!AfxOleInit()) return FALSE;
    AfxEnableControlContainer();

    SetAppID(_T("WoWSimbotQuick.AppID.NoVersion"));

    GetSettingsManager()->Initialize();

    m_strSimcPath = GetProfileString(_T("Settings"), _T("SimcPath"), _T(""));

    CMainFrame* pFrame = new CMainFrame;
    if (!pFrame) return FALSE;
    
    // Use Create with the menu resource ID
    if (!pFrame->Create(NULL, _T("WoW Simbot Quick"), WS_OVERLAPPEDWINDOW, CFrameWnd::rectDefault, NULL, MAKEINTRESOURCE(IDR_MAINFRAME))) return FALSE;
    
    m_pMainWnd = pFrame;

    m_hAccelTable = LoadAccelerators(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDR_MAINFRAME));

    pFrame->ShowWindow(SW_SHOW);
    pFrame->UpdateWindow();

    return TRUE;
}

int CWoWSimbotQuickApp::ExitInstance()
{
    GetSettingsManager()->Shutdown();
    WriteProfileString(_T("Settings"), _T("SimcPath"), m_strSimcPath);
    return CWinApp::ExitInstance();
}

void CWoWSimbotQuickApp::LoadSettings() {}
void CWoWSimbotQuickApp::SaveSettings() const {}
void CWoWSimbotQuickApp::OnAppAbout() { AfxMessageBox(_T("WoW Simbot Quick v1.0"), MB_OK | MB_ICONINFORMATION); }
void CWoWSimbotQuickApp::OnFileSettings()
{
    CSimOptionsDlg dlg(m_pMainWnd);
    if (dlg.DoModal() == IDOK)
    {
        // Settings were saved, may need to update some UI elements if necessary
        CMainFrame* pFrame = (CMainFrame*)m_pMainWnd;
        if (pFrame)
        {
            pFrame->UpdateStatus(_T("설정 저장됨"));
        }
    }
}

void CWoWSimbotQuickApp::OnToolsRunSim()
{
    CMainFrame* pFrame = (CMainFrame*)m_pMainWnd;
    if (pFrame) pFrame->StartSimulation();
}

void CWoWSimbotQuickApp::OnToolsStopSim()
{
    CMainFrame* pFrame = (CMainFrame*)m_pMainWnd;
    if (pFrame) pFrame->StopSimulation();
}

void CWoWSimbotQuickApp::OnUpdateToolsStopSim(CCmdUI* pCmdUI) { pCmdUI->Enable(m_bSimRunning); }

void CWoWSimbotQuickApp::OnPresetsManage() { }
void CWoWSimbotQuickApp::OnShowHistory() { }
void CWoWSimbotQuickApp::OnDebugConsole() { }
void CWoWSimbotQuickApp::OnDebugLogViewer() { }
void CWoWSimbotQuickApp::OnDebugDiagnostics() { }
