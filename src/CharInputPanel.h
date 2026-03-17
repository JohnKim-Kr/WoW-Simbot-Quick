#pragma once

class CCharacterData;
class CSimSettingsPanel;
struct CTrinketOption;

// Character input panel - allows user to input simc profile string
// and displays character information after parsing
class CCharInputPanel : public CView
{
    DECLARE_DYNCREATE(CCharInputPanel)

public:
    CCharInputPanel();
    virtual ~CCharInputPanel();

protected:
    virtual void OnDraw(CDC* pDC);
    virtual void OnInitialUpdate();
    afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
    afx_msg void OnSize(UINT nType, int cx, int cy);
    afx_msg void OnBnClickedCheckTrinket1();
    afx_msg void OnBnClickedCheckTrinket2();
    afx_msg void OnCbnSelchangeComboTrinketFilter();
    afx_msg void OnCbnSelchangeComboTrinket1();
    afx_msg void OnCbnSelchangeComboTrinket2();
    afx_msg void OnCbnSelchangeComboTrinket1ItemLevel();
    afx_msg void OnCbnSelchangeComboTrinket2ItemLevel();

protected:
    // Controls
    CStatic     m_staticLabel;
    CEdit       m_editProfile;
    CButton     m_btnClear;
    CButton     m_btnSettings;
    CButton     m_btnSimControl;
    CStatic     m_staticTrinketHeader;
    CStatic     m_staticTrinketFilterLabel;
    CComboBox   m_comboTrinketFilter;
    CButton     m_checkTrinket1;
    CComboBox   m_comboTrinket1;
    CComboBox   m_comboTrinket1ItemLevel;
    CStatic     m_staticTrinket1Info;
    CButton     m_checkTrinket2;
    CComboBox   m_comboTrinket2;
    CComboBox   m_comboTrinket2ItemLevel;
    CStatic     m_staticTrinket2Info;
    CStatic     m_staticTrinketStatus;

    // Data
    CString     m_strProfile;
    int         m_selectedTrinket1EntryId;
    int         m_selectedTrinket2EntryId;

    DECLARE_MESSAGE_MAP()

public:
    afx_msg void OnBnClickedButtonClear();
    afx_msg void OnBnClickedButtonSettings();
    afx_msg void OnBnClickedButtonSimControl();

    void UpdateSimButtonState(BOOL bRunning);

    BOOL ParseSimcProfile();
    void ClearProfile();
    void DisplayCharacterInfo(const CCharacterData* pCharData);
    void ClearCharacterInfo();
    void PopulateTrinketCombos(BOOL preserveSelection = TRUE);
    void PopulateItemLevelPresets(CComboBox& combo, const CTrinketOption* option, BOOL preserveSelection = TRUE);
    void UpdateTrinketControlState();
    void UpdateTrinketSelectionSummary();
    void SyncTrinketSelectionsToCharacterData(CCharacterData* pCharData) const;
    int GetSelectedTrinketEntryId(const CComboBox& combo) const;
    int GetSelectedFilterIndex() const;
    double GetSelectedItemLevel(const CComboBox& combo) const;

    CString GetProfileString() const { return m_strProfile; }
};
