#include "pch.h"
#include "framework.h"
#include "SimbotQuick.h"
#include "MainFrame.h"
#include "CharInputPanel.h"
#include "SimSettingsPanel.h"
#include "CharacterData.h"
#include "resource.h"

IMPLEMENT_DYNCREATE(CCharInputPanel, CFormView)

BEGIN_MESSAGE_MAP(CCharInputPanel, CFormView)
    ON_BN_CLICKED(IDC_BUTTON_LOAD, &CCharInputPanel::OnBnClickedButtonLoad)
    ON_BN_CLICKED(IDC_BUTTON_OAUTH, &CCharInputPanel::OnBnClickedButtonOAuth)
    ON_CBN_SELCHANGE(IDC_COMBO_REGION, &CCharInputPanel::OnCbnSelchangeComboRegion)
    ON_EN_CHANGE(IDC_EDIT_SERVER, &CCharInputPanel::OnEnChangeEditServer)
    ON_EN_CHANGE(IDC_EDIT_CHARACTER, &CCharInputPanel::OnEnChangeEditCharacter)
END_MESSAGE_MAP()

CCharInputPanel::CCharInputPanel()
    : CFormView(IDD_FORMVIEW_CHAR_INPUT)
    , m_pNestedSplitter(nullptr)
{
}

CCharInputPanel::~CCharInputPanel()
{
    delete m_pNestedSplitter;
}

void CCharInputPanel::DoDataExchange(CDataExchange* pDX)
{
    CFormView::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_COMBO_REGION, m_comboRegion);
    DDX_Control(pDX, IDC_EDIT_SERVER, m_editServer);
    DDX_Control(pDX, IDC_EDIT_CHARACTER, m_editCharacter);
    DDX_Control(pDX, IDC_BUTTON_LOAD, m_btnLoad);
    DDX_Control(pDX, IDC_BUTTON_OAUTH, m_btnOAuth);
    DDX_Control(pDX, IDC_STATIC_CHAR_INFO, m_staticCharInfo);
    DDX_Control(pDX, IDC_STATIC_ITEM_LEVEL, m_staticItemLevel);
    DDX_Control(pDX, IDC_STATIC_CLASS_SPEC, m_staticClassSpec);
    DDX_Text(pDX, IDC_EDIT_SERVER, m_strServer);
    DDX_Text(pDX, IDC_EDIT_CHARACTER, m_strCharacter);
}

void CCharInputPanel::OnInitialUpdate()
{
    CFormView::OnInitialUpdate();

    // Initialize region combo box
    m_comboRegion.AddString(_T("kr"));
    m_comboRegion.AddString(_T("us"));
    m_comboRegion.AddString(_T("eu"));
    m_comboRegion.AddString(_T("tw"));
    m_comboRegion.SetCurSel(0);  // Default to Korea
    m_strRegion = _T("kr");

    // Set initial text for info labels
    m_staticCharInfo.SetWindowText(_T("No character loaded"));
    m_staticItemLevel.SetWindowText(_T("Item Level: --"));
    m_staticClassSpec.SetWindowText(_T("Class/Spec: --"));

    UpdateData(FALSE);
}

void CCharInputPanel::OnBnClickedButtonLoad()
{
    UpdateData(TRUE);

    if (m_strServer.IsEmpty() || m_strCharacter.IsEmpty())
    {
        AfxMessageBox(_T("Please enter server and character name."), MB_ICONWARNING);
        return;
    }

    // Get the main frame and call load character
    CMainFrame* pFrame = static_cast<CMainFrame*>(AfxGetMainWnd());
    if (pFrame)
    {
        m_btnLoad.EnableWindow(FALSE);
        m_staticCharInfo.SetWindowText(_T("Loading..."));
        pFrame->LoadCharacter(m_strRegion, m_strServer, m_strCharacter);
    }
}

void CCharInputPanel::OnBnClickedButtonOAuth()
{
    // TODO: Open OAuth dialog for Battle.net authentication
    AfxMessageBox(_T("OAuth authentication not yet implemented."), MB_ICONINFORMATION);
}

void CCharInputPanel::OnCbnSelchangeComboRegion()
{
    int sel = m_comboRegion.GetCurSel();
    if (sel >= 0)
    {
        m_comboRegion.GetLBText(sel, m_strRegion);
    }
}

void CCharInputPanel::OnEnChangeEditServer()
{
    UpdateData(TRUE);
}

void CCharInputPanel::OnEnChangeEditCharacter()
{
    UpdateData(TRUE);
}

void CCharInputPanel::DisplayCharacterInfo(const CCharacterData* pCharData)
{
    if (!pCharData)
    {
        ClearCharacterInfo();
        return;
    }

    CString info;
    info.Format(_T("%s - %s (%s)"),
        CString(pCharData->GetName().c_str()),
        CString(pCharData->GetRealm().c_str()),
        CString(pCharData->GetRegion().c_str()));
    m_staticCharInfo.SetWindowText(info);

    CString ilvl;
    ilvl.Format(_T("Item Level: %.1f"), pCharData->GetItemLevel());
    m_staticItemLevel.SetWindowText(ilvl);

    CString spec;
    spec.Format(_T("Class/Spec: %s / %s"),
        CString(pCharData->GetClassName().c_str()),
        CString(pCharData->GetActiveSpecName().c_str()));
    m_staticClassSpec.SetWindowText(spec);

    m_btnLoad.EnableWindow(TRUE);
}

void CCharInputPanel::ClearCharacterInfo()
{
    m_staticCharInfo.SetWindowText(_T("No character loaded"));
    m_staticItemLevel.SetWindowText(_T("Item Level: --"));
    m_staticClassSpec.SetWindowText(_T("Class/Spec: --"));
    m_btnLoad.EnableWindow(TRUE);
}

BOOL CCharInputPanel::CreateNestedSplitter(CSimSettingsPanel** ppSettingsPanel)
{
    // Create a nested static splitter with 1 row, 2 columns
    m_pNestedSplitter = new CSplitterWnd();

    if (!m_pNestedSplitter->CreateStatic(this, 1, 2))
    {
        TRACE0("Failed to create nested splitter\n");
        return FALSE;
    }

    CCreateContext context;
    context.m_pNewViewClass = RUNTIME_CLASS(CSimSettingsPanel);
    context.m_pCurrentFrame = AfxGetMainWnd();

    CRect rect;
    GetClientRect(&rect);

    // Create the left pane (this panel) - actually the nested splitter does this automatically
    // Create the right pane (settings panel)
    if (!m_pNestedSplitter->CreateView(0, 1, RUNTIME_CLASS(CSimSettingsPanel),
        CSize(rect.Width() / 2, rect.Height()), &context))
    {
        TRACE0("Failed to create settings panel\n");
        return FALSE;
    }

    if (ppSettingsPanel)
    {
        *ppSettingsPanel = static_cast<CSimSettingsPanel*>(m_pNestedSplitter->GetPane(0, 1));
    }

    return TRUE;
}
