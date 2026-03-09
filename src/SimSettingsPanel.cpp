#include "pch.h"
#include "framework.h"
#include "SimSettingsPanel.h"
#include "SettingsManager.h"
#include "resource.h"

IMPLEMENT_DYNCREATE(CSimSettingsPanel, CFormView)

BEGIN_MESSAGE_MAP(CSimSettingsPanel, CFormView)
    ON_CBN_SELCHANGE(IDC_COMBO_FIGHT_STYLE, &CSimSettingsPanel::OnCbnSelchangeComboFightStyle)
    ON_EN_CHANGE(IDC_EDIT_DURATION, &CSimSettingsPanel::OnEnChangeEditDuration)
    ON_EN_CHANGE(IDC_EDIT_ITERATIONS, &CSimSettingsPanel::OnEnChangeEditIterations)
    ON_CBN_SELCHANGE(IDC_COMBO_TARGETS, &CSimSettingsPanel::OnCbnSelchangeComboTargets)
    ON_BN_CLICKED(IDC_CHECK_FLASK, &CSimSettingsPanel::OnBnClickedCheckBuffs)
    ON_BN_CLICKED(IDC_CHECK_FOOD, &CSimSettingsPanel::OnBnClickedCheckBuffs)
    ON_BN_CLICKED(IDC_CHECK_RUNE, &CSimSettingsPanel::OnBnClickedCheckBuffs)
    ON_BN_CLICKED(IDC_CHECK_AUGMENT, &CSimSettingsPanel::OnBnClickedCheckBuffs)
    ON_BN_CLICKED(IDC_CHECK_BLOODLUST, &CSimSettingsPanel::OnBnClickedCheckBuffs)
    ON_BN_CLICKED(IDC_CHECK_POTION, &CSimSettingsPanel::OnBnClickedCheckBuffs)
END_MESSAGE_MAP()

CSimSettingsPanel::CSimSettingsPanel()
    : CFormView(IDD_FORMVIEW_SIM_SETTINGS)
    , m_nDuration(300)
    , m_nIterations(10000)
    , m_nTargetCount(1)
    , m_bFlask(TRUE)
    , m_bFood(TRUE)
    , m_bRune(TRUE)
    , m_bAugment(TRUE)
    , m_bBloodlust(TRUE)
    , m_bPotion(TRUE)
{
    m_strFightStyle = _T("patchwerk");
}

CSimSettingsPanel::~CSimSettingsPanel()
{
}

void CSimSettingsPanel::DoDataExchange(CDataExchange* pDX)
{
    CFormView::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_COMBO_FIGHT_STYLE, m_comboFightStyle);
    DDX_Control(pDX, IDC_EDIT_DURATION, m_editDuration);
    DDX_Control(pDX, IDC_EDIT_ITERATIONS, m_editIterations);
    DDX_Control(pDX, IDC_COMBO_TARGETS, m_comboTargets);
    DDX_Control(pDX, IDC_CHECK_FLASK, m_checkFlask);
    DDX_Control(pDX, IDC_CHECK_FOOD, m_checkFood);
    DDX_Control(pDX, IDC_CHECK_RUNE, m_checkRune);
    DDX_Control(pDX, IDC_CHECK_AUGMENT, m_checkAugment);
    DDX_Control(pDX, IDC_CHECK_BLOODLUST, m_checkBloodlust);
    DDX_Control(pDX, IDC_CHECK_POTION, m_checkPotion);
    DDX_Text(pDX, IDC_EDIT_DURATION, m_nDuration);
    DDX_Text(pDX, IDC_EDIT_ITERATIONS, m_nIterations);
    DDX_Check(pDX, IDC_CHECK_FLASK, m_bFlask);
    DDX_Check(pDX, IDC_CHECK_FOOD, m_bFood);
    DDX_Check(pDX, IDC_CHECK_RUNE, m_bRune);
    DDX_Check(pDX, IDC_CHECK_AUGMENT, m_bAugment);
    DDX_Check(pDX, IDC_CHECK_BLOODLUST, m_bBloodlust);
    DDX_Check(pDX, IDC_CHECK_POTION, m_bPotion);
}

void CSimSettingsPanel::OnInitialUpdate()
{
    CFormView::OnInitialUpdate();

    // Initialize fight style combo
    auto fightStyles = CSettingsManager::GetFightStyles();
    for (const auto& style : fightStyles)
    {
        m_comboFightStyle.AddString(CSettingsManager::GetFightStyleDisplayName(style));
    }

    // Initialize target count combo
    m_comboTargets.AddString(_T("1"));
    m_comboTargets.AddString(_T("2"));
    m_comboTargets.AddString(_T("3"));
    m_comboTargets.AddString(_T("4"));
    m_comboTargets.AddString(_T("5"));
    m_comboTargets.AddString(_T("8"));

    // Load settings from manager
    LoadSettingsFromManager();

    UpdateData(FALSE);
}

