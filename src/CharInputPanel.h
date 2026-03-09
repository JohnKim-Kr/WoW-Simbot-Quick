#pragma once

class CCharacterData;
class CSimSettingsPanel;

// Character input panel - allows user to input region, server, character name
// and displays character information after loading
class CCharInputPanel : public CFormView
{
    DECLARE_DYNCREATE(CCharInputPanel)

public:
    CCharInputPanel();
    virtual ~CCharInputPanel();

#ifdef AFX_DESIGN_TIME
    enum { IDD = IDD_FORMVIEW_CHAR_INPUT };
#endif

protected:
    virtual void DoDataExchange(CDataExchange* pDX);
    virtual void OnInitialUpdate();

protected:
    // Controls
    CComboBox   m_comboRegion;
    CEdit       m_editServer;
    CEdit       m_editCharacter;
    CButton     m_btnLoad;
    CButton     m_btnOAuth;
    CStatic     m_staticCharInfo;
    CStatic     m_staticItemLevel;
    CStatic     m_staticClassSpec;

    // Nested splitter for settings panel
    CSplitterWnd* m_pNestedSplitter;

    // Data
    CString m_strRegion;
    CString m_strServer;
    CString m_strCharacter;

    DECLARE_MESSAGE_MAP()

public:
    afx_msg void OnBnClickedButtonLoad();
    afx_msg void OnBnClickedButtonOAuth();
    afx_msg void OnCbnSelchangeComboRegion();
    afx_msg void OnEnChangeEditServer();
    afx_msg void OnEnChangeEditCharacter();

    void DisplayCharacterInfo(const CCharacterData* pCharData);
    void ClearCharacterInfo();

    CString GetRegion() const { return m_strRegion; }
    CString GetServer() const { return m_strServer; }
    CString GetCharacterName() const { return m_strCharacter; }

    // Creates a nested splitter for the settings panel
    BOOL CreateNestedSplitter(CSimSettingsPanel** ppSettingsPanel);
};
