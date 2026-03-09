#pragma once

#include "CharInputPanel.h"
#include "SimSettingsPanel.h"
#include "ResultsPanel.h"
#include "BnetApiClient.h"
#include "CharacterData.h"
#include "SimcRunner.h"

// Main frame window class
class CMainFrame : public CFrameWnd
{
    DECLARE_DYNAMIC(CMainFrame)

protected:
    CSplitterWnd m_wndSplitter;

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
    CReBar      m_wndReBar;
    CDialogBar  m_wndDlgBar;

    // Panel views
    CCharInputPanel*    m_pCharInputPanel;
    CSimSettingsPanel*  m_pSimSettingsPanel;
    CResultsPanel*      m_pResultsPanel;

    // Core components
    std::unique_ptr<CBnetApiClient> m_pApiClient;
    std::unique_ptr<CSimcRunner>   m_pSimcRunner;
    std::unique_ptr<CCharacterData> m_pCharacterData;

    // Generated message map functions
protected:
    afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
    afx_msg void OnSetFocus(CWnd* pOldWnd);
    afx_msg void OnSize(UINT nType, int cx, int cy);
    afx_msg LRESULT OnUserApiCharacterLoaded(WPARAM wParam, LPARAM lParam);
    afx_msg LRESULT OnUserSimProgress(WPARAM wParam, LPARAM lParam);
    afx_msg LRESULT OnUserSimComplete(WPARAM wParam, LPARAM lParam);
    afx_msg void OnSettingsChanged();
    DECLARE_MESSAGE_MAP()

public:
    // Character loading
    BOOL LoadCharacter(const CString& region, const CString& server, const CString& characterName);
    void OnCharacterLoaded();

    // Simulation control
    void StartSimulation();
    void StopSimulation();
    void OnSimulationComplete(const CString& resultJson);

    // API Access
    CBnetApiClient* GetApiClient() const { return m_pApiClient.get(); }
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
#define WM_USER_API_CHAR_LOADED (WM_USER + 100)
#define WM_USER_SIM_PROGRESS    (WM_USER + 101)
#define WM_USER_SIM_COMPLETE    (WM_USER + 102)
