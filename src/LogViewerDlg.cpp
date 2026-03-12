#include "pch.h"
#include "framework.h"
#include "LogViewerDlg.h"
#include "Logger.h"
#include "resource.h"

IMPLEMENT_DYNAMIC(CLogViewerDlg, CDialogEx)

CLogViewerDlg::CLogViewerDlg(CWnd* pParent /*=nullptr*/)
    : CDialogEx(IDD_LOG_VIEWER, pParent)
    , m_bAutoScroll(TRUE)
{
}

CLogViewerDlg::~CLogViewerDlg()
{
}

void CLogViewerDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialogEx::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_LIST_LOGS, m_listLogs);
    DDX_Control(pDX, IDC_EDIT_LOG_DETAILS, m_editDetails);
    DDX_Control(pDX, IDC_COMBO_LOG_LEVEL, m_comboLevel);
    DDX_Control(pDX, IDC_COMBO_LOG_CATEGORY, m_comboCategory);
    DDX_Control(pDX, IDC_BUTTON_REFRESH_LOGS, m_btnRefresh);
    DDX_Control(pDX, IDC_BUTTON_CLEAR_LOGS, m_btnClear);
    DDX_Control(pDX, IDC_BUTTON_EXPORT_LOGS, m_btnExport);
    DDX_Control(pDX, IDC_BUTTON_CLOSE_LOG, m_btnClose);
    DDX_Control(pDX, IDC_CHECK_AUTOSCROLL, m_chkAutoScroll);
    DDX_Check(pDX, IDC_CHECK_AUTOSCROLL, m_bAutoScroll);
}

BEGIN_MESSAGE_MAP(CLogViewerDlg, CDialogEx)
    ON_NOTIFY(LVN_ITEMCHANGED, IDC_LIST_LOGS, &CLogViewerDlg::OnLvnItemchangedListLogs)
    ON_CBN_SELCHANGE(IDC_COMBO_LOG_LEVEL, &CLogViewerDlg::OnCbnSelchangeComboLevel)
    ON_CBN_SELCHANGE(IDC_COMBO_LOG_CATEGORY, &CLogViewerDlg::OnCbnSelchangeComboCategory)
    ON_BN_CLICKED(IDC_BUTTON_REFRESH_LOGS, &CLogViewerDlg::OnBnClickedButtonRefresh)
    ON_BN_CLICKED(IDC_BUTTON_CLEAR_LOGS, &CLogViewerDlg::OnBnClickedButtonClearLogs)
    ON_BN_CLICKED(IDC_BUTTON_EXPORT_LOGS, &CLogViewerDlg::OnBnClickedButtonExportLogs)
    ON_BN_CLICKED(IDC_BUTTON_CLOSE_LOG, &CLogViewerDlg::OnBnClickedButtonCloseLog)
    ON_BN_CLICKED(IDC_CHECK_AUTOSCROLL, &CLogViewerDlg::OnBnClickedCheckAutoscroll)
    ON_WM_TIMER()
END_MESSAGE_MAP()

BOOL CLogViewerDlg::OnInitDialog()
{
    CDialogEx::OnInitDialog();

    SetWindowText(_T("로그 뷰어"));

    // 리스트 컨트롤 설정
    m_listLogs.SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);
    SetupLogColumns();

    // 레벨 필터
    m_comboLevel.AddString(_T("전체"));
    m_comboLevel.AddString(_T("Debug"));
    m_comboLevel.AddString(_T("Info"));
    m_comboLevel.AddString(_T("Warning"));
    m_comboLevel.AddString(_T("Error"));
    m_comboLevel.AddString(_T("Critical"));
    m_comboLevel.SetCurSel(1); // Info 기본

    // 카테고리 필터
    m_comboCategory.AddString(_T("전체"));
    m_comboCategory.AddString(_T("General"));
    m_comboCategory.AddString(_T("API"));
    m_comboCategory.AddString(_T("Simc"));
    m_comboCategory.AddString(_T("Simulation"));
    m_comboCategory.AddString(_T("UI"));
    m_comboCategory.AddString(_T("Settings"));
    m_comboCategory.AddString(_T("Network"));
    m_comboCategory.SetCurSel(0);

    // 자동 스크롤
    m_chkAutoScroll.SetCheck(BST_CHECKED);

    // 타이머 시작 (1초 간격)
    SetTimer(LOG_REFRESH_TIMER, 1000, nullptr);

    // 초기 로그 로드
    RefreshLogList();

    return TRUE;
}

