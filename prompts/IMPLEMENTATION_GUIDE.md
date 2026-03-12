# Implementation & Coding Standards

## Coding Patterns
- **Memory Management**: Use `std::unique_ptr` for component ownership (e.g., in `CMainFrame`). Avoid manual `new`/`delete` where possible.
- **String Handling**: 
  - Use `CString` (TCHAR based) for MFC/Win32 API interactions and UI display.
  - Use `std::string` (UTF-8) for internal data logic, JSON processing, and Simc profile generation.
  - Convert between them using `CW2A` / `CA2W` or project-specific helpers.
- **Concurrency**:
  - Heavy tasks (Simc execution) must be asynchronous or executed in a way that doesn't freeze the UI.
  - Use `PostMessage` / `SendMessage` (e.g., `WM_USER_SIM_PROGRESS`) to update UI from background threads or processes.

## Error Handling
- **Parsing Errors**: `CSimcParser` should return `FALSE` if the input text is malformed or missing critical character identifiers.
- **Process Errors**: `CSimcRunner` returns `FALSE` on failure; check `GetLastError()` for details (e.g., `simc.exe` not found).
- **Validation**: Always validate `CCharacterData::IsValid()` before attempting to generate a profile or start a simulation.

## UI & MFC Conventions
- **Message Maps**: Use the standard MFC `DECLARE_MESSAGE_MAP()` and `BEGIN_MESSAGE_MAP` macros.
- **Resource IDs**: Define all UI control IDs in `Resource.h`.
- **Layout**: Use `OnSize` in `CMainFrame` to ensure splitters and panels resize correctly.

## SimulationCraft Integration
- **Profile Generation**: `CCharacterData::ToSimcProfile()` is the source of truth for the `.simc` file content. 
- **Talents**: Support for Dragonflight/Midnight Base64 talent strings is mandatory.
- **Addon Profiles**: The application primary input is the raw text from the SimulationCraft in-game addon.
