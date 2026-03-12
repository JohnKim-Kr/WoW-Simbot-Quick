# Build Failure Info Checklist (Other Windows PC)

다른 Windows PC에서 `WoWSimbotQuick` 빌드가 실패할 때, 원인을 빠르게 특정하기 위한 필수 정보 수집 가이드입니다.

## 1) 빌드 명령(원문 그대로)
- 실제 실행한 명령 전체를 붙여주세요.
- 예시:

```powershell
msbuild WoWSimbotQuick.vcxproj -p:Configuration=Release -p:Platform=x64 -m
```

## 2) 실패 로그(원문)
- 아래 코드가 포함된 줄은 반드시 포함:
- `error Cxxxx`
- `LNKxxxx`
- `MSBxxxx`
- 가능하면 `msbuild.binlog`도 함께 첨부:

```powershell
msbuild WoWSimbotQuick.vcxproj -p:Configuration=Release -p:Platform=x64 -m /bl
```

## 3) 빌드 환경 정보
- Windows 버전
- Visual Studio 버전/에디션 (예: Visual Studio 2026 Community 18.x)
- 설치 워크로드
  - Desktop development with C++
  - MFC/ATL support
- MSVC Toolset 버전 (v14x)
- Windows SDK 버전 (10.0.x)

## 4) 소스 코드 상태
- 브랜치명
- 커밋 해시
- `git status --short` 결과 (로컬 수정 유무 확인)

## 5) 프로젝트/플랫폼 설정
- 실제 선택한:
  - `Configuration` (`Debug` / `Release`)
  - `Platform` (`x64` / `Win32`)
- `.vcxproj`, `.props`, `.targets` 로컬 변경 여부

## 6) 의존 파일 존재 여부
- `include/json.hpp` 파일 존재 여부
- `simc.exe` 경로 이슈는 보통 런타임 문제이므로, 빌드 실패와 직접 관련 있는지 분리해서 기록

---

## 공유 템플릿 (복붙용)

```text
[Build Command]
...

[Error Log]
...

[Environment]
Windows:
Visual Studio:
Workloads (C++/MFC):
MSVC Toolset:
Windows SDK:

[Git]
Branch:
Commit:
git status --short:

[Project Settings]
Configuration:
Platform:
Local .vcxproj/.props/.targets edits:

[Notes]
...
```

