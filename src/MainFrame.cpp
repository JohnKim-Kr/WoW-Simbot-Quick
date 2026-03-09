
// MainFrame.cpp : implementation of the CMainFrame class
//

#include "pch.h"
#include "framework.h"
#include "SimbotQuick.h"
#include "MainFrame.h"
#include "SettingsManager.h"
#include "ResultHistoryManager.h"
#include "CharacterData.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CMainFrame

IMPLEMENT_DYNAMIC(CMainFrame, CFrameWnd)

BEGIN_MESSAGE_MAP(CMainFrame, CFrameWnd)
    ON_WM_CREATE()
    ON_WM_SETFOCUS()
    ON_WM_SIZE()
    ON_MESSAGE(WM_USER_API_CHAR_LOADED, &CMainFrame::OnUserApiCharacterLoaded)
    ON_MESSAGE(WM_USER_SIM_PROGRESS, &CMainFrame::OnUserSimProgress)
    ON_MESSAGE(WM_USER_SIM_COMPLETE, &CMainFrame::OnUserSimComplete)
    ON_COMMAND(ID_SETTINGS_CHANGED, &CMainFrame::OnSettingsChanged)
END_MESSAGE_MAP()

static UINT indicators[] =
{
    ID_SEPARATOR,           // status line indicator
    ID_INDICATOR_CAPS,
    ID_INDICATOR_NUM,
    ID_INDICATOR_SCRL,
};

// CMainFrame construction/destruction

CMainFrame::CMainFrame() noexcept
    : m_pCharInputPanel(nullptr)
    , m_pSimSettingsPanel(nullptr)
    , m_pResultsPanel(nullptr)
{
    // Initialize core components
    m_pApiClient = std::make_unique<CBnetApiClient>();
    m_pSimcRunner = std::make_unique<CSimcRunner>();
    m_pCharacterData = std::make_unique<CCharacterData>();
}

CMainFrame::~CMainFrame()
{
}

int CMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
    if (CFrameWnd::OnCreate(lpCreateStruct) == -1)
        return -1;

    // Create status bar
    if (!m_wndStatusBar.Create(this))
    {
        TRACE0("Failed to create status bar\n");
        return -1;
    }
    m_wndStatusBar.SetIndicators(indicators, sizeof(indicators) / sizeof(UINT));

    // Create toolbar
    if (!m_wndToolBar.CreateEx(this, TBSTYLE_FLAT, WS_CHILD | WS_VISIBLE | CBRS_TOP | CBRS_GRIPPER | CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC) ||
        !m_wndToolBar.LoadToolBar(IDR_MAINFRAME))
    {
        TRACE0("Failed to create toolbar\n");
        return -1;
    }

    // Create rebar
    if (!m_wndReBar.Create(this) ||
        !m_wndReBar.AddBar(&m_wndToolBar))
    {
        TRACE0("Failed to create rebar\n");
        return -1;
    }

    // Set toolbar styles
    m_wndToolBar.EnableDocking(CBRS_ALIGN_ANY);
    EnableDocking(CBRS_ALIGN_ANY);
    DockPane(&m_wndToolBar);

    // Set main window properties
    SetWindowText(_T("WoW Simbot Quick"));

    // Initialize status
    UpdateStatus(_T("Ready"));

    return 0;
}

