#pragma once

// Results panel - displays simulation results and progress
class CResultsPanel : public CFormView
{
    DECLARE_DYNCREATE(CResultsPanel)

public:
    CResultsPanel();
    virtual ~CResultsPanel();

#ifdef AFX_DESIGN_TIME
    enum { IDD = IDD_FORMVIEW_RESULTS };
#endif

protected:
    virtual void DoDataExchange(CDataExchange* pDX);
    virtual void OnInitialUpdate();

protected:
    // Controls
    CProgressCtrl   m_progressSim;
    CEdit           m_editDPS;
    CEdit           m_editSimTime;
    CListCtrl       m_listResults;
    CButton         m_btnCopyResult;
    CButton         m_btnExportJson;
    CButton         m_btnExportCsv;

    // Data
    CString m_strDPS;
    CString m_strSimTime;
    CString m_strResultJson;

    DECLARE_MESSAGE_MAP()

public:
    afx_msg void OnBnClickedButtonCopyResult();
    afx_msg void OnBnClickedButtonExportJson();
    afx_msg void OnBnClickedButtonExportCsv();

    void SetProgress(int percent);
    void LoadResults(const CString& resultJson);
    void ClearResults();
    void UpdateResultDisplay();

    // Parse simc JSON result
    BOOL ParseSimcJson(const CString& jsonFile);

    // Display detailed result
    void DisplayResult(const class CSimResult& result);
};
