#include "pch.h"
#pragma execution_character_set("utf-8")
#include "framework.h"
#include "SimbotQuick.h"
#include "MainFrame.h"
#include "CharInputPanel.h"
#include "ResultsPanel.h"
#include "SimSettingsPanel.h"
#include "SettingsManager.h"
#include "ResultHistoryManager.h"
#include "CharacterData.h"
#include "SimcParser.h"
#include <filesystem>
#include <thread>

namespace fs = std::filesystem;

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

IMPLEMENT_DYNAMIC(CMainFrame, CFrameWnd)

BEGIN_MESSAGE_MAP(CMainFrame, CFrameWnd)
    ON_WM_CREATE()
    ON_WM_SETFOCUS()
    ON_WM_SIZE()
    ON_MESSAGE(WM_USER_SIM_PROGRESS, &CMainFrame::OnUserSimProgress)
    ON_MESSAGE(WM_USER_SIM_COMPLETE, &CMainFrame::OnUserSimComplete)
    ON_COMMAND(ID_SETTINGS_CHANGED, &CMainFrame::OnSettingsChanged)
END_MESSAGE_MAP()

static UINT indicators[] = { ID_SEPARATOR };

CMainFrame::CMainFrame() noexcept : m_pCharInputPanel(nullptr), m_pSimSettingsPanel(nullptr), m_pResultsPanel(nullptr)
{
    m_pSimcRunner = std::make_unique<CSimcRunner>();
    m_pCharacterData = std::make_unique<CCharacterData>();
}

CMainFrame::~CMainFrame() {}

int CMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
    if (CFrameWnd::OnCreate(lpCreateStruct) == -1) return -1;
    if (!m_wndStatusBar.Create(this)) return -1;
    m_wndStatusBar.SetIndicators(indicators, 1);
    SetWindowText(_T("WoW Simbot Quick"));
    UpdateStatus(_T("Ready"));
    return 0;
}

BOOL CMainFrame::OnCreateClient(LPCREATESTRUCT lp, CCreateContext* pContext)
{
    CRect r; GetClientRect(&r);
    
    // Create views manually without splitters
    m_pCharInputPanel = new CCharInputPanel();
    if (!m_pCharInputPanel->Create(NULL, NULL, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS, r, this, 20001, pContext)) return FALSE;
    
    m_pResultsPanel = new CResultsPanel();
    if (!m_pResultsPanel->Create(NULL, NULL, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS, r, this, 20002, pContext)) return FALSE;
    
    m_pSimSettingsPanel = (CSimSettingsPanel*)RUNTIME_CLASS(CSimSettingsPanel)->CreateObject();
    if (!m_pSimSettingsPanel->CreateView(this, 20003, pContext)) return FALSE;

    // Manually trigger initial updates since we are not using standard MFC frame logic
    if (m_pCharInputPanel) m_pCharInputPanel->SendMessage(WM_INITIALUPDATE);
    if (m_pResultsPanel) m_pResultsPanel->SendMessage(WM_INITIALUPDATE);
    if (m_pSimSettingsPanel) m_pSimSettingsPanel->SendMessage(WM_INITIALUPDATE);

    return TRUE;
}

BOOL CMainFrame::PreCreateWindow(CREATESTRUCT& cs)
{
    if (!CFrameWnd::PreCreateWindow(cs)) return FALSE;
    cs.cx = 1200; cs.cy = 900; return TRUE;
}

void CMainFrame::OnSetFocus(CWnd*) { if (m_pCharInputPanel) m_pCharInputPanel->SetFocus(); }

void CMainFrame::OnSize(UINT nType, int cx, int cy)
{
    CFrameWnd::OnSize(nType, cx, cy);
    if (cx <= 0 || cy <= 0) return;

    // Position the status bar and get its height
    if (m_wndStatusBar.GetSafeHwnd())
    {
        m_wndStatusBar.SendMessage(WM_SIZE, nType, MAKELPARAM(cx, cy));
        CRect rectStatus;
        m_wndStatusBar.GetWindowRect(&rectStatus);
        cy -= rectStatus.Height();
    }

    int lw = cx * 2 / 3;
    int th = cy * 2 / 3;

    if (m_pCharInputPanel && m_pCharInputPanel->GetSafeHwnd())
        m_pCharInputPanel->MoveWindow(0, 0, lw, th);

    if (m_pResultsPanel && m_pResultsPanel->GetSafeHwnd())
        m_pResultsPanel->MoveWindow(0, th, lw, cy - th);

    if (m_pSimSettingsPanel && m_pSimSettingsPanel->GetSafeHwnd())
        m_pSimSettingsPanel->MoveWindow(lw, 0, cx - lw, cy);
}

