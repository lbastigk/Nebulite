# Nebulite Game Engine
[![Author](https://img.shields.io/badge/Author-lbastigk-blue)](https://github.com/lbastigk)

## Overview

**Nebulite** is a C++-based 2D game engine built for arbitrary rulesets and flexible inter-object logic.
In Nebulite, `RenderObjects` can interact with each other through a self-other-global relationship using the `Invoke` class, 
provided logical conditions between the two are satisfied.

This non-hierarchical architecture enables complex interactive systems and modular subsystems. Examples:
- movement being triggered by `$(global.input.keyboard.w)` -> sets velocity or request to move a distance
- animation being triggered by attributes: `$(self.isMoving)` -> increment spritesheet offset
- boundary check being triggered by `$(other.isSolid)` -> forces velocity of self to 0

This engine is built using **SDL** for rendering and **RapidJSON** as well as a custom caching system to handle the fast structuring of game data such as `RenderObjects`, `Levels`, and their respective attributes.

The main engine provides the core functionality of the game, handling:

- Rendering with **SDL**.
- Game logic and entity management.
- Level loading and object management using **RapidJSON** for structured data.
- Environments, Renderobjects etc. can be loaded with tasks. See `./Application/TaskFiles/` for examples.

## Usage Examples

### Basic Engine Operations
```bash
# Run with immediate commands
./bin/Nebulite 'set-fps 60 ; spawn Resources/Renderobjects/standard.json ; wait 1 ; snapshot'

# Batch operations from script
./bin/Nebulite task TaskFiles/Benchmarks/gravity.txt
```

### Interactive Mode
Start engine and enter console mode with `^`. Enter `help` to see available commands

### Headless Mode (for automation/testing)
```bash
# Generate previews without GUI
./bin/Nebulite --headless 'spawn ... ; wait 1 ; snapshot preview.png ; exit'

# Automated testing
./bin/Nebulite --headless task ...
```

## Directory Structure

```
Nebulite/
├── .recall/                  # Database for pseudo project documentation (recall tool). See: https://github.com/lbastigk/recall
├── Application/              # Runtime environment
│   ├── bin/                  # Compiled binaries
│   ├── Resources/            # Game assets and data
│   │   ├── CreationScripts   # Python scripts for mock-asset-creation
│   │   ├── ...
│   │   ├── Levels/           # Environments
│   │   ├── Renderobjects/    # JSON object definitions
│   │   ├── Snapshots/        # Screenshot output
│   │   └── Sprites/          # Image assets
│   └── TaskFiles/            # Example scripts
├── build/                    # Temporary build output for cmake
├── doc/                      # UML-Diagrams
├── external/                 # Third-party dependencies
├── src/                      # Engine source code
│   ├── main.cpp              # Entry point
│   └── Engine/               # Core engine modules
│       ├── Data/             # JSON, KeyNames, ErrorTypes
│       ├── FuncTrees/        # Command processing
│       ├── Helper/           # Utility functions
│       └── Rendering/        # Rendering and objects
├── build.sh                  # Build automation
├── install.sh                # Setup and installation
└── CMakeLists.txt            # CMake configuration
```

## Platform Support

- **Linux**: Native build and run supported.
- **Windows**: Cross-compiled from Linux using MinGW-w64.

## Dependencies

Nebulite relies on the following third-party dependencies:

| Project                                               | Purpose                                      |
|-------------------------------------------------------|----------------------------------------------|
| [SDL2](https://www.libsdl.org/)                       | Rendering and input handling                 |
| [SDL_ttf](https://github.com/libsdl-org/SDL_ttf)      | Font rendering                               |
| [SDL_image](https://github.com/libsdl-org/SDL_image)  | Extended image format support                |
| [RapidJSON](https://github.com/Tencent/rapidjson)     | JSON parsing for game data                   |
| [Abseil](https://abseil.io/)                          | Fast variable caching                        |
| [Tinyexpr](https://github.com/codeplea/tinyexpr)      | Parsing and evaluating expressions           |

as well as optional dependencies:

| Project                                               | Purpose                                      |
|-------------------------------------------------------|----------------------------------------------|
| [recall](https://github.com/lbastigk/recall/)         | Custom CLI-Tool for quick documentation      |
| [PlantUML](https://plantuml.com/)                     | For compiling UML-Diagrams in ./doc/         |

## Quick start / Contributing

### Development Setup
1. Fork the repository
2. Install dependencies: `./install.sh`
3. Create feature branch: `git checkout -b feature/my-feature`
4. Make changes and test: `./build.sh && cd Application && ./Tests.sh`
5. Submit pull request

### Testing
```bash
# Run test suite
cd Application
./Tests.sh
```

### Adding Features

Nebulite offers clean extensions of its functionality through its FuncTrees:
- New Global commands: Extend `MainTree` class
- New local commands: Extend `RenderObjectTree` class