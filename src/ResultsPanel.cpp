#include "pch.h"
#include "framework.h"
#include "ResultsPanel.h"
#include "ResultHistoryManager.h"
#include "resource.h"
#include <algorithm>

IMPLEMENT_DYNCREATE(CResultsPanel, CView)

BEGIN_MESSAGE_MAP(CResultsPanel, CView)
    ON_WM_CREATE()
    ON_WM_SIZE()
    ON_NOTIFY(TCN_SELCHANGE, IDC_TAB_RESULTS, &CResultsPanel::OnTcnSelchangeTabResults)
    ON_BN_CLICKED(IDC_BUTTON_COPY_RESULT, &CResultsPanel::OnBnClickedButtonCopyResult)
    ON_BN_CLICKED(IDC_BUTTON_EXPORT_JSON, &CResultsPanel::OnBnClickedButtonExportJson)
    ON_BN_CLICKED(IDC_BUTTON_EXPORT_CSV, &CResultsPanel::OnBnClickedButtonExportCsv)
END_MESSAGE_MAP()

IMPLEMENT_DYNAMIC(CDpsGraphCtrl, CStatic)

BEGIN_MESSAGE_MAP(CDpsGraphCtrl, CStatic)
    ON_WM_PAINT()
END_MESSAGE_MAP()

CDpsGraphCtrl::CDpsGraphCtrl() {}
CDpsGraphCtrl::~CDpsGraphCtrl() {}

void CDpsGraphCtrl::SetData(const std::vector<DpsDataPoint>& data) { m_data = data; Invalidate(); }
void CDpsGraphCtrl::Clear() { m_data.clear(); Invalidate(); }

