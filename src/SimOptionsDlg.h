#pragma once

#include "SimcDownloader.h"
#include <thread>
#include <atomic>

// SimOptionsDlg dialog - application settings
class CSimOptionsDlg : public CDialogEx
{
    DECLARE_DYNAMIC(CSimOptionsDlg)

public:
    CSimOptionsDlg(CWnd* pParent = nullptr);
    virtual ~CSimOptionsDlg();

// Dialog Data
#ifdef AFX_DESIGN_TIME
    enum { IDD = IDD_SIMOPTIONS };
#endif

protected:
    CEdit   m_editSimcPath;
    CButton m_btnBrowse;
    CButton m_btnSave;
    CButton m_btnDownload;
    CProgressCtrl m_progressDownload;
    CStatic m_staticStatus;

    CString m_strSimcPath;

    // 다운로드 관련
    std::unique_ptr<CSimcDownloader> m_pDownloader;
    std::thread m_downloadThread;
    std::atomic<bool> m_bDownloading;

protected:
    virtual void DoDataExchange(CDataExchange* pDX);
    virtual BOOL OnInitDialog();
    virtual void OnCancel();

    DECLARE_MESSAGE_MAP()

public:
    afx_msg void OnBnClickedButtonBrowseSimc();
    afx_msg void OnBnClickedButtonSaveSettings();
    afx_msg void OnBnClickedButtonDownloadSimc();

    void LoadSettings();
    void SaveSettings();

    // 다운로드 진행 콜백
    void OnDownloadProgress(int percent, const CString& status);
    void OnDownloadComplete(BOOL success, CString* pSimcPath = nullptr);

    // UI 업데이트 (메인 스레드에서 호출)
    afx_msg LRESULT OnUpdateDownloadProgress(WPARAM wParam, LPARAM lParam);
    afx_msg LRESULT OnDownloadComplete(WPARAM wParam, LPARAM lParam);

    static constexpr UINT WM_UPDATE_DOWNLOAD_PROGRESS = WM_USER + 100;
    static constexpr UINT WM_DOWNLOAD_COMPLETE = WM_USER + 101;
};
