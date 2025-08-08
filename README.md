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
- Environments, Renderobjects, etc. may be loaded with tasks. See `./Application/TaskFiles/` for examples.

This non-hierarchical architecture enables complex interactive systems and modular subsystems. 

## Architecture

Nebulite implements a **Domain-Specific Language (DSL)** for game logic configuration, built on a modular **FuncTree and Invoke system** that provides runtime command execution and reflection capabilities.

### Key Architectural Components

- **FuncTree Framework**: Template-based command system with hierarchical inheritance
- **Expression Engine**: Runtime evaluation of mathematical and logical expressions (`$()` syntax)  
- **Plugin Architecture**: Modular expansions for different functionality domains
- **Data-Driven Design**: JSON-configured game behavior

The system allows complex game mechanics to be defined declaratively in JSON while maintaining type safety and performance through the underlying C++ engine.

### Core Philosophy: Self-Other-Global Interactions

Objects interact through a three-tier context system:
- **SELF**: The object broadcasting the logic
- **OTHER**: Target objects being evaluated  
- **GLOBAL**: Shared game state (time, input, settings)

Examples:
- movement being triggered by `$(global.input.keyboard.w)`: sets velocity or request to move a distance
- animation being triggered by attributes: `$(self.isMoving)`: increment spritesheet offset
- boundary check being triggered by `$(other.isSolid)`: forces velocity of self to 0

Nebulite also offers a flexible resource-retrieval system through `$(<linkToFile>:<key>)`, allowing for easy implementation of structured read-only data as JSON. Currently, only the retrieval of simple data types is supported. Full object/array-access yields `{Object}`/`{Array}`.

Functioncalls are bound to specific objects or the global space, allowing for, but not limited to, the following commands:
- **Global:** Loading new levels, moving camera, ...
- **Object-Specific:** aligning text, deletion, ...
- **Document-Specific:** moving/copying/deleting data, read-only data retrieval, ...

### Language Features

The Nebulite DSL provides:
- **Runtime Reflection**: Dynamic function binding and introspection
- **Expression Evaluation**: Mathematical expressions with context variables
- **Command Composition**: Chainable operations with semicolon syntax  
- **Type Safety**: Compile-time verification through C++ templates
- **Function Collision Prevention**: Automatic detection of naming conflicts

### Invoke-Files

Invoke json-files are used to describe how objects interact with each other. They contain:
- The topic of the ruleset (broadcaster `self` sends topic, listener `other` must be subscribed to it)
- A logical argument (must be true for it to be executed)
- A list of expressions
- Lists of functioncalls to be executed on different domains

