# WoW Simbot Quick 개발 환경 구축 가이드

이 문서는 새 컴퓨터에서 **WoW Simbot Quick** 프로젝트의 개발 환경을 설정하는 단계를 상세히 설명합니다.

## 1. 시스템 요구 사양
- **운영체제**: Windows 10 또는 Windows 11 (64비트)
- **여유 공간**: 최소 5GB 이상의 디스크 공간 (Visual Studio 설치 포함)

## 2. 필수 도구 설치

### A. Visual Studio 2026 설치
이 프로젝트는 최신 C++ 기능을 활용하므로 Visual Studio 2026 이상이 필요합니다.
- **워크로드 선택**: `C++를 사용한 데스크톱 개발` (Desktop development with C++)
- **개별 구성 요소**:
    - `최신 v14x 빌드 도구용 C++ MFC(x86 및 x64)` (C++ MFC for latest v14x build tools)
    - `C++ CMake 도구` (선택 사항이지만 권장)
    - `Windows 11 SDK` (최신 버전)

### B. Git 설치
- [git-scm.com](https://git-scm.com/)에서 Windows용 Git을 설치합니다.

### C. Inno Setup (설치 파일 생성용)
- 설치 프로그램(`.exe`)을 제작하려면 [Inno Setup 6](https://jrsoftware.org/isinfo.php) 이상이 필요합니다.

## 3. 프로젝트 설정

### A. 저장소 복제
```powershell
git clone <repository-url>
cd wow-simc
```

### B. 외부 종속성 확인
프로젝트는 다음 라이브러리를 사용하며, 이미 프로젝트 내에 포함되어 있습니다.
- **nlohmann/json**: `include/json.hpp`에 위치한 헤더 전용 라이브러리입니다.
- **SimulationCraft (simc.exe)**: 실행 시 필요한 엔진입니다. [SimulationCraft Releases](https://github.com/simulationcraft/simc/releases)에서 최신 버전을 다운로드하여 실행 파일이 위치할 폴더(또는 `bin/Release/`)에 배치해야 합니다.

## 4. 빌드 가이드

### Visual Studio IDE 사용
1. `WoWSimbotQuick.sln` 파일을 엽니다.
2. 솔루션 구성에서 `Debug` 또는 `Release`를 선택합니다.
3. 플랫폼을 `x64`로 설정합니다.
4. `F7` 키 또는 `빌드 > 솔루션 빌드`를 선택합니다.

### 명령줄(MSBuild) 사용
PowerShell을 사용하여 빌드할 수 있습니다. (MSBuild 경로는 설치된 VS 버전에 따라 다를 수 있습니다.)

**Release 빌드:**
```powershell
powershell.exe -NoProfile -Command "& 'C:\Program Files\Microsoft Visual Studio\18\Community\MSBuild\Current\Bin\msbuild.exe' WoWSimbotQuick.vcxproj -p:Configuration=Release -p:Platform=x64 -m"
```

**Debug 빌드:**
```powershell
powershell.exe -NoProfile -Command "& 'C:\Program Files\Microsoft Visual Studio\18\Community\MSBuild\Current\Bin\msbuild.exe' WoWSimbotQuick.vcxproj -p:Configuration=Debug -p:Platform=x64 -m"
```

## 5. 주요 설정 및 주의사항

- **C++ 표준**: ISO C++20 표준(`/std:c++20`)을 사용합니다.
- **MFC 링크**: `공유 DLL에서 MFC 사용` (Shared DLL)으로 설정되어 있습니다.
- **런타임 라이브러리**:
  - Release: `다중 스레드 DLL (/MD)`
  - Debug: `다중 스레드 디버그 DLL (/MDd)`
- **문자 집합**: `유니코드 문자 집합 사용`이 기본입니다. 한국어 환경에서의 인코딩 문제를 방지하기 위해 소스 파일은 `UTF-8 (BOM 포함)`로 저장하는 것을 권장합니다.

## 6. 트러블슈팅
1. **MFC 관련 링크 오류**: Visual Studio 설치 시 "C++ MFC" 구성 요소가 누락되었는지 확인하세요.
2. **simc.exe 못 찾음**: 시뮬레이션 실행 시 오류가 발생한다면 `simc.exe`가 실행 파일과 동일한 경로에 있는지 확인하세요.
3. **MSBuild 경로 오류**: 본인의 Visual Studio 설치 경로에 맞춰 빌드 스크립트의 MSBuild 경로를 수정해야 할 수 있습니다.

---
*마지막 업데이트: 2026년 3월 11일*