void CLogViewerDlg::SetupLogColumns()
{
    m_listLogs.InsertColumn(0, _T("시간"), LVCFMT_LEFT, 130);
    m_listLogs.InsertColumn(1, _T("레벨"), LVCFMT_CENTER, 60);
    m_listLogs.InsertColumn(2, _T("카테고리"), LVCFMT_CENTER, 80);
    m_listLogs.InsertColumn(3, _T("메시지"), LVCFMT_LEFT, 350);
    m_listLogs.InsertColumn(4, _T("소스"), LVCFMT_LEFT, 150);
}

void CLogViewerDlg::RefreshLogList()
{
    CLogger* pLogger = GetLogger();
    if (!pLogger)
        return;

    // 최근 로그 읽기
    CString recentLogs = pLogger->GetRecentLogs(500);

    // 파싱 및 표시
    FilterLogs();
}

void CLogViewerDlg::FilterLogs()
{
    // 선택된 필터 가져오기
    int levelSel = m_comboLevel.GetCurSel();
    int catSel = m_comboCategory.GetCurSel();

    // 로거에서 최근 로그 가져오기
    CLogger* pLogger = GetLogger();
    if (!pLogger)
        return;

    CString recentLogs = pLogger->GetRecentLogs(1000);

    // 리스트 초기화
    m_listLogs.DeleteAllItems();

    // 로그 파싱 및 필터링
    std::vector<CString> lines;
    int pos = 0;
    CString line;
    while (AfxExtractSubString(line, recentLogs, pos++, '\n'))
    {
        line.Trim();
        if (line.IsEmpty())
            continue;

        // 필터 적용
        if (ShouldShowLog(line, levelSel, catSel))
        {
            AddLogLineToList(line);
        }
    }

    // 자동 스크롤
    if (m_bAutoScroll && m_listLogs.GetItemCount() > 0)
    {
        m_listLogs.EnsureVisible(m_listLogs.GetItemCount() - 1, FALSE);
    }
}

BOOL CLogViewerDlg::ShouldShowLog(const CString& line, int levelSel, int catSel)
{
    // 레벨 필터 체크
    if (levelSel > 0) // 0 = 전체
    {
        CString levelStr;
        switch (levelSel)
        {
        case 1: levelStr = _T("DEBUG"); break;
        case 2: levelStr = _T("INFO"); break;
        case 3: levelStr = _T("WARN"); break;
        case 4: levelStr = _T("ERROR"); break;
        case 5: levelStr = _T("CRIT"); break;
        }

        if (line.Find(levelStr) == -1)
        {
            // 다른 레벨이 포함되어 있는지 확인
            return FALSE;
        }
    }

    // 카테고리 필터 체크
    if (catSel > 0) // 0 = 전체
    {
        CString catStr;
        switch (catSel)
        {
        case 1: catStr = _T("GENERAL"); break;
        case 2: catStr = _T("API"); break;
        case 3: catStr = _T("SIMC"); break;
        case 4: catStr = _T("SIM"); break;
        case 5: catStr = _T("UI"); break;
        case 6: catStr = _T("CONFIG"); break;
        case 7: catStr = _T("NET"); break;
        }

        if (line.Find(catStr) == -1)
            return FALSE;
    }

    return TRUE;
}

void CLogViewerDlg::AddLogLineToList(const CString& line)
{
    // 로그 라인 파싱
    // 형식: [2026-03-09 10:30:45] [INFO] [GENERAL] 메시지 (파일:라인)

    CString timestamp, level, category, message, source;

    int pos1 = line.Find('[');
    int pos2 = line.Find(']', pos1);
    if (pos1 >= 0 && pos2 > pos1)
    {
        timestamp = line.Mid(pos1 + 1, pos2 - pos1 - 1);
    }

    pos1 = line.Find('[', pos2);
    pos2 = line.Find(']', pos1);
    if (pos1 >= 0 && pos2 > pos1)
    {
        level = line.Mid(pos1 + 1, pos2 - pos1 - 1);
    }

    pos1 = line.Find('[', pos2);
    pos2 = line.Find(']', pos1);
    if (pos1 >= 0 && pos2 > pos1)
    {
        category = line.Mid(pos1 + 1, pos2 - pos1 - 1);
    }

    int msgStart = pos2 + 2;
    int srcPos = line.Find(_T(" ("), msgStart);
    if (srcPos > 0)
    {
        message = line.Mid(msgStart, srcPos - msgStart);
        source = line.Mid(srcPos + 2, line.GetLength() - srcPos - 3);
    }
    else
    {
        message = line.Mid(msgStart);
    }

    int nItem = m_listLogs.InsertItem(m_listLogs.GetItemCount(), timestamp);
    m_listLogs.SetItemText(nItem, 1, level);
    m_listLogs.SetItemText(nItem, 2, category);
    m_listLogs.SetItemText(nItem, 3, message);
    m_listLogs.SetItemText(nItem, 4, source);
}

