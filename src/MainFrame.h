#pragma once

#include "CharInputPanel.h"
#include "SimSettingsPanel.h"
#include "ResultsPanel.h"
#include "CharacterData.h"
#include "SimcRunner.h"

// Main frame window class
class CMainFrame : public CFrameWnd
{
    DECLARE_DYNAMIC(CMainFrame)

public:
    CMainFrame() noexcept;
    virtual ~CMainFrame();

    // Overrides
public:
    virtual BOOL OnCreateClient(LPCREATESTRUCT lpcs, CCreateContext* pContext);
    virtual BOOL PreCreateWindow(CREATESTRUCT& cs);

    // Implementation
public:
#ifdef _DEBUG
    virtual void AssertValid() const;
    virtual void Dump(CDumpContext& dc) const;
#endif

protected:
    CStatusBar  m_wndStatusBar;
    CToolBar    m_wndToolBar;

    // Panel views
    CCharInputPanel*    m_pCharInputPanel;
    CSimSettingsPanel*  m_pSimSettingsPanel;
    CResultsPanel*      m_pResultsPanel;

    // Core components
    std::unique_ptr<CSimcRunner>   m_pSimcRunner;
    std::unique_ptr<CCharacterData> m_pCharacterData;

    // Generated message map functions
protected:
    afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
    afx_msg void OnSetFocus(CWnd* pOldWnd);
    afx_msg void OnSize(UINT nType, int cx, int cy);
    afx_msg LRESULT OnUserSimProgress(WPARAM wParam, LPARAM lParam);
    afx_msg LRESULT OnUserSimComplete(WPARAM wParam, LPARAM lParam);
    afx_msg void OnSettingsChanged();
    DECLARE_MESSAGE_MAP()

public:
    // Profile parsing from simc addon
    BOOL ParseSimcProfile(const std::string& profileStr);
    void OnCharacterLoaded();

    // Simulation control
    void StartSimulation();
    void StopSimulation();
    void OnSimulationComplete(const CString& resultJson);

    // Character data access
    CCharacterData* GetCharacterData() const { return m_pCharacterData.get(); }

    // UI Updates
    void UpdateStatus(const CString& message);
    void SetProgress(int percent);

    // Sim options access
    CString GetFightStyle() const;
    int GetDuration() const;
    int GetIterations() const;
    int GetTargetCount() const;
    BOOL GetUseFlask() const;
    BOOL GetUseFood() const;
    BOOL GetUseRune() const;
    BOOL GetUseAugment() const;
    BOOL GetUseBloodlust() const;
    BOOL GetUsePotion() const;
};

// Custom window messages
#define WM_USER_SIM_PROGRESS    (WM_USER + 101)
#define WM_USER_SIM_COMPLETE    (WM_USER + 102)