void CDpsGraphCtrl::OnPaint()
{
    CPaintDC dc(this); CRect rect; GetClientRect(&rect);
    dc.FillSolidRect(&rect, RGB(255, 255, 255));
    if (m_data.empty()) {
        dc.DrawText(_T("No DPS data"), -1, &rect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
        return;
    }
    DrawGraph(&dc, rect);
}

void CDpsGraphCtrl::DrawGraph(CDC* pDC, CRect rect)
{
    const int marginLeft = 60, marginRight = 20, marginTop = 30, marginBottom = 50;
    CRect graphRect(rect.left + marginLeft, rect.top + marginTop, rect.right - marginRight, rect.bottom - marginBottom);
    pDC->Draw3dRect(&graphRect, RGB(200, 200, 200), RGB(200, 200, 200));
    double minTime = m_data.front().time, maxTime = m_data.back().time;
    double minDps = m_data[0].dps, maxDps = m_data[0].dps;
    for (const auto& point : m_data) { if (point.dps < minDps) minDps = point.dps; if (point.dps > maxDps) maxDps = point.dps; }
    double dpsRange = maxDps - minDps; if (dpsRange < 0.001) dpsRange = 1.0;
    minDps -= dpsRange * 0.1; maxDps += dpsRange * 0.1;
    CPen axisPen(PS_SOLID, 1, RGB(100, 100, 100)); CPen* pOldPen = pDC->SelectObject(&axisPen);
    pDC->MoveTo(graphRect.left, graphRect.top); pDC->LineTo(graphRect.left, graphRect.bottom);
    pDC->MoveTo(graphRect.left, graphRect.bottom); pDC->LineTo(graphRect.right, graphRect.bottom);
    pDC->SetTextColor(RGB(100, 100, 100)); pDC->SetBkMode(TRANSPARENT);
    int numYTicks = 5;
    for (int i = 0; i <= numYTicks; i++) {
        double value = minDps + (maxDps - minDps) * i / numYTicks;
        int y = graphRect.bottom - (graphRect.Height() * i / numYTicks);
        if (i > 0 && i < numYTicks) {
            CPen gridPen(PS_DOT, 1, RGB(230, 230, 230)); pDC->SelectObject(&gridPen);
            pDC->MoveTo(graphRect.left, y); pDC->LineTo(graphRect.right, y); pDC->SelectObject(&axisPen);
        }
        CString label; label.Format(_T("%.0f"), value);
        CRect labelRect(graphRect.left - 55, y - 8, graphRect.left - 5, y + 8);
        pDC->DrawText(label, &labelRect, DT_RIGHT | DT_VCENTER | DT_SINGLELINE);
    }
    int numXTicks = 5;
    for (int i = 0; i <= numXTicks; i++) {
        double value = minTime + (maxTime - minTime) * i / numXTicks;
        int x = graphRect.left + (graphRect.Width() * i / numXTicks);
        if (i > 0 && i < numXTicks) {
            CPen gridPen(PS_DOT, 1, RGB(230, 230, 230)); pDC->SelectObject(&gridPen);
            pDC->MoveTo(x, graphRect.top); pDC->LineTo(x, graphRect.bottom); pDC->SelectObject(&axisPen);
        }
        CString label; label.Format(_T("%.0fs"), value);
        CRect labelRect(x - 30, graphRect.bottom + 5, x + 30, graphRect.bottom + 20);
        pDC->DrawText(label, &labelRect, DT_CENTER | DT_TOP | DT_SINGLELINE);
    }
    CFont font; font.CreatePointFont(90, _T("MS Shell Dlg")); CFont* pOldFont = pDC->SelectObject(&font);
    pDC->DrawText(_T("DPS"), CRect(rect.left + 5, graphRect.top, graphRect.left - 10, graphRect.bottom), DT_LEFT | DT_VCENTER | DT_SINGLELINE);
    pDC->DrawText(_T("Time (s)"), CRect(graphRect.left, graphRect.bottom + 25, graphRect.right, rect.bottom), DT_CENTER | DT_TOP | DT_SINGLELINE);
    CPen dpsPen(PS_SOLID, 2, RGB(0, 120, 215)); pDC->SelectObject(&dpsPen);
    bool first = true;
    for (const auto& point : m_data) {
        int x = graphRect.left + static_cast<int>((point.time - minTime) / (maxTime - minTime) * graphRect.Width());
        int y = graphRect.bottom - static_cast<int>((point.dps - minDps) / (maxDps - minDps) * graphRect.Height());
        if (x < graphRect.left) x = graphRect.left; if (x > graphRect.right) x = graphRect.right;
        if (y < graphRect.top) y = graphRect.top; if (y > graphRect.bottom) y = graphRect.bottom;
        if (first) { pDC->MoveTo(x, y); first = false; } else pDC->LineTo(x, y);
    }
    double avgDps = 0; for (const auto& point : m_data) avgDps += point.dps; avgDps /= m_data.size();
    int yAvg = graphRect.bottom - static_cast<int>((avgDps - minDps) / (maxDps - minDps) * graphRect.Height());
    if (yAvg >= graphRect.top && yAvg <= graphRect.bottom) {
        CPen avgPen(PS_DASH, 1, RGB(255, 100, 100)); pDC->SelectObject(&avgPen);
        pDC->MoveTo(graphRect.left, yAvg); pDC->LineTo(graphRect.right, yAvg);
        CString avgLabel; avgLabel.Format(_T("Avg: %.0f"), avgDps); pDC->SetTextColor(RGB(255, 100, 100));
        pDC->DrawText(avgLabel, CRect(graphRect.right - 80, yAvg - 15, graphRect.right, yAvg - 2), DT_RIGHT | DT_BOTTOM | DT_SINGLELINE);
    }
    pDC->SetTextColor(RGB(0, 0, 0));
    pDC->DrawText(_T("DPS Over Time"), CRect(graphRect.left, rect.top + 5, graphRect.right, graphRect.top), DT_CENTER | DT_TOP | DT_SINGLELINE);
    pDC->SelectObject(pOldPen); pDC->SelectObject(pOldFont);
}

CResultsPanel::CResultsPanel() {}
CResultsPanel::~CResultsPanel() {}

void CResultsPanel::OnDraw(CDC* pDC) { CRect rect; GetClientRect(&rect); pDC->FillSolidRect(&rect, GetSysColor(COLOR_WINDOW)); }

int CResultsPanel::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
    if (CView::OnCreate(lpCreateStruct) == -1) return -1;
    CRect rect; GetClientRect(&rect);
    int x = 10, y = 10, w = rect.Width() - 20, h = 25;
    if (!m_progressSim.Create(WS_CHILD | WS_VISIBLE | PBS_SMOOTH, CRect(x, y, x + w, y + h), this, IDC_PROGRESS_SIM)) return -1;
    m_progressSim.SetRange(0, 100); m_progressSim.SetPos(0);
    y += h + 10;
    m_editDPS.Create(WS_CHILD | WS_VISIBLE | WS_BORDER | ES_READONLY, CRect(x, y, x + w/2, y + h), this, IDC_EDIT_DPS);
    m_editDPS.SetWindowText(_T("DPS: --"));
    m_editSimTime.Create(WS_CHILD | WS_VISIBLE | WS_BORDER | ES_READONLY, CRect(x + w/2 + 10, y, x + w, y + h), this, IDC_EDIT_SIM_TIME);
    m_editSimTime.SetWindowText(_T("시간: --"));
    y += h + 10;
    m_btnCopyResult.Create(_T("결과 복사"), WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, CRect(x, y, x + w/3 - 5, y + h), this, IDC_BUTTON_COPY_RESULT);
    m_btnExportJson.Create(_T("JSON 저장"), WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, CRect(x + w/3 + 5, y, x + 2*w/3 - 5, y + h), this, IDC_BUTTON_EXPORT_JSON);
    m_btnExportCsv.Create(_T("CSV 저장"), WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, CRect(x + 2*w/3 + 5, y, x + w, y + h), this, IDC_BUTTON_EXPORT_CSV);
    y += h + 10;
    if (!m_tabResults.Create(WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS, CRect(x, y, x + w, rect.bottom - 10), this, IDC_TAB_RESULTS)) return -1;
    SetupTabs();
    int tabHeight = rect.bottom - y - 10, listTop = y + 30;
    if (!m_listResults.Create(WS_CHILD | WS_VISIBLE | WS_BORDER | LVS_REPORT, CRect(x + 5, listTop, x + w - 5, y + tabHeight - 5), this, IDC_LIST_RESULTS)) return -1;
    m_listResults.SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);
    m_listResults.InsertColumn(0, _T("항목"), LVCFMT_LEFT, 150);
    m_listResults.InsertColumn(1, _T("값"), LVCFMT_LEFT, 200);
    m_dpsGraph.Create(_T(""), WS_CHILD | WS_BORDER | SS_OWNERDRAW, CRect(x + 5, listTop, x + w - 5, y + tabHeight - 5), this, 65535);
    if (!m_listRotation.Create(WS_CHILD | WS_BORDER | LVS_REPORT, CRect(x + 5, listTop, x + w - 5, y + tabHeight - 5), this, IDC_LIST_ROTATION)) return -1;
    m_listRotation.SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);
    m_listRotation.InsertColumn(0, _T("시간"), LVCFMT_LEFT, 80);
    m_listRotation.InsertColumn(1, _T("기술명"), LVCFMT_LEFT, 150);
    m_listRotation.InsertColumn(2, _T("대상"), LVCFMT_LEFT, 100);
    m_listRotation.InsertColumn(3, _T("데미지"), LVCFMT_RIGHT, 100);
    if (!m_listActions.Create(WS_CHILD | WS_BORDER | LVS_REPORT, CRect(x + 5, listTop, x + w - 5, y + tabHeight - 5), this, IDC_LIST_ACTIONS)) return -1;
    m_listActions.SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);
    m_listActions.InsertColumn(0, _T("기술"), LVCFMT_LEFT, 150);
    m_listActions.InsertColumn(1, _T("횟수"), LVCFMT_RIGHT, 70);
    m_listActions.InsertColumn(2, _T("총 데미지"), LVCFMT_RIGHT, 100);
    m_listActions.InsertColumn(3, _T("평균"), LVCFMT_RIGHT, 80);
    m_listActions.InsertColumn(4, _T("DPS"), LVCFMT_RIGHT, 80);
    m_listActions.InsertColumn(5, _T("%"), LVCFMT_RIGHT, 60);
    ShowTab(0);
    return 0;
}

