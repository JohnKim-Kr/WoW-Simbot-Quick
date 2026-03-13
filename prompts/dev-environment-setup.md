# 개발환경 설정 가이드

---

## 1. 개요

### 1.1 목적
Windows 개발을 위한 개발환경 설정 가이드입니다.
Windows C++ 개발이 처음인 팀원도 본 문서를 따라 환경 구축이 가능하도록 작성했습니다.

### 1.2 개발환경 요약

| 항목 | 값 |
|:-----|:---|
| **IDE** | Visual Studio 2022 (v143) |
| **플랫폼** | x64 |
| **문자 세트** | Unicode |
| **런타임** | MultiThreadedDLL (동적 CRT) |
| **MFC** | 동적 링크 (MFC DLL) |
| **OS** | Windows 10/11 |

---

## 2. Visual Studio 2022 설치

### 2.1 다운로드
- [Visual Studio 2022 다운로드 페이지](https://visualstudio.microsoft.com/ko/downloads/)
- **에디션**: Community (무료) 또는 Professional/Enterprise

### 2.2 워크로드 선택

Visual Studio Installer에서 아래 워크로드를 반드시 선택합니다.

#### 필수 워크로드
- **C++를 사용한 데스크톱 개발** (Desktop development with C++)

#### 필수 개별 구성 요소
워크로드 선택 후 "개별 구성 요소" 탭에서 아래 항목을 확인합니다.

| 구성 요소 | 필수 여부 | 설명 |
|:----------|:---------:|:-----|
| MSVC v143 - VS 2022 C++ x64/x86 빌드 도구 | 필수 | C++ 컴파일러 |
| Windows 10/11 SDK (최신) | 필수 | Win32 API 헤더/라이브러리 |
| **C++ MFC (최신, x86 및 x64)** | 필수 | MFC 라이브러리 |
| C++ ATL (최신, x86 및 x64) | 권장 | ATL 라이브러리 |
| C++ 핵심 기능 | 필수 | 기본 C++ 지원 |

> **주의**: MFC 구성 요소를 누락하면 빌드 시 `afxwin.h` 등을 찾을 수 없다는 오류가 발생합니다.

### 2.3 설치 확인
1. Visual Studio 2022 실행
2. "새 프로젝트 만들기" → "MFC 앱" 템플릿이 보이면 MFC 설치 완료
3. "Windows 데스크톱 마법사" 템플릿이 보이면 Win32 SDK 설치 완료

---

## 3. 프로젝트 빌드 설정

### 3.1 솔루션/프로젝트 파일

```
src/Win/AICollector/
├── AICollector.sln              # 솔루션 파일 (VS에서 이 파일을 엶)
├── AICollector.vcxproj          # 프로젝트 파일
└── src/                         # 소스 코드
```

### 3.2 프로젝트 속성 (확인 사항)

솔루션 파일을 열고, 프로젝트 속성에서 아래 설정을 확인합니다.

| 속성 경로 | 값 | 비고 |
|:----------|:---|:-----|
| 구성 관리자 → 활성 솔루션 플랫폼 | **x64** | 64비트 빌드 |
| 일반 → 플랫폼 도구 집합 | **v143** | VS2022 |
| 일반 → 문자 집합 | **유니코드 문자 집합 사용** | Unicode |
| 일반 → MFC 사용 | **공유 DLL에서 MFC 사용** | 동적 링크 |
| C/C++ → 코드 생성 → 런타임 라이브러리 | **다중 스레드 DLL (/MD)** (Release) | |
| | **다중 스레드 디버그 DLL (/MDd)** (Debug) | |
| C/C++ → 미리 컴파일된 헤더 | **사용 (/Yu)** | stdafx.h |

### 3.3 플랫폼 선택

본 프로젝트는 **x64** 빌드를 대상으로 합니다.

| 플랫폼 | 설명 | 출력 경로 (Debug) | 출력 경로 (Release) |
|:-------|:-----|:-----------------|:-------------------|
| **x64** | 64-bit 빌드 | `x64\Debug\` | `x64\Release\` |

---

## 4. 라이브러리 설정

### 4.1 추가 링크 라이브러리

프로젝트 속성 → 링커 → 입력 → 추가 종속성에 아래 라이브러리를 추가합니다.

| 라이브러리 | 용도 | 필요 모듈 |
|:----------|:-----|:----------|
| `iphlpapi.lib` | 네트워크 연결 정보 (GetExtendedTcpTable 등) | NetworkMonitor |
| `pdh.lib` | 성능 카운터 (패킷량 등) | NetworkMonitor |
| `winhttp.lib` | HTTPS POST 전송 | DataSender |
| `psapi.lib` | 프로세스 메모리 정보 (GetProcessMemoryInfo) | ProcessMonitor |
| `wtsapi32.lib` | 터미널 서비스 API | AgentService |
| `advapi32.lib` | 서비스 관리, 레지스트리 | AgentService, SystemInfo |
| `ole32.lib` | COM 초기화 (UIAutomation) | UIAutomation |
| `oleaut32.lib` | COM Automation | UIAutomation |

---

## 5. 트러블슈팅

### 5.1 MFC 헤더를 찾을 수 없음

**증상**: `fatal error C1083: 포함 파일을 열 수 없습니다: 'afxwin.h'`

**원인**: VS2022 설치 시 MFC 구성 요소 미설치

**해결**:
1. Visual Studio Installer 실행
2. "수정" 클릭
3. "개별 구성 요소" 탭
4. "C++ MFC (최신, x86 및 x64)" 체크
5. "수정" 클릭하여 설치

### 5.2 x64 플랫폼 구성이 없음

**증상**: 구성 관리자에 x64 플랫폼이 보이지 않음

**원인**: x64 플랫폼 구성이 생성되지 않음

**해결**:
1. 구성 관리자 열기 (빌드 → 구성 관리자)
2. "활성 솔루션 플랫폼" 드롭다운 → "새로 만들기" 클릭
3. "새 플랫폼" → **x64** 선택
4. "다음에서 설정 복사" → 기존 플랫폼 선택
5. "확인" 클릭

### 5.3 DLL 누락 오류

**증상**: `프로그램을 시작할 수 없습니다. xxx.dll이(가) 없습니다.`

**원인**: MFC DLL 등이 실행 경로에 없음

**해결**:
1. VS2022 재배포 가능 패키지 설치 확인
2. 프로젝트 속성 → 일반 → MFC 사용이 "공유 DLL에서 MFC 사용"인지 확인

### 5.4 링크 오류 (LNK2019: 확인할 수 없는 외부 기호)

**증상**: `error LNK2019: 확인할 수 없는 외부 기호 "xxx" ...`

**원인**: 필요한 라이브러리가 링커에 추가되지 않음

**해결**:
1. 프로젝트 속성 → 링커 → 입력 → 추가 종속성 확인
2. 섹션 4.2의 라이브러리 목록과 비교하여 누락된 항목 추가

### 5.5 Unicode 관련 컴파일 오류

**증상**: `'LPCSTR'에서 'LPCWSTR'(으)로 변환할 수 없습니다`

**원인**: 문자열 리터럴에 `L` 접두사 누락 또는 `_T()` 매크로 미사용

**해결**:
```cpp
// 잘못된 예
const char* str = "Hello";        // ANSI

// 올바른 예
const TCHAR* str = _T("Hello");   // Unicode 호환
LPCWSTR str = L"Hello";           // Wide string
```

### 5.6 미리 컴파일된 헤더 오류

**증상**: `fatal error C1010: 미리 컴파일된 헤더를 찾는 동안 예기치 않은 파일의 끝`

**원인**: .cpp 파일 첫 줄에 `#include "stdafx.h"` 누락

**해결**:
```cpp
// 모든 .cpp 파일의 첫 번째 줄에 추가
#include "stdafx.h"
```

---

## 6. 참고 자료

| 항목 | 링크/위치 |
|:-----|:----------|
| VS2022 다운로드 | https://visualstudio.microsoft.com/ko/downloads/ |
| nlohmann/json 문서 | https://github.com/nlohmann/json |
| Win32 API 레퍼런스 | https://learn.microsoft.com/ko-kr/windows/win32/ |
| MFC 레퍼런스 | https://learn.microsoft.com/ko-kr/cpp/mfc/ |
| 아키텍처 설계 문서 | docs/prompts/Win/architecture-design.md |
| 상세 기술 명세 | docs/prompts/Win/spec.md |

---

*문서 종료*