void CSimSettingsPanel::OnCbnSelchangeComboFightStyle()
{
    int sel = m_comboFightStyle.GetCurSel();
    if (sel >= 0)
    {
        m_comboFightStyle.GetLBText(sel, m_strFightStyle);
    }
}

void CSimSettingsPanel::OnEnChangeEditDuration()
{
    UpdateData(TRUE);
    // Validate minimum duration
    if (m_nDuration < 10)
        m_nDuration = 10;
    if (m_nDuration > 1800)
        m_nDuration = 1800;
}

void CSimSettingsPanel::OnEnChangeEditIterations()
{
    UpdateData(TRUE);
    // Validate minimum iterations
    if (m_nIterations < 100)
        m_nIterations = 100;
    if (m_nIterations > 1000000)
        m_nIterations = 1000000;
}

void CSimSettingsPanel::OnCbnSelchangeComboTargets()
{
    int sel = m_comboTargets.GetCurSel();
    if (sel >= 0)
    {
        CString strTargets;
        m_comboTargets.GetLBText(sel, strTargets);
        m_nTargetCount = _ttoi(strTargets);
    }
}

void CSimSettingsPanel::OnBnClickedCheckBuffs()
{
    UpdateData(TRUE);
}

void CSimSettingsPanel::LoadDefaults()
{
    CSettingsManager* pMgr = GetSettingsManager();
    if (pMgr)
    {
        pMgr->RestoreDefaults();
        LoadSettingsFromManager();
    }
}

void CSimSettingsPanel::LoadSettingsFromManager()
{
    CSettingsManager* pMgr = GetSettingsManager();
    if (!pMgr)
        return;

    const SimulationSettings& settings = pMgr->GetCurrentSettings();

    // 전투 유형 설정
    auto fightStyles = CSettingsManager::GetFightStyles();
    auto it = std::find(fightStyles.begin(), fightStyles.end(), settings.fightStyle);
    if (it != fightStyles.end())
    {
        int idx = static_cast<int>(std::distance(fightStyles.begin(), it));
        m_comboFightStyle.SetCurSel(idx);
        m_strFightStyle = settings.fightStyle;
    }
    else
    {
        m_comboFightStyle.SetCurSel(0);
        m_strFightStyle = _T("patchwerk");
    }

    // 지속 시간 및 반복 횟수
    m_nDuration = settings.duration;
    m_nIterations = settings.iterations;

    // 대상 수 설정
    CString strTargetCount;
    strTargetCount.Format(_T("%d"), settings.targetCount);
    int targetIdx = m_comboTargets.FindStringExact(0, strTargetCount);
    if (targetIdx >= 0)
    {
        m_comboTargets.SetCurSel(targetIdx);
    }
    else
    {
        m_comboTargets.SetCurSel(0);
    }
    m_nTargetCount = settings.targetCount;

    // 버프 설정
    m_bFlask = settings.useFlask;
    m_bFood = settings.useFood;
    m_bRune = settings.useRune;
    m_bAugment = settings.useAugment;
    m_bBloodlust = settings.useBloodlust;
    m_bPotion = settings.usePotion;

    m_checkFlask.SetCheck(m_bFlask ? BST_CHECKED : BST_UNCHECKED);
    m_checkFood.SetCheck(m_bFood ? BST_CHECKED : BST_UNCHECKED);
    m_checkRune.SetCheck(m_bRune ? BST_CHECKED : BST_UNCHECKED);
    m_checkAugment.SetCheck(m_bAugment ? BST_CHECKED : BST_UNCHECKED);
    m_checkBloodlust.SetCheck(m_bBloodlust ? BST_CHECKED : BST_UNCHECKED);
    m_checkPotion.SetCheck(m_bPotion ? BST_CHECKED : BST_UNCHECKED);

    UpdateData(FALSE);
}

void CSimSettingsPanel::SaveSettingsToManager()
{
    UpdateData(TRUE);

    CSettingsManager* pMgr = GetSettingsManager();
    if (!pMgr)
        return;

    SimulationSettings& settings = pMgr->GetCurrentSettings();

    settings.fightStyle = m_strFightStyle;
    settings.duration = m_nDuration;
    settings.iterations = m_nIterations;
    settings.targetCount = m_nTargetCount;

    settings.useFlask = m_bFlask;
    settings.useFood = m_bFood;
    settings.useRune = m_bRune;
    settings.useAugment = m_bAugment;
    settings.useBloodlust = m_bBloodlust;
    settings.usePotion = m_bPotion;
}
