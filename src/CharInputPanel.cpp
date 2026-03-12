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
    ON_BN_CLICKED(IDC_BUTTON_LOAD, &CCharInputPanel::OnBnClickedButtonParse)
    ON_BN_CLICKED(IDC_BUTTON_CLEAR, &CCharInputPanel::OnBnClickedButtonClear)
    ON_BN_CLICKED(ID_BUTTON_SETTINGS, &CCharInputPanel::OnBnClickedButtonSettings)
END_MESSAGE_MAP()

CCharInputPanel::CCharInputPanel()
{
}

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

    if (!m_btnParse.Create(_T("프로필 파싱"), WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        CRect(x, y, x + w/2 - 5, y + h), this, IDC_BUTTON_LOAD))
        return -1;

    if (!m_btnClear.Create(_T("초기화"), WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        CRect(x + w/2 + 5, y, x + w, y + h), this, IDC_BUTTON_CLEAR))
        return -1;

    y += h + 5;

    if (!m_btnSettings.Create(_T("시뮬레이터 설정..."), WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        CRect(x, y, x + w, y + h), this, ID_BUTTON_SETTINGS))
        return -1;

    y += h + 5;

    if (!m_staticCharInfo.Create(_T("로드된 프로필 없음"), WS_CHILD | WS_VISIBLE | SS_LEFT,
        CRect(x, y, x + w, rect.Height() - y), this, IDC_STATIC_CHAR_INFO))
        return -1;

    return 0;
}

void CCharInputPanel::OnSize(UINT nType, int cx, int cy)
{
    CView::OnSize(nType, cx, cy);

    if (m_staticLabel.GetSafeHwnd())
    {
        int x = 10, y = 10, w = cx - 20, h = 25;

        m_staticLabel.MoveWindow(x, y, w, 20);
        y += 25;
        m_editProfile.MoveWindow(x, y, w, 200);
        y += 210;
        m_btnParse.MoveWindow(x, y, w/2 - 5, h);
        m_btnClear.MoveWindow(x + w/2 + 5, y, w/2 - 5, h);
        y += h + 5;
        m_btnSettings.MoveWindow(x, y, w, h);
        y += h + 5;

        m_staticCharInfo.MoveWindow(x, y, w, cy - y - 10);
    }
}

void CCharInputPanel::OnInitialUpdate()
{
    CView::OnInitialUpdate();
}

void CCharInputPanel::OnBnClickedButtonParse()
{
    ParseSimcProfile();
}

void CCharInputPanel::OnBnClickedButtonClear()
{
    ClearProfile();
}

void CCharInputPanel::OnBnClickedButtonSettings()
{
    CWoWSimbotQuickApp* pApp = static_cast<CWoWSimbotQuickApp*>(AfxGetApp());
    if (pApp)
    {
        pApp->OnFileSettings();
    }
}

void CCharInputPanel::ParseSimcProfile()
{
    m_editProfile.GetWindowText(m_strProfile);

    if (m_strProfile.IsEmpty())
    {
        AfxMessageBox(_T("먼저 simc 프로필을 붙여넣어 주세요."), MB_ICONWARNING);
        return;
    }

    std::string profileStr = std::string(CT2A(m_strProfile));

    CMainFrame* pFrame = static_cast<CMainFrame*>(AfxGetMainWnd());
    if (pFrame)
    {
        m_btnParse.EnableWindow(FALSE);
        BOOL success = pFrame->ParseSimcProfile(profileStr);
        m_btnParse.EnableWindow(TRUE);

        if (success)
        {
            AfxMessageBox(_T("프로필 파싱 성공!"), MB_ICONINFORMATION);
        }
        else
        {
            AfxMessageBox(_T("프로필 파싱 실패."), MB_ICONERROR);
        }
    }
}

void CCharInputPanel::ClearProfile()
{
    m_strProfile.Empty();
    m_editProfile.SetWindowText(_T(""));
    ClearCharacterInfo();
}

void CCharInputPanel::DisplayCharacterInfo(const CCharacterData* pCharData)
{
    if (!pCharData)
    {
        ClearCharacterInfo();
        return;
    }

    CString info;
    info.Format(_T("캐릭터: %s\r\n")
                _T("직업: %s\r\n")
                _T("전문화: %s\r\n")
                _T("아이템 레벨: %.1f\r\n")
                _T("서버: %s-%s"),
        CString(pCharData->GetName().c_str()),
        CString(pCharData->GetClassName().c_str()),
        CString(pCharData->GetActiveSpecName().c_str()),
        pCharData->GetItemLevel(),
        CString(pCharData->GetRegion().c_str()),
        CString(pCharData->GetRealm().c_str()));

    m_staticCharInfo.SetWindowText(info);
}

void CCharInputPanel::ClearCharacterInfo()
{
    m_staticCharInfo.SetWindowText(_T("로드된 프로필 없음"));
}