void CResultsPanel::SetupTabs()
{
    m_tabResults.InsertItem(0, _T("요약"));
    m_tabResults.InsertItem(1, _T("그래프"));
    m_tabResults.InsertItem(2, _T("딜 사이클"));
    m_tabResults.InsertItem(3, _T("기술 통계"));
}

void CResultsPanel::ShowTab(int nTab)
{
    m_listResults.ShowWindow(SW_HIDE); m_dpsGraph.ShowWindow(SW_HIDE);
    m_listRotation.ShowWindow(SW_HIDE); m_listActions.ShowWindow(SW_HIDE);
    switch (nTab) {
    case 0: m_listResults.ShowWindow(SW_SHOW); break;
    case 1: m_dpsGraph.ShowWindow(SW_SHOW); m_dpsGraph.Invalidate(); break;
    case 2: m_listRotation.ShowWindow(SW_SHOW); break;
    case 3: m_listActions.ShowWindow(SW_SHOW); break;
    }
}

void CResultsPanel::OnSize(UINT nType, int cx, int cy)
{
    CView::OnSize(nType, cx, cy);
    if (m_progressSim.GetSafeHwnd()) {
        int x = 10, y = 10, w = cx - 20, h = 25;
        m_progressSim.MoveWindow(x, y, w, h); y += h + 10;
        m_editDPS.MoveWindow(x, y, w/2 - 5, h); m_editSimTime.MoveWindow(x + w/2 + 5, y, w/2 - 5, h); y += h + 10;
        m_btnCopyResult.MoveWindow(x, y, w/3 - 5, h); m_btnExportJson.MoveWindow(x + w/3 + 5, y, w/3 - 5, h); m_btnExportCsv.MoveWindow(x + 2*w/3 + 5, y, w/3 - 5, h); y += h + 10;
        int tabBottom = cy - 10; m_tabResults.MoveWindow(x, y, w, tabBottom - y);
        int listTop = y + 30, listBottom = tabBottom - 5;
        m_listResults.MoveWindow(x + 5, listTop, w - 10, listBottom - listTop);
        m_dpsGraph.MoveWindow(x + 5, listTop, w - 10, listBottom - listTop);
        m_listRotation.MoveWindow(x + 5, listTop, w - 10, listBottom - listTop);
        m_listActions.MoveWindow(x + 5, listTop, w - 10, listBottom - listTop);
    }
}

