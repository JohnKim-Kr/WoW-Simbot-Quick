#pragma once

// Simulation settings panel - allows user to configure simulation parameters
class CSimSettingsPanel : public CFormView
{
    DECLARE_DYNCREATE(CSimSettingsPanel)

public:
    CSimSettingsPanel();
    virtual ~CSimSettingsPanel();

#ifdef AFX_DESIGN_TIME
    enum { IDD = IDD_FORMVIEW_SIM_SETTINGS };
#endif

protected:
    virtual void DoDataExchange(CDataExchange* pDX);
    virtual void OnInitialUpdate();

protected:
    // Controls
    CComboBox   m_comboFightStyle;
    CEdit       m_editDuration;
    CEdit       m_editIterations;
    CComboBox   m_comboTargets;
    CButton     m_checkFlask;
    CButton     m_checkFood;
    CButton     m_checkRune;
    CButton     m_checkAugment;
    CButton     m_checkBloodlust;
    CButton     m_checkPotion;

    // Data
    CString m_strFightStyle;
    int     m_nDuration;
    int     m_nIterations;
    int     m_nTargetCount;
    BOOL    m_bFlask;
    BOOL    m_bFood;
    BOOL    m_bRune;
    BOOL    m_bAugment;
    BOOL    m_bBloodlust;
    BOOL    m_bPotion;

    DECLARE_MESSAGE_MAP()

public:
    afx_msg void OnCbnSelchangeComboFightStyle();
    afx_msg void OnEnChangeEditDuration();
    afx_msg void OnEnChangeEditIterations();
    afx_msg void OnCbnSelchangeComboTargets();
    afx_msg void OnBnClickedCheckBuffs();

    // Getters for settings
    CString GetFightStyle() const { return m_strFightStyle; }
    int GetDuration() const { return m_nDuration; }
    int GetIterations() const { return m_nIterations; }
    int GetTargetCount() const { return m_nTargetCount; }
    BOOL GetUseFlask() const { return m_bFlask; }
    BOOL GetUseFood() const { return m_bFood; }
    BOOL GetUseRune() const { return m_bRune; }
    BOOL GetUseAugment() const { return m_bAugment; }
    BOOL GetUseBloodlust() const { return m_bBloodlust; }
    BOOL GetUsePotion() const { return m_bPotion; }

    void LoadDefaults();
    void LoadSettingsFromManager();
    void SaveSettingsToManager();
};
