<div align="center">

# Nebulite Game Engine

[![Author](https://img.shields.io/badge/Author-lbastigk-blue)](https://github.com/lbastigk)
[![License](https://img.shields.io/badge/License-MIT-green)](LICENSE.md)
[![C++ Standard](https://img.shields.io/badge/C%2B%2B-20-blue)]()
[![Status](https://img.shields.io/badge/State-Active%20Dev-orange)]()
<!-- Add a CI badge once available: [![Build](https://github.com/<org>/<repo>/actions/workflows/build.yml/badge.svg)]() -->

<strong>A data‑driven 2D engine + declarative DSL for rapid experimentation with object interactions and emergent mechanics.</strong>


<p align="center">
  <img src="doc/demo.gif" alt="Conventional gravity test" width="45%">
  <img src="doc/walking.gif" alt="Walking animations" width="45%">
</p>

<p align="center">
  <img src="doc/pong.gif" alt="Pong" width="45%">
  <img src="doc/xxl_test.gif" alt="Gravity + Elastic collision" width="45%">
</p>

</div>

## Table of Contents
1. [Overview](#overview)
2. [Quick Start](#quick-start)
3. [Prerequisites](#prerequisites)
4. [Expression Cheat Sheet](#dsl-cheat-sheet)
5. [Runtime Modes](#runtime-modes)
6. [Architecture](#architecture)
7. [The Invoke Class](#the-invoke-class)
8. [Usage Examples](#usage-examples)
9. [Directory Structure](#directory-structure)
10. [Platform Support](#platform-support)
11. [Dependencies](#dependencies)
12. [Testing](#testing)
13. [Contributing](#contributing)
14. [Language Extension](#language-extension)
15. [License](#license)

## Overview

**Nebulite** is a C++20 2D game engine with a custom Domain-Specific Language (DSL) expressed in JSON / JSONC. It focuses on:
- Declarative composition of rulesets (gravity, AI, triggers) via small JSON fragments
- Flexible SELF / OTHER / GLOBAL interaction model
- Fast runtime expression evaluation with caching
- Seamless headless + interactive execution modes

The goal: quickly prototype and iterate on emergent object logic without rebuilding C++ code.

## Quick Start

1. Clone & enter repo
2. Install bundled dependencies (SDL variants, Abseil, etc.):
  ```bash
  ./install.sh
  ```
3. Build (debug + release):
  ```bash
  ./build.sh
  ```
4. Run any script:
  ```bash
  ./bin/Nebulite task TaskFiles/Benchmarks/gravity.txt 
  ```
5. Open console (press `^`) and type `help` for interactive commands.

Minimal one‑liner spawn:
```bash
./bin/Nebulite 'set-fps 60 ; spawn Resources/Renderobjects/standard.json ; wait 1 ; snapshot'
```

## Prerequisites

| Tool / Aspect | Requirement / Notes                                           |
|---------------|---------------------------------------------------------------|
| CMake         | >= 3.16 (see `CMakeLists.txt`)                                |
| C++ Compiler  | C++20 capable (Clang 14+, GCC 11+, MinGW-w64 for cross build) |
| Python        | 3.8+ (only for asset/mock creation scripts)                   |
| Build Tools   | make / ninja (default: make)                                  |
| OS            | Linux (native), Windows (produced via cross-compilation)      |

All core third‑party libs are vendored and installed via `install.sh` (no system SDL required). Static linking on Linux improves portability.

## Expression Cheat Sheet

| Feature                                     | Syntax                                    | Example                                    |
|---------------------------------------------|-------------------------------------------|-------------------------------------------|
| Expression evaluation                       | `$()`                                     | `$(1 + {self.physics.mass})`                            |
| Expression evaluation with Integer cast     | `$i(expr)`                                | `$i({global.time.frame} / 2)`                               |
| Expression evaluation with formatting       | `$<format><size>.<accuracy>(expr)`        | `$03i(1)`, `$13.1f(pi)`, `013.1f(e)`                         |
| Context values                              | `{self.*}`, `{other.*}`, `{global.*}`     | `{other.physics.mass}`                                     |
| External JSON value                         | `{<file>:<key.path>}`                     | `{./Resources/.../names.jsonc:characters.level1.npc_guard}`  |
| Chained commands                            | `;` separator                             | `spawn ... ; wait 1 ; snapshot`                              |
| Logical helpers                             | `gt, lt, geq, leq, eq, neq, and, or, not` | `$(gt({self.hp}, 0))`                                       |
| Sign function                               | `sgn(a)`                                  | `$(sgn({self.physics.vX}))`                             |

Core concept: Expressions can access document values of domains, functioncalls mutate state and can trigger other logic passes of domains.

### Minimal Invoke Snippet
```jsonc
{
  "topic": "gravity_Y",
  "logicalArg": "1",
  "exprs": [
   "other.physics.aY += $({global.physics.G} * {self.physics.mass} / (({self.posY}-{other.posY})^2 + 1))"
  ],
  "functioncalls_global": [],
  "functioncalls_self": [],
  "functioncalls_other": []
}
```
Attach via adding path/string inside an object's `invokes` list and ensuring `"gravity_Y"` appears in `invokeSubscriptions` for any other object that should adhere to the rule.

## Runtime Modes

| Mode | Trigger | Use Case |
|------|---------|----------|
| Interactive | Run binary and press `^` | Live console experimentation |
| Task File | `task TaskFiles/.../script.txt` | Batch scripted scenarios, benchmarks |
| Headless | `--headless ...` | CI, automated tests, snapshot generation |
| Immediate CLI | Quoted command string | Quick one‑offs, scripting in shell |

Headless still evaluates DSL and produces snapshots / logs without opening a window.

## Architecture

Nebulite implements a **Domain-Specific Language (DSL)** for game logic configuration, built on a modular **Invoke system** that provides runtime command execution and expression logic.

### Key Architectural Components

- **Functioncall Framework**: Template-based command system with hierarchical inheritance
- **Expression Engine**: Runtime evaluation of mathematical and logical expressions (`$()` for evaluation and `{}` for variables)  
- **Plugin Architecture**: Modular expansions for different functionality domains
- **Data-Driven Design**: JSON-configured game behavior
- **Annotated Configuration**: Full JSONC comment support for documenting complex expressions and game mechanics

The system allows complex game mechanics to be defined declaratively in JSON while maintaining type safety and performance through the underlying C++ engine.

### Core Philosophy: Self–Other–Global Interactions

Inside the expression engine, objects interact through a three-tier context system:
- **SELF**: The object broadcasting the logic
- **OTHER**: Target objects being evaluated  
- **GLOBAL**: Shared game state (time, input, settings)

Examples:
- movement being triggered by `{global.input.keyboard.w}`: sets velocity or request to move a distance
- animation being triggered by attributes: `{self.isMoving}`: increment spritesheet offset
- boundary check being triggered by `{other.isSolid}`: forces velocity of self to 0

Nebulite's expression system also offers a flexible, cached, resource-retrieval system through `{<linkToFile>:<key>}`, allowing for easy implementation of structured read-only data as JSON:

```
eval echo Hello, my name is {./Resources/.../names.jsonc:characters.level1.npc_guard}.
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

## The Invoke Class

The Invoke class is the core game state modification class, which parses JSON-defined game logic.

### Invoke Files

Invoke-Objects/Files contain:
- The topic of the ruleset (broadcaster `self` sends topic, listener `other` must be subscribed to it)
- A logical argument (must be true for it to be executed)
- A list of expressions
- Lists of functioncalls to be executed on different domains

If the topic string is empty, the invoke entry is local only (self-global interaction)

Example (expanded) gravity ruleset:
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
    "other.physics.aX += $({global.physics.G} * {self.physics.mass} * ( {self.posX} - {other.posX}  ) / ( ( ({self.posX} - {other.posX})^2 + ({self.posY} - {other.posY})^2 + 1)^(3/2) ))",
    "other.physics.aY += $({global.physics.G} * {self.physics.mass} * ( {self.posY} - {other.posY}  ) / ( ( ({self.posX} - {other.posX})^2 + ({self.posY} - {other.posY})^2 + 1)^(3/2) ))"
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
    "self.physics.vX += $({self.physics.aX} * {global.time.dt})",
    "self.physics.vY += $({self.physics.aY} * {global.time.dt})"
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


### Mathematical Expressions

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


## Serialization Modifiers

Apply transient mutations inline while spawning / serializing objects. General pattern:

```
spawn <path/to/object.json>|<modifier1> <args>|<modifier2> <args>|...
```

Example:
```bash
./bin/Nebulite 'spawn ./Resources/Renderobjects/standard.json|set posX 500|copy . backup'
```
Explanation:
- `set posX 500` shifts initial position
- `copy . backup` clones all fields into `backup.*` namespace

Use for prototyping variations without editing source JSON.

<!-- Quick Start moved earlier -->

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

```bash
Nebulite/
├── bin                       # Compiled binaries
├── doc                       # Documentation: UML-Diagrams, example gifs, screenshots
├── external                  # Third-party dependencies
├── include                   # Header files:
│   ├── Constants             # - constants like key names, thread settings etc.
│   ├── Core                  # - core components
│   ├── DomainModule          # - modules specific to certain domains
│   ├── Interaction           # - parsing, expressions, manipulation
│   ├── Nebulite.h            # - namespace documentation
│   └── Utility               # - string-modification, JSON, caching etc.
├── install.sh                # Installation of Repository
├── nebulite-logic-vscode     # VSCode extension for .nebl
├── nebulite-script-vscode    # VSCode extension for .nebs
├── Resources                 # Game assets and data
├── Scripts                   # Various scripts for asset creation, file validation, tests etc.
├── src                       # Engine source code
└── TaskFiles                 # Example scripts
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
| [PlantUML](https://plantuml.com/)                     | For compiling UML-Diagrams in ./doc/         |

## Testing

Two primary pathways:

1. Shell script (stops on first failure):
```bash
./Scripts/Tests.sh --stop
```
2. Combined build + test task (VS Code task):
```
[BUILD + TEST]
```

JSON validation runs via `./Scripts/validate_json.sh` prior to tests.

## Language Extension

Nebulite includes a language extension for `.nebs` scripting files under `./nebulite-script-vscode/`. 
The extension offers proper syntax highlight for:
- available functions
- variables
- comments
- print-strings (everything after echo/error : meaning its printed to the command line)

Run `build-and-install.sh` inside its directory for installation.

## Contributing

Contributions welcome. Quick path:
1. Fork & create feature branch (`feature/<short-desc>`)
2. Run install + build + tests
3. Add/update invokes or DSL features with tests / demo TaskFile
4. Open PR referencing related roadmap item or issue

See full details in [CONTRIBUTING.md](CONTRIBUTING.md).

## License

Distributed under the MIT License. See [LICENSE.md](LICENSE.md).

---