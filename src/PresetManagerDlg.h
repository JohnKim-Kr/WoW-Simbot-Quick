#pragma once

// 프리셋 관리 대화상자
class CPresetManagerDlg : public CDialogEx
{
    DECLARE_DYNAMIC(CPresetManagerDlg)

public:
    CPresetManagerDlg(CWnd* pParent = nullptr);
    virtual ~CPresetManagerDlg();

#ifdef AFX_DESIGN_TIME
    enum { IDD = IDD_PRESET_MANAGER };
#endif

protected:
    CListBox    m_listPresets;
    CEdit       m_editPresetName;
    CEdit       m_editDescription;
    CButton     m_btnSave;
    CButton     m_btnLoad;
    CButton     m_btnDelete;
    CButton     m_btnClose;

    CString     m_strSelectedPreset;

protected:
    virtual void DoDataExchange(CDataExchange* pDX);
    virtual BOOL OnInitDialog();
    void RefreshPresetList();

    DECLARE_MESSAGE_MAP()

public:
    afx_msg void OnLbnSelchangeListPresets();
    afx_msg void OnBnClickedButtonSavePreset();
    afx_msg void OnBnClickedButtonLoadPreset();
    afx_msg void OnBnClickedButtonDeletePreset();
    afx_msg void OnBnClickedButtonClose();

    CString GetSelectedPreset() const { return m_strSelectedPreset; }
};