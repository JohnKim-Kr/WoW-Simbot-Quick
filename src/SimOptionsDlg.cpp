#include "pch.h"
#include "framework.h"
#include "SimbotQuick.h"
#include "SimOptionsDlg.h"
#include "resource.h"
#include <filesystem>
namespace fs = std::filesystem;

IMPLEMENT_DYNAMIC(CSimOptionsDlg, CDialogEx)

CSimOptionsDlg::CSimOptionsDlg(CWnd* pParent /*=nullptr*/)
    : CDialogEx(IDD_SIMOPTIONS, pParent)
    , m_bDownloading(false)
{
    m_pDownloader = std::make_unique<CSimcDownloader>();
}

CSimOptionsDlg::~CSimOptionsDlg()
{
    if (m_bDownloading && m_pDownloader)
    {
        m_pDownloader->Cancel();
    }
    if (m_downloadThread.joinable())
    {
        m_downloadThread.join();
    }
}

void CSimOptionsDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialogEx::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_EDIT_SIMC_PATH, m_editSimcPath);
    DDX_Control(pDX, IDC_BUTTON_BROWSE_SIMC, m_btnBrowse);
    DDX_Control(pDX, IDC_BUTTON_SAVE_SETTINGS, m_btnSave);
    DDX_Control(pDX, IDC_BUTTON_DOWNLOAD_SIMC, m_btnDownload);
    DDX_Control(pDX, IDC_PROGRESS_DOWNLOAD, m_progressDownload);
    DDX_Control(pDX, IDC_STATIC_DOWNLOAD_STATUS, m_staticStatus);
    DDX_Text(pDX, IDC_EDIT_SIMC_PATH, m_strSimcPath);
}

BEGIN_MESSAGE_MAP(CSimOptionsDlg, CDialogEx)
    ON_BN_CLICKED(IDC_BUTTON_BROWSE_SIMC, &CSimOptionsDlg::OnBnClickedButtonBrowseSimc)
    ON_BN_CLICKED(IDC_BUTTON_SAVE_SETTINGS, &CSimOptionsDlg::OnBnClickedButtonSaveSettings)
    ON_BN_CLICKED(IDC_BUTTON_DOWNLOAD_SIMC, &CSimOptionsDlg::OnBnClickedButtonDownloadSimc)
    ON_MESSAGE(WM_UPDATE_DOWNLOAD_PROGRESS, &CSimOptionsDlg::OnUpdateDownloadProgress)
    ON_MESSAGE(WM_DOWNLOAD_COMPLETE, &CSimOptionsDlg::OnDownloadComplete)
END_MESSAGE_MAP()

BOOL CSimOptionsDlg::OnInitDialog()
{
    CDialogEx::OnInitDialog();

    LoadSettings();

    m_progressDownload.SetRange(0, 100);
    m_progressDownload.SetPos(0);

    UpdateData(FALSE);

    return TRUE;
}

void CSimOptionsDlg::OnCancel()
{
    if (m_bDownloading)
    {
        if (m_pDownloader)
        {
            m_pDownloader->Cancel();
        }
        return;
    }

    CDialogEx::OnCancel();
}

void CSimOptionsDlg::LoadSettings()
{
    CWoWSimbotQuickApp* pApp = static_cast<CWoWSimbotQuickApp*>(AfxGetApp());

    m_strSimcPath = pApp->m_strSimcPath;

    if (m_strSimcPath.IsEmpty())
    {
        CString defaultPath = CSimcDownloader::GetDefaultInstallPath() + _T("\\latest\\simc.exe");
        if (PathFileExists(defaultPath))
        {
            m_strSimcPath = defaultPath;
        }
    }
}

void CSimOptionsDlg::SaveSettings()
{
    CWoWSimbotQuickApp* pApp = static_cast<CWoWSimbotQuickApp*>(AfxGetApp());

    pApp->m_strSimcPath = m_strSimcPath;

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

    if (!m_strSimcPath.IsEmpty() && !fs::exists(std::filesystem::path(std::wstring(m_strSimcPath))))
    {
        AfxMessageBox(_T("The specified simc.exe path does not exist."), MB_ICONWARNING);
        return;
    }

    SaveSettings();

    AfxMessageBox(_T("Settings saved successfully."), MB_ICONINFORMATION);
    EndDialog(IDOK);
}