BOOL CMainFrame::OnCreateClient(LPCREATESTRUCT /*lpcs*/, CCreateContext* pContext)
{
    // Create a static splitter with 2 rows
    if (!m_wndSplitter.CreateStatic(this, 2, 1))
    {
        TRACE0("Failed to create splitter\n");
        return FALSE;
    }

    // Get client area size
    CRect rect;
    GetClientRect(&rect);

    int topHeight = rect.Height() * 2 / 3;
    int bottomHeight = rect.Height() - topHeight;

    // Create the top splitter (nested for left/right panels)
    CRect topRect(0, 0, rect.Width(), topHeight);

    // First row: Create a nested splitter for char input and sim settings
    if (!m_wndSplitter.CreateView(0, 0, RUNTIME_CLASS(CCharInputPanel), CSize(rect.Width() / 2, topHeight), pContext))
    {
        TRACE0("Failed to create character input panel\n");
        m_wndSplitter.DestroyWindow();
        return FALSE;
    }

    // Second row: Results panel
    if (!m_wndSplitter.CreateView(1, 0, RUNTIME_CLASS(CResultsPanel), CSize(rect.Width(), bottomHeight), pContext))
    {
        TRACE0("Failed to create results panel\n");
        m_wndSplitter.DestroyWindow();
        return FALSE;
    }

    // Store panel pointers
    m_pCharInputPanel = static_cast<CCharInputPanel*>(m_wndSplitter.GetPane(0, 0));
    m_pResultsPanel = static_cast<CResultsPanel*>(m_wndSplitter.GetPane(1, 0));

    // Create a nested splitter in the top row for left/right panels
    if (m_pCharInputPanel)
    {
        CCreateContext nestedContext;
        nestedContext.m_pCurrentFrame = this;
        nestedContext.m_pCurrentDoc = pContext->m_pCurrentDoc;
        nestedContext.m_pNewDocTemplate = pContext->m_pNewDocTemplate;

        if (!m_pCharInputPanel->CreateNestedSplitter(&m_pSimSettingsPanel))
        {
            TRACE0("Failed to create nested splitter for settings panel\n");
        }
    }

    return TRUE;
}

BOOL CMainFrame::PreCreateWindow(CREATESTRUCT& cs)
{
    if (!CFrameWnd::PreCreateWindow(cs))
        return FALSE;

    cs.dwExStyle &= ~WS_EX_CLIENTEDGE;
    cs.lpszClass = AfxRegisterWndClass(0);

    // Set initial window size
    cs.cx = 1200;
    cs.cy = 900;

    return TRUE;
}

// CMainFrame diagnostics

#ifdef _DEBUG
void CMainFrame::AssertValid() const
{
    CFrameWnd::AssertValid();
}

void CMainFrame::Dump(CDumpContext& dc) const
{
    CFrameWnd::Dump(dc);
}
#endif //_DEBUG

// CMainFrame message handlers

void CMainFrame::OnSetFocus(CWnd* /*pOldWnd*/)
{
    // forward focus to the view window
    if (m_pCharInputPanel)
        m_pCharInputPanel->SetFocus();
}

void CMainFrame::OnSize(UINT nType, int cx, int cy)
{
    CFrameWnd::OnSize(nType, cx, cy);

    // Adjust splitter row heights
    if (m_wndSplitter.GetSafeHwnd() && cy > 0)
    {
        int topHeight = cy * 2 / 3;
        int bottomHeight = cy - topHeight - m_wndSplitter.GetRowInfo(0);
        m_wndSplitter.SetRowInfo(0, topHeight, 100);
        m_wndSplitter.SetRowInfo(1, bottomHeight, 100);
        m_wndSplitter.RecalcLayout();
    }
}

// Character loading
BOOL CMainFrame::LoadCharacter(const CString& region, const CString& server, const CString& characterName)
{
    UpdateStatus(_T("Loading character..."));

    if (!m_pApiClient)
    {
        UpdateStatus(_T("API client not initialized"));
        return FALSE;
    }

    // Start async character load
    std::thread([this, region, server, characterName]() {
        BOOL success = m_pApiClient->FetchCharacter(
            CT2A(region), CT2A(server), CT2A(characterName), m_pCharacterData.get());

        // Post message back to UI thread
        PostMessage(WM_USER_API_CHAR_LOADED, success ? 1 : 0, 0);
    }).detach();

    return TRUE;
}

