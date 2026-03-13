# Trinket Override UI Implementation Plan

이 문서는 `WoW Simbot Quick`에서 **프로필 파싱 후 trinket1/trinket2를 UI에서 교체**하고 시뮬레이션에 반영하기 위한 구현 계획이다.

## 1. 목표
- 사용자가 현재 캐릭터가 소유하지 않은 장신구도 `trinket1`, `trinket2`에 직접 입력/교체하여 DPS를 비교할 수 있게 한다.
- 입력 가능한 필드:
  - `id` (필수)
  - `bonus_id` (선택, 정확도 향상 핵심)
  - `context` (선택)
  - `ilevel` (선택, `bonus_id`가 없을 때 1차 비교용)
- 기존 파싱/생성 흐름을 최대한 유지하고 최소 침습으로 확장한다.

## 2. 용어/정책
- `id`: 아이템 고유 ID
- `bonus_id`: 아이템 변형 식별자 목록(`/` 구분)
- `context`: 획득 컨텍스트 코드 (선택값, 필수 아님)

정책:
- 정확도 우선 비교: `id + bonus_id`
- 빠른 후보 비교: `id + ilevel`
- `context`는 입력 시 출력하되, 미입력 시 생략

## 3. 범위
- 포함:
  - 오른쪽 설정 패널에 Trinket Override UI 추가
  - 파싱된 값 로드 + 사용자 편집 + simc 출력 반영
  - 설정 저장/복원(선택 정책 포함)
- 제외:
  - Midnight 전체 장신구 DB 내장
  - 자동 `bonus_id` 추천 엔진

## 4. 현재 코드 기준 변경 포인트
- 모델/출력:
  - `src/CharacterData.h`
  - `src/CharacterData.cpp`
- 파서:
  - `src/SimcParser.cpp`
- 설정 패널:
  - `resources/WoWSimbotQuick.rc`
  - `src/SimSettingsPanel.h`
  - `src/SimSettingsPanel.cpp`
- 리소스 ID:
  - `src/Resource.h`
- 설정 저장:
  - `src/SimulationSettings.h`
  - `src/SettingsManager.cpp`

## 5. 설계안
### 5.1 데이터 모델
- `SimulationSettings`에 오버라이드 필드 추가:
  - `BOOL useTrinketOverride1`, `BOOL useTrinketOverride2`
  - `CString trinket1Id`, `trinket2Id`
  - `CString trinket1BonusId`, `trinket2BonusId`
  - `CString trinket1Context`, `trinket2Context`
  - `CString trinket1Ilevel`, `trinket2Ilevel`

선택 이유:
- 기존 앱은 설정 패널 입력값을 `SimulationSettings` 중심으로 관리하므로 일관성이 높다.

### 5.2 파싱
- `SimcParser::ParseGearLine`에서 `trinket1=`, `trinket2=` 라인의
  - `id`
  - `bonus_id`
  - `context`
  - `ilevel`
  값을 읽어 `SimulationSettings` 초기값에도 반영한다.

목적:
- 프로필 파싱 직후 UI가 실제 파싱값을 기본값으로 보여주게 한다.

### 5.3 UI
- `SimSettings` 폼에 섹션 추가:
  - `장신구 교체(Override)`
  - `trinket1`: 사용 체크 + id + bonus_id + context + ilevel
  - `trinket2`: 사용 체크 + id + bonus_id + context + ilevel

레이아웃 원칙:
- 현재 오른쪽 패널 400px 기준 유지
- 라벨열/입력열 정렬 일관성 유지
- 너무 긴 필드(`bonus_id`)는 단일 라인 `EDITTEXT`로 가로폭 우선 배치

### 5.4 시뮬레이션 출력 반영
- `CCharacterData::ToSimcProfile()`에서 최종 장비 라인 생성 시:
  - override 사용 체크 ON이면 `SimulationSettings`의 trinket 값으로 강제 출력
  - OFF이면 기존 파싱 아이템값 유지

출력 규칙:
- `id` 필수. 비어있으면 해당 override 무효화
- `bonus_id/context/ilevel`은 비어있지 않을 때만 출력

예:
```text
trinket1=,id=250257,bonus_id=13439/40/12699/12780,context=23
trinket2=,id=250259,ilevel=665
```

## 6. 유효성 검증
- `id/context/ilevel` 숫자 형식 검증
- `bonus_id`는 `숫자(/숫자)*` 패턴 검증
- 검증 실패 시:
  - 저장/실행 전 메시지 표시
  - 해당 필드 포커스 이동

## 7. 단계별 구현 순서
1. `SimulationSettings` 필드 추가 + `SettingsManager` 저장/복원 키 추가
2. `Resource.h` ID 추가 + `.rc`에 Trinket Override UI 배치
3. `SimSettingsPanel` DDX/메시지맵/Load/Save 연동
4. `SimcParser`에서 trinket 라인의 `context/ilevel` 포함 파싱 및 settings 동기화
5. `CharacterData::ToSimcProfile`에 override 우선 출력 로직 추가
6. 입력 유효성 검증 추가
7. Debug/Release x64 빌드 및 기본 동작 검증

## 8. 테스트 체크리스트
- 파싱 직후 trinket1/2 값이 UI에 표시되는가
- override OFF: 기존 파싱된 trinket 라인이 유지되는가
- override ON + `id`만 입력: 라인 출력되는가
- override ON + `id+bonus_id`: 보너스 라인 정상 출력되는가
- override ON + `context`: context가 함께 출력되는가
- 잘못된 `bonus_id` 형식 입력 시 검증 메시지 발생하는가
- Debug x64 / Release x64 빌드 성공 여부

## 9. 리스크와 대응
- 리스크: 기존 장비 파싱 로직과 override 로직 충돌
  - 대응: trinket 슬롯만 override 우선권 부여, 나머지 슬롯 기존 로직 유지
- 리스크: UI 밀집으로 가독성 저하
  - 대응: 섹션 구분 라벨과 열 정렬 고정, 필드폭 최소 기준 유지
- 리스크: `context` 의미 혼동
  - 대응: UI 도움말에 "선택값(비워도 동작)" 명시

## 10. 완료 기준(Definition of Done)
- UI에서 trinket1/trinket2를 개별 편집 가능
- `id/bonus_id/context/ilevel`이 시뮬레이션 입력에 정확히 반영
- 기존 파싱/시뮬레이션 기능 회귀 없음
- Debug/Release x64 빌드 성공
