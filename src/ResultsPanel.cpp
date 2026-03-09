#include "pch.h"
#include "framework.h"
#include "ResultsPanel.h"
#include "ResultHistoryManager.h"
#include "resource.h"

IMPLEMENT_DYNCREATE(CResultsPanel, CFormView)

BEGIN_MESSAGE_MAP(CResultsPanel, CFormView)
    ON_BN_CLICKED(IDC_BUTTON_COPY_RESULT, &CResultsPanel::OnBnClickedButtonCopyResult)
    ON_BN_CLICKED(IDC_BUTTON_EXPORT_JSON, &CResultsPanel::OnBnClickedButtonExportJson)
    ON_BN_CLICKED(IDC_BUTTON_EXPORT_CSV, &CResultsPanel::OnBnClickedButtonExportCsv)
END_MESSAGE_MAP()

CResultsPanel::CResultsPanel()
    : CFormView(IDD_FORMVIEW_RESULTS)
{
}

CResultsPanel::~CResultsPanel()
{
}

void CResultsPanel::DoDataExchange(CDataExchange* pDX)
{
    CFormView::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_PROGRESS_SIM, m_progressSim);
    DDX_Control(pDX, IDC_EDIT_DPS, m_editDPS);
    DDX_Control(pDX, IDC_EDIT_SIM_TIME, m_editSimTime);
    DDX_Control(pDX, IDC_LIST_RESULTS, m_listResults);
    DDX_Control(pDX, IDC_BUTTON_COPY_RESULT, m_btnCopyResult);
    DDX_Control(pDX, IDC_BUTTON_EXPORT_JSON, m_btnExportJson);
    DDX_Control(pDX, IDC_BUTTON_EXPORT_CSV, m_btnExportCsv);
    DDX_Text(pDX, IDC_EDIT_DPS, m_strDPS);
    DDX_Text(pDX, IDC_EDIT_SIM_TIME, m_strSimTime);
}

void CResultsPanel::OnInitialUpdate()
{
    CFormView::OnInitialUpdate();

    // Initialize progress bar
    m_progressSim.SetRange(0, 100);
    m_progressSim.SetPos(0);

    // Initialize results list
    m_listResults.SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);
    m_listResults.InsertColumn(0, _T("Metric"), LVCFMT_LEFT, 150);
    m_listResults.InsertColumn(1, _T("Value"), LVCFMT_LEFT, 200);

    // Set initial text
    m_editDPS.SetWindowText(_T("--"));
    m_editSimTime.SetWindowText(_T("--"));

    UpdateData(FALSE);
}

void CResultsPanel::SetProgress(int percent)
{
    m_progressSim.SetPos(percent);
}

void CResultsPanel::LoadResults(const CString& resultJson)
{
    m_strResultJson = resultJson;

    // Use ResultHistoryManager to parse
    CResultHistoryManager* pMgr = GetResultHistoryManager();
    if (pMgr)
    {
        CSimResult result;
        if (pMgr->ParseSimcJson(resultJson, result))
        {
            DisplayResult(result);
        }
        else
        {
            m_editDPS.SetWindowText(_T("Error"));
            m_editSimTime.SetWindowText(_T("Error"));
        }
    }
}

void CResultsPanel::ClearResults()
{
    m_strDPS.Empty();
    m_strSimTime.Empty();
    m_strResultJson.Empty();
    m_editDPS.SetWindowText(_T("--"));
    m_editSimTime.SetWindowText(_T("--"));
    m_listResults.DeleteAllItems();
    m_progressSim.SetPos(0);
}

void CResultsPanel::UpdateResultDisplay()
{
    UpdateData(FALSE);
}

BOOL CResultsPanel::ParseSimcJson(const CString& jsonFile)
{
    try
    {
        // Read JSON file
        std::ifstream file(CT2A(jsonFile));
        if (!file.is_open())
        {
            return FALSE;
        }

        json j;
        file >> j;

        // Extract DPS
        if (j.contains("sim") && j["sim"].contains("players"))
        {
            auto& players = j["sim"]["players"];
            if (!players.empty() && players[0].contains("collected_data"))
            {
                auto& data = players[0]["collected_data"];
                if (data.contains("dps"))
                {
                    double dps = data["dps"]["mean"].get<double>();
                    m_strDPS.Format(_T("%.1f"), dps);
                }
                if (data.contains("execution_time"))
                {
                    double execTime = data["execution_time"]["mean"].get<double>();
                    m_strSimTime.Format(_T("%.2f sec"), execTime);
                }
            }
        }

        // Populate list with additional metrics
        m_listResults.DeleteAllItems();

        int idx = 0;
        if (j.contains("sim") && j["sim"].contains("statistics"))
        {
            auto& stats = j["sim"]["statistics"];
            if (stats.contains("elapsed_cpu_seconds"))
            {
                CString item;
                item.Format(_T("%.2f"), stats["elapsed_cpu_seconds"].get<double>());
                m_listResults.InsertItem(idx, _T("CPU Time"));
                m_listResults.SetItemText(idx++, 1, item);
            }
            if (stats.contains("sim_seconds"))
            {
                CString item;
                item.Format(_T("%.2f"), stats["sim_seconds"].get<double>());
                m_listResults.InsertItem(idx, _T("Sim Time"));
                m_listResults.SetItemText(idx++, 1, item);
            }
        }

        return TRUE;
    }
    catch (const std::exception& e)
    {
        TRACE1("Error parsing JSON: %s\n", e.what());
        return FALSE;
    }
}

