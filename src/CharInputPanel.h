#pragma once

class CCharacterData;
class CSimSettingsPanel;

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

protected:
    // Controls
    CStatic     m_staticLabel;
    CEdit       m_editProfile;
    CButton     m_btnParse;
    CButton     m_btnClear;
    CButton     m_btnSettings;
    CButton     m_btnSimControl;

    // Data
    CString     m_strProfile;

    DECLARE_MESSAGE_MAP()

public:
    afx_msg void OnBnClickedButtonParse();
    afx_msg void OnBnClickedButtonClear();
    afx_msg void OnBnClickedButtonSettings();
    afx_msg void OnBnClickedButtonSimControl();

    void UpdateSimButtonState(BOOL bRunning);

    void ParseSimcProfile();
    void ClearProfile();
    void DisplayCharacterInfo(const CCharacterData* pCharData);
    void ClearCharacterInfo();

    CString GetProfileString() const { return m_strProfile; }
};
