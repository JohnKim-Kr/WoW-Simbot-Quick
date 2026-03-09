#include "pch.h"
#include "framework.h"
#include "PresetManagerDlg.h"
#include "SettingsManager.h"
#include "resource.h"

IMPLEMENT_DYNAMIC(CPresetManagerDlg, CDialogEx)

CPresetManagerDlg::CPresetManagerDlg(CWnd* pParent /*=nullptr*/)
    : CDialogEx(IDD_PRESET_MANAGER, pParent)
{
}

CPresetManagerDlg::~CPresetManagerDlg()
{
}

void CPresetManagerDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialogEx::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_LIST_PRESETS, m_listPresets);
    DDX_Control(pDX, IDC_EDIT_PRESET_NAME, m_editPresetName);
    DDX_Control(pDX, IDC_EDIT_PRESET_DESC, m_editDescription);
    DDX_Control(pDX, IDC_BUTTON_SAVE_PRESET, m_btnSave);
    DDX_Control(pDX, IDC_BUTTON_LOAD_PRESET, m_btnLoad);
    DDX_Control(pDX, IDC_BUTTON_DELETE_PRESET, m_btnDelete);
    DDX_Control(pDX, IDC_BUTTON_CLOSE_PRESETS, m_btnClose);
}

BEGIN_MESSAGE_MAP(CPresetManagerDlg, CDialogEx)
    ON_LBN_SELCHANGE(IDC_LIST_PRESETS, &CPresetManagerDlg::OnLbnSelchangeListPresets)
    ON_BN_CLICKED(IDC_BUTTON_SAVE_PRESET, &CPresetManagerDlg::OnBnClickedButtonSavePreset)
    ON_BN_CLICKED(IDC_BUTTON_LOAD_PRESET, &CPresetManagerDlg::OnBnClickedButtonLoadPreset)
    ON_BN_CLICKED(IDC_BUTTON_DELETE_PRESET, &CPresetManagerDlg::OnBnClickedButtonDeletePreset)
    ON_BN_CLICKED(IDC_BUTTON_CLOSE_PRESETS, &CPresetManagerDlg::OnBnClickedButtonClose)
END_MESSAGE_MAP()

BOOL CPresetManagerDlg::OnInitDialog()
{
    CDialogEx::OnInitDialog();

    SetWindowText(_T("프리셋 관리"));

    RefreshPresetList();

    // 버튼 초기 상태
    m_btnLoad.EnableWindow(FALSE);
    m_btnDelete.EnableWindow(FALSE);

    return TRUE;
}

void CPresetManagerDlg::RefreshPresetList()
{
    m_listPresets.ResetContent();

    CSettingsManager* pMgr = GetSettingsManager();
    if (pMgr)
    {
        std::vector<CString> presets = pMgr->GetPresetNames();
        for (const auto& name : presets)
        {
            m_listPresets.AddString(name);
        }
    }
}

void CPresetManagerDlg::OnLbnSelchangeListPresets()
{
    int sel = m_listPresets.GetCurSel();
    if (sel >= 0)
    {
        CString presetName;
        m_listPresets.GetText(sel, presetName);
        m_editPresetName.SetWindowText(presetName);

        SettingsPreset preset;
        CSettingsManager* pMgr = GetSettingsManager();
        if (pMgr && pMgr->GetPresetInfo(presetName, preset))
        {
            m_editDescription.SetWindowText(preset.description);
        }

        m_btnLoad.EnableWindow(TRUE);
        m_btnDelete.EnableWindow(TRUE);
    }
    else
    {
        m_btnLoad.EnableWindow(FALSE);
        m_btnDelete.EnableWindow(FALSE);
    }
}

void CPresetManagerDlg::OnBnClickedButtonSavePreset()
{
    CString presetName, description;
    m_editPresetName.GetWindowText(presetName);
    m_editDescription.GetWindowText(description);

    if (presetName.IsEmpty())
    {
        AfxMessageBox(_T("프리셋 이름을 입력하세요."), MB_ICONWARNING);
        return;
    }

    // 특수문자 검사
    if (presetName.FindOneOf(_T("\\/:*?\"<>|")) >= 0)
    {
        AfxMessageBox(_T("프리셋 이름에 사용할 수 없는 문자가 포함되어 있습니다."), MB_ICONWARNING);
        return;
    }

    CSettingsManager* pMgr = GetSettingsManager();
    if (pMgr)
    {
        BOOL exists = pMgr->PresetExists(presetName);

        if (exists)
        {
            if (AfxMessageBox(_T("이미 존재하는 프리셋입니다. 덮어쓰시겠습니까?"),
                MB_YESNO | MB_ICONQUESTION) != IDYES)
            {
                return;
            }
        }

        if (pMgr->SavePreset(presetName, description))
        {
            if (!exists)
            {
                m_listPresets.AddString(presetName);
            }
            AfxMessageBox(_T("프리셋이 저장되었습니다."), MB_ICONINFORMATION);
        }
        else
        {
            AfxMessageBox(_T("프리셋 저장에 실패했습니다."), MB_ICONERROR);
        }
    }
}

void CPresetManagerDlg::OnBnClickedButtonLoadPreset()
{
    int sel = m_listPresets.GetCurSel();
    if (sel < 0)
        return;

    CString presetName;
    m_listPresets.GetText(sel, presetName);

    CSettingsManager* pMgr = GetSettingsManager();
    if (pMgr && pMgr->LoadPreset(presetName))
    {
        m_strSelectedPreset = presetName;
        AfxMessageBox(_T("프리셋이 로드되었습니다.\n설정을 적용하려면 대화상자를 닫으세요."), MB_ICONINFORMATION);

        // 부모 윈도우에 설정 변경 알림
        CWnd* pParent = GetParent();
        if (pParent)
        {
            pParent->PostMessage(WM_COMMAND, MAKEWPARAM(ID_SETTINGS_CHANGED, 0), 0);
        }
    }
    else
    {
        AfxMessageBox(_T("프리셋 로드에 실패했습니다."), MB_ICONERROR);
    }
}

void CPresetManagerDlg::OnBnClickedButtonDeletePreset()
{
    int sel = m_listPresets.GetCurSel();
    if (sel < 0)
        return;

    CString presetName;
    m_listPresets.GetText(sel, presetName);

    if (AfxMessageBox(_T("정말 이 프리셋을 삭제하시겠습니까?"),
        MB_YESNO | MB_ICONQUESTION) != IDYES)
    {
        return;
    }

    CSettingsManager* pMgr = GetSettingsManager();
    if (pMgr && pMgr->DeletePreset(presetName))
    {
        m_listPresets.DeleteString(sel);
        m_editPresetName.SetWindowText(_T(""));
        m_editDescription.SetWindowText(_T(""));
        m_btnLoad.EnableWindow(FALSE);
        m_btnDelete.EnableWindow(FALSE);
        AfxMessageBox(_T("프리셋이 삭제되었습니다."), MB_ICONINFORMATION);
    }
    else
    {
        AfxMessageBox(_T("프리셋 삭제에 실패했습니다."), MB_ICONERROR);
    }
}

void CPresetManagerDlg::OnBnClickedButtonClose()
{
    EndDialog(IDOK);
}