BOOL CMainFrame::ParseSimcProfile(const std::string& str)
{
    UpdateStatus(_T("프로필 파싱 중..."));
    if (m_pCharacterData) m_pCharacterData->Clear();
    CSimcParser parser;
    if (!parser.Parse(str, m_pCharacterData.get()))
    {
        UpdateStatus(_T("파싱 실패"));
        return FALSE;
    }
    OnCharacterLoaded(); 
    UpdateStatus(_T("준비됨"));
    return TRUE;
}

void CMainFrame::OnCharacterLoaded()
{
    if (m_pCharacterData && m_pCharacterData->IsValid()) {
        CSettingsManager* pMgr = GetSettingsManager();
        if (pMgr) { pMgr->GetCurrentSettings() = m_pCharacterData->GetSettings(); if (m_pSimSettingsPanel) m_pSimSettingsPanel->LoadSettingsFromManager(); }
        if (m_pCharInputPanel) m_pCharInputPanel->DisplayCharacterInfo(m_pCharacterData.get());
    }
}

void CMainFrame::StartSimulation()
{
    if (!m_pCharacterData || !m_pCharacterData->IsValid())
    {
        AfxMessageBox(_T("먼저 프로필을 파싱해 주세요."), MB_ICONWARNING);
        return;
    }

    CWoWSimbotQuickApp* pApp = (CWoWSimbotQuickApp*)AfxGetApp();
    if (pApp->m_strSimcPath.IsEmpty())
    {
        AfxMessageBox(_T("시뮬레이션을 시작하기 전에 simc.exe 경로를 설정해야 합니다."), MB_ICONINFORMATION);
        pApp->OnFileSettings();
        if (pApp->m_strSimcPath.IsEmpty()) return;
    }

    if (m_pSimSettingsPanel) m_pSimSettingsPanel->SaveSettingsToManager();
    pApp->m_bSimRunning = TRUE; 
    if (m_pCharInputPanel) m_pCharInputPanel->UpdateSimButtonState(TRUE);
    SetProgress(0);
    UpdateStatus(_T("시뮬레이션 중..."));

    CString profile = m_pCharacterData->ToSimcProfile();
    CSettingsManager* pMgr = GetSettingsManager();
    CString out;
    if (pMgr) {
        profile += _T("\n"); 
        profile += pMgr->GenerateSimcOptions();
        TCHAR tmp[MAX_PATH]; GetTempPath(MAX_PATH, tmp);
        out.Format(_T("%s\\sim_result.json"), tmp);
        CString line; line.Format(_T("json=%s\n"), (LPCTSTR)out);
        profile += line;
    } else out = _T("sim_result.json");

    std::thread([this, pApp, profile, out]() {
        BOOL ok = m_pSimcRunner->RunSimulation(pApp->m_strSimcPath, profile, out, [this](int p) { PostMessage(WM_USER_SIM_PROGRESS, (WPARAM)p, 0); });
        WPARAM wp = (ok ? 1 : 0);
        PostMessage(WM_USER_SIM_COMPLETE, wp, (LPARAM)new CString(out.GetString()));
    }).detach();
}

void CMainFrame::StopSimulation() 
{ 
    if (m_pSimcRunner) m_pSimcRunner->Cancel(); 
    ((CWoWSimbotQuickApp*)AfxGetApp())->m_bSimRunning = FALSE; 
    if (m_pCharInputPanel) m_pCharInputPanel->UpdateSimButtonState(FALSE);
}

