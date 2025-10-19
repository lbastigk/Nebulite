<div align="center">

<!-- TOC --><a name="nebulite-game-engine"></a>
# Nebulite Game Engine

[![Author](https://img.shields.io/badge/Author-lbastigk-blue)](https://github.com/lbastigk)
[![License](https://img.shields.io/badge/License-MIT-green)](LICENSE.md)
[![C++ Standard](https://img.shields.io/badge/C%2B%2B-20-blue)]()
[![Status](https://img.shields.io/badge/State-Active%20Dev-orange)]()
<!-- Add a CI badge once available: [![Build](https://github.com/<org>/<repo>/actions/workflows/build.yml/badge.svg)]() -->

<strong>A data‑driven 2D engine + declarative DSL for rapid experimentation with object interactions and emergent mechanics.</strong>


<p align="center">
  <img src="doc/images/demo.gif" alt="Conventional gravity test" width="45%">
  <img src="doc/images/walking.gif" alt="Walking animations" width="45%">
</p>

<p align="center">
  <img src="doc/images/pong.gif" alt="Pong" width="45%">
  <img src="doc/images/xxl_test.gif" alt="Gravity + Elastic collision" width="45%">
</p>

</div>

<!-- TOC start (generated with https://github.com/derlin/bitdowntoc) -->

- [Overview](#overview)
- [Quick Start](#quick-start)
- [Learn More](#learn-more)
- [Core Concepts](#core-concepts)
   * [Expression System](#expression-system)
   * [Invoke System  ](#invoke-system)
   * [Runtime Modes](#runtime-modes)
- [Directory Structure](#directory-structure)
- [Platform Support & Dependencies](#platform-support-dependencies)
- [Testing](#testing)
- [Languages](#languages)
   * [Nebulite Script](#nebulite-script)
   * [Nebulite Logic](#nebulite-logic)
- [Contributing](#contributing)
- [License](#license)

<!-- TOC end -->

<!-- TOC --><a name="overview"></a>
## Overview

**Nebulite** is a C++20 2D game engine with a custom Domain-Specific Language (DSL) expressed in JSON / JSONC. It focuses on:
- Declarative composition of rulesets (gravity, AI, triggers) via small JSON fragments
- Flexible SELF / OTHER / GLOBAL interaction model
- Fast runtime expression evaluation with caching
- Seamless headless + interactive execution modes

The goal: quickly prototype and iterate on emergent object logic without rebuilding C++ code.

<!-- TOC --><a name="quick-start"></a>
## Quick Start

1. Clone & enter repo
```bash
  git clone https://github.com/lbastigk/Nebulite && cd Nebulite
```
2. Install dependencies, if necessary
```bash
  Scripts/install_dependencies.sh
```
2. Build binaries
```bash
  # Linux
  cmake --preset linux-debug && cmake --build --preset linux-debug
  cmake --preset linux-release && cmake --build --preset linux-release
  cmake --preset linux-coverage && cmake --build --preset linux-coverage

  # Windows
  cmake --preset windows-debug && cmake --build --preset windows-debug
  cmake --preset windows-release && cmake --build --preset windows-release

  # Mac (Work in Progress)
  cmake --preset macos-debug && cmake --build --preset macos-debug
  cmake --preset macos-release && cmake --build --preset macos-release
```
3. Download Resources
```bash
  Scripts/AssetCreation/create_resources_directory.sh
```
3. Run any script:
  ```bash
  ./bin/Nebulite task TaskFiles/Benchmarks/gravity.nebs 
  ```
4. Open console (press `tab`) and type `help` for interactive commands.

After making changes, you can run the VSCode task `[BUILD]` to recompile, or run `./build.sh`.

<!-- TOC --><a name="learn-more"></a>
## Learn More

- **Tutorials & Examples**: [Nebulite_Examples](https://github.com/lbastigk/Nebulite_Examples) - hands-on tutorials and sample projects
- **Glossary**: [./doc/Glossary.md](./doc/Glossary.md) - definitions of key terms and concepts
- **Command Reference**: [./doc/Commands.md](./doc/Commands.md) - comprehensive documentation of all available commands for both GlobalSpace and RenderObject domains (automatically generated with Scripts/MakeCommandDocumentation.py)

<!-- TOC --><a name="core-concepts"></a>
## Core Concepts

<!-- TOC --><a name="expression-system"></a>
### Expression System
Access and manipulate data using variables `{...}` and mathematical expressions `$(...)`:

**Variable Contexts:**
- `{self.*}` - the object broadcasting logic
- `{other.*}` - objects listening to the broadcast  
- `{global.*}` - shared engine state
- `{file.json:key.path}` - external read-only JSON files
- `{global.{self.id}}` - nested resolution (multiresolve), works only outside of mathematical expressions.

**Mathematical Expressions:**
- `$(1 + 2 * {self.mass})` - arithmetic with variables
- `$(gt({self.hp}, 0))` - logical operations (gt, lt, eq, and, or, not)
- `$i(3.14)` - cast to integer

**Example:** `"other.physics.aY += $({global.physics.G} * {self.physics.mass})"`

<!-- TOC --><a name="invoke-system"></a>
### Invoke System  
Define object interactions via JSON rulesets:
```jsonc
{
  "topic": "gravity",           // Broadcast channel
  "logicalArg": "1",           // Condition to execute
  "exprs": [                   // Modify values
    "other.physics.aY += $({global.physics.G} * {self.physics.mass})"
  ],
  "functioncalls_global": [...], // Commands to parse on domain GlobalSpace
  "functioncalls_self": [...],   // Commands to parse on domain RenderObject, self
  "functioncalls_other": [...]   // Commands to parse on domain RenderObject, other
}
```

<!-- TOC --><a name="runtime-modes"></a>
### Runtime Modes
- **Interactive**: Press `tab` for live console
- **Task Files**: `./bin/Nebulite task script.nebs` 
- **Headless**: `--headless` for automation/testing
- **CLI**: `./bin/Nebulite 'command ; chain'`

<!-- TOC --><a name="directory-structure"></a>
## Directory Structure

```bash
Nebulite/
├── bin/                      # Compiled binaries
├── doc/                      # Documentation: UML-Diagrams, example gifs, screenshots
├── external/                 # Third-party dependencies
├── include/                  # Header files:
│   ├── Constants/            # - constants like key names, thread settings etc.
│   ├── Core/                 # - core components
│   ├── DomainModule/         # - modules specific to certain domains
│   ├── Interaction/          # - parsing, expressions, manipulation
│   ├── Utility/              # - string-modification, JSON, caching etc.
│   └── Nebulite.h            # - namespace documentation
├── Languages/                # VSCode extension for .nebs and .nebl files
├── Resources/                # Game assets and data
├── Scripts/                  # Various scripts for asset creation, file validation, tests etc.
├── src/                      # Engine source code
├── TaskFiles/                # Example scripts
├── Tools/                    # CMake Toolchains, Test definitions
└── Unimplemented/            # Unimplemented DomainModules
```

<!-- TOC --><a name="platform-support-dependencies"></a>
## Platform Support & Dependencies

**Platforms**: Linux (native), Windows (cross-compiled via MinGW-w64)

**Requirements**: CMake 3.16+, C++20 compiler (GCC 11+/Clang 14+), Python 3.8+ (for testing and mock asset creation)

**Dependencies**
- SDL2, SDL_ttf, SDL_image - rendering and input
- RapidJSON - JSON parsing  
- Abseil - fast hash maps
- Tinyexpr - expression evaluation

<!-- TOC --><a name="testing"></a>
## Testing

Run validation and tests:
```bash
python Scripts/TestingSuite.py --stop --verbose
```
Or use VS Code task: `[BUILD + TEST]`

<!-- TOC --><a name="languages"></a>
## Languages

<!-- TOC --><a name="nebulite-script"></a>
### Nebulite Script

The `.nebs` *(Nebulite Script)* language is used for parsing commands in different domains.

Nebulite includes a VSCode language extension in `./Languages/nebulite-script-vscode/`. 
The extension offers proper syntax highlight for:
- available functions
- variables
- comments
- print-strings (everything after echo/error : meaning its printed to the command line)

Run `build-and-install.sh` inside its directory for installation.

<!-- TOC --><a name="nebulite-logic"></a>
### Nebulite Logic

The `.nebl` *(Nebulite Logic)* language is a work in progress and not yet useable.

<!-- TOC --><a name="contributing"></a>
## Contributing

Contributions welcome. Quick start:
1. Fork & create feature branch (`feature/<short-desc>`)
2. Run install + build + tests
3. Add/update invokes or DSL features with tests / demo TaskFile
4. Open PR referencing related roadmap item or issue

See full details in [CONTRIBUTING.md](CONTRIBUTING.md).

<!-- TOC --><a name="license"></a>
## License

Distributed under the MIT License. See [LICENSE.md](LICENSE.md).
