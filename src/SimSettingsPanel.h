#pragma once

// Simulation settings panel - allows user to configure simulation parameters
class CSimSettingsPanel : public CFormView
{
    DECLARE_DYNCREATE(CSimSettingsPanel)

protected:
    CSimSettingsPanel();
    virtual ~CSimSettingsPanel();

#ifdef AFX_DESIGN_TIME
    enum { IDD = 107 }; // Hardcoded to match RC file IDD_FORMVIEW_SIM_SETTINGS
#endif

    virtual void OnDraw(CDC* pDC) {}

public:
    BOOL CreateView(CWnd* pParent, UINT nID, CCreateContext* pContext)
    {
        return Create(NULL, NULL, WS_CHILD | WS_VISIBLE | WS_BORDER, CRect(0,0,0,0), pParent, nID, pContext);
    }

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

    CComboBox   m_comboTempEnchant;
    CButton     m_checkCrucibleViolence;
    CButton     m_checkCrucibleSustenance;
    CButton     m_checkCruciblePredation;

    // Data
    CString m_strFightStyle;
    int     m_nDuration;
    int     m_nIterations;
    CString m_strTargetCount;
    BOOL    m_bFlask;
    BOOL    m_bFood;
    BOOL    m_bRune;
    BOOL    m_bAugment;
    BOOL    m_bBloodlust;
    BOOL    m_bPotion;
    CString m_strTempEnchant;
    BOOL    m_bCrucibleViolence;
    BOOL    m_bCrucibleSustenance;
    BOOL    m_bCruciblePredation;

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
    int GetTargetCount() const { return _ttoi(m_strTargetCount); }
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