void CResultsPanel::OnInitialUpdate() { CView::OnInitialUpdate(); }
void CResultsPanel::OnTcnSelchangeTabResults(NMHDR*, LRESULT* pResult) { ShowTab(m_tabResults.GetCurSel()); *pResult = 0; }
void CResultsPanel::SetProgress(int percent) { m_progressSim.SetPos(percent); }

void CResultsPanel::LoadResults(const CString& resultJson)
{
    m_strResultJson = resultJson;
    CResultHistoryManager* pMgr = GetResultHistoryManager();
    if (pMgr) {
        CSimResult result;
        if (pMgr->ParseSimcJson(resultJson, result)) { DisplayResult(result); ParseSimcJsonDetailed(resultJson); }
        else { m_editDPS.SetWindowText(_T("DPS: Error")); m_editSimTime.SetWindowText(_T("Time: Error")); }
    }
}

void CResultsPanel::ClearResults()
{
    m_strDPS.Empty(); m_strSimTime.Empty(); m_strResultJson.Empty();
    m_editDPS.SetWindowText(_T("DPS: --")); m_editSimTime.SetWindowText(_T("Time: --"));
    m_listResults.DeleteAllItems(); m_listRotation.DeleteAllItems(); m_listActions.DeleteAllItems();
    m_dpsGraph.Clear(); m_progressSim.SetPos(0); m_dpsData.clear(); m_actionData.clear();
}

BOOL CResultsPanel::ParseSimcJson(const CString& jsonFile)
{
    try {
        std::string filePath = std::string(CT2A(jsonFile)); std::ifstream file(filePath); if (!file.is_open()) return FALSE;
        json j; file >> j;
        if (j.contains("sim") && j["sim"].contains("players")) {
            auto& players = j["sim"]["players"];
            if (!players.empty() && players[0].contains("collected_data")) {
                auto& data = players[0]["collected_data"];
                if (data.contains("dps")) {
                    double dps = data["dps"]["mean"].get<double>();
                    CString dpsStr; dpsStr.Format(_T("DPS: %.1f"), dps); m_editDPS.SetWindowText(dpsStr);
                }
            }
        }
        return TRUE;
    } catch (...) { return FALSE; }
}

