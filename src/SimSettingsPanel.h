#pragma once

class CCharacterData;
struct CTrinketOption;

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
    afx_msg void OnSize(UINT nType, int cx, int cy);

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
    CButton     m_checkArcaneIntellect;
    CButton     m_checkPowerWordFortitude;
    CButton     m_checkBattleShout;
    CButton     m_checkMysticTouch;
    CButton     m_checkChaosBrand;
    CButton     m_checkSkyfury;
    CButton     m_checkMarkOfTheWild;
    CButton     m_checkHuntersMark;
    CButton     m_checkBleeding;
    CButton     m_checkCalculateScaleFactors;
    CButton     m_checkReportDetails;
    CButton     m_checkSingleActorBatch;
    CButton     m_checkOptimizeExpressions;
    CEdit       m_editScaleOnly;
    CEdit       m_editTargetError;
    CStatic     m_staticTrinketHeader;
    CStatic     m_staticTrinketFilterLabel;
    CComboBox   m_comboTrinketFilter;
    CButton     m_checkTrinket1;
    CComboBox   m_comboTrinket1;
    CStatic     m_staticTrinket1BonusLabel;
    CEdit       m_editTrinket1BonusId;
    CStatic     m_staticTrinket1ContextLabel;
    CEdit       m_editTrinket1Context;
    CStatic     m_staticTrinket1ItemLevelLabel;
    CEdit       m_editTrinket1ItemLevel;
    CStatic     m_staticTrinket1Info;
    CButton     m_checkTrinket2;
    CComboBox   m_comboTrinket2;
    CStatic     m_staticTrinket2BonusLabel;
    CEdit       m_editTrinket2BonusId;
    CStatic     m_staticTrinket2ContextLabel;
    CEdit       m_editTrinket2Context;
    CStatic     m_staticTrinket2ItemLevelLabel;
    CEdit       m_editTrinket2ItemLevel;
    CStatic     m_staticTrinket2Info;
    CStatic     m_staticTrinketStatus;

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
    BOOL    m_bArcaneIntellect;
    BOOL    m_bPowerWordFortitude;
    BOOL    m_bBattleShout;
    BOOL    m_bMysticTouch;
    BOOL    m_bChaosBrand;
    BOOL    m_bSkyfury;
    BOOL    m_bMarkOfTheWild;
    BOOL    m_bHuntersMark;
    BOOL    m_bBleeding;
    BOOL    m_bCalculateScaleFactors;
    BOOL    m_bReportDetails;
    BOOL    m_bSingleActorBatch;
    BOOL    m_bOptimizeExpressions;
    CString m_strScaleOnly;
    double  m_dTargetError;
    CString m_strTrinket1BonusId;
    CString m_strTrinket2BonusId;
    CString m_strTrinket1Context;
    CString m_strTrinket2Context;
    CString m_strTrinket1ItemLevel;
    CString m_strTrinket2ItemLevel;
    int     m_selectedTrinket1EntryId;
    int     m_selectedTrinket2EntryId;

    DECLARE_MESSAGE_MAP()

public:
    afx_msg void OnCbnSelchangeComboFightStyle();
    afx_msg void OnEnChangeEditDuration();
    afx_msg void OnEnChangeEditIterations();
    afx_msg void OnEnChangeEditScaleOnly();
    afx_msg void OnEnChangeEditTargetError();
    afx_msg void OnCbnSelchangeComboTargets();
    afx_msg void OnBnClickedCheckBuffs();
    afx_msg void OnBnClickedCheckTrinket1();
    afx_msg void OnBnClickedCheckTrinket2();
    afx_msg void OnCbnSelchangeComboTrinketFilter();
    afx_msg void OnCbnSelchangeComboTrinket1();
    afx_msg void OnCbnSelchangeComboTrinket2();
    afx_msg void OnEnChangeEditTrinket1BonusId();
    afx_msg void OnEnChangeEditTrinket2BonusId();
    afx_msg void OnEnChangeEditTrinket1Context();
    afx_msg void OnEnChangeEditTrinket2Context();
    afx_msg void OnEnChangeEditTrinket1ItemLevel();
    afx_msg void OnEnChangeEditTrinket2ItemLevel();

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
    BOOL ValidateTrinketInputs(CString& errorMessage) const;
    void DisplayCharacterInfo(const CCharacterData* pCharData);
    void PopulateTrinketCombos(BOOL preserveSelection = TRUE);
    void ApplyTrinketOptionDefaults(int slotIndex, const CTrinketOption* option);
    void LayoutTrinketControls(int panelWidth);
    void UpdateTrinketControlState();
    void UpdateTrinketSelectionSummary();
    void SyncTrinketSelectionsToCharacterData(CCharacterData* pCharData) const;
    int GetSelectedTrinketEntryId(const CComboBox& combo) const;
    int GetSelectedFilterIndex() const;
};
