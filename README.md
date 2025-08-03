# Nebulite Game Engine
[![Author](https://img.shields.io/badge/Author-lbastigk-blue)](https://github.com/lbastigk)

## Overview

**Nebulite** is a C++-based 2D game engine built for arbitrary rulesets and flexible inter-object logic.
In Nebulite, `RenderObjects` can interact with each other through a self-other-global relationship using the `Invoke` class, 
provided logical conditions between the two are satisfied.

The main engine provides the core functionality of the game, handling:

- Rendering with **SDL**.
- Game logic and entity management.
- Level loading and object management using **RapidJSON** for structured data.
- Environments, Renderobjects etc. can be loaded with tasks. See `./Application/TaskFiles/` for examples.

This non-hierarchical architecture enables complex interactive systems and modular subsystems. 

### Core Philosophy: Self-Other-Global Interactions

Objects interact through a three-tier context system:
- **SELF**: The object broadcasting the logic
- **OTHER**: Target objects being evaluated  
- **GLOBAL**: Shared game state (time, input, settings)

Examples:
- movement being triggered by `$(global.input.keyboard.w)` -> sets velocity or request to move a distance
- animation being triggered by attributes: `$(self.isMoving)` -> increment spritesheet offset
- boundary check being triggered by `$(other.isSolid)` -> forces velocity of self to 0

### Invoke-Files

Invoke json-files are used to describe how objects interact with each other.
Example of an Invoke for a gravity ruleset:
```jsonc
// Add this or a link as string ("./Resources/Invokes/.../gravity.json") to any RenderObject.json that should conform to the gravity ruleset:
// -> append str/object    to its key "invokes"
// Make sure the object listens to topic gravity as well
// -> append str "gravity" to its key "invokeSubscriptions"
{
  "topic": "gravity",   // 
  "logicalArg": "1",    // We already ensured that only objects subscribed to gravity interact
  "exprs": [
    // Pulling in the other object:
    // F  = G * m1 * m2 / r^2
    // a2 = G * m1 / r^2
    // Component form: 
    "other.physics.aX += $($(global.physics.G) * $(self.physics.mass) * ( $(self.posX) - $(other.posX)  ) / ( ( ($(self.posX) - $(other.posX))^2 + ($(self.posY) - $(other.posY))^2 + 1)^(3/2) ))",
    "other.physics.aY += $($(global.physics.G) * $(self.physics.mass) * ( $(self.posY) - $(other.posY)  ) / ( ( ($(self.posX) - $(other.posX))^2 + ($(self.posY) - $(other.posY))^2 + 1)^(3/2) ))"
  ],
  "functioncalls_global": [],   // If necessary, we could add debug statements like "echo here!"
  "functioncalls_self": [],     // Useful for new object-alignments or copying the current state of the object: "copy physics backup.physics", "copy posX backup.posX" ...
  "functioncalls_other": []
}
```
*This creates realistic planetary motion with just JSON configuration!*

## Quick start

1. Fork the repository
2. Install dependencies: `./install.sh`
3. Create feature branch: `git checkout -b feature/my-feature`
4. Make changes and test: `./build.sh && cd Application && ./Tests.sh`

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
├── include/                  # Header files
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

Please note that CLI-Based documenation with recall is a work-in-progress.

## Contributing

We welcome contributions! Nebulite's modular architecture makes it easy to add features in separate files.

### Development Setup

Using VSCode is recommended for an optimal workflow.

### Testing

Go into the Application directory first: `cd Application/`

- Use `Tests.sh` for preconfigured tests.
- Use `CrashDebug.sh` for debugging crashes
- Use `MemLeakTest.sh` for Memory Leak testing

You can add custom taskfiles to the test suite as well by extending its variable `tests`, or run them on your own with:
```bash
./bin/Nebulite task TaskFiles/.../your_test.txt
```

VSCode users benefit from preconfigured tasks: `CTRL + SHIFT + T` (test options) or `CTRL + SHIFT + B` (build options)

### Preview Editing

Preview Editing is a work-in-progress. The current plan is to use the headless rendering mode of Nebulite in combination with 
either a taskfile or a python-script to allow rendering snapshots while editing json files.

### Adding Features

Nebulite offers clean expansions of its functionality through its FuncTrees. 
Maintainers can create their own Tree-expansion classes and add them to the specifc FuncTree.

| New commands operating on... | Action                        | Info                                                                    |
|------------------------------|-------------------------------|-------------------------------------------------------------------------|
| global level                 | Extend the `MainTree`         | See `include/MainTree.h` and its expansions `include/MTE_*.h`           |
| specific RenderObjects       | Extend the `RenderObjectTree` | See `include/RenderObjectTree.h` and its expansions `include/RTE_*.h`   |
| specific JSON-Documents      | Extend the `JSONTree`         | See `include/JSONTree.h` and its expansions `include/JTE_*.h`           |

Example procedure for a new MainTree feature:
1. **Create expansion file:** `MTE_MyFeature.{h,cpp}`
2. **Inherit from wrapper:** Create class inheriting from `MainTreeExpansion::Wrapper<MyFeature>`
3. **Implement command methods:** Functions with `ERROR_TYPE (int argc, char* argv[])` signature
4. **Override setupBindings():** Register your commands with the function tree
5. **Add to MainTree:** Include in `include/MainTree.h` and initialize in constructor

**Complete code example:**

Inside MyFeature.h:

```cpp
class MyFeature : public MainTreeExpansion::Wrapper<MyFeature> {
public:
    ERROR_TYPE spawnCircle(int argc, char* argv[]);
private:
    void setupBindings() override;
};
```
Inside MyFeature.cpp:
```cpp
#include "MyFeature.h"
ERROR_TYPE spawnCircle(int argc, char* argv[]){
    /*...*/
}
```
Then add to include/MainTree.h:
```cpp
#include "MyFeature.h"
class MainTree public FuncTreeWrapper<Nebulite::ERROR_TYPE>{
    /*...*/
private:
    /*...*/
    std::unique_ptr<MainTreeExpansion::MyFeature> myFeature;
}
```
And initialize in MainTree.cpp:
```cpp
Nebulite::MainTree::MainTree(/*...*/) : /*...*/
{
    // Initialize all expansions
    /*...*/
    myFeature = createExpansionOfType<MainTreeExpansion::MyFeature>();
}
```