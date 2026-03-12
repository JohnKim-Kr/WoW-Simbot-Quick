# System Architecture

## Core Design
**WoW Simbot Quick** follows a standard MFC Doc/View-like architecture (using `CFrameWnd` and `CSplitterWnd`) to manage a multi-panel UI while delegating heavy operations to specialized worker classes.

### Component Interaction
- **CMainFrame**: The central orchestrator. It manages the UI layout (splitters), holds instances of `CSimcRunner` and `CCharacterData`, and coordinates data flow between panels.
- **CSimcRunner**: Encapsulates process management for `simc.exe`. It handles asynchronous execution, stdout redirection for progress tracking, and error reporting.
- **CSimcParser**: Specialized parser that converts raw SimulationCraft addon text into structured `CCharacterData`.
- **CCharacterData**: The primary data model. It stores character state and provides logic to serialize this state into SimulationCraft profile format (`.simc`).

## Data Flow
1. **Input Phase**:
   - User pastes raw profile text from the SimulationCraft in-game addon into the `CCharInputPanel`.
   - `CCharInputPanel` triggers `CMainFrame::ParseSimcProfile()`.
   - `CSimcParser` populates `CCharacterData` from the raw text.
2. **Configuration Phase**:
   - User adjusts simulation options (Fight style, iterations, etc.) in `CSimSettingsPanel`.
3. **Execution Phase**:
   - `CMainFrame` calls `CCharacterData::ToSimcProfile()`.
   - `CSimcRunner` starts `simc.exe` with the generated profile and UI options.
   - Progress is piped back to `CMainFrame` via `WM_USER_SIM_PROGRESS`.
4. **Result Phase**:
   - `simc.exe` produces a JSON/HTML report.
   - `CResultsPanel` displays the outcome.

## UI Structure
- `m_wndMainSplitter`: Splits the frame into Left (Input/Results) and Right (Settings).
- `m_wndLeftSplitter`: Splits the Left side into Top (Character Input) and Bottom (Results Display).
- Panels:
  - `CCharInputPanel`: Text area for pasting addon output and parsing controls.
  - `CSimSettingsPanel`: Fight parameters and buff overrides.
  - `CResultsPanel`: Performance metrics and report viewing.
