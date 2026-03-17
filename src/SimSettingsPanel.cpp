#include "pch.h"
#include "framework.h"
#include "SimSettingsPanel.h"
#include "SettingsManager.h"
#include "MainFrame.h"
#include "CharacterData.h"
#include "TrinketData.h"
#include "resource.h"
#include <vector>

IMPLEMENT_DYNCREATE(CSimSettingsPanel, CFormView)

BEGIN_MESSAGE_MAP(CSimSettingsPanel, CFormView)
    ON_WM_SIZE()
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
    ON_BN_CLICKED(IDC_CHECK_TRINKET1_OVERRIDE, &CSimSettingsPanel::OnBnClickedCheckTrinket1)
    ON_BN_CLICKED(IDC_CHECK_TRINKET2_OVERRIDE, &CSimSettingsPanel::OnBnClickedCheckTrinket2)
    ON_CBN_SELCHANGE(IDC_COMBO_TRINKET_FILTER, &CSimSettingsPanel::OnCbnSelchangeComboTrinketFilter)
    ON_CBN_SELCHANGE(IDC_COMBO_TRINKET1_OVERRIDE, &CSimSettingsPanel::OnCbnSelchangeComboTrinket1)
    ON_CBN_SELCHANGE(IDC_COMBO_TRINKET2_OVERRIDE, &CSimSettingsPanel::OnCbnSelchangeComboTrinket2)
    ON_EN_CHANGE(IDC_COMBO_TRINKET1_ILVL, &CSimSettingsPanel::OnEnChangeEditTrinket1ItemLevel)
    ON_EN_CHANGE(IDC_COMBO_TRINKET2_ILVL, &CSimSettingsPanel::OnEnChangeEditTrinket2ItemLevel)
    ON_EN_CHANGE(IDC_EDIT_TRINKET1_BONUS_ID, &CSimSettingsPanel::OnEnChangeEditTrinket1BonusId)
    ON_EN_CHANGE(IDC_EDIT_TRINKET2_BONUS_ID, &CSimSettingsPanel::OnEnChangeEditTrinket2BonusId)
    ON_EN_CHANGE(IDC_EDIT_TRINKET1_CONTEXT, &CSimSettingsPanel::OnEnChangeEditTrinket1Context)
    ON_EN_CHANGE(IDC_EDIT_TRINKET2_CONTEXT, &CSimSettingsPanel::OnEnChangeEditTrinket2Context)
END_MESSAGE_MAP()

namespace
{
    CString TrimField(const CString& value)
    {
        CString trimmed(value);
        trimmed.Trim();
        return trimmed;
    }

    bool IsDigitsOnly(const CString& value)
    {
        for (int i = 0; i < value.GetLength(); ++i)
        {
            if (!_istdigit(value[i]))
            {
                return false;
            }
        }

        return true;
    }

    bool IsBonusIdList(const CString& value)
    {
        if (value.IsEmpty())
        {
            return true;
        }

        bool expectDigit = true;
        for (int i = 0; i < value.GetLength(); ++i)
        {
            const TCHAR ch = value[i];
            if (ch == _T('/'))
            {
                if (expectDigit)
                {
                    return false;
                }

                expectDigit = true;
                continue;
            }

            if (!_istdigit(ch))
            {
                return false;
            }

            expectDigit = false;
        }

        return !expectDigit;
    }

