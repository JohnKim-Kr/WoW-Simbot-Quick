#include "pch.h"
#include "framework.h"
#include "SimSettingsPanel.h"
#include "SettingsManager.h"
#include "resource.h"

IMPLEMENT_DYNCREATE(CSimSettingsPanel, CFormView)

BEGIN_MESSAGE_MAP(CSimSettingsPanel, CFormView)
    ON_CBN_SELCHANGE(2101, &CSimSettingsPanel::OnCbnSelchangeComboFightStyle)
    ON_EN_CHANGE(2102, &CSimSettingsPanel::OnEnChangeEditDuration)
    ON_EN_CHANGE(2103, &CSimSettingsPanel::OnEnChangeEditIterations)
    ON_CBN_SELCHANGE(2104, &CSimSettingsPanel::OnCbnSelchangeComboTargets)
    ON_BN_CLICKED(2110, &CSimSettingsPanel::OnBnClickedCheckBuffs)
    ON_BN_CLICKED(2111, &CSimSettingsPanel::OnBnClickedCheckBuffs)
    ON_BN_CLICKED(2112, &CSimSettingsPanel::OnBnClickedCheckBuffs)
    ON_BN_CLICKED(2113, &CSimSettingsPanel::OnBnClickedCheckBuffs)
    ON_BN_CLICKED(2114, &CSimSettingsPanel::OnBnClickedCheckBuffs)
    ON_BN_CLICKED(2115, &CSimSettingsPanel::OnBnClickedCheckBuffs)
END_MESSAGE_MAP()

CSimSettingsPanel::CSimSettingsPanel()
    : CFormView(107)
    , m_nDuration(300)
    , m_nIterations(10000)
    , m_strTargetCount(_T("1"))
    , m_bFlask(TRUE)
    , m_bFood(TRUE)
    , m_bRune(TRUE)
    , m_bAugment(TRUE)
    , m_bBloodlust(TRUE)
    , m_bPotion(TRUE)
    , m_bCrucibleViolence(FALSE)
    , m_bCrucibleSustenance(FALSE)
    , m_bCruciblePredation(FALSE)
{
}

CSimSettingsPanel::~CSimSettingsPanel()
{
}

void CSimSettingsPanel::DoDataExchange(CDataExchange* pDX)
{
    CFormView::DoDataExchange(pDX);
    DDX_Control(pDX, 2101, m_comboFightStyle);
    DDX_Control(pDX, 2102, m_editDuration);
    DDX_Control(pDX, 2103, m_editIterations);
    DDX_Control(pDX, 2104, m_comboTargets);
    DDX_Control(pDX, 2110, m_checkFlask);
    DDX_Control(pDX, 2111, m_checkFood);
    DDX_Control(pDX, 2112, m_checkRune);
    DDX_Control(pDX, 2113, m_checkAugment);
    DDX_Control(pDX, 2114, m_checkBloodlust);
    DDX_Control(pDX, 2115, m_checkPotion);
    DDX_Control(pDX, 2120, m_comboTempEnchant);
    DDX_Control(pDX, 2121, m_checkCrucibleViolence);
    DDX_Control(pDX, 2122, m_checkCrucibleSustenance);
    DDX_Control(pDX, 2123, m_checkCruciblePredation);

    DDX_CBString(pDX, 2101, m_strFightStyle);
    DDX_Text(pDX, 2102, m_nDuration);
    DDX_Text(pDX, 2103, m_nIterations);
    DDX_CBString(pDX, 2104, m_strTargetCount); // Treat combo as string for manual int parsing
    DDX_Check(pDX, 2110, m_bFlask);
    DDX_Check(pDX, 2111, m_bFood);
    DDX_Check(pDX, 2112, m_bRune);
    DDX_Check(pDX, 2113, m_bAugment);
    DDX_Check(pDX, 2114, m_bBloodlust);
    DDX_Check(pDX, 2115, m_bPotion);
    DDX_CBString(pDX, 2120, m_strTempEnchant);
    DDX_Check(pDX, 2121, m_bCrucibleViolence);
    DDX_Check(pDX, 2122, m_bCrucibleSustenance);
    DDX_Check(pDX, 2123, m_bCruciblePredation);
}