void CMainFrame::OnSimulationComplete(const CString& res)
{
    ((CWoWSimbotQuickApp*)AfxGetApp())->m_bSimRunning = FALSE; 
    if (m_pCharInputPanel) m_pCharInputPanel->UpdateSimButtonState(FALSE);
    SetProgress(100);
    CResultHistoryManager* h = GetResultHistoryManager();
    if (h) {
        CSimResult r;
        if (h->ParseSimcJson(res, r)) {
            if (m_pCharacterData) {
                r.GetBuildInfo().characterName = CString(m_pCharacterData->GetName().c_str());
                r.GetBuildInfo().server = CString(m_pCharacterData->GetRealm().c_str());
                r.GetBuildInfo().region = CString(m_pCharacterData->GetRegion().c_str());
                r.GetBuildInfo().className = CString(m_pCharacterData->GetClassName().c_str());
                r.GetBuildInfo().specName = CString(m_pCharacterData->GetActiveSpecName().c_str());
                r.GetBuildInfo().itemLevel = m_pCharacterData->GetItemLevel();
            }
            CSettingsManager* s = GetSettingsManager();
            if (s) { r.SetFightStyle(s->GetCurrentSettings().fightStyle); r.SetDuration(s->GetCurrentSettings().duration); r.SetIterations(s->GetCurrentSettings().iterations); }
            h->AddResult(r);
        }
    }
    if (m_pResultsPanel) m_pResultsPanel->LoadResults(res);
    UpdateStatus(_T("Complete"));
}

LRESULT CMainFrame::OnUserSimProgress(WPARAM wp, LPARAM) { int p = (int)wp; SetProgress(p); return 0; }
void CMainFrame::OnSettingsChanged() { if (m_pSimSettingsPanel) m_pSimSettingsPanel->LoadSettingsFromManager(); }

LRESULT CMainFrame::OnUserSimComplete(WPARAM wp, LPARAM lp)
{
    CString* p = (CString*)lp;
    if (wp != 0 && p != nullptr) 
    {
        OnSimulationComplete(*p);
    }
    else 
    { 
        ((CWoWSimbotQuickApp*)AfxGetApp())->m_bSimRunning = FALSE; 
        UpdateStatus(_T("시뮬레이션 실패"));
        AfxMessageBox(_T("시뮬레이션 실행 중 오류가 발생했습니다. simc.exe 경로와 프로필 내용을 확인해 주세요."), MB_ICONERROR);
    }
    if (p != nullptr) delete p;
    return 0;
}

void CMainFrame::UpdateStatus(const CString& m) 
{ 
    if (m_wndStatusBar.GetSafeHwnd())
    {
        m_wndStatusBar.SetPaneText(0, m); 
    }
}
void CMainFrame::SetProgress(int p) { if (m_pResultsPanel) m_pResultsPanel->SetProgress(p); }
CString CMainFrame::GetFightStyle() const { if (m_pSimSettingsPanel) return m_pSimSettingsPanel->GetFightStyle(); return _T("patchwerk"); }
int CMainFrame::GetDuration() const { return m_pSimSettingsPanel ? m_pSimSettingsPanel->GetDuration() : 300; }
int CMainFrame::GetIterations() const { return m_pSimSettingsPanel ? m_pSimSettingsPanel->GetIterations() : 10000; }
int CMainFrame::GetTargetCount() const { return m_pSimSettingsPanel ? m_pSimSettingsPanel->GetTargetCount() : 1; }
BOOL CMainFrame::GetUseFlask() const { return m_pSimSettingsPanel ? m_pSimSettingsPanel->GetUseFlask() : TRUE; }
BOOL CMainFrame::GetUseFood() const { return m_pSimSettingsPanel ? m_pSimSettingsPanel->GetUseFood() : TRUE; }
BOOL CMainFrame::GetUseRune() const { return m_pSimSettingsPanel ? m_pSimSettingsPanel->GetUseRune() : TRUE; }
BOOL CMainFrame::GetUseAugment() const { return m_pSimSettingsPanel ? m_pSimSettingsPanel->GetUseAugment() : TRUE; }
BOOL CMainFrame::GetUseBloodlust() const { return m_pSimSettingsPanel ? m_pSimSettingsPanel->GetUseBloodlust() : TRUE; }
BOOL CMainFrame::GetUsePotion() const { return m_pSimSettingsPanel ? m_pSimSettingsPanel->GetUsePotion() : TRUE; }

#ifdef _DEBUG
void CMainFrame::AssertValid() const
{
    CFrameWnd::AssertValid();
}

void CMainFrame::Dump(CDumpContext& dc) const
{
    CFrameWnd::Dump(dc);
}
#endif