BOOL CResultsPanel::ParseSimcJsonDetailed(const CString& jsonStr)
{
    try {
        std::string jsonData = std::string(CT2A(jsonStr, CP_UTF8)); json j = json::parse(jsonData);
        m_dpsData.clear();
        if (j.contains("sim") && j["sim"].contains("players")) {
            auto& player = j["sim"]["players"][0];
            if (player.contains("collected_data") && player["collected_data"].contains("timeline")) {
                auto& dpsData = player["collected_data"]["timeline"]["dps"];
                for (size_t i = 0; i < dpsData.size(); ++i) {
                    DpsDataPoint p; p.time = (double)i; p.dps = dpsData[i].get<double>(); p.count = 1; m_dpsData.push_back(p);
                }
            }
            m_actionData.clear();
            if (player.contains("actions")) {
                for (auto& action : player["actions"]) {
                    ActionData data; data.name = CString(action["name"].get<std::string>().c_str());
                    data.count = action.value("num_executes", 0.0); data.totalDmg = action.value("total_amount", 0.0);
                    data.avgDmg = action.value("avg_actual_amount", 0.0); data.dps = action.value("aps", 0.0) * data.avgDmg;
                    data.percentage = action.value("portion_amount", 0.0) * 100.0; m_actionData.push_back(data);
                }
                std::sort(m_actionData.begin(), m_actionData.end(), [](const ActionData& a, const ActionData& b) { return a.totalDmg > b.totalDmg; });
            }
        }
        DisplayDpsGraph(); DisplayRotation(); DisplayActions(); return TRUE;
    } catch (...) { return FALSE; }
}

void CResultsPanel::DisplayResult(const CSimResult& result)
{
    CString dpsStr; dpsStr.Format(_T("DPS: %.1f"), result.GetDpsStats().mean); m_editDPS.SetWindowText(dpsStr);
    CString timeStr; timeStr.Format(_T("시간: %.1f 초"), result.GetSimTime()); m_editSimTime.SetWindowText(timeStr);
    m_listResults.DeleteAllItems(); int idx = 0; CString value;
    m_listResults.InsertItem(idx, _T("평균 DPS")); value.Format(_T("%.1f"), result.GetDpsStats().mean); m_listResults.SetItemText(idx++, 1, value);
    m_listResults.InsertItem(idx, _T("최소 DPS")); value.Format(_T("%.1f"), result.GetDpsStats().minVal); m_listResults.SetItemText(idx++, 1, value);
    m_listResults.InsertItem(idx, _T("최대 DPS")); value.Format(_T("%.1f"), result.GetDpsStats().maxVal); m_listResults.SetItemText(idx++, 1, value);
    if (result.GetDpsStats().stdDev > 0) { m_listResults.InsertItem(idx, _T("표준 편차")); value.Format(_T("%.1f"), result.GetDpsStats().stdDev); m_listResults.SetItemText(idx++, 1, value); }
    m_listResults.InsertItem(idx, _T("반복 횟수")); value.Format(_T("%d"), result.GetIterations()); m_listResults.SetItemText(idx++, 1, value);
    m_listResults.InsertItem(idx, _T("전투 시간")); value.Format(_T("%.1f 초"), result.GetSimTime()); m_listResults.SetItemText(idx++, 1, value);
}

void CResultsPanel::DisplayDpsGraph() { if (!m_dpsData.empty()) m_dpsGraph.SetData(m_dpsData); }

void CResultsPanel::DisplayRotation()
{
    m_listRotation.DeleteAllItems(); int idx = 0;
    if (m_actionData.empty()) { m_listRotation.InsertItem(idx, _T("-")); m_listRotation.SetItemText(idx, 1, _T("데이터 없음")); return; }
    for (const auto& action : m_actionData) {
        if (action.count > 0) {
            m_listRotation.InsertItem(idx, _T("-")); m_listRotation.SetItemText(idx, 1, action.name); m_listRotation.SetItemText(idx, 2, _T("대상"));
            CString dmg; dmg.Format(_T("%.0f"), action.totalDmg); m_listRotation.SetItemText(idx, 3, dmg);
            idx++; if (idx >= 50) break;
        }
    }
}