void CLogViewerDlg::OnLvnItemchangedListLogs(NMHDR* pNMHDR, LRESULT* pResult)
{
    LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);
    *pResult = 0;

    POSITION pos = m_listLogs.GetFirstSelectedItemPosition();
    if (pos)
    {
        int nItem = m_listLogs.GetNextSelectedItem(pos);

        // 상세 정보 표시
        CString details;
        details += _T("시간: ");
        details += m_listLogs.GetItemText(nItem, 0);
        details += _T("\r\n레벨: ");
        details += m_listLogs.GetItemText(nItem, 1);
        details += _T("\r\n카테고리: ");
        details += m_listLogs.GetItemText(nItem, 2);
        details += _T("\r\n메시지: ");
        details += m_listLogs.GetItemText(nItem, 3);
        details += _T("\r\n소스: ");
        details += m_listLogs.GetItemText(nItem, 4);

        m_editDetails.SetWindowText(details);
    }
}

void CLogViewerDlg::OnCbnSelchangeComboLevel()
{
    FilterLogs();
}

void CLogViewerDlg::OnCbnSelchangeComboCategory()
{
    FilterLogs();
}

void CLogViewerDlg::OnBnClickedButtonRefresh()
{
    RefreshLogList();
}

void CLogViewerDlg::OnBnClickedButtonClearLogs()
{
    if (AfxMessageBox(_T("모든 로그를 삭제하시겠습니까?"), MB_YESNO | MB_ICONQUESTION) == IDYES)
    {
        CLogger* pLogger = GetLogger();
        if (pLogger)
        {
            pLogger->ClearAllLogs();
            m_listLogs.DeleteAllItems();
            m_editDetails.SetWindowText(_T(""));
        }
    }
}

void CLogViewerDlg::OnBnClickedButtonExportLogs()
{
    CFileDialog dlg(FALSE, _T("txt"), _T("WoWSimbotQuick_Logs"),
        OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
        _T("텍스트 파일 (*.txt)|*.txt|로그 파일 (*.log)|*.log|CSV 파일 (*.csv)|*.csv||"));

    if (dlg.DoModal() == IDOK)
    {
        CLogger* pLogger = GetLogger();
        if (pLogger)
        {
            CString content = pLogger->GetRecentLogs(1000);

            CStdioFile file;
            if (file.Open(dlg.GetPathName(),
                CFile::modeCreate | CFile::modeWrite | CFile::typeText))
            {
                file.WriteString(content);
                file.Close();

                AfxMessageBox(_T("로그가 낵스포트되었습니다."), MB_ICONINFORMATION);
            }
        }
    }
}

void CLogViewerDlg::OnBnClickedButtonCloseLog()
{
    EndDialog(IDOK);
}

void CLogViewerDlg::OnBnClickedCheckAutoscroll()
{
    UpdateData(TRUE);
}

void CLogViewerDlg::OnTimer(UINT_PTR nIDEvent)
{
    if (nIDEvent == LOG_REFRESH_TIMER)
    {
        // 주기적으로 새 로그 확인
        RefreshLogList();
    }

    CDialogEx::OnTimer(nIDEvent);
}

void CLogViewerDlg::OnNewLogEntry(const LogEntry& entry)
{
    // 필터 체크
    int levelSel = m_comboLevel.GetCurSel();
    if (levelSel > 0) // 0 = 전체
    {
        if ((int)entry.level < levelSel - 1)
            return;
    }

    // 리스트에 추가
    int nItem = m_listLogs.InsertItem(m_listLogs.GetItemCount(), entry.GetTimestampString());
    m_listLogs.SetItemText(nItem, 1, entry.GetLevelString());
    m_listLogs.SetItemText(nItem, 2, entry.GetCategoryString());
    m_listLogs.SetItemText(nItem, 3, entry.message);

    CString source;
    if (!entry.sourceFile.IsEmpty())
    {
        source.Format(_T("%s:%d"), entry.sourceFile, entry.lineNumber);
    }
    m_listLogs.SetItemText(nItem, 4, source);

    // 자동 스크롤
    if (m_bAutoScroll)
    {
        m_listLogs.EnsureVisible(nItem, FALSE);
    }

    // 최대 항목 수 제한
    if (m_listLogs.GetItemCount() > 1000)
    {
        m_listLogs.DeleteItem(0);
    }
}