    std::vector<uint32_t> ParseBonusIds(const CString& bonusIdText)
    {
        std::vector<uint32_t> bonusIds;
        CString remaining = bonusIdText;
        int position = 0;
        CString token = remaining.Tokenize(_T("/"), position);
        while (!token.IsEmpty())
        {
            bonusIds.push_back(static_cast<uint32_t>(_tcstoul(token, nullptr, 10)));
            token = remaining.Tokenize(_T("/"), position);
        }

        return bonusIds;
    }
}

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
    , m_selectedTrinket1EntryId(0)
    , m_selectedTrinket2EntryId(0)
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

    const int x = 10;
    const int w = 240;
    int y = 338;
    const int h = 22;

    m_staticTrinketHeader.Create(_T("Midnight Trinket Override"), WS_CHILD | WS_VISIBLE | SS_LEFT,
        CRect(x, y, x + w, y + 18), this, 65520);
    y += 20;
    m_staticTrinketFilterLabel.Create(_T("List:"), WS_CHILD | WS_VISIBLE | SS_LEFT,
        CRect(x, y + 4, x + 40, y + 18), this, 65521);
    m_comboTrinketFilter.Create(WS_CHILD | WS_VISIBLE | WS_VSCROLL | CBS_DROPDOWNLIST,
        CRect(x + 45, y, x + w, y + 200), this, IDC_COMBO_TRINKET_FILTER);
    m_comboTrinketFilter.AddString(_T("All"));
    m_comboTrinketFilter.AddString(_T("Dungeon"));
    m_comboTrinketFilter.AddString(_T("Raid"));
    m_comboTrinketFilter.SetCurSel(0);
    y += 28;

    m_checkTrinket1.Create(_T("Override Trinket 1"), WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX,
        CRect(x, y, x + w, y + h), this, IDC_CHECK_TRINKET1_OVERRIDE);
    y += h + 2;
    m_comboTrinket1.Create(WS_CHILD | WS_VISIBLE | WS_VSCROLL | CBS_DROPDOWNLIST,
        CRect(x, y, x + w, y + 300), this, IDC_COMBO_TRINKET1_OVERRIDE);
    y += h + 2;
    m_staticTrinket1BonusLabel.Create(_T("bonus_id"), WS_CHILD | WS_VISIBLE | SS_LEFT,
        CRect(x, y + 4, x + 58, y + 18), this, 65525);
    m_editTrinket1BonusId.Create(WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL,
        CRect(x + 60, y, x + w, y + h), this, IDC_EDIT_TRINKET1_BONUS_ID);
    y += h + 2;
    m_staticTrinket1ContextLabel.Create(_T("context"), WS_CHILD | WS_VISIBLE | SS_LEFT,
        CRect(x, y + 4, x + 58, y + 18), this, 65526);
    m_editTrinket1Context.Create(WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL,
        CRect(x + 60, y, x + 120, y + h), this, IDC_EDIT_TRINKET1_CONTEXT);
    m_staticTrinket1ItemLevelLabel.Create(_T("ilevel"), WS_CHILD | WS_VISIBLE | SS_LEFT,
        CRect(x + 128, y + 4, x + 170, y + 18), this, 65527);
    m_editTrinket1ItemLevel.Create(WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL,
        CRect(x + 174, y, x + w, y + h), this, IDC_COMBO_TRINKET1_ILVL);
    y += h + 2;
    m_staticTrinket1Info.Create(_T("No current trinket info"), WS_CHILD | WS_VISIBLE | SS_LEFT,
        CRect(x, y, x + w, y + 18), this, 65522);
    y += 20;

    m_checkTrinket2.Create(_T("Override Trinket 2"), WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX,
        CRect(x, y, x + w, y + h), this, IDC_CHECK_TRINKET2_OVERRIDE);
    y += h + 2;
    m_comboTrinket2.Create(WS_CHILD | WS_VISIBLE | WS_VSCROLL | CBS_DROPDOWNLIST,
        CRect(x, y, x + w, y + 300), this, IDC_COMBO_TRINKET2_OVERRIDE);
    y += h + 2;
    m_staticTrinket2BonusLabel.Create(_T("bonus_id"), WS_CHILD | WS_VISIBLE | SS_LEFT,
        CRect(x, y + 4, x + 58, y + 18), this, 65528);
    m_editTrinket2BonusId.Create(WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL,
        CRect(x + 60, y, x + w, y + h), this, IDC_EDIT_TRINKET2_BONUS_ID);
    y += h + 2;
    m_staticTrinket2ContextLabel.Create(_T("context"), WS_CHILD | WS_VISIBLE | SS_LEFT,
        CRect(x, y + 4, x + 58, y + 18), this, 65529);
    m_editTrinket2Context.Create(WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL,
        CRect(x + 60, y, x + 120, y + h), this, IDC_EDIT_TRINKET2_CONTEXT);
    m_staticTrinket2ItemLevelLabel.Create(_T("ilevel"), WS_CHILD | WS_VISIBLE | SS_LEFT,
        CRect(x + 128, y + 4, x + 170, y + 18), this, 65530);
    m_editTrinket2ItemLevel.Create(WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL,
        CRect(x + 174, y, x + w, y + h), this, IDC_COMBO_TRINKET2_ILVL);
    y += h + 2;
    m_staticTrinket2Info.Create(_T("No current trinket info"), WS_CHILD | WS_VISIBLE | SS_LEFT,
        CRect(x, y, x + w, y + 18), this, 65523);
    y += 20;
    m_staticTrinketStatus.Create(_T("Selected trinkets can use optional bonus_id/context/ilevel."),
        WS_CHILD | WS_VISIBLE | SS_LEFT, CRect(x, y, x + w, y + 30), this, 65524);

    PopulateTrinketCombos(FALSE);
    UpdateTrinketControlState();
    UpdateTrinketSelectionSummary();
    LayoutTrinketControls(280);

    LoadSettingsFromManager();
    UpdateData(FALSE);
}

