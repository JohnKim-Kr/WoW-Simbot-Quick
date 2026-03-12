# Profile Parsing & Engine Integration

## SimulationCraft Addon Parsing
Instead of external APIs, the application relies on the text output from the in-game SimulationCraft addon.

- **Parser**: `CSimcParser` iterates through the lines of the pasted text.
- **Key Fields**:
  - `druid="CharacterName"` (or other class name) identifies the character.
  - `level`, `race`, `region`, `server` define basic identity.
  - `spec` defines the active talent specialization.
  - `talents` contains the Base64 talent string (Dragonflight/Midnight format).
  - Gear lines (e.g., `head=,id=...,bonus_id=...`) are parsed into `ItemData`.

## SimulationCraft (Simc) Engine
- **Binary**: Expects `simc.exe` to be available in a path configured in settings.
- **Execution**: 
  - Command: `simc.exe <profile.simc> <overrides>`
  - Overrides are passed as CLI arguments (e.g., `iterations=10000`).
- **Input Format**: 
  - `CCharacterData::ToSimcProfile()` reconstructs the profile from the parsed data to ensure it is clean and optimized for the local simulation.
- **Output Parsing**:
  - Simc writes progress to `stderr` (e.g., `1/100000`).
  - Results are read from a generated JSON file (typically `sim_result.json` in the temp directory).

## WoW Midnight Specifics
- **Max Level**: 80.
- **Talents**: Uses the 10.0+ "Base64" talent export string format.
- **Expansion Options**:
  - `temporary_enchant` for weapon oils/stones.
  - `midnight.*` flags for expansion-specific mechanics like the Crucible of Erratic Energies.
