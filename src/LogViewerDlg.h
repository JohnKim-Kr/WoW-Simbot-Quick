#pragma once

// 로그 뷰어 대화상자
class CLogViewerDlg : public CDialogEx
{
    DECLARE_DYNAMIC(CLogViewerDlg)

public:
    CLogViewerDlg(CWnd* pParent = nullptr);
    virtual ~CLogViewerDlg();

#ifdef AFX_DESIGN_TIME
    enum { IDD = IDD_LOG_VIEWER };
#endif

protected:
    CListCtrl   m_listLogs;
    CEdit       m_editDetails;
    CComboBox   m_comboLevel;
    CComboBox   m_comboCategory;
    CButton     m_btnRefresh;
    CButton     m_btnClear;
    CButton     m_btnExport;
    CButton     m_btnClose;
    CButton     m_chkAutoScroll;

    BOOL        m_bAutoScroll;

protected:
    virtual void DoDataExchange(CDataExchange* pDX);
    virtual BOOL OnInitDialog();
    void RefreshLogList();
    void FilterLogs();
    void SetupLogColumns();

    DECLARE_MESSAGE_MAP()

public:
    afx_msg void OnLvnItemchangedListLogs(NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg void OnCbnSelchangeComboLevel();
    afx_msg void OnCbnSelchangeComboCategory();
    afx_msg void OnBnClickedButtonRefresh();
    afx_msg void OnBnClickedButtonClearLogs();
    afx_msg void OnBnClickedButtonExportLogs();
    afx_msg void OnBnClickedButtonCloseLog();
    afx_msg void OnBnClickedCheckAutoscroll();
    afx_msg void OnTimer(UINT_PTR nIDEvent);

    // 실시간 로그 업데이트
    void OnNewLogEntry(const LogEntry& entry);

private:
    static const UINT LOG_REFRESH_TIMER = 1001;
    std::vector<LogEntry> m_cachedEntries;
};