void CSimOptionsDlg::OnBnClickedButtonDownloadSimc()
{
    if (m_bDownloading)
        return;

    CString version, url;
    m_staticStatus.SetWindowText(_T("Checking latest version..."));
    m_btnDownload.EnableWindow(FALSE);
    m_progressDownload.SetPos(5);

    // 버퍼 크기 제한 메시지
    TRACE(_T("Starting version check...\n"));

    if (!m_pDownloader->CheckLatestVersion(version, url))
    {
        m_staticStatus.SetWindowText(_T("Failed to check version"));
        m_btnDownload.EnableWindow(TRUE);
        m_progressDownload.SetPos(0);

        // 수동 다운로드 안내
        CString msg;
        msg = _T("Failed to download simc automatically.\n\n")
              _T("Please download and install manually:\n\n")
              _T("1. Visit: http://downloads.simulationcraft.org/nightly/\n")
              _T("2. Download the latest 'simc-*-win64.7z' file\n")
              _T("3. Install 7-Zip from https://www.7-zip.org/ (if not installed)\n")
              _T("4. Extract the .7z file to:\n   ") + CSimcDownloader::GetDefaultInstallPath() + _T("\n\n")
              _T("Or click 'Browse...' to select an existing simc.exe\n\n")
              _T("Open download page in browser?");

        if (AfxMessageBox(msg, MB_YESNO | MB_ICONQUESTION) == IDYES)
        {
            ShellExecute(NULL, _T("open"), _T("http://downloads.simulationcraft.org/nightly/"),
                NULL, NULL, SW_SHOWNORMAL);
        }
        return;
    }

    CString installDir = CSimcDownloader::GetDefaultInstallPath();
    CString versionDir = installDir + _T("\\") + version;
    CString simcPath = versionDir + _T("\\simc.exe");

    if (PathFileExists(simcPath))
    {
        CString msg;
        msg.Format(_T("Version %s is already installed.\nDo you want to reinstall?"), version);
        if (AfxMessageBox(msg, MB_YESNO | MB_ICONQUESTION) == IDNO)
        {
            m_strSimcPath = simcPath;
            UpdateData(FALSE);
            m_staticStatus.SetWindowText(_T("Using existing installation"));
            m_btnDownload.EnableWindow(TRUE);
            return;
        }
    }

    m_bDownloading = true;
    m_progressDownload.SetPos(0);
    m_staticStatus.SetWindowText(_T("Starting download..."));

    m_downloadThread = std::thread([this, installDir]()
    {
        CString simcPath;
        BOOL success = m_pDownloader->DownloadAndInstall(
            installDir,
            simcPath,
            [this](int percent, const CString& status)
            {
                CString* pStatus = new CString(status);
                PostMessage(WM_UPDATE_DOWNLOAD_PROGRESS, (WPARAM)percent, (LPARAM)pStatus);
            });

        // simc.exe 경로를 LPARAM으로 전달 (성공한 경우에만)
        CString* pPath = success ? new CString(simcPath) : nullptr;
        PostMessage(WM_DOWNLOAD_COMPLETE, (WPARAM)success, (LPARAM)pPath);
    });
}

void CSimOptionsDlg::OnDownloadComplete(BOOL success, CString* pSimcPath)
{
    m_bDownloading = false;
    m_btnDownload.EnableWindow(TRUE);

    if (success && pSimcPath && !pSimcPath->IsEmpty())
    {
        // 다운로더에서 반환된 경로 사용
        m_strSimcPath = *pSimcPath;
        delete pSimcPath;

        UpdateData(FALSE);
        m_staticStatus.SetWindowText(_T("Download complete!"));
        AfxMessageBox(_T("simc has been downloaded and installed successfully!"), MB_ICONINFORMATION);
    }
    else
    {
        if (pSimcPath) delete pSimcPath;

        m_staticStatus.SetWindowText(_T("Download failed"));

        // 수동 다운로드 안내
        CString msg;
        msg = _T("Automatic download failed.\n\n")
              _T("Please download simc manually:\n")
              _T("1. Visit: https://github.com/simulationcraft/simc/releases\n")
              _T("2. Download the latest 'simc-*-win64.7z' file\n")
              _T("3. Extract to: ") + CSimcDownloader::GetDefaultInstallPath() + _T("\n\n")
              _T("Would you like to open the download page in your browser?");

        if (AfxMessageBox(msg, MB_YESNO | MB_ICONQUESTION) == IDYES)
        {
            ShellExecute(NULL, _T("open"), _T("https://github.com/simulationcraft/simc/releases"),
                NULL, NULL, SW_SHOWNORMAL);
        }
    }

    if (m_downloadThread.joinable())
    {
        m_downloadThread.join();
    }
}

LRESULT CSimOptionsDlg::OnUpdateDownloadProgress(WPARAM wParam, LPARAM lParam)
{
    int percent = (int)wParam;
    CString* pStatus = (CString*)lParam;

    m_progressDownload.SetPos(percent);
    if (pStatus)
    {
        m_staticStatus.SetWindowText(*pStatus);
        delete pStatus;
    }

    return 0;
}

LRESULT CSimOptionsDlg::OnDownloadComplete(WPARAM wParam, LPARAM lParam)
{
    CString* pSimcPath = (CString*)lParam;
    OnDownloadComplete((BOOL)wParam, pSimcPath);
    return 0;
}