void CResultsPanel::DisplayResult(const CSimResult& result)
{
    // DPS 표시
    m_strDPS.Format(_T("%.1f"), result.GetDpsStats().mean);

    // HPS (힐러인 경우)
    if (result.GetHpsStats().mean > 0)
    {
        m_strDPS.Format(_T("DPS: %.1f / HPS: %.1f"),
            result.GetDpsStats().mean, result.GetHpsStats().mean);
    }

    // 시뮬레이션 시간
    m_strSimTime.Format(_T("%.2f sec"), result.GetSimTime());

    UpdateData(FALSE);

    // 리스트에 상세 정보 표시
    m_listResults.DeleteAllItems();
    int idx = 0;

    // DPS 통계
    CString value;
    m_listResults.InsertItem(idx, _T("평균 DPS"));
    value.Format(_T("%.1f (±%.1f)"), result.GetDpsStats().mean, result.GetDpsStats().error);
    m_listResults.SetItemText(idx++, 1, value);

    m_listResults.InsertItem(idx, _T("최소 DPS"));
    value.Format(_T("%.1f"), result.GetDpsStats().min);
    m_listResults.SetItemText(idx++, 1, value);

    m_listResults.InsertItem(idx, _T("최대 DPS"));
    value.Format(_T("%.1f"), result.GetDpsStats().max);
    m_listResults.SetItemText(idx++, 1, value);

    m_listResults.InsertItem(idx, _T("표준편차"));
    value.Format(_T("%.2f"), result.GetDpsStats().stdDev);
    m_listResults.SetItemText(idx++, 1, value);

    // HPS (힐러인 경우)
    if (result.GetHpsStats().mean > 0)
    {
        m_listResults.InsertItem(idx, _T("평균 HPS"));
        value.Format(_T("%.1f (±%.1f)"), result.GetHpsStats().mean, result.GetHpsStats().error);
        m_listResults.SetItemText(idx++, 1, value);
    }

    // 시뮬레이션 정보
    m_listResults.InsertItem(idx, _T("전투 유형"));
    m_listResults.SetItemText(idx++, 1, result.GetFightStyle());

    m_listResults.InsertItem(idx, _T("지속 시간"));
    value.Format(_T("%d초"), result.GetDuration());
    m_listResults.SetItemText(idx++, 1, value);

    m_listResults.InsertItem(idx, _T("반복 횟수"));
    value.Format(_T("%d"), result.GetIterations());
    m_listResults.SetItemText(idx++, 1, value);

    m_listResults.InsertItem(idx, _T("실행 시간"));
    value.Format(_T("%.2f초"), result.GetSimTime());
    m_listResults.SetItemText(idx++, 1, value);

    // 상위 스킬 기여도
    const auto& abilities = result.GetAbilities();
    if (!abilities.empty())
    {
        m_listResults.InsertItem(idx, _T("---"));
        m_listResults.SetItemText(idx++, 1, _T("주요 스킬"));

        int skillCount = 0;
        for (const auto& ability : abilities)
        {
            if (ability.pct < 1.0) // 1% 미만 제외
                continue;

            CString itemText;
            itemText.Format(_T("%s (%.1f%%)"), ability.name, ability.pct);
            m_listResults.InsertItem(idx, itemText);

            value.Format(_T("%.1f DPS"), ability.dps);
            m_listResults.SetItemText(idx++, 1, value);

            if (++skillCount >= 5) // 상위 5개만
                break;
        }
    }
}

void CResultsPanel::OnBnClickedButtonCopyResult()
{
    if (m_strDPS.IsEmpty())
        return;

    CString text;
    text.Format(_T("DPS: %s\nSim Time: %s"), m_strDPS, m_strSimTime);

    if (OpenClipboard())
    {
        EmptyClipboard();
        HGLOBAL hGlobal = GlobalAlloc(GMEM_MOVEABLE, (text.GetLength() + 1) * sizeof(TCHAR));
        if (hGlobal)
        {
            LPTSTR pData = (LPTSTR)GlobalLock(hGlobal);
            _tcscpy_s(pData, text.GetLength() + 1, text);
            GlobalUnlock(hGlobal);
            SetClipboardData(CF_UNICODETEXT, hGlobal);
        }
        CloseClipboard();
    }
}

void CResultsPanel::OnBnClickedButtonExportJson()
{
    if (m_strResultJson.IsEmpty())
    {
        AfxMessageBox(_T("No results to export."), MB_ICONWARNING);
        return;
    }

    CFileDialog dlg(FALSE, _T("json"), _T("sim_result"),
        OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
        _T("JSON Files (*.json)|*.json|All Files (*.*)|*.*||"));

    if (dlg.DoModal() == IDOK)
    {
        CString destPath = dlg.GetPathName();
        if (!CopyFile(m_strResultJson, destPath, FALSE))
        {
            AfxMessageBox(_T("Failed to export file."), MB_ICONERROR);
        }
        else
        {
            AfxMessageBox(_T("Results exported successfully."), MB_ICONINFORMATION);
        }
    }
}

void CResultsPanel::OnBnClickedButtonExportCsv()
{
    // TODO: Implement CSV export
    AfxMessageBox(_T("CSV export not yet implemented."), MB_ICONINFORMATION);
}
