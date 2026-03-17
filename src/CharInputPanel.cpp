#include "pch.h"
#include "framework.h"
#include "SimbotQuick.h"
#include "MainFrame.h"
#include "CharInputPanel.h"
#include "CharacterData.h"
#include "resource.h"

IMPLEMENT_DYNCREATE(CCharInputPanel, CView)

BEGIN_MESSAGE_MAP(CCharInputPanel, CView)
    ON_WM_CREATE()
    ON_WM_SIZE()
    ON_BN_CLICKED(IDC_BUTTON_CLEAR, &CCharInputPanel::OnBnClickedButtonClear)
    ON_BN_CLICKED(ID_BUTTON_SETTINGS, &CCharInputPanel::OnBnClickedButtonSettings)
    ON_BN_CLICKED(ID_BUTTON_RUN_SIM, &CCharInputPanel::OnBnClickedButtonSimControl)
END_MESSAGE_MAP()

CCharInputPanel::CCharInputPanel() {}

CCharInputPanel::~CCharInputPanel()
{
}

void CCharInputPanel::OnDraw(CDC* pDC)
{
    CRect rect;
    GetClientRect(&rect);
    pDC->FillSolidRect(&rect, GetSysColor(COLOR_WINDOW));
}

int CCharInputPanel::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
    if (CView::OnCreate(lpCreateStruct) == -1)
        return -1;

    CRect rect;
    GetClientRect(&rect);

    int x = 10, y = 10, w = rect.Width() - 20, h = 25;

    m_staticLabel.Create(_T("/simc 애드온 출력을 여기에 붙여넣으세요:"), WS_CHILD | WS_VISIBLE | SS_LEFT,
        CRect(x, y, x + w, y + 20), this, 65535);

    y += 25;

    if (!m_editProfile.Create(WS_CHILD | WS_VISIBLE | WS_BORDER | WS_VSCROLL | ES_MULTILINE | ES_AUTOVSCROLL,
        CRect(x, y, x + w, y + 200), this, IDC_EDIT_PROFILE))
        return -1;

    y += 210;

    if (!m_btnClear.Create(_T("초기화"), WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        CRect(x, y, x + w, y + h), this, IDC_BUTTON_CLEAR))
        return -1;

    y += h + 5;

    if (!m_btnSettings.Create(_T("시뮬레이터 설정..."), WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        CRect(x, y, x + w, y + h), this, ID_BUTTON_SETTINGS))
        return -1;

    y += h + 5;

    if (!m_btnSimControl.Create(_T("시뮬레이션 시작"), WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        CRect(x, y, x + w, y + h), this, ID_BUTTON_RUN_SIM))
        return -1;

    return 0;
}

void CCharInputPanel::OnSize(UINT nType, int cx, int cy)
{
    CView::OnSize(nType, cx, cy);

    if (m_staticLabel.GetSafeHwnd())
    {
        int x = 10, y = 10, w = cx - 20, h = 25;
        const int bottomMargin = 10;
        const int fixedAreaHeight = 25 + h + 5 + h + 5 + h + bottomMargin;
        int editHeight = cy - y - fixedAreaHeight;
        if (editHeight < 120) editHeight = 120;

        m_staticLabel.MoveWindow(x, y, w, 20);
        y += 25;
        m_editProfile.MoveWindow(x, y, w, editHeight);
        y += editHeight + 10;
        m_btnClear.MoveWindow(x, y, w, h);
        y += h + 5;
        m_btnSettings.MoveWindow(x, y, w, h);
        y += h + 5;
        m_btnSimControl.MoveWindow(x, y, w, h);
    }
}

void CCharInputPanel::OnInitialUpdate()
{
    CView::OnInitialUpdate();
}

void CCharInputPanel::OnBnClickedButtonClear()
{
    ClearProfile();
}