void CSimSettingsPanel::OnInitialUpdate()
{
    ModifyStyle(WS_POPUP | WS_CAPTION | WS_THICKFRAME, WS_CHILD);
    CFormView::OnInitialUpdate();

    auto fightStyles = CSettingsManager::GetFightStyles();
    for (const auto& style : fightStyles)
    {
        m_comboFightStyle.AddString(style);
    }

    for (int i = 1; i <= 10; ++i)
    {
        CString str;
        str.Format(_T("%d"), i);
        m_comboTargets.AddString(str);
    }

    auto tempEnchants = CSettingsManager::GetTempEnchants();
    m_comboTempEnchant.AddString(_T("None"));
    for (const auto& enchant : tempEnchants)
    {
        m_comboTempEnchant.AddString(enchant);
    }

    LoadSettingsFromManager();
    UpdateData(FALSE);
}

void CSimSettingsPanel::LoadSettingsFromManager()
{
    CSettingsManager* pMgr = GetSettingsManager();
    if (!pMgr) return;

    const SimulationSettings& settings = pMgr->GetCurrentSettings();
    
    m_strFightStyle = settings.fightStyle;
    m_nDuration = settings.duration;
    m_nIterations = settings.iterations;
    m_strTargetCount.Format(_T("%d"), settings.targetCount);
    m_bFlask = settings.useFlask;
    m_bFood = settings.useFood;
    m_bRune = settings.useRune;
    m_bAugment = settings.useAugment;
    m_bBloodlust = settings.useBloodlust;
    m_bPotion = settings.usePotion;
    m_strTempEnchant = settings.temporaryEnchant;
    m_bCrucibleViolence = settings.useCrucibleViolence;
    m_bCrucibleSustenance = settings.useCrucibleSustenance;
    m_bCruciblePredation = settings.useCruciblePredation;

    if (GetSafeHwnd())
    {
        UpdateData(FALSE);
    }
}

void CSimSettingsPanel::SaveSettingsToManager()
{
    if (GetSafeHwnd())
    {
        UpdateData(TRUE);
    }

    CSettingsManager* pMgr = GetSettingsManager();
    if (!pMgr) return;

    SimulationSettings& settings = pMgr->GetCurrentSettings();
    
    settings.fightStyle = m_strFightStyle;
    settings.duration = m_nDuration;
    settings.iterations = m_nIterations;
    settings.targetCount = _ttoi(m_strTargetCount);
    settings.useFlask = m_bFlask;
    settings.useFood = m_bFood;
    settings.useRune = m_bRune;
    settings.useAugment = m_bAugment;
    settings.useBloodlust = m_bBloodlust;
    settings.usePotion = m_bPotion;
    settings.temporaryEnchant = m_strTempEnchant;
    settings.useCrucibleViolence = m_bCrucibleViolence;
    settings.useCrucibleSustenance = m_bCrucibleSustenance;
    settings.useCruciblePredation = m_bCruciblePredation;
}

void CSimSettingsPanel::OnCbnSelchangeComboFightStyle() { SaveSettingsToManager(); }
void CSimSettingsPanel::OnEnChangeEditDuration() { SaveSettingsToManager(); }
void CSimSettingsPanel::OnEnChangeEditIterations() { SaveSettingsToManager(); }
void CSimSettingsPanel::OnCbnSelchangeComboTargets() { SaveSettingsToManager(); }
void CSimSettingsPanel::OnBnClickedCheckBuffs() { SaveSettingsToManager(); }

void CSimSettingsPanel::LoadDefaults()
{
    CSettingsManager* pMgr = GetSettingsManager();
    if (pMgr)
    {
        pMgr->RestoreDefaults();
        LoadSettingsFromManager();
    }
}
