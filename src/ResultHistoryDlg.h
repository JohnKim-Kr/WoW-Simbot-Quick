#pragma once

#include "SimResult.h"

// 결과 히스토리 대화상자
class CResultHistoryDlg : public CDialogEx
{
    DECLARE_DYNAMIC(CResultHistoryDlg)

public:
    CResultHistoryDlg(CWnd* pParent = nullptr);
    virtual ~CResultHistoryDlg();

#ifdef AFX_DESIGN_TIME
    enum { IDD = IDD_RESULT_HISTORY };
#endif

protected:
    CListCtrl   m_listResults;
    CEdit       m_editDetails;
    CButton     m_btnView;
    CButton     m_btnCompare;
    CButton     m_btnDelete;
    CButton     m_btnExport;
    CButton     m_btnClose;
    CComboBox   m_comboFilter;

    // 선택된 결과 인덱스들
    std::vector<int> m_selectedIndices;

protected:
    virtual void DoDataExchange(CDataExchange* pDX);
    virtual BOOL OnInitDialog();
    void RefreshResultList();
    void UpdateDetails();
    void ExportSelectedResult();

    DECLARE_MESSAGE_MAP()

public:
    afx_msg void OnLvnItemchangedListResults(NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg void OnNMDblclkListResults(NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg void OnBnClickedButtonViewResult();
    afx_msg void OnBnClickedButtonCompare();
    afx_msg void OnBnClickedButtonDeleteResult();
    afx_msg void OnBnClickedButtonExportResult();
    afx_msg void OnBnClickedButtonCloseHistory();
    afx_msg void OnCbnSelchangeComboFilter();

    // 결과 ID 반환 (View용)
    CString GetSelectedResultId() const;
};