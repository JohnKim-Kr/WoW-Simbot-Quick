# Changelog

All notable changes to WoW Simbot Quick will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [1.0.0] - 2026-03-09

### Added
- Initial release of WoW Simbot Quick
- Battle.net API integration for character data retrieval
- SimulationCraft (simc) integration for DPS/HPS simulation
- Multiple fight styles: Patchwerk, Light Movement, Heavy Movement, Dungeon
- Simulation settings: duration, iterations, target count
- Buff/debuff configuration: Flask, Food, Rune, Augment, Bloodlust, Potions
- External buffs support: Arcane Intellect, Battle Shout, Mystic Touch, etc.
- Results history with comparison feature
- Settings presets management
- Comprehensive logging system with 5 log levels
- Debug console and diagnostics tools
- Export results to JSON and CSV formats
- Korean and English language support

### Features
- **Character Management**: Load characters from Battle.net API with OAuth2
- **Simulation Control**: Run/stop simulations with real-time progress
- **Result Analysis**: View DPS/HPS statistics, stat weights, ability contributions
- **History Tracking**: Automatic saving of simulation results
- **Settings Management**: Save/load simulation presets
- **Logging**: Detailed logging for troubleshooting
- **Diagnostics**: System information and validation tools

### Dependencies
- Windows 10/11
- Visual C++ Redistributable 2015-2022
- SimulationCraft (simc.exe)

## [Unreleased]

### Planned
- Dark mode support
- Additional language translations
- Direct export to Raidbots
- Gearset comparison
- Stat weight calculation
- Custom APL editor
- Integration with WoW Addons (Simulationcraft, etc.)
