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
    ON_EN_CHANGE(IDC_EDIT_SCALE_ONLY, &CSimSettingsPanel::OnEnChangeEditScaleOnly)
    ON_EN_CHANGE(IDC_EDIT_TARGET_ERROR, &CSimSettingsPanel::OnEnChangeEditTargetError)
    ON_CBN_SELCHANGE(2104, &CSimSettingsPanel::OnCbnSelchangeComboTargets)
    ON_BN_CLICKED(2110, &CSimSettingsPanel::OnBnClickedCheckBuffs)
    ON_BN_CLICKED(2111, &CSimSettingsPanel::OnBnClickedCheckBuffs)
    ON_BN_CLICKED(2112, &CSimSettingsPanel::OnBnClickedCheckBuffs)
    ON_BN_CLICKED(2113, &CSimSettingsPanel::OnBnClickedCheckBuffs)
    ON_BN_CLICKED(2114, &CSimSettingsPanel::OnBnClickedCheckBuffs)
    ON_BN_CLICKED(2115, &CSimSettingsPanel::OnBnClickedCheckBuffs)
    ON_BN_CLICKED(IDC_CHECK_ARCANE_INTELLECT, &CSimSettingsPanel::OnBnClickedCheckBuffs)
    ON_BN_CLICKED(IDC_CHECK_POWER_WORD_FORTITUDE, &CSimSettingsPanel::OnBnClickedCheckBuffs)
    ON_BN_CLICKED(IDC_CHECK_BATTLE_SHOUT, &CSimSettingsPanel::OnBnClickedCheckBuffs)
    ON_BN_CLICKED(IDC_CHECK_MYSTIC_TOUCH, &CSimSettingsPanel::OnBnClickedCheckBuffs)
    ON_BN_CLICKED(IDC_CHECK_CHAOS_BRAND, &CSimSettingsPanel::OnBnClickedCheckBuffs)
    ON_BN_CLICKED(IDC_CHECK_SKYFURY, &CSimSettingsPanel::OnBnClickedCheckBuffs)
    ON_BN_CLICKED(IDC_CHECK_MARK_OF_THE_WILD, &CSimSettingsPanel::OnBnClickedCheckBuffs)
    ON_BN_CLICKED(IDC_CHECK_HUNTERS_MARK, &CSimSettingsPanel::OnBnClickedCheckBuffs)
    ON_BN_CLICKED(IDC_CHECK_BLEEDING, &CSimSettingsPanel::OnBnClickedCheckBuffs)
    ON_BN_CLICKED(IDC_CHECK_CALC_SCALE_FACTORS, &CSimSettingsPanel::OnBnClickedCheckBuffs)
    ON_BN_CLICKED(IDC_CHECK_REPORT_DETAILS, &CSimSettingsPanel::OnBnClickedCheckBuffs)
    ON_BN_CLICKED(IDC_CHECK_SINGLE_ACTOR_BATCH, &CSimSettingsPanel::OnBnClickedCheckBuffs)
    ON_BN_CLICKED(IDC_CHECK_OPTIMIZE_EXPRESSIONS, &CSimSettingsPanel::OnBnClickedCheckBuffs)
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
    , m_bArcaneIntellect(TRUE)
    , m_bPowerWordFortitude(TRUE)
    , m_bBattleShout(TRUE)
    , m_bMysticTouch(TRUE)
    , m_bChaosBrand(TRUE)
    , m_bSkyfury(TRUE)
    , m_bMarkOfTheWild(TRUE)
    , m_bHuntersMark(TRUE)
    , m_bBleeding(TRUE)
    , m_bCalculateScaleFactors(FALSE)
    , m_bReportDetails(TRUE)
    , m_bSingleActorBatch(TRUE)
    , m_bOptimizeExpressions(TRUE)
    , m_strScaleOnly(_T("strength,intellect,agility,crit,mastery,vers,haste,weapon_dps,weapon_offhand_dps"))
    , m_dTargetError(0.05)
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
    DDX_Control(pDX, IDC_CHECK_ARCANE_INTELLECT, m_checkArcaneIntellect);
    DDX_Control(pDX, IDC_CHECK_POWER_WORD_FORTITUDE, m_checkPowerWordFortitude);
    DDX_Control(pDX, IDC_CHECK_BATTLE_SHOUT, m_checkBattleShout);
    DDX_Control(pDX, IDC_CHECK_MYSTIC_TOUCH, m_checkMysticTouch);
    DDX_Control(pDX, IDC_CHECK_CHAOS_BRAND, m_checkChaosBrand);
    DDX_Control(pDX, IDC_CHECK_SKYFURY, m_checkSkyfury);
    DDX_Control(pDX, IDC_CHECK_MARK_OF_THE_WILD, m_checkMarkOfTheWild);
    DDX_Control(pDX, IDC_CHECK_HUNTERS_MARK, m_checkHuntersMark);
    DDX_Control(pDX, IDC_CHECK_BLEEDING, m_checkBleeding);
    DDX_Control(pDX, IDC_CHECK_CALC_SCALE_FACTORS, m_checkCalculateScaleFactors);
    DDX_Control(pDX, IDC_CHECK_REPORT_DETAILS, m_checkReportDetails);
    DDX_Control(pDX, IDC_CHECK_SINGLE_ACTOR_BATCH, m_checkSingleActorBatch);
    DDX_Control(pDX, IDC_CHECK_OPTIMIZE_EXPRESSIONS, m_checkOptimizeExpressions);
    DDX_Control(pDX, IDC_EDIT_SCALE_ONLY, m_editScaleOnly);
    DDX_Control(pDX, IDC_EDIT_TARGET_ERROR, m_editTargetError);

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
    DDX_Check(pDX, IDC_CHECK_ARCANE_INTELLECT, m_bArcaneIntellect);
    DDX_Check(pDX, IDC_CHECK_POWER_WORD_FORTITUDE, m_bPowerWordFortitude);
    DDX_Check(pDX, IDC_CHECK_BATTLE_SHOUT, m_bBattleShout);
    DDX_Check(pDX, IDC_CHECK_MYSTIC_TOUCH, m_bMysticTouch);
    DDX_Check(pDX, IDC_CHECK_CHAOS_BRAND, m_bChaosBrand);
    DDX_Check(pDX, IDC_CHECK_SKYFURY, m_bSkyfury);
    DDX_Check(pDX, IDC_CHECK_MARK_OF_THE_WILD, m_bMarkOfTheWild);
    DDX_Check(pDX, IDC_CHECK_HUNTERS_MARK, m_bHuntersMark);
    DDX_Check(pDX, IDC_CHECK_BLEEDING, m_bBleeding);
    DDX_Check(pDX, IDC_CHECK_CALC_SCALE_FACTORS, m_bCalculateScaleFactors);
    DDX_Check(pDX, IDC_CHECK_REPORT_DETAILS, m_bReportDetails);
    DDX_Check(pDX, IDC_CHECK_SINGLE_ACTOR_BATCH, m_bSingleActorBatch);
    DDX_Check(pDX, IDC_CHECK_OPTIMIZE_EXPRESSIONS, m_bOptimizeExpressions);
    DDX_Text(pDX, IDC_EDIT_SCALE_ONLY, m_strScaleOnly);
    DDX_Text(pDX, IDC_EDIT_TARGET_ERROR, m_dTargetError);
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
    m_bArcaneIntellect = settings.useArcaneIntellect;
    m_bPowerWordFortitude = settings.usePowerWordFortitude;
    m_bBattleShout = settings.useBattleShout;
    m_bMysticTouch = settings.useMysticTouch;
    m_bChaosBrand = settings.useChaosBrand;
    m_bSkyfury = settings.useSkyfury;
    m_bMarkOfTheWild = settings.useMarkOfTheWild;
    m_bHuntersMark = settings.useHuntersMark;
    m_bBleeding = settings.useBleeding;
    m_bCalculateScaleFactors = settings.calculateScaleFactors;
    m_bReportDetails = settings.reportDetails;
    m_bSingleActorBatch = settings.singleActorBatch;
    m_bOptimizeExpressions = settings.optimizeExpressions;
    m_strScaleOnly = settings.scaleOnly;
    m_dTargetError = settings.targetError;

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
    settings.useArcaneIntellect = m_bArcaneIntellect;
    settings.usePowerWordFortitude = m_bPowerWordFortitude;
    settings.useBattleShout = m_bBattleShout;
    settings.useMysticTouch = m_bMysticTouch;
    settings.useChaosBrand = m_bChaosBrand;
    settings.useSkyfury = m_bSkyfury;
    settings.useMarkOfTheWild = m_bMarkOfTheWild;
    settings.useHuntersMark = m_bHuntersMark;
    settings.useBleeding = m_bBleeding;
    settings.calculateScaleFactors = m_bCalculateScaleFactors;
    settings.reportDetails = m_bReportDetails;
    settings.singleActorBatch = m_bSingleActorBatch;
    settings.optimizeExpressions = m_bOptimizeExpressions;
    settings.scaleOnly = m_strScaleOnly;
    settings.targetError = m_dTargetError;
}

void CSimSettingsPanel::OnCbnSelchangeComboFightStyle() { SaveSettingsToManager(); }
void CSimSettingsPanel::OnEnChangeEditDuration() { SaveSettingsToManager(); }
void CSimSettingsPanel::OnEnChangeEditIterations() { SaveSettingsToManager(); }
void CSimSettingsPanel::OnEnChangeEditScaleOnly() { SaveSettingsToManager(); }
void CSimSettingsPanel::OnEnChangeEditTargetError() { SaveSettingsToManager(); }
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
