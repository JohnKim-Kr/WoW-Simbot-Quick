# WoW Simbot Quick

A Windows native MFC application for World of Warcraft DPS/HPS simulation using the SimulationCraft (simc) engine.

## Features

- **Character Data Import**: Fetch character information from Battle.net API with OAuth2 authentication
- **Simulation Control**: Run simc.exe locally with real-time progress tracking
- **Fight Styles**: Patchwerk, Light Movement, Heavy Movement, Dungeon, and more
- **Buff Configuration**: Flask, Food, Rune, Augment, Bloodlust, Potions, and external buffs
- **Result Analysis**: DPS/HPS statistics, stat weights, ability contributions
- **History Tracking**: Automatic saving and comparison of simulation results
- **Settings Management**: Presets for different simulation configurations
- **Logging**: Comprehensive logging system for troubleshooting
- **Export**: Results export to JSON and CSV formats

## System Requirements

- **OS**: Windows 10/11 (64-bit)
- **Runtime**: Visual C++ Redistributable 2015-2022 (x64)
- **Dependencies**: SimulationCraft (simc.exe) - not included

## Download

### Option 1: Installer (Recommended)
1. Download `WoWSimbotQuick_Setup_1.0.0.exe` from [Releases](../../releases)
2. Run the installer and follow the instructions
3. During installation, specify the path to simc.exe

### Option 2: Portable
1. Download `WoWSimbotQuick_v1.0.0_portable.zip` from [Releases](../../releases)
2. Extract to a folder of your choice
3. Run `WoWSimbotQuick.exe`

## Building from Source

### Prerequisites
- Visual Studio 2022 (with MFC workload)
- Windows SDK 10.0 or later
- Inno Setup 6 (optional, for installer creation)

### Build Steps
```batch
# Clone repository
git clone https://github.com/wowsimbot/wow-simbot-quick.git
cd wow-simbot-quick

# Build release version
scripts\build_release.bat

# Build installer (requires Inno Setup)
scripts\build_installer.bat
```

## Setup

### 1. Install simc.exe
Download SimulationCraft from the official website:
- https://www.simulationcraft.org/download.html

Extract simc.exe to a known location (e.g., `C:\SimC\simc.exe`)

### 2. Configure simc Path
1. Launch WoW Simbot Quick
2. Go to Tools → Settings
3. Browse to simc.exe location
4. Click Save

### 3. (Optional) Configure Battle.net API
1. Create a client at https://develop.battle.net/
2. Get Client ID and Client Secret
3. Enter them in Tools → Settings
4. Save settings

## Usage

### Loading a Character
1. Select region (kr/us/eu/tw)
2. Enter server name and character name
3. Click "Load Character"
4. For private profiles, use OAuth Login

### Running a Simulation
1. Configure simulation settings (fight style, duration, buffs)
2. Click Run (Ctrl+R) or use toolbar button
3. Monitor progress in the progress bar
4. View results when complete

### Managing Results
- View history: Tools → History
- Compare results: Select two results and click Compare
- Export results: Copy to clipboard or export as JSON/CSV

## Project Structure

```
WoWSimbotQuick/
├── src/                    # Source files
│   ├── SimbotQuick.cpp     # Application entry
│   ├── MainFrame.cpp       # Main window
│   ├── BnetApiClient.cpp   # Battle.net API
│   ├── SimcRunner.cpp      # Simc execution
│   ├── SettingsManager.cpp # Settings management
│   ├── ResultHistoryManager.cpp  # History
│   ├── Logger.cpp          # Logging system
│   └── ...
├── include/                # Third-party headers
│   └── json.hpp           # nlohmann/json
├── resources/             # Resources (RC file)
├── setup/                 # Inno Setup script
├── scripts/               # Build scripts
└── bin/                   # Output directory
```

## Keyboard Shortcuts

| Shortcut | Action |
|----------|--------|
| Ctrl+R | Run Simulation |
| Ctrl+Break | Stop Simulation |
| Ctrl+P | Open Presets |
| Ctrl+H | Open History |
| F5 | Refresh |

## Troubleshooting

### simc.exe not found
- Verify simc.exe path in Tools → Settings
- Ensure simc.exe exists at the specified location

### API authentication fails
- Check Client ID and Client Secret
- Verify network connection
- Check firewall settings

### Simulation fails
- Check simc.exe version (requires 10.0.0 or later)
- Review log: Debug → Log Viewer
- Generate diagnostics: Debug → Diagnostics

## License

This project is licensed under the MIT License - see [LICENSE.txt](LICENSE.txt)

Third-party libraries:
- nlohmann/json: MIT License
- SimulationCraft: GPL v3

## Contributing

Contributions are welcome! Please feel free to submit a Pull Request.

## Support

- Report issues: [GitHub Issues](../../issues)
- Discussion: [GitHub Discussions](../../discussions)

## Acknowledgments

- SimulationCraft team for the amazing simulation engine
- nlohmann for the excellent JSON library
- World of Warcraft community