Example of an Invoke for a gravity ruleset:
```jsonc
// Add this or a link as string ("./Resources/Invokes/.../gravity.json") to any RenderObject.json that should conform to the gravity ruleset:
// -> append str/object    to its key "invokes"
// Make sure the object listens to topic gravity as well
// -> append str "gravity" to its key "invokeSubscriptions"
{
  "topic": "gravity",   // Broadcasting this ruleset to any other object subscribed to "gravity"
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
},
// Acceleration to velocity:
{
  "topic": "",          // Internal invoke, no broadcasting
  "logicalArg": "1",
  "exprs": [
    "self.physics.vX += $($(self.physics.aX) * $(global.time.dt))",
    "self.physics.vY += $($(self.physics.aY) * $(global.time.dt))"
  ],
  "functioncalls_global": [],
  "functioncalls_self": [],
  "functioncalls_other": []
}
// Velocity to Position:
// ...
// Reset acceleration to 0 for next frame:
// ...
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
Start engine and enter console mode with `^`. Enter `help` to see available commands in the command line.

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
- Use `CrashDebug.sh` for debugging crashes with predefined taskfiles
- Use `MemLeakTest.sh` for memory leak testing using `valgrind` and `massif-visualizer`

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
Maintainers can create their own Tree-expansion classes and add them to a specifc FuncTree.

| New commands operating on... | Action                        | Info                                                                    |
|------------------------------|-------------------------------|-------------------------------------------------------------------------|
| global level                 | Extend the `GlobalSpaceTree`  | See `include/GlobalSpaceTree.h` and its expansions `include/GTE_*.h`    |
| specific RenderObjects       | Extend the `RenderObjectTree` | See `include/RenderObjectTree.h` and its expansions `include/RTE_*.h`   |
| specific JSON-Documents      | Extend the `JSONTree`         | See `include/JSONTree.h` and its expansions `include/JTE_*.h`           |

Each Class has access to a different tree through `funcTree->...` and a different domain through `self->...`: 
- `GlobalSpaceTree` can access the global space
- `RenderObjectTree` can access the attached RenderObject
- `JSONTree` can access the attached JSON

It is recommended to implement unfinished functions inside the cpp file via a return of `Nebulite::ERROR_TYPE::CRITICAL_FUNCTION_NOT_IMPLEMENTED`

Note that:
- the `GlobalSpaceTree` automatically inherits all functions from `JSONTree`, which act on the global document.
- the `RenderObjectTree` automatically inherits all functions from `JSONTree`, which act on the objects document.

It is **not allowed** to overwrite already existing functions:
- If the function `set` was already declared, it is not possible to declare a new `set` function in that same tree
- If the function `set` was already declared for the subtree, it is not possible to declare a new `set` function in the Tree that inherits the function

---------------------------------------------------

**Example procedure for a new GlobalSpaceTree feature:**

1. **Create expansion file:** `GTE_MyFeature.{h,cpp}`
2. **Inherit from wrapper:** Create class inheriting from `Nebulite::FuncTreeExpansion::Wrapper<DomainClass, MyFeatureClass>`
3. **Implement command methods:** Functions with `ERROR_TYPE (int argc, char* argv[])` signature
4. **setupBindings():** Register your commands with the function tree
5. **Add to GlobalSpaceTree:** Include in `include/GlobalSpaceTree.h` and initialize in constructor
6. **Command line Variables** are more difficult to implement, as they require the full domain definition. Bind them in `GlobalSpaceTree.cpp`

---------------------------------------------------

**Complete code example:**

Inside MyFeature.h:

```cpp
namespace Nebulite{
class GlobalSpace; // Forward declaration of Domain class GlobalSpace 
namespace GlobalSpaceTreeExpansion {
class MyFeature : public Nebulite::FuncTreeExpansion::Wrapper<Nebulite::GlobalSpace, Debug> {
public:
    using Wrapper<Nebulite::GlobalSpace, MyFeature>::Wrapper; // Templated constructor from Wrapper, calls setupBindings

    //----------------------------------------
    // Available Functions
    ERROR_TYPE spawnCircle(int argc, char* argv[]);

    //----------------------------------------
    // Binding Functions
    void setupBindings()  {
        bindFunction(&MyFeature::spawnCircle, "spawn-circle", "Spawn a circle");
        /*Bind more functions of MyFeature here*/
    }
};
}
}
```
Inside MyFeature.cpp:
```cpp
#include "MyFeature.h"
#include "GlobalSpace.h"       // Global Space for Nebulite

Nebulite::ERROR_TYPE Nebulite::GlobalSpaceTreeExpansion::MyFeature::spawnCircle(int argc, char* argv[]){
    /*
    Implementation here.
    You can access the global space and its members through: self->...
    As well as the funcTree through: funcTree->...
    */
}
```
Then add to include/GlobalSpaceTree.h:
```cpp
#include "MyFeature.h"
class GlobalSpaceTree : public FuncTree<Nebulite::ERROR_TYPE>{
    /*...*/
private:
    /*...*/
    std::unique_ptr<GlobalSpaceTreeExpansion::MyFeature> myFeature;
}
```
And initialize in GlobalSpaceTree.cpp:
```cpp
Nebulite::GlobalSpaceTree::GlobalSpaceTree(/*...*/) : /*...*/
{
    // Initialize all expansions
    /*...*/
    myFeature = createExpansionOfType<GlobalSpaceTreeExpansion::MyFeature>();

    // Initialize Variable Bindings here, due to circular dependency issues
    /*...*/
    bindVariable(&self->myVariable, "myVariable", "This is a variable inside globalSpace");
}
```

If necessary, the entire feature can then be:
- **disabled** by commenting out `createExpansionOfType` in the Constructor
- **removed** by undoing the changes inside `GlobalSpaceTree.{h,cpp}`