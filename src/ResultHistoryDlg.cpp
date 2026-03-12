#include "pch.h"
#include "framework.h"
#include "ResultHistoryDlg.h"
#include "ResultHistoryManager.h"
#include "MainFrame.h"
#include "CharacterData.h"
#include "resource.h"

IMPLEMENT_DYNAMIC(CResultHistoryDlg, CDialogEx)

CResultHistoryDlg::CResultHistoryDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_RESULT_HISTORY, pParent)
{
}

CResultHistoryDlg::~CResultHistoryDlg()
{
}

void CResultHistoryDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST_HISTORY, m_listResults);
	DDX_Control(pDX, IDC_EDIT_HISTORY_DETAILS, m_editDetails);
	DDX_Control(pDX, IDC_BUTTON_VIEW_RESULT, m_btnView);
	DDX_Control(pDX, IDC_BUTTON_COMPARE_RESULTS, m_btnCompare);
	DDX_Control(pDX, IDC_BUTTON_DELETE_HISTORY, m_btnDelete);
	DDX_Control(pDX, IDC_BUTTON_EXPORT_HISTORY, m_btnExport);
	DDX_Control(pDX, IDC_BUTTON_CLOSE_HISTORY, m_btnClose);
	DDX_Control(pDX, IDC_COMBO_FILTER, m_comboFilter);
}

BEGIN_MESSAGE_MAP(CResultHistoryDlg, CDialogEx)
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_LIST_HISTORY, &CResultHistoryDlg::OnLvnItemchangedListResults)
	ON_NOTIFY(NM_DBLCLK, IDC_LIST_HISTORY, &CResultHistoryDlg::OnNMDblclkListResults)
	ON_BN_CLICKED(IDC_BUTTON_VIEW_RESULT, &CResultHistoryDlg::OnBnClickedButtonViewResult)
	ON_BN_CLICKED(IDC_BUTTON_COMPARE_RESULTS, &CResultHistoryDlg::OnBnClickedButtonCompare)
	ON_BN_CLICKED(IDC_BUTTON_DELETE_HISTORY, &CResultHistoryDlg::OnBnClickedButtonDeleteResult)
	ON_BN_CLICKED(IDC_BUTTON_EXPORT_HISTORY, &CResultHistoryDlg::OnBnClickedButtonExportResult)
	ON_BN_CLICKED(IDC_BUTTON_CLOSE_HISTORY, &CResultHistoryDlg::OnBnClickedButtonCloseHistory)
	ON_CBN_SELCHANGE(IDC_COMBO_FILTER, &CResultHistoryDlg::OnCbnSelchangeComboFilter)
END_MESSAGE_MAP()

BOOL CResultHistoryDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	SetWindowText(_T("시뮬레이션 히스토리"));

	// 리스트 컨트롤 설정
	m_listResults.SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);
	m_listResults.InsertColumn(0, _T("날짜"), LVCFMT_LEFT, 130);
	m_listResults.InsertColumn(1, _T("캐릭터"), LVCFMT_LEFT, 100);
	m_listResults.InsertColumn(2, _T("전문화"), LVCFMT_LEFT, 80);
	m_listResults.InsertColumn(3, _T("IL"), LVCFMT_RIGHT, 50);
	m_listResults.InsertColumn(4, _T("DPS"), LVCFMT_RIGHT, 80);
	m_listResults.InsertColumn(5, _T("전투유형"), LVCFMT_LEFT, 100);

	// 필터 콤본스 설정
	m_comboFilter.AddString(_T("전체 결과"));
	m_comboFilter.AddString(_T("현재 캐릭터만"));
	m_comboFilter.SetCurSel(0);

	// 결과 목록 채우기
	RefreshResultList();

	// 버튼 초기 상태
	m_btnView.EnableWindow(FALSE);
	m_btnCompare.EnableWindow(FALSE);
	m_btnDelete.EnableWindow(FALSE);
	m_btnExport.EnableWindow(FALSE);

	return TRUE;
}

void CResultHistoryDlg::OnLvnItemchangedListResults(NMHDR* pNMHDR, LRESULT* pResult)
{
	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);
	*pResult = 0;

	// 선택된 항목 수집
	m_selectedIndices.clear();
	POSITION pos = m_listResults.GetFirstSelectedItemPosition();
	while (pos)
	{
		int nItem = m_listResults.GetNextSelectedItem(pos);
		int index = (int)m_listResults.GetItemData(nItem);
		m_selectedIndices.push_back(index);
	}

	// 버튼 상태 업데이트
	BOOL hasSelection = !m_selectedIndices.empty();
	BOOL hasSingleSelection = m_selectedIndices.size() == 1;
	BOOL hasDoubleSelection = m_selectedIndices.size() == 2;

	m_btnView.EnableWindow(hasSingleSelection);
	m_btnExport.EnableWindow(hasSingleSelection);
	m_btnDelete.EnableWindow(hasSelection);
	m_btnCompare.EnableWindow(hasDoubleSelection);

	UpdateDetails();
}

void CResultHistoryDlg::OnNMDblclkListResults(NMHDR* pNMHDR, LRESULT* pResult)
{
	*pResult = 0;
	OnBnClickedButtonViewResult();
}

void CResultHistoryDlg::UpdateDetails()
{
	CString details;

	if (m_selectedIndices.size() == 1)
	{
		CResultHistoryManager* pMgr = GetResultHistoryManager();
		if (pMgr)
		{
			CSimResult* pResult = pMgr->GetResultByIndex(m_selectedIndices[0]);
			if (pResult)
			{
				details += _T("=== 시뮬레이션 정보 ===\r\n");
				details.AppendFormat(_T("시간: %s\r\n"), pResult->GetTimestampString());
				details.AppendFormat(_T("캐릭터: %s - %s (%s)\r\n"),
					CString(pResult->GetBuildInfo().characterName),
					CString(pResult->GetBuildInfo().specName),
					CString(pResult->GetBuildInfo().className));
				details.AppendFormat(_T("아이템 레벨: %.1f\r\n"), pResult->GetBuildInfo().itemLevel);

				details += _T("\r\n=== DPS 통계 ===\r\n");
				details.AppendFormat(_T("평균: %.1f\r\n"), pResult->GetDpsStats().mean);
				details.AppendFormat(_T("최소: %.1f\r\n"), pResult->GetDpsStats().minVal);
				details.AppendFormat(_T("최대: %.1f\r\n"), pResult->GetDpsStats().maxVal);
				details.AppendFormat(_T("표준편차: %.2f\r\n"), pResult->GetDpsStats().stdDev);
				details.AppendFormat(_T("오차: %.2f\r\n"), pResult->GetDpsStats().error);

				details += _T("\r\n=== 시뮬레이션 설정 ===\r\n");
				details.AppendFormat(_T("전투 유형: %s\r\n"), pResult->GetFightStyle());
				details.AppendFormat(_T("지속 시간: %d초\r\n"), pResult->GetDuration());
				details.AppendFormat(_T("반복 횟수: %d\r\n"), pResult->GetIterations());
				details.AppendFormat(_T("실행 시간: %.1f초\r\n"), pResult->GetSimTime());

				// 상위 스킬 5개
				const auto& abilities = pResult->GetAbilities();
				if (!abilities.empty())
				{
					details += _T("\r\n=== 주요 스킬 ===\r\n");
					details += _T("스킬명 / DPS / 비율\r\n");

					int count = 0;
					for (const auto& ability : abilities)
					{
						if (ability.pct < 1.0) // 1% 미만 제외
							continue;

						details.AppendFormat(_T("%s / %.1f / %.1f%%\r\n"),
							ability.name, ability.dps, ability.pct);

						if (++count >= 5)
							break;
					}
				}
			}
		}
	}
	else if (m_selectedIndices.size() == 2)
	{
		details = _T("두 개의 결과를 선택했습니다.\r\n'비교' 버튼을 클릭하여 상세 비교를 확인하세요.");
	}
	else if (m_selectedIndices.size() > 2)
	{
		details.Format(_T("%d개의 결과가 선택되었습니다.\r\n삭제하려면 '삭제' 버튼을 클릭하세요."),
			(int)m_selectedIndices.size());
	}
	else
	{
		details = _T("결과를 선택하세요.");
	}

	m_editDetails.SetWindowText(details);
}

void CResultHistoryDlg::OnBnClickedButtonViewResult()
{
	if (m_selectedIndices.size() != 1)
		return;

	// 결과 상세 보기
	CResultHistoryManager* pMgr = GetResultHistoryManager();
	if (!pMgr)
		return;

	CSimResult* pResult = pMgr->GetResultByIndex(m_selectedIndices[0]);
	if (pResult)
	{
		// JSON 파일 열기
		CString jsonPath = pResult->GetJsonPath();
		if (!jsonPath.IsEmpty() && PathFileExists(jsonPath))
		{
			ShellExecute(NULL, _T("open"), jsonPath, NULL, NULL, SW_SHOWNORMAL);
		}
		else
		{
			AfxMessageBox(_T("원본 JSON 파일을 찾을 수 없습니다."), MB_ICONWARNING);
		}
	}
}

void CResultHistoryDlg::OnBnClickedButtonCompare()
{
	if (m_selectedIndices.size() != 2)
		return;

	CResultHistoryManager* pMgr = GetResultHistoryManager();
	if (!pMgr)
		return;

	CString comparison = pMgr->CompareResultsByIndex(m_selectedIndices[0], m_selectedIndices[1]);

	// 비교 결과를 메시지 박스 또는 별도 다이얼로그로 표시
	CFont font;
	font.CreatePointFont(90, _T("Consolas"));

	// 간단히 메시지 박스 사용 (향후 별도 다이얼로그로 개선 가능)
	AfxMessageBox(comparison, MB_ICONINFORMATION);
}

void CResultHistoryDlg::OnBnClickedButtonDeleteResult()
{
	if (m_selectedIndices.empty())
		return;

	CString msg;
	if (m_selectedIndices.size() == 1)
		msg = _T("선택한 결과를 삭제하시겠습니까?");
	else
		msg.Format(_T("선택한 %d개의 결과를 삭제하시겠습니까?"), (int)m_selectedIndices.size());

	if (AfxMessageBox(msg, MB_YESNO | MB_ICONQUESTION) != IDYES)
		return;

	// 역순으로 삭제 (인덱스 유지를 위해)
	CResultHistoryManager* pMgr = GetResultHistoryManager();
	if (pMgr)
	{
		std::sort(m_selectedIndices.begin(), m_selectedIndices.end(), std::greater<int>());
		for (int index : m_selectedIndices)
		{
			pMgr->DeleteResultByIndex(index);
		}

		RefreshResultList();
		m_editDetails.SetWindowText(_T(""));
	}
}

void CResultHistoryDlg::OnBnClickedButtonExportResult()
{
	ExportSelectedResult();
}

void CResultHistoryDlg::ExportSelectedResult()
{
	if (m_selectedIndices.size() != 1)
		return;

	CResultHistoryManager* pMgr = GetResultHistoryManager();
	if (!pMgr)
		return;

	CSimResult* pResult = pMgr->GetResultByIndex(m_selectedIndices[0]);
	if (!pResult)
		return;

	CFileDialog dlg(FALSE, _T("csv"), NULL,
		OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
		_T("CSV 파일 (*.csv)|*.csv|JSON 파일 (*.json)|*.json||"));

	if (dlg.DoModal() == IDOK)
	{
		CString path = dlg.GetPathName();
		CString ext = PathFindExtension(path);
		ext.MakeLower();

		BOOL success = FALSE;
		if (ext == _T(".json"))
		{
			success = pMgr->ExportResultToJson(pResult->GetId(), path);
		}
		else
		{
			success = pMgr->ExportResultToCsv(pResult->GetId(), path);
		}

		if (success)
		{
			AfxMessageBox(_T("결과가 낵스포트되었습니다."), MB_ICONINFORMATION);
		}
		else
		{
			AfxMessageBox(_T("낵스포트에 실패했습니다."), MB_ICONERROR);
		}
	}
}

void CResultHistoryDlg::OnBnClickedButtonCloseHistory()
{
	EndDialog(IDOK);
}

void CResultHistoryDlg::OnCbnSelchangeComboFilter()
{
	// 필터 변경 시 목록 갱신
	RefreshResultList();
}

void CResultHistoryDlg::RefreshResultList()
{
	m_listResults.DeleteAllItems();

	CResultHistoryManager* pMgr = GetResultHistoryManager();
	if (!pMgr)
		return;

	// 현재 필터 설정 확인
	int filterSel = m_comboFilter.GetCurSel();
	CString currentCharacter;

	if (filterSel == 1) // "현재 캐릭터만" 필터
	{
		// 메인 프레임에서 현재 캐릭터 정보 가져오기
		CMainFrame* pFrame = static_cast<CMainFrame*>(AfxGetMainWnd());
		if (pFrame && pFrame->GetCharacterData())
		{
			currentCharacter = CString(pFrame->GetCharacterData()->GetName().c_str());
		}
	}

	const auto& results = pMgr->GetAllResults();
	for (int i = 0; i < pMgr->GetResultCount(); ++i)
	{
		const auto& result = results[i];
		const auto& build = result.GetBuildInfo();

		// 캐릭터 필터 적용
		if (filterSel == 1 && !currentCharacter.IsEmpty())
		{
			CString resultChar(build.characterName);
			if (resultChar.CompareNoCase(currentCharacter) != 0)
				continue; // 현재 캐릭터가 아니면 스킵
		}

		int nItem = m_listResults.InsertItem(i, result.GetTimestampString());
		m_listResults.SetItemText(nItem, 1, build.characterName);
		m_listResults.SetItemText(nItem, 2, build.specName);

		CString ilvl;
		ilvl.Format(_T("%.1f"), build.itemLevel);
		m_listResults.SetItemText(nItem, 3, ilvl);

		CString dps;
		dps.Format(_T("%.1f"), result.GetDpsStats().mean);
		m_listResults.SetItemText(nItem, 4, dps);

		m_listResults.SetItemText(nItem, 5, result.GetFightStyle());

		// 결과 ID를 항목 데이터로 저장
		m_listResults.SetItemData(nItem, (DWORD_PTR)i);
	}
}

CString CResultHistoryDlg::GetSelectedResultId() const
{
	if (m_selectedIndices.size() != 1)
		return CString();

	CResultHistoryManager* pMgr = GetResultHistoryManager();
	if (!pMgr)
		return CString();

	CSimResult* pResult = pMgr->GetResultByIndex(m_selectedIndices[0]);
	if (pResult)
		return pResult->GetId();

	return CString();
}