void CSimSettingsPanel::OnSize(UINT nType, int cx, int cy)
{
    CFormView::OnSize(nType, cx, cy);

    if (!m_staticTrinketHeader.GetSafeHwnd())
    {
        return;
    }

    LayoutTrinketControls(cx);
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
    m_selectedTrinket1EntryId = settings.trinket1EntryId;
    m_selectedTrinket2EntryId = settings.trinket2EntryId;
    m_strTrinket1BonusId = settings.trinket1BonusId;
    m_strTrinket2BonusId = settings.trinket2BonusId;
    m_strTrinket1Context = settings.trinket1Context;
    m_strTrinket2Context = settings.trinket2Context;
    m_strTrinket1ItemLevel = settings.trinket1ItemLevel;
    m_strTrinket2ItemLevel = settings.trinket2ItemLevel;

    if (GetSafeHwnd())
    {
        UpdateData(FALSE);
        m_comboTrinketFilter.SetCurSel(settings.trinketFilterIndex);
        PopulateTrinketCombos(FALSE);
        m_checkTrinket1.SetCheck(settings.useTrinket1Override ? BST_CHECKED : BST_UNCHECKED);
        m_checkTrinket2.SetCheck(settings.useTrinket2Override ? BST_CHECKED : BST_UNCHECKED);
        m_editTrinket1BonusId.SetWindowText(m_strTrinket1BonusId);
        m_editTrinket2BonusId.SetWindowText(m_strTrinket2BonusId);
        m_editTrinket1Context.SetWindowText(m_strTrinket1Context);
        m_editTrinket2Context.SetWindowText(m_strTrinket2Context);
        m_editTrinket1ItemLevel.SetWindowText(m_strTrinket1ItemLevel);
        m_editTrinket2ItemLevel.SetWindowText(m_strTrinket2ItemLevel);
        UpdateTrinketControlState();
        UpdateTrinketSelectionSummary();
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
    settings.useTrinket1Override = (m_checkTrinket1.GetCheck() == BST_CHECKED);
    settings.useTrinket2Override = (m_checkTrinket2.GetCheck() == BST_CHECKED);
    settings.trinketFilterIndex = GetSelectedFilterIndex();
    settings.trinket1EntryId = GetSelectedTrinketEntryId(m_comboTrinket1);
    settings.trinket2EntryId = GetSelectedTrinketEntryId(m_comboTrinket2);
    m_editTrinket1BonusId.GetWindowText(m_strTrinket1BonusId);
    m_editTrinket2BonusId.GetWindowText(m_strTrinket2BonusId);
    m_editTrinket1Context.GetWindowText(m_strTrinket1Context);
    m_editTrinket2Context.GetWindowText(m_strTrinket2Context);
    m_editTrinket1ItemLevel.GetWindowText(m_strTrinket1ItemLevel);
    m_editTrinket2ItemLevel.GetWindowText(m_strTrinket2ItemLevel);
    settings.trinket1BonusId = TrimField(m_strTrinket1BonusId);
    settings.trinket2BonusId = TrimField(m_strTrinket2BonusId);
    settings.trinket1Context = TrimField(m_strTrinket1Context);
    settings.trinket2Context = TrimField(m_strTrinket2Context);
    settings.trinket1ItemLevel = TrimField(m_strTrinket1ItemLevel);
    settings.trinket2ItemLevel = TrimField(m_strTrinket2ItemLevel);

    CMainFrame* pFrame = static_cast<CMainFrame*>(AfxGetMainWnd());
    if (pFrame != nullptr)
    {
        SyncTrinketSelectionsToCharacterData(pFrame->GetCharacterData());
    }
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

void CSimSettingsPanel::DisplayCharacterInfo(const CCharacterData* pCharData)
{
    if (pCharData == nullptr || !m_staticTrinket1Info.GetSafeHwnd())
    {
        return;
    }

    m_staticTrinket1Info.SetWindowText(pCharData->DescribeItemSlot(CCharacterData::ItemSlot::TRINKET1));
    m_staticTrinket2Info.SetWindowText(pCharData->DescribeItemSlot(CCharacterData::ItemSlot::TRINKET2));
    SyncTrinketSelectionsToCharacterData(const_cast<CCharacterData*>(pCharData));
    UpdateTrinketSelectionSummary();
}

BOOL CSimSettingsPanel::ValidateTrinketInputs(CString& errorMessage) const
{
    struct ValidationTarget
    {
        const CButton* checkBox;
        const CEdit* bonusEdit;
        const CEdit* contextEdit;
        const CEdit* itemLevelEdit;
        LPCTSTR slotName;
    };

    const ValidationTarget targets[] = {
        { &m_checkTrinket1, &m_editTrinket1BonusId, &m_editTrinket1Context, &m_editTrinket1ItemLevel, _T("trinket1") },
        { &m_checkTrinket2, &m_editTrinket2BonusId, &m_editTrinket2Context, &m_editTrinket2ItemLevel, _T("trinket2") }
    };

    for (const ValidationTarget& target : targets)
    {
        if (target.checkBox->GetCheck() != BST_CHECKED)
        {
            continue;
        }

        CString bonusId;
        CString context;
        CString itemLevel;
        target.bonusEdit->GetWindowText(bonusId);
        target.contextEdit->GetWindowText(context);
        target.itemLevelEdit->GetWindowText(itemLevel);
        bonusId = TrimField(bonusId);
        context = TrimField(context);
        itemLevel = TrimField(itemLevel);

        if (!IsBonusIdList(bonusId))
        {
            errorMessage.Format(_T("%s bonus_id 형식이 올바르지 않습니다. 숫자와 '/'만 사용할 수 있습니다."), target.slotName);
            return FALSE;
        }

        if (!context.IsEmpty() && !IsDigitsOnly(context))
        {
            errorMessage.Format(_T("%s context 값은 숫자만 입력할 수 있습니다."), target.slotName);
            return FALSE;
        }

        if (!itemLevel.IsEmpty())
        {
            TCHAR* endPtr = nullptr;
            const double value = _tcstod(itemLevel, &endPtr);
            if (endPtr == itemLevel.GetString() || (endPtr != nullptr && *endPtr != _T('\0')) || value <= 0.0)
            {
                errorMessage.Format(_T("%s ilevel 값은 0보다 큰 숫자여야 합니다."), target.slotName);
                return FALSE;
            }
        }
    }

    return TRUE;
}

void CSimSettingsPanel::PopulateTrinketCombos(BOOL preserveSelection)
{
    const int previousSelection1 = preserveSelection ? GetSelectedTrinketEntryId(m_comboTrinket1) : m_selectedTrinket1EntryId;
    const int previousSelection2 = preserveSelection ? GetSelectedTrinketEntryId(m_comboTrinket2) : m_selectedTrinket2EntryId;

    m_comboTrinket1.ResetContent();
    m_comboTrinket2.ResetContent();

    const int filterIndex = GetSelectedFilterIndex();
    const auto& options = GetMidnightTrinketOptions();
    for (const auto& option : options)
    {
        const bool includeOption =
            filterIndex == 0 ||
            (filterIndex == 1 && option.sourceType.CompareNoCase(_T("dungeon")) == 0) ||
            (filterIndex == 2 && option.sourceType.CompareNoCase(_T("raid")) == 0);

        if (!includeOption)
        {
            continue;
        }

        CString label;
        label.Format(_T("%s [%s]"), option.displayName.GetString(), option.sourceName.GetString());

        const int index1 = m_comboTrinket1.AddString(label);
        m_comboTrinket1.SetItemData(index1, static_cast<DWORD_PTR>(option.entryId));
        const int index2 = m_comboTrinket2.AddString(label);
        m_comboTrinket2.SetItemData(index2, static_cast<DWORD_PTR>(option.entryId));
    }

    int selectedIndex1 = 0;
    int selectedIndex2 = 0;
    for (int i = 0; i < m_comboTrinket1.GetCount(); ++i)
    {
        if (static_cast<int>(m_comboTrinket1.GetItemData(i)) == previousSelection1)
        {
            selectedIndex1 = i;
            break;
        }
    }
    for (int i = 0; i < m_comboTrinket2.GetCount(); ++i)
    {
        if (static_cast<int>(m_comboTrinket2.GetItemData(i)) == previousSelection2)
        {
            selectedIndex2 = i;
            break;
        }
    }

    if (m_comboTrinket1.GetCount() > 0) m_comboTrinket1.SetCurSel(selectedIndex1);
    if (m_comboTrinket2.GetCount() > 0) m_comboTrinket2.SetCurSel(selectedIndex2);

    m_selectedTrinket1EntryId = GetSelectedTrinketEntryId(m_comboTrinket1);
    m_selectedTrinket2EntryId = GetSelectedTrinketEntryId(m_comboTrinket2);
    if (!preserveSelection)
    {
        ApplyTrinketOptionDefaults(1, FindMidnightTrinketOption(m_selectedTrinket1EntryId));
        ApplyTrinketOptionDefaults(2, FindMidnightTrinketOption(m_selectedTrinket2EntryId));
    }
}

void CSimSettingsPanel::ApplyTrinketOptionDefaults(int slotIndex, const CTrinketOption* option)
{
    if (option == nullptr)
    {
        return;
    }

    CEdit* bonusEdit = (slotIndex == 1) ? &m_editTrinket1BonusId : &m_editTrinket2BonusId;
    CEdit* contextEdit = (slotIndex == 1) ? &m_editTrinket1Context : &m_editTrinket2Context;
    CEdit* itemLevelEdit = (slotIndex == 1) ? &m_editTrinket1ItemLevel : &m_editTrinket2ItemLevel;
    CString currentText;

    bonusEdit->GetWindowText(currentText);
    if (TrimField(currentText).IsEmpty() && !option->defaultBonusId.IsEmpty())
    {
        bonusEdit->SetWindowText(option->defaultBonusId);
    }

    contextEdit->GetWindowText(currentText);
    if (TrimField(currentText).IsEmpty() && !option->defaultContext.IsEmpty())
    {
        contextEdit->SetWindowText(option->defaultContext);
    }

    itemLevelEdit->GetWindowText(currentText);
    if (TrimField(currentText).IsEmpty() && option->defaultItemLevel > 0)
    {
        CString itemLevel;
        itemLevel.Format(_T("%d"), option->defaultItemLevel);
        itemLevelEdit->SetWindowText(itemLevel);
    }
}

void CSimSettingsPanel::LayoutTrinketControls(int panelWidth)
{
    if (!m_staticTrinketHeader.GetSafeHwnd())
    {
        return;
    }

    const int x = 10;
    const int w = max(180, panelWidth - 20);
    const int h = 22;

    int anchorBottom = 330;
    CWnd* anchors[] = {
        &m_editTargetError,
        &m_checkOptimizeExpressions,
        &m_checkSingleActorBatch,
        &m_checkReportDetails,
        &m_editScaleOnly
    };

    for (CWnd* anchor : anchors)
    {
        if (anchor != nullptr && anchor->GetSafeHwnd())
        {
            CRect rect;
            anchor->GetWindowRect(&rect);
            ScreenToClient(&rect);
            anchorBottom = max(anchorBottom, rect.bottom);
        }
    }

    int y = anchorBottom + 18;

    m_staticTrinketHeader.MoveWindow(x, y, w, 18);
    y += 20;
    m_staticTrinketFilterLabel.MoveWindow(x, y + 4, 40, 16);
    m_comboTrinketFilter.MoveWindow(x + 45, y, max(120, w - 45), 300);
    y += 28;
    m_checkTrinket1.MoveWindow(x, y, w, h);
    y += h + 2;
    m_comboTrinket1.MoveWindow(x, y, w, 300);
    y += h + 2;
    m_staticTrinket1BonusLabel.MoveWindow(x, y + 4, 58, 16);
    m_editTrinket1BonusId.MoveWindow(x + 60, y, w - 60, h);
    y += h + 2;
    m_staticTrinket1ContextLabel.MoveWindow(x, y + 4, 58, 16);
    m_editTrinket1Context.MoveWindow(x + 60, y, 60, h);
    m_staticTrinket1ItemLevelLabel.MoveWindow(x + 128, y + 4, 42, 16);
    m_editTrinket1ItemLevel.MoveWindow(x + 174, y, max(60, w - 174), h);
    y += h + 2;
    m_staticTrinket1Info.MoveWindow(x, y, w, 18);
    y += 20;
    m_checkTrinket2.MoveWindow(x, y, w, h);
    y += h + 2;
    m_comboTrinket2.MoveWindow(x, y, w, 300);
    y += h + 2;
    m_staticTrinket2BonusLabel.MoveWindow(x, y + 4, 58, 16);
    m_editTrinket2BonusId.MoveWindow(x + 60, y, w - 60, h);
    y += h + 2;
    m_staticTrinket2ContextLabel.MoveWindow(x, y + 4, 58, 16);
    m_editTrinket2Context.MoveWindow(x + 60, y, 60, h);
    m_staticTrinket2ItemLevelLabel.MoveWindow(x + 128, y + 4, 42, 16);
    m_editTrinket2ItemLevel.MoveWindow(x + 174, y, max(60, w - 174), h);
    y += h + 2;
    m_staticTrinket2Info.MoveWindow(x, y, w, 18);
    y += 20;
    m_staticTrinketStatus.MoveWindow(x, y, w, 30);
}

void CSimSettingsPanel::UpdateTrinketControlState()
{
    const BOOL useTrinket1Override = m_checkTrinket1.GetCheck() == BST_CHECKED;
    const BOOL useTrinket2Override = m_checkTrinket2.GetCheck() == BST_CHECKED;

    m_comboTrinket1.EnableWindow(useTrinket1Override);
    m_editTrinket1BonusId.EnableWindow(useTrinket1Override);
    m_editTrinket1Context.EnableWindow(useTrinket1Override);
    m_editTrinket1ItemLevel.EnableWindow(useTrinket1Override);
    m_comboTrinket2.EnableWindow(useTrinket2Override);
    m_editTrinket2BonusId.EnableWindow(useTrinket2Override);
    m_editTrinket2Context.EnableWindow(useTrinket2Override);
    m_editTrinket2ItemLevel.EnableWindow(useTrinket2Override);
}

void CSimSettingsPanel::UpdateTrinketSelectionSummary()
{
    CString status = _T("Default: use parsed current trinkets.");
    const bool useTrinket1Override = m_checkTrinket1.GetCheck() == BST_CHECKED;
    const bool useTrinket2Override = m_checkTrinket2.GetCheck() == BST_CHECKED;

    if (useTrinket1Override || useTrinket2Override)
    {
        status = _T("Selected trinkets use selected itemId with optional bonus_id/context/ilevel.");
        const CTrinketOption* trinket1 = FindMidnightTrinketOption(GetSelectedTrinketEntryId(m_comboTrinket1));
        const CTrinketOption* trinket2 = FindMidnightTrinketOption(GetSelectedTrinketEntryId(m_comboTrinket2));
        if (useTrinket1Override && useTrinket2Override &&
            trinket1 != nullptr && trinket2 != nullptr &&
            trinket1->uniqueEquipped && trinket2->uniqueEquipped &&
            trinket1->entryId == trinket2->entryId)
        {
            status = _T("Duplicate unique-equipped trinkets are selected.");
        }
    }

    if (m_staticTrinketStatus.GetSafeHwnd())
    {
        m_staticTrinketStatus.SetWindowText(status);
    }
}

void CSimSettingsPanel::SyncTrinketSelectionsToCharacterData(CCharacterData* pCharData) const
{
    if (pCharData == nullptr)
    {
        return;
    }

    const auto applySelection = [this, pCharData](CCharacterData::ItemSlot slot, const CButton& checkBox, const CComboBox& combo,
        const CEdit& bonusEdit, const CEdit& contextEdit, const CEdit& itemLevelEdit)
    {
        if (checkBox.GetCheck() != BST_CHECKED)
        {
            pCharData->ClearTrinketOverride(slot);
            return;
        }

        const CTrinketOption* option = FindMidnightTrinketOption(GetSelectedTrinketEntryId(combo));
        if (option == nullptr)
        {
            pCharData->ClearTrinketOverride(slot);
            return;
        }

        CCharacterData::TrinketOverrideSelection selection;
        selection.enabled = true;
        selection.slot = slot;
        selection.itemId = option->itemId;
        selection.displayName = CT2A(option->displayName);
        selection.simcToken = CT2A(option->simcToken);
        selection.bonusIds = option->defaultBonusId.IsEmpty() ? std::vector<uint32_t>() : ParseBonusIds(option->defaultBonusId);
        selection.contextId = option->defaultContext.IsEmpty() ? 0U : static_cast<uint32_t>(_tcstoul(option->defaultContext, nullptr, 10));
        selection.uniqueEquipped = option->uniqueEquipped;

        CString bonusText;
        CString contextText;
        CString itemLevelText;
        bonusEdit.GetWindowText(bonusText);
        contextEdit.GetWindowText(contextText);
        itemLevelEdit.GetWindowText(itemLevelText);
        bonusText = TrimField(bonusText);
        contextText = TrimField(contextText);
        itemLevelText = TrimField(itemLevelText);

        if (!bonusText.IsEmpty())
        {
            selection.bonusIds = ParseBonusIds(bonusText);
        }

        if (!contextText.IsEmpty())
        {
            selection.contextId = static_cast<uint32_t>(_tcstoul(contextText, nullptr, 10));
        }

        if (!itemLevelText.IsEmpty())
        {
            selection.itemLevel = _tcstod(itemLevelText, nullptr);
        }

        pCharData->SetTrinketOverride(selection);
    };

    applySelection(CCharacterData::ItemSlot::TRINKET1, m_checkTrinket1, m_comboTrinket1, m_editTrinket1BonusId, m_editTrinket1Context, m_editTrinket1ItemLevel);
    applySelection(CCharacterData::ItemSlot::TRINKET2, m_checkTrinket2, m_comboTrinket2, m_editTrinket2BonusId, m_editTrinket2Context, m_editTrinket2ItemLevel);
}

int CSimSettingsPanel::GetSelectedTrinketEntryId(const CComboBox& combo) const
{
    const int index = combo.GetCurSel();
    return index == CB_ERR ? 0 : static_cast<int>(combo.GetItemData(index));
}

int CSimSettingsPanel::GetSelectedFilterIndex() const
{
    const int index = m_comboTrinketFilter.GetCurSel();
    return index == CB_ERR ? 0 : index;
}

void CSimSettingsPanel::OnBnClickedCheckTrinket1() { UpdateTrinketControlState(); UpdateTrinketSelectionSummary(); SaveSettingsToManager(); }
void CSimSettingsPanel::OnBnClickedCheckTrinket2() { UpdateTrinketControlState(); UpdateTrinketSelectionSummary(); SaveSettingsToManager(); }
void CSimSettingsPanel::OnCbnSelchangeComboTrinketFilter() { PopulateTrinketCombos(TRUE); UpdateTrinketSelectionSummary(); SaveSettingsToManager(); }
void CSimSettingsPanel::OnCbnSelchangeComboTrinket1() { m_selectedTrinket1EntryId = GetSelectedTrinketEntryId(m_comboTrinket1); ApplyTrinketOptionDefaults(1, FindMidnightTrinketOption(m_selectedTrinket1EntryId)); UpdateTrinketSelectionSummary(); SaveSettingsToManager(); }
void CSimSettingsPanel::OnCbnSelchangeComboTrinket2() { m_selectedTrinket2EntryId = GetSelectedTrinketEntryId(m_comboTrinket2); ApplyTrinketOptionDefaults(2, FindMidnightTrinketOption(m_selectedTrinket2EntryId)); UpdateTrinketSelectionSummary(); SaveSettingsToManager(); }
void CSimSettingsPanel::OnEnChangeEditTrinket1BonusId() { UpdateTrinketSelectionSummary(); SaveSettingsToManager(); }
void CSimSettingsPanel::OnEnChangeEditTrinket2BonusId() { UpdateTrinketSelectionSummary(); SaveSettingsToManager(); }
void CSimSettingsPanel::OnEnChangeEditTrinket1Context() { UpdateTrinketSelectionSummary(); SaveSettingsToManager(); }
void CSimSettingsPanel::OnEnChangeEditTrinket2Context() { UpdateTrinketSelectionSummary(); SaveSettingsToManager(); }
void CSimSettingsPanel::OnEnChangeEditTrinket1ItemLevel() { UpdateTrinketSelectionSummary(); SaveSettingsToManager(); }
void CSimSettingsPanel::OnEnChangeEditTrinket2ItemLevel() { UpdateTrinketSelectionSummary(); SaveSettingsToManager(); }
