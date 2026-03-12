# WoW Simbot Quick - Project Mandates

This document contains foundational mandates for the **WoW Simbot Quick** project. Adhere to these instructions strictly during development.

## Project Overview
**WoW Simbot Quick** is a Windows native application implementing the Simbot/Quick functionality from raidbots.com. It enables fast local DPS/HPS simulations for World of Warcraft characters using the SimulationCraft (simc) engine.

## Engineering Standards & Technical Stack
- **Language**: C++ (ISO C++20 Standard `/std:c++20`)
- **Framework**: MFC (Microsoft Foundation Classes) - **Linked as Shared DLL**
- **IDE**: Visual Studio 2026 (Windows 10/11)
- **Runtime Library**: 
  - Release: Multi-threaded DLL (`/MD`)
  - Debug: Multi-threaded Debug DLL (`/MDd`)
- **JSON**: `nlohmann/json` (header-only, located in `include/json.hpp`)
- **Input Method**: SimulationCraft (simc) addon profile text (pasted directly into the UI)

## Architecture & Project Structure
- `src/`: Source files (`.cpp`, `.h`)
- `include/`: Third-party headers
- `resources/`: Resource files (`.rc`, `.h`)
- `bin/`: Output directory for executables
- `obj/`: Intermediate build files

### Key Components
- `MainFrame.h/cpp`: MFC Main window and UI orchestration.
- `SimcRunner.h/cpp`: Manages `simc.exe` process execution and async progress tracking.
- `SimcParser.h/cpp`: Logic for parsing simc addon profile text and talent strings.
- `CharacterData.h/cpp`: Data models for characters, gear, and talents; generates simc profile lines.
- `CharInputPanel.h/cpp`: UI for pasting and triggering the parsing of simc addon output.

## Development Context & Guidance
For deep technical details and specific implementation workflows, strictly follow the documentation in the `prompts/` directory. These files are mandatory reading for all development tasks:
- **`prompts/ARCHITECTURE.md`**: Core system design and data flow.
- **`prompts/IMPLEMENTATION_GUIDE.md`**: Coding standards, string handling, and MFC/Win32 conventions.
- **`prompts/API_INTEGRATION.md`**: Detailed specs for Battle.net API and Simc engine interaction.

## Build Commands

### MSBuild (Command Line)
Use the following commands to build the project from the shell:

**Release x64:**
```powershell
powershell.exe -NoProfile -Command "& 'C:\Program Files\Microsoft Visual Studio\18\Community\MSBuild\Current\Bin\msbuild.exe' WoWSimbotQuick.vcxproj -p:Configuration=Release -p:Platform=x64 -m"
```

**Debug x64:**
```powershell
powershell.exe -NoProfile -Command "& 'C:\Program Files\Microsoft Visual Studio\18\Community\MSBuild\Current\Bin\msbuild.exe' WoWSimbotQuick.vcxproj -p:Configuration=Debug -p:Platform=x64 -m"
```

*Note: Adjust MSBuild path if Visual Studio is installed in a different location (e.g., Professional/Enterprise).*

## WoW Expansion: MIDNIGHT Mandates
- **Talent Strings**: Base64 encoded. Use `SimcParser::ParseTalentLine` to handle Dragonflight/MIDNIGHT formats.
- **Output Format**: Ensure talent lines are formatted as `talents=BASE64_STRING`.
- **Expansion Specific Options**:
  - `temporary_enchant=main_hand:algari_mana_oil_3`
  - `midnight.crucible_of_erratic_energies_violence=1` (and related nodes)

## Development Workflow
1. **Verification**: Before submitting any change, ensure the project builds successfully using MSBuild.
2. **Consistency**: Maintain `/MD` vs `/MDd` settings strictly. Do not mix runtime library configurations.
3. **External Dependencies**: `simc.exe` is required at runtime but is NOT part of this repository's build process.
4. **Encoding**: Use UTF-8 for source files. Ensure Korean characters in comments or UI resources are handled correctly.