void CMainFrame::OnCharacterLoaded()
{
    if (m_pCharacterData && m_pCharacterData->IsValid())
    {
        UpdateStatus(_T("Character loaded successfully"));

        if (m_pCharInputPanel)
        {
            m_pCharInputPanel->DisplayCharacterInfo(m_pCharacterData.get());
        }
    }
    else
    {
        UpdateStatus(_T("Failed to load character"));
        AfxMessageBox(_T("Failed to load character data. Please check your settings and try again."), MB_ICONERROR);
    }
}

// Simulation control
void CMainFrame::StartSimulation()
{
    if (!m_pCharacterData || !m_pCharacterData->IsValid())
    {
        AfxMessageBox(_T("Please load a character first."), MB_ICONWARNING);
        return;
    }

    CWoWSimbotQuickApp* pApp = static_cast<CWoWSimbotQuickApp*>(AfxGetApp());
    CString simcPath = pApp->GetSimcPath();

    if (simcPath.IsEmpty() || !fs::exists(CT2A(simcPath)))
    {
        AfxMessageBox(_T("Please configure simc.exe path in Settings first."), MB_ICONWARNING);
        return;
    }

    // 현재 UI 설정을 SettingsManager에 저장
    if (m_pSimSettingsPanel)
    {
        m_pSimSettingsPanel->SaveSettingsToManager();
    }

    pApp->SetSimRunning(TRUE);
    UpdateStatus(_T("Starting simulation..."));
    SetProgress(0);

    // Build simc profile
    CString profile = m_pCharacterData->ToSimcProfile();

    // Add simulation options from SettingsManager
    CSettingsManager* pMgr = GetSettingsManager();
    if (pMgr)
    {
        profile += _T("\n");
        profile += pMgr->GenerateSimcOptions();
        profile += _T("json=sim_result.json\n");
    }

    // Start simc in a separate thread
    std::thread([this, simcPath, profile]() {
        CString outputFile = _T("sim_result.json");
        BOOL success = m_pSimcRunner->RunSimulation(
            simcPath, profile, outputFile,
            [this](int progress) {
                PostMessage(WM_USER_SIM_PROGRESS, progress, 0);
            });

        PostMessage(WM_USER_SIM_COMPLETE, success ? 1 : 0, (LPARAM)new CString(outputFile));
    }).detach();
}

void CMainFrame::StopSimulation()
{
    if (m_pSimcRunner)
    {
        m_pSimcRunner->Cancel();
        UpdateStatus(_T("Simulation cancelled"));
    }

    CWoWSimbotQuickApp* pApp = static_cast<CWoWSimbotQuickApp*>(AfxGetApp());
    pApp->SetSimRunning(FALSE);
}

void CMainFrame::OnSimulationComplete(const CString& resultJson)
{
    CWoWSimbotQuickApp* pApp = static_cast<CWoWSimbotQuickApp*>(AfxGetApp());
    pApp->SetSimRunning(FALSE);
    SetProgress(100);

    // Parse simc JSON
    CResultHistoryManager* pHistoryMgr = GetResultHistoryManager();
    if (pHistoryMgr)
    {
        CSimResult result;
        if (pHistoryMgr->ParseSimcJson(resultJson, result))
        {
            // 빌드 정보 설정
            if (m_pCharacterData)
            {
                result.GetBuildInfo().characterName = m_pCharacterData->GetName();
                result.GetBuildInfo().server = m_pCharacterData->GetRealm();
                result.GetBuildInfo().region = m_pCharacterData->GetRegion();
                result.GetBuildInfo().className = m_pCharacterData->GetClassName();
                result.GetBuildInfo().specName = m_pCharacterData->GetActiveSpecName();
                result.GetBuildInfo().itemLevel = m_pCharacterData->GetItemLevel();
            }

            // 시뮬레이션 설정 복사
            CSettingsManager* pSettingsMgr = GetSettingsManager();
            if (pSettingsMgr)
            {
                const SimulationSettings& settings = pSettingsMgr->GetCurrentSettings();
                result.SetFightStyle(settings.fightStyle);
                result.SetDuration(settings.duration);
                result.SetIterations(settings.iterations);
            }

            // 히스토리에 저장
            pHistoryMgr->AddResult(result);
        }
    }

    // Parse and display results
    if (m_pResultsPanel)
    {
        m_pResultsPanel->LoadResults(resultJson);
    }

    UpdateStatus(_T("Simulation complete"));
}