void CCharInputPanel::OnBnClickedCheckTrinket1() {}
void CCharInputPanel::OnBnClickedCheckTrinket2() {}
void CCharInputPanel::OnCbnSelchangeComboTrinketFilter() {}
void CCharInputPanel::OnCbnSelchangeComboTrinket1() {}
void CCharInputPanel::OnCbnSelchangeComboTrinket2() {}
void CCharInputPanel::OnCbnSelchangeComboTrinket1ItemLevel() {}
void CCharInputPanel::OnCbnSelchangeComboTrinket2ItemLevel() {}

void CCharInputPanel::OnBnClickedButtonSettings()
{
    CWoWSimbotQuickApp* pApp = static_cast<CWoWSimbotQuickApp*>(AfxGetApp());
    if (pApp)
    {
        pApp->OnFileSettings();
    }
}

void CCharInputPanel::OnBnClickedButtonSimControl()
{
    CWoWSimbotQuickApp* pApp = static_cast<CWoWSimbotQuickApp*>(AfxGetApp());
    if (pApp)
    {
        if (pApp->IsSimRunning())
        {
            pApp->OnToolsStopSim();
        }
        else
        {
            if (!ParseSimcProfile())
            {
                UpdateSimButtonState(FALSE);
                return;
            }

            pApp->OnToolsRunSim();
        }
        UpdateSimButtonState(pApp->IsSimRunning());
    }
}

void CCharInputPanel::UpdateSimButtonState(BOOL bRunning)
{
    if (m_btnSimControl.GetSafeHwnd())
    {
        m_btnSimControl.SetWindowText(bRunning ? _T("시뮬레이션 중단") : _T("시뮬레이션 시작"));
    }
}

BOOL CCharInputPanel::ParseSimcProfile()
{
    m_editProfile.GetWindowText(m_strProfile);

    if (m_strProfile.IsEmpty())
    {
        AfxMessageBox(_T("먼저 simc 프로필을 붙여넣어 주세요."), MB_ICONWARNING);
        return FALSE;
    }

    std::string profileStr = std::string(CT2A(m_strProfile));

    CMainFrame* pFrame = static_cast<CMainFrame*>(AfxGetMainWnd());
    if (pFrame)
    {
        BOOL success = pFrame->ParseSimcProfile(profileStr);

        if (!success)
        {
            AfxMessageBox(_T("프로필 파싱 실패."), MB_ICONERROR);
        }
        return success;
    }
    return FALSE;
}

void CCharInputPanel::ClearProfile()
{
    m_strProfile.Empty();
    m_editProfile.SetWindowText(_T(""));
    ClearCharacterInfo();
}

void CCharInputPanel::DisplayCharacterInfo(const CCharacterData* pCharData)
{
    UNREFERENCED_PARAMETER(pCharData);
}

void CCharInputPanel::ClearCharacterInfo() {}
void CCharInputPanel::PopulateTrinketCombos(BOOL preserveSelection) { UNREFERENCED_PARAMETER(preserveSelection); }
void CCharInputPanel::PopulateItemLevelPresets(CComboBox& combo, const CTrinketOption* option, BOOL preserveSelection) { UNREFERENCED_PARAMETER(combo); UNREFERENCED_PARAMETER(option); UNREFERENCED_PARAMETER(preserveSelection); }
void CCharInputPanel::UpdateTrinketControlState() {}
void CCharInputPanel::UpdateTrinketSelectionSummary() {}
void CCharInputPanel::SyncTrinketSelectionsToCharacterData(CCharacterData* pCharData) const { UNREFERENCED_PARAMETER(pCharData); }
int CCharInputPanel::GetSelectedTrinketEntryId(const CComboBox& combo) const { UNREFERENCED_PARAMETER(combo); return 0; }
int CCharInputPanel::GetSelectedFilterIndex() const { return 0; }
double CCharInputPanel::GetSelectedItemLevel(const CComboBox& combo) const { UNREFERENCED_PARAMETER(combo); return 0.0; }
