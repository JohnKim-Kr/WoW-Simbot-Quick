#pragma once

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
    CEdit   m_editClientId;
    CEdit   m_editClientSecret;
    CButton m_btnBrowse;
    CButton m_btnSave;

    CString m_strSimcPath;
    CString m_strClientId;
    CString m_strClientSecret;

protected:
    virtual void DoDataExchange(CDataExchange* pDX);
    virtual BOOL OnInitDialog();

    DECLARE_MESSAGE_MAP()

public:
    afx_msg void OnBnClickedButtonBrowseSimc();
    afx_msg void OnBnClickedButtonSaveSettings();

    void LoadSettings();
    void SaveSettings();
};
