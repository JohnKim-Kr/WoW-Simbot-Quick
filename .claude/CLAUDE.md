# Windows C++ 개발 환경 설정

## 프로젝트 개요

**WoW Simbot Quick** - raidbots.com의 Simbot/Quick 기능을 Windows 네이티브 애플리케이션으로 구현

- WoW 캐릭터의 DPS/HPS 시뮬레이션을 로컬에서 빠르게 실행
- SimulationCraft(simc) 엔진 연동

## 개발 환경

- **언어**: C++
- **IDE**: Visual Studio 2026
- **타겟 OS**: Windows 10/11

## 런타임 설정

- **런타임 라이브러리**: Multi-threaded DLL (`/MD`)
- **MFC**: 동적 링크 (Shared DLL)

## 프로젝트 설정 가이드

### Visual Studio 프로젝트 속성

1. **C/C++ > 코드 생성 > 런타임 라이브러리**
   - 디버그: `다중 스레드 디버그 DLL (/MDd)`
   - 릴리스: `다중 스레드 DLL (/MD)`

2. **구성 속성 > 고급 > MFC 사용**
   - `공유 DLL에서 MFC 사용`

3. **C/C++ > 언어 > C++ 언어 표준**
   - ISO C++20 표준 (/std:c++20)

## 애플리케이션 아키텍처

### 주요 기능

1. **캐릭터 데이터 가져오기**
   - Battle.net API 연동 (OAuth2)
   - 캐릭터 장비/특성/재능 정보 조회

2. **시뮬레이션 설정**
   - 전투 유형 선택 (Patchwerk, Dungeon, etc.)
   - 지속 시간/적 수 설정
   - 반복 횟수 설정

3. **Simc 엔진 실행**
   - simc.exe 프로세스 호출
   - 명령줄 인자 구성
   - 비동기 실행 및 진행률 표시

4. **결과 표시**
   - DPS/HPS 출력
   - 스탯 우선순위
   - 결과 낵스포트

### 프로젝트 구조

```
WoWSimbot/
├── src/
│   ├── main.cpp              # 진입점
│   ├── MainFrame.cpp/.h      # MFC 메인 프레임
│   ├── SimcRunner.cpp/.h     # simc 프로세스 관리
│   ├── BnetApiClient.cpp/.h  # Battle.net API
│   ├── CharacterData.cpp/.h  # 캐릭터 데이터 모델
│   └── SimOptionsDlg.cpp/.h  # 시뮬레이션 설정 대화상자
├── include/
│   └── json.hpp              # JSON 파싱 (nlohmann/json)
└── resources/
    └── app.rc                # 리소스 파일
```

### 의존성

- **simc**: SimulationCraft 엔진 (외부 실행 파일)
- **WinHTTP**: Windows HTTP API (Battle.net API 호출)
- **nlohmann/json**: JSON 파싱 라이브러리 (헤더만 포함)

## 주의사항

- 모든 프로젝트에서 런타임 라이브러리 설정을 일관되게 유지
- `/MD`와 `/MDd`를 혼용하지 않도록 주의
- MFC 동적 링크 시 `mfc140.dll` 등의 런타임 의존성 필요
- 배포 시 Visual C++ 재배포 가능 패키지 포함 필요
- simc.exe는 별도 빌드 필요 (https://github.com/simulationcraft/simc)
