#include "pch.h"
#include "framework.h"
#include "SimbotQuick.h"
#include "SimOptionsDlg.h"
#include "resource.h"

IMPLEMENT_DYNAMIC(CSimOptionsDlg, CDialogEx)

CSimOptionsDlg::CSimOptionsDlg(CWnd* pParent /*=nullptr*/)
    : CDialogEx(IDD_SIMOPTIONS, pParent)
{
}

CSimOptionsDlg::~CSimOptionsDlg()
{
}

void CSimOptionsDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialogEx::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_EDIT_SIMC_PATH, m_editSimcPath);
    DDX_Control(pDX, IDC_EDIT_CLIENT_ID, m_editClientId);
    DDX_Control(pDX, IDC_EDIT_CLIENT_SECRET, m_editClientSecret);
    DDX_Control(pDX, IDC_BUTTON_BROWSE_SIMC, m_btnBrowse);
    DDX_Control(pDX, IDC_BUTTON_SAVE_SETTINGS, m_btnSave);
    DDX_Text(pDX, IDC_EDIT_SIMC_PATH, m_strSimcPath);
    DDX_Text(pDX, IDC_EDIT_CLIENT_ID, m_strClientId);
    DDX_Text(pDX, IDC_EDIT_CLIENT_SECRET, m_strClientSecret);
}

BEGIN_MESSAGE_MAP(CSimOptionsDlg, CDialogEx)
    ON_BN_CLICKED(IDC_BUTTON_BROWSE_SIMC, &CSimOptionsDlg::OnBnClickedButtonBrowseSimc)
    ON_BN_CLICKED(IDC_BUTTON_SAVE_SETTINGS, &CSimOptionsDlg::OnBnClickedButtonSaveSettings)
END_MESSAGE_MAP()

BOOL CSimOptionsDlg::OnInitDialog()
{
    CDialogEx::OnInitDialog();

    LoadSettings();

    UpdateData(FALSE);

    return TRUE;
}

void CSimOptionsDlg::LoadSettings()
{
    CWoWSimbotQuickApp* pApp = static_cast<CWoWSimbotQuickApp*>(AfxGetApp());

    m_strSimcPath = pApp->GetSimcPath();
    m_strClientId = pApp->GetBnetClientId();
    m_strClientSecret = pApp->GetBnetClientSecret();
}

void CSimOptionsDlg::SaveSettings()
{
    CWoWSimbotQuickApp* pApp = static_cast<CWoWSimbotQuickApp*>(AfxGetApp());

    pApp->SetSimcPath(m_strSimcPath);
    pApp->SetBnetClientId(m_strClientId);
    pApp->SetBnetClientSecret(m_strClientSecret);

    pApp->SaveSettings();
}

void CSimOptionsDlg::OnBnClickedButtonBrowseSimc()
{
    CFileDialog dlg(TRUE, _T("exe"), NULL,
        OFN_HIDEREADONLY | OFN_FILEMUSTEXIST,
        _T("Executable Files (*.exe)|*.exe|All Files (*.*)|*.*||"));

    if (dlg.DoModal() == IDOK)
    {
        m_strSimcPath = dlg.GetPathName();
        UpdateData(FALSE);
    }
}

void CSimOptionsDlg::OnBnClickedButtonSaveSettings()
{
    UpdateData(TRUE);

    // Validate paths
    if (!m_strSimcPath.IsEmpty() && !fs::exists(CT2A(m_strSimcPath)))
    {
        AfxMessageBox(_T("The specified simc.exe path does not exist."), MB_ICONWARNING);
        return;
    }

    SaveSettings();

    AfxMessageBox(_T("Settings saved successfully."), MB_ICONINFORMATION);
    EndDialog(IDOK);
}
