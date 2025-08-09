# Nebulite Game Engine
[![Author](https://img.shields.io/badge/Author-lbastigk-blue)](https://github.com/lbastigk)

## Overview

**Nebulite** is a C++-based 2D game engine featuring a custom Domain-Specific Language (DSL) for game logic configuration. 
Built for arbitrary rulesets and flexible inter-object logic, Nebulite enables complex game mechanics to be defined declaratively in 
JSON / JSONC while maintaining performance through an underlying C++ engine.

## Architecture

Nebulite implements a **Domain-Specific Language (DSL)** for game logic configuration, built on a modular **Invoke system** that provides runtime command execution and expression logic.

### Key Architectural Components

- **Functioncall Framework**: Template-based command system with hierarchical inheritance
- **Expression Engine**: Runtime evaluation of mathematical and logical expressions (`$()` syntax)  
- **Plugin Architecture**: Modular expansions for different functionality domains
- **Data-Driven Design**: JSON-configured game behavior
- **Annotated Configuration**: Full JSONC comment support for documenting complex expressions and game mechanics

The system allows complex game mechanics to be defined declaratively in JSON while maintaining type safety and performance through the underlying C++ engine.

### Core Philosophy: Self-Other-Global Interactions

Inside the expression engine, objects interact through a three-tier context system:
- **SELF**: The object broadcasting the logic
- **OTHER**: Target objects being evaluated  
- **GLOBAL**: Shared game state (time, input, settings)

Examples:
- movement being triggered by `$(global.input.keyboard.w)`: sets velocity or request to move a distance
- animation being triggered by attributes: `$(self.isMoving)`: increment spritesheet offset
- boundary check being triggered by `$(other.isSolid)`: forces velocity of self to 0

Nebulites expression system also offers a flexible, cached, resource-retrieval system through `$(<linkToFile>:<key>)`, allowing for easy implementation of structured read-only data as JSON:

```
eval echo Hello, my name is $(./Resources/.../names.jsonc:characters.level1.npc_guard).
```
Only the retrieval of simple data types is supported. Full object/array-access yields `{Object}`/`{Array}`.
If you wish to retrieve complex data, use JSON-Specific functioncalls to copy from read-only data:

```
set-from-json names_in_level1 ./Resources/.../names.jsonc:characters.level1
```

Functioncalls are bound to different domains: specific objects or the global space. This allows for, but is not limited to, the following commands:
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

## The Invoke-Class

The Invoke class is the core game state modification class, which parses JSON-defined game logic.

### Invoke-Files

Invoke-Objects/Files contain:
- The topic of the ruleset (broadcaster `self` sends topic, listener `other` must be subscribed to it)
- A logical argument (must be true for it to be executed)
- A list of expressions
- Lists of functioncalls to be executed on different domains

If the topic string is empty, the invoke entry is local only (self-global interaction)

Example of an Invoke for a gravity ruleset:
```jsonc
/*
Add these individual invokes as object or a link as string ("./Resources/Invokes/.../gravity.json") to any RenderObject.json that should conform to the gravity ruleset:
-> append str/object    to its key "invokes"
Make sure the object listens to topic gravity as well
-> append str "gravity" to its key "invokeSubscriptions"
*/
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
  // Internal invoke, no broadcasting (self-global interaction). 
  // whenever possible, an empty broadcast should be chosen.
  "topic": "",          
  "logicalArg": "1",
  "exprs": [
    "self.physics.vX += $($(self.physics.aX) * $(global.time.dt))",
    "self.physics.vY += $($(self.physics.aY) * $(global.time.dt))"
  ],
  "functioncalls_global": [],
  "functioncalls_self": [],
  "functioncalls_other": []
},
// Velocity to Position:
// ...
// Reset acceleration to 0 for next frame:
// ...
```
*This creates realistic planetary motion with just JSON configuration!*


### Mathematical expressions

Nebulite offers all mathematical operations from [Tinyexpr](https://github.com/codeplea/tinyexpr) as well as integer casting with '$i(...)' and the following custom operators:

| Operator   | C++ Code                  |
|------------|---------------------------|
| `gt(a,b)`  | a > b                     |
| `lt(a,b)`  | a < b                     |
| `geq(a,b)` | a >= b                    |
| `leq(a,b)` | a <= b                    |
| `eq(a,b)`  | a == b                    |
| `neq(a,b)` | a != b                    |
| `and(a,b)` | a && b                    |
| `or(a,b)`  | a || b                    |
| `not(a)`   | !a                        |
| `sgn(a)`   | std::copysign(1.0, a)     |

You can quickly verify the correctness of an expression with the command line:
```bash
./bin/Nebulite 'set myVariable 2 ; eval echo $i(1 + $(global.myVariable))' # returns 3
```

### Modifiers on serialization

Nebulite allows for domain-specific functioncalls on serialization
```bash
# 1st Modifier: moves object from standard X position to X=500
# 2nd Modifier: Copies all values into a backup field
./bin/Nebulite 'spawn ./Resources/Renderobjects/standard.json|set posX 500|copy . backup'
```

## Quick Start

1. Install dependencies: `./install.sh`
2. Build the project: `./build.sh`
3. Run examples: `cd Application && ./bin/Nebulite task TaskFiles/Demos/basic.txt`

For development setup and contribution guidelines, see [CONTRIBUTING.md](CONTRIBUTING.md).

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

Optional dependencies:

| Project                                               | Purpose                                      |
|-------------------------------------------------------|----------------------------------------------|
| [recall](https://github.com/lbastigk/recall/)         | Custom CLI-Tool for quick documentation      |
| [PlantUML](https://plantuml.com/)                     | For compiling UML-Diagrams in ./doc/         |

## Contributing

We welcome contributions! See [CONTRIBUTING.md](CONTRIBUTING.md) for detailed development setup, testing procedures, and guidelines for adding new features to Nebulite's modular architecture.

## License

See [LICENSE.md](LICENSE.md) for licensing information.