# WoW Simbot Quick - 테스트 계획

**버전:** 1.0.0
**작성일:** 2026-03-09

---

## 테스트 환경

| 항목 | 요구사항 |
|------|----------|
| OS | Windows 10/11 (64-bit) |
| 런타임 | Visual C++ Redistributable 2015-2022 (x64) |
| IDE | Visual Studio 2022 (테스트 시에만 필요) |
| 의존성 | simc.exe (SimulationCraft) |

---

## 테스트 단계

### Phase 1: 단위 테스트

#### 1.1 캐릭터 데이터 (CharacterData)
- [ ] CCharacterData 생성/소멸 테스트
- [ ] 기본 정보 설정/조회 (이름, 서버, 지역)
- [ ] 장비 정보 추가/조회
- [ ] 특성 정보 설정
- [ ] simc 형식 변환 (ToSimcProfile)

#### 1.2 시뮬레이션 결과 (SimResult)
- [ ] CSimResult 생성 테스트
- [ ] DPS/HPS 설정/조회
- [ ] StatWeight 추가/조회
- [ ] 능력치 기여도 추가
- [ ] JSON 직렬화/역직렬화

#### 1.3 설정 관리 (SettingsManager)
- [ ] 기본값 로드 테스트
- [ ] 설정 저장/로드 (레지스트리)
- [ ] simc 경로 설정/조회
- [ ] API 키 암호화 저장/복호화

#### 1.4 로거 (Logger)
- [ ] 로그 초기화/정리
- [ ] 5단계 로그 작성 (Debug~Critical)
- [ ] 카테고리별 로그 작성
- [ ] 로그 파일 순환 (Rotation)
- [ ] 로그 콜백 기능

---

### Phase 2: 통합 테스트

#### 2.1 Battle.net API 연동 (BnetApiClient)
- [ ] OAuth2 인증 URL 생성
- [ ] Access Token 획득 (Mock 테스트)
- [ ] 캐릭터 프로필 조회
- [ ] 장비 정보 조회
- [ ] 특성 정보 조회
- [ ] 에러 처리 (401, 404, 500)
- [ ] 재시도 로직 테스트

#### 2.2 simc 실행 (SimcRunner)
- [ ] simc.exe 경로 검증
- [ ] 프로세스 생성/실행
- [ ] 명령줄 인자 구성
- [ ] 표준 출력 캡처
- [ ] 진행률 파싱
- [ ] 실행 중 취소
- [ ] 타임아웃 처리

#### 2.3 결과 히스토리 (ResultHistoryManager)
- [ ] 결과 저장
- [ ] 결과 로드
- [ ] 결과 목록 조회
- [ ] 결과 삭제
- [ ] 캐릭터별 필터링
- [ ] 날짜별 정렬

---

### Phase 3: UI 테스트

#### 3.1 메인 프레임 (MainFrame)
- [ ] 윈도우 생성/표시
- [ ] 메뉴 동작 확인
- [ ] 툴팁 표시
- [ ] 상태 표시줄 업데이트
- [ ] 키보드 단축키 (Ctrl+R 등)
- [ ] 크기 조정/최소화/최대화

#### 3.2 캐릭터 입력 패널 (CharInputPanel)
- [ ] 지역 콤볰ox (kr/us/eu/tw)
- [ ] 서버 입력 필드
- [ ] 캐릭터명 입력 필드
- [ ] "Load Character" 버튼
- [ ] "OAuth Login" 버튼
- [ ] 캐릭터 정보 표시 영역

#### 3.3 시뮬레이션 설정 패널 (SimSettingsPanel)
- [ ] 전투 유형 선택 (Patchwerk, Dungeon 등)
- [ ] 지속 시간 설정 (슬라이더 + 입력)
- [ ] 반복 횟수 설정
- [ ] 버프 옵션 체크박스
- [ ] 설정 변경 시 UI 업데이트

#### 3.4 결과 패널 (ResultsPanel)
- [ ] DPS/HPS 표시
- [ ] 아이템 레벨 표시
- [ ] 전투 시간 표시
- [ ] 날짜/시간 표시
- [ ] 상세 결과 버튼
- [ ] 히스토리 저장 버튼

#### 3.5 대화상자
- [ ] 시뮬레이션 옵션 (SimOptionsDlg)
  - [ ] 전투 유형 선택
  - [ ] 지속 시간 입력
  - [ ] 반복 횟수 입력
  - [ ] 확인/취소 버튼
- [ ] 프리셋 관리 (PresetManagerDlg)
  - [ ] 프리셋 목록 표시
  - [ ] 프리셋 추가/삭제
  - [ ] 프리셋 적용
- [ ] 히스토리 (ResultHistoryDlg)
  - [ ] 결과 목록 표시
  - [ ] 결과 선택/로드
  - [ ] 결과 비교
  - [ ] 결과 삭제
- [ ] 로그 뷰어 (LogViewerDlg)
  - [ ] 로그 표시
  - [ ] 로그 레벨 필터
  - [ ] 카테고리 필터
  - [ ] 자동 스크롤

---

### Phase 4: 시스템 테스트

#### 4.1 완전한 시뮬레이션 플로우
- [ ] 애플리케이션 시작
- [ ] simc.exe 경로 설정
- [ ] 캐릭터 정보 입력
- [ ] (Optional) Battle.net API로 캐릭터 로드
- [ ] 시뮬레이션 설정 조정
- [ ] 시뮬레이션 실행
- [ ] 진행률 표시 확인
- [ ] 결과 표시 확인
- [ ] 결과 히스토리 저장
- [ ] 애플리케이션 종료

#### 4.2 에러 시나리오
- [ ] simc.exe 미설치 시 에러 메시지
- [ ] 잘못된 simc 경로 설정
- [ ] 잘못된 캐릭터명 입력
- [ ] 존재하지 않는 서버 입력
- [ ] 네트워크 연결 없음 (API 호출 시)
- [ ] 잘못된 API 키
- [ ] simc 실행 중 충돌

#### 4.3 성능 테스트
- [ ] 애플리케이션 시작 시간 < 3초
- [ ] UI 응답성 (실행 중에도 반응)
- [ ] 메모리 사용량 < 100MB (유휴 시)
- [ ] 로그 파일 10MB 이상 시 성능

---

### Phase 5: 설치/배포 테스트

#### 5.1 설치 프로그램
- [ ] 설치 프로그램 실행
- [ ] 라이선스 동의
- [ ] 설치 경로 선택
- [ ] 설치 진행
- [ ] 바로가기 생성
- [ ] 설치 완료

#### 5.2 제거
- [ ] 제거 프로그램 실행
- [ ] 파일 정리
- [ ] 레지스트리 정리
- [ ] 제거 완료

#### 5.3 휴식성
- [ ] Windows 10에서 실행
- [ ] Windows 11에서 실행
- [ ] 고해상도 디스플레이 (DPI)
- [ ] 다양한 화면 크기

---

## 테스트 체크리스트 요약

```
[ ] Phase 1: 단위 테스트 완료
    [ ] CharacterData
    [ ] SimResult
    [ ] SettingsManager
    [ ] Logger

[ ] Phase 2: 통합 테스트 완료
    [ ] BnetApiClient
    [ ] SimcRunner
    [ ] ResultHistoryManager

[ ] Phase 3: UI 테스트 완료
    [ ] MainFrame
    [ ] CharInputPanel
    [ ] SimSettingsPanel
    [ ] ResultsPanel
    [ ] Dialogs

[ ] Phase 4: 시스템 테스트 완료
    [ ] End-to-end flow
    [ ] Error scenarios
    [ ] Performance

[ ] Phase 5: 설치/배포 테스트 완료
    [ ] Installation
    [ ] Uninstallation
    [ ] Portability
```

---

## 알려진 제한사항

1. **Battle.net OAuth** - 현재 실제 OAuth 콜백 서버 미구현
2. **CSV 낵스포트** - TODO로 남아있음
3. **실시간 로그 업데이트** - 구현 미완료
4. **다국어 지원** - 현재 한국어/영어 혼용 상태

---

## 버그 리포트 템플릿

```
**버그 ID:** BUG-XXX
**심각도:** [Critical/Major/Minor]
**우선순위:** [High/Medium/Low]
**환경:** [Windows 버전, simc 버전]

**설명:**
버그에 대한 간단한 설명

**재현 단계:**
1. 단계 1
2. 단계 2
3. ...

**예상 결과:**
정상 동작 시 결과

**실제 결과:**
버그 발생 시 결과

**스크린샷:**
(첨부)

**로그:**
```
관련 로그 내용
```
```

---

**문서 버전 이력**
| 버전 | 날짜 | 작성자 | 변경 내용 |
|------|------|--------|-----------|
| 1.0 | 2026-03-09 | AI Assistant | 초기 작성 |
