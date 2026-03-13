#pragma once

#include <vector>

// DPS data point for graph
struct DpsDataPoint
{
    double time;        // Time in seconds
    double dps;         // DPS at that time
    double count;       // Iteration count
};

// Action/spell in rotation
struct ActionData
{
    CString name;       // Spell name
    double count;       // Number of casts
    double totalDmg;    // Total damage
    double avgDmg;      // Average damage per cast
    double dps;         // DPS contribution
    double percentage;  // % of total damage
};

// Time-ordered action entry from simc action_sequence
struct RotationEventData
{
    double time;        // Event time in seconds
    CString spellName;  // Spell or action name
    CString target;     // Target name
    CString detail;     // Amount or fallback detail
};

// DPS Graph Control
class CDpsGraphCtrl : public CStatic
{
    DECLARE_DYNAMIC(CDpsGraphCtrl)

public:
    CDpsGraphCtrl();
    virtual ~CDpsGraphCtrl();

    void SetData(const std::vector<DpsDataPoint>& data);
    void Clear();

protected:
    afx_msg void OnPaint();
    DECLARE_MESSAGE_MAP()

private:
    std::vector<DpsDataPoint> m_data;
    void DrawGraph(CDC* pDC, CRect rect);
};

// Results panel - displays simulation results, DPS graph, and rotation
class CResultsPanel : public CView
{
    DECLARE_DYNCREATE(CResultsPanel)

public:
    CResultsPanel();
    virtual ~CResultsPanel();

protected:
    virtual void OnDraw(CDC* pDC);
    virtual void OnInitialUpdate();
    afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
    afx_msg void OnSize(UINT nType, int cx, int cy);

protected:
    // Controls
    CProgressCtrl   m_progressSim;
    CEdit           m_editDPS;
    CEdit           m_editSimTime;
    CListCtrl       m_listResults;
    CButton         m_btnCopyResult;
    CButton         m_btnExportJson;
    CButton         m_btnExportCsv;

    // New controls for graph and rotation
    CTabCtrl        m_tabResults;
    CDpsGraphCtrl   m_dpsGraph;
    CListCtrl       m_listRotation;
    CListCtrl       m_listActions;
    CStatic         m_staticGraphLabel;
    CStatic         m_staticRotationLabel;

    // Data
    CString m_strDPS;
    CString m_strSimTime;
    CString m_strResultJson;
    std::vector<DpsDataPoint> m_dpsData;
    std::vector<ActionData> m_actionData;
    std::vector<RotationEventData> m_rotationData;

    DECLARE_MESSAGE_MAP()

public:
    afx_msg void OnBnClickedButtonCopyResult();
    afx_msg void OnBnClickedButtonExportJson();
    afx_msg void OnBnClickedButtonExportCsv();
    afx_msg void OnTcnSelchangeTabResults(NMHDR *pNMHDR, LRESULT *pResult);

    void SetProgress(int percent);
    void LoadResults(const CString& resultJson);
    void ClearResults();
    void UpdateResultDisplay();

    // Parse simc JSON result
    BOOL ParseSimcJson(const CString& jsonFile);
    BOOL ParseSimcJsonDetailed(const CString& jsonFile);

    // Display detailed result
    void DisplayResult(const class CSimResult& result);
    void DisplayDpsGraph();
    void DisplayRotation();
    void DisplayActions();

    // Export to CSV
    BOOL ExportResultsToCsv(const CString& filePath);
    CString FormatCsvValue(const json& j, const char* key);

private:
    void SetupTabs();
    void ShowTab(int nTab);
};
