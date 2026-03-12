# WoW Simbot Quick - GPT Working Guide

This document is the project-specific instruction set for GPT-based coding agents working in the **WoW Simbot Quick** repository. Treat it as the local operating guide for implementation, review, and maintenance work.

## Project Overview
**WoW Simbot Quick** is a Windows native application that implements the Simbot/Quick workflow from raidbots.com. Its purpose is to run fast local DPS/HPS simulations for World of Warcraft characters by preparing SimulationCraft (`simc`) inputs, launching `simc.exe`, and presenting results in an MFC desktop UI.

## Agent Operating Rules
- Read this file first, then consult the relevant documents under `prompts/` before making substantial code changes.
- Prefer minimal, targeted edits that preserve the current architecture and MFC conventions.
- Do not rewrite working UI or parsing flows without a concrete reason tied to a bug, requirement, or maintainability issue.
- Assume the repository may contain unrelated user changes. Avoid reverting or reformatting files that are outside the task.
- Validate behavior with the narrowest useful verification step available. A full build is preferred when the change could affect compilation or linking.

## Engineering Standards & Technical Stack
- **Language**: C++ with ISO C++20 (`/std:c++20`)
- **Framework**: MFC (Microsoft Foundation Classes), linked as a shared DLL
- **IDE / Toolchain**: Visual Studio 2026 on Windows 10/11
- **Runtime Library**:
  - Release: Multi-threaded DLL (`/MD`)
  - Debug: Multi-threaded Debug DLL (`/MDd`)
- **JSON**: `nlohmann/json` header-only library in `include/json.hpp`
- **Primary Input**: SimulationCraft addon profile text pasted into the application UI

## Architecture & Project Structure
- `src/`: application source files
- `include/`: third-party headers and shared includes
- `resources/`: Win32 and MFC resource files
- `bin/`: executable output directory
- `obj/`: intermediate build artifacts

### Key Components
- `MainFrame.h/cpp`: main window, layout, and top-level UI orchestration
- `SimcRunner.h/cpp`: `simc.exe` process launch, async execution, and progress tracking
- `SimcParser.h/cpp`: parsing of simc addon text, profile lines, and talent strings
- `CharacterData.h/cpp`: character, gear, and talent data models plus profile generation
- `CharInputPanel.h/cpp`: UI for pasting addon output and triggering parse actions

## Required Reference Documents
The files in `prompts/` are the authoritative project references for implementation details. Load only the documents relevant to the current task, but treat them as mandatory when their subject area applies.

- **`prompts/ARCHITECTURE.md`**: system design, ownership boundaries, and data flow
- **`prompts/IMPLEMENTATION_GUIDE.md`**: coding style, string handling, MFC, and Win32 conventions
- **`prompts/API_INTEGRATION.md`**: Battle.net API behavior and SimulationCraft integration details

## Build Commands
Use MSBuild from PowerShell when you need to verify compilation.

**Release x64**
```powershell
powershell.exe -NoProfile -Command "& 'C:\Program Files\Microsoft Visual Studio\18\Community\MSBuild\Current\Bin\msbuild.exe' WoWSimbotQuick.vcxproj -p:Configuration=Release -p:Platform=x64 -m"
```

**Debug x64**
```powershell
powershell.exe -NoProfile -Command "& 'C:\Program Files\Microsoft Visual Studio\18\Community\MSBuild\Current\Bin\msbuild.exe' WoWSimbotQuick.vcxproj -p:Configuration=Debug -p:Platform=x64 -m"
```

Adjust the MSBuild path if Visual Studio is installed in a different edition or location.

## WoW Expansion Mandates
- **Talent Strings**: Treat talent exports as Base64-encoded strings.
- **Parsing Path**: Use `SimcParser::ParseTalentLine` for Dragonflight and MIDNIGHT-compatible talent formats.
- **Output Format**: Emit talent lines as `talents=BASE64_STRING`.
- **Expansion-Specific Options**: Preserve support for options such as `temporary_enchant=main_hand:algari_mana_oil_3` and MIDNIGHT node flags like `midnight.crucible_of_erratic_energies_violence=1` when the feature set requires them.

## Development Workflow
1. Read the relevant project docs in `prompts/` before changing architecture-sensitive code.
2. Keep `/MD` and `/MDd` usage consistent with the active configuration.
3. Remember that `simc.exe` is a runtime dependency, not a build dependency managed by this repository.
4. Use UTF-8 source encoding and preserve Korean text correctly in comments, resources, and UI strings.
5. Before finishing, run an appropriate verification step and report clearly if verification could not be completed.