// Message handlers
LRESULT CMainFrame::OnUserApiCharacterLoaded(WPARAM wParam, LPARAM /*lParam*/)
{
    if (wParam)
    {
        OnCharacterLoaded();
    }
    else
    {
        UpdateStatus(_T("Failed to load character"));
    }
    return 0;
}

LRESULT CMainFrame::OnUserSimProgress(WPARAM wParam, LPARAM /*lParam*/)
{
    int progress = static_cast<int>(wParam);
    SetProgress(progress);

    CString status;
    status.Format(_T("Simulating... %d%%"), progress);
    UpdateStatus(status);

    return 0;
}

void CMainFrame::OnSettingsChanged()
{
    // 설정이 변경되었을 때 UI 업데이트
    if (m_pSimSettingsPanel)
    {
        m_pSimSettingsPanel->LoadSettingsFromManager();
    }
}

LRESULT CMainFrame::OnUserSimComplete(WPARAM wParam, LPARAM lParam)
{
    CString* pResultFile = reinterpret_cast<CString*>(lParam);

    if (wParam && pResultFile)
    {
        OnSimulationComplete(*pResultFile);
    }
    else
    {
        UpdateStatus(_T("Simulation failed"));
        CWoWSimbotQuickApp* pApp = static_cast<CWoWSimbotQuickApp*>(AfxGetApp());
        pApp->SetSimRunning(FALSE);
    }

    delete pResultFile;
    return 0;
}

// UI Updates
void CMainFrame::UpdateStatus(const CString& message)
{
    m_wndStatusBar.SetPaneText(0, message);
}

void CMainFrame::SetProgress(int percent)
{
    if (m_pResultsPanel)
    {
        m_pResultsPanel->SetProgress(percent);
    }
}

// Sim options accessors
CString CMainFrame::GetFightStyle() const
{
    if (m_pSimSettingsPanel)
        return m_pSimSettingsPanel->GetFightStyle();
    return _T("patchwerk");
}

int CMainFrame::GetDuration() const
{
    if (m_pSimSettingsPanel)
        return m_pSimSettingsPanel->GetDuration();
    return 300;
}

int CMainFrame::GetIterations() const
{
    if (m_pSimSettingsPanel)
        return m_pSimSettingsPanel->GetIterations();
    return 10000;
}

int CMainFrame::GetTargetCount() const
{
    if (m_pSimSettingsPanel)
        return m_pSimSettingsPanel->GetTargetCount();
    return 1;
}

BOOL CMainFrame::GetUseFlask() const
{
    if (m_pSimSettingsPanel)
        return m_pSimSettingsPanel->GetUseFlask();
    return TRUE;
}

BOOL CMainFrame::GetUseFood() const
{
    if (m_pSimSettingsPanel)
        return m_pSimSettingsPanel->GetUseFood();
    return TRUE;
}

BOOL CMainFrame::GetUseRune() const
{
    if (m_pSimSettingsPanel)
        return m_pSimSettingsPanel->GetUseRune();
    return TRUE;
}

BOOL CMainFrame::GetUseAugment() const
{
    if (m_pSimSettingsPanel)
        return m_pSimSettingsPanel->GetUseAugment();
    return TRUE;
}

BOOL CMainFrame::GetUseBloodlust() const
{
    if (m_pSimSettingsPanel)
        return m_pSimSettingsPanel->GetUseBloodlust();
    return TRUE;
}

BOOL CMainFrame::GetUsePotion() const
{
    if (m_pSimSettingsPanel)
        return m_pSimSettingsPanel->GetUsePotion();
    return TRUE;
}