void CResultsPanel::DisplayActions()
{
    m_listActions.DeleteAllItems(); int idx = 0; CString value;
    for (const auto& action : m_actionData) {
        m_listActions.InsertItem(idx, action.name);
        value.Format(_T("%.0f"), action.count); m_listActions.SetItemText(idx, 1, value);
        value.Format(_T("%.0f"), action.totalDmg); m_listActions.SetItemText(idx, 2, value);
        value.Format(_T("%.0f"), action.avgDmg); m_listActions.SetItemText(idx, 3, value);
        value.Format(_T("%.0f"), action.dps); m_listActions.SetItemText(idx, 4, value);
        value.Format(_T("%.1f%%"), action.percentage); m_listActions.SetItemText(idx, 5, value);
        idx++;
    }
}

void CResultsPanel::OnBnClickedButtonCopyResult()
{
    if (OpenClipboard()) {
        EmptyClipboard(); CString text = m_strResultJson;
        HGLOBAL hClip = GlobalAlloc(GMEM_MOVEABLE, (text.GetLength() + 1) * sizeof(TCHAR));
        if (hClip) { LPTSTR pClip = (LPTSTR)GlobalLock(hClip); _tcscpy_s(pClip, text.GetLength() + 1, text); GlobalUnlock(hClip); SetClipboardData(CF_UNICODETEXT, hClip); }
        CloseClipboard(); AfxMessageBox(_T("클립보드에 복사되었습니다."), MB_ICONINFORMATION);
    }
}

void CResultsPanel::OnBnClickedButtonExportJson()
{
    CFileDialog dlg(FALSE, _T("json"), _T("result.json"), OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, _T("JSON (*.json)|*.json||"), this);
    if (dlg.DoModal() == IDOK) {
        CStdioFile file; if (file.Open(dlg.GetPathName(), CFile::modeCreate | CFile::modeWrite | CFile::typeText)) { file.WriteString(m_strResultJson); file.Close(); AfxMessageBox(_T("JSON으로 저장되었습니다."), MB_ICONINFORMATION); }
    }
}

void CResultsPanel::OnBnClickedButtonExportCsv()
{
    CFileDialog dlg(FALSE, _T("csv"), _T("result.csv"), OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, _T("CSV (*.csv)|*.csv||"), this);
    if (dlg.DoModal() == IDOK) ExportResultsToCsv(dlg.GetPathName());
}

BOOL CResultsPanel::ExportResultsToCsv(const CString& filePath)
{
    try {
        std::string jsonData = std::string(CT2A(m_strResultJson, CP_UTF8)); json j = json::parse(jsonData);
        CStdioFile csvFile; if (!csvFile.Open(filePath, CFile::modeCreate | CFile::modeWrite | CFile::typeText)) return FALSE;
        const char bom[] = "\xEF\xBB\xBF"; csvFile.Write(bom, 3);
        csvFile.WriteString(_T("Metric,Value\n"));
        if (j.contains("sim") && j["sim"].contains("players")) {
            auto& player = j["sim"]["players"][0];
            if (player.contains("collected_data") && player["collected_data"].contains("dps")) {
                auto& dps = player["collected_data"]["dps"]; CString line;
                if (dps.contains("mean")) { line.Format(_T("Mean DPS,%.2f\n"), dps["mean"].get<double>()); csvFile.WriteString(line); }
                if (dps.contains("min")) { line.Format(_T("Min DPS,%.2f\n"), dps["min"].get<double>()); csvFile.WriteString(line); }
                if (dps.contains("max")) { line.Format(_T("Max DPS,%.2f\n"), dps["max"].get<double>()); csvFile.WriteString(line); }
            }
        }
        csvFile.Close(); AfxMessageBox(_T("CSV로 저장되었습니다."), MB_ICONINFORMATION); return TRUE;
    } catch (...) { return FALSE; }
}

CString CResultsPanel::FormatCsvValue(const json& j, const char* key)
{
    if (j.contains(key)) {
        auto& val = j[key]; if (val.is_number()) { CString str; str.Format(_T("%.2f"), val.get<double>()); return str; }
        else if (val.is_string()) return CString(val.get<std::string>().c_str());
    }
    return _T("");
}
