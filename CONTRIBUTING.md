# Contributing to Nebulite

We welcome contributions! Nebulite's modular architecture makes it easy to add features in separate files and barely touching existing ones.

<!-- TOC start (generated with https://github.com/derlin/bitdowntoc) -->

- [Development Setup](#development-setup)
   * [Prerequisites](#prerequisites)
   * [Recommended Environment](#recommended-environment)
   * [Building](#building)
- [Testing](#testing)
   * [Quick Expression Testing](#quick-expression-testing)
- [Adding Features](#adding-features)
   * [Function Collision Prevention](#function-collision-prevention)
   * [Example: Adding a New GlobalSpace Feature](#example-adding-a-new-globalspace-feature)
      + [Step-by-Step Process](#step-by-step-process)
      + [Complete Code Example](#complete-code-example)
   * [Feature Management](#feature-management)
   * [Implementation Guidelines](#implementation-guidelines)
- [Preview Editing (Work in Progress)](#preview-editing-work-in-progress)
- [Submitting Changes](#submitting-changes)
- [Code Style](#code-style)
- [Getting Help](#getting-help)

<!-- TOC end -->

<!-- TOC --><a name="development-setup"></a>
## Development Setup

<!-- TOC --><a name="prerequisites"></a>
### Prerequisites

1. Fork the repository
2. Install dependencies: `./install.sh`
3. Create feature branch: `git checkout -b feature/my-feature`

<!-- TOC --><a name="recommended-environment"></a>
### Recommended Environment

Using VSCode is recommended for an optimal workflow. The project includes preconfigured tasks:
- `CTRL + SHIFT + T` for test options
- `CTRL + SHIFT + B` for build options

<!-- TOC --><a name="building"></a>
### Building

Build and test your changes:
```bash
./build.sh && ./Scripts/TestingSuite.py
```

<!-- TOC --><a name="testing"></a>
## Testing

- Use `python ./Scripts/TestingSuite.py` for preconfigured tests
- Use the VSCode tasks for Memory leak testing, profiling and more
- Use the VSCode debugger and its existing tasks

You can add custom taskfiles to the test suite by extending the `tests.json` file in the projects root directory.
```bash
./bin/Nebulite task TaskFiles/.../your_test.txt
```

<!-- TOC --><a name="quick-expression-testing"></a>
### Quick Expression Testing

You can quickly verify the correctness of an expression with the command line:
```bash
./bin/Nebulite 'set myVariable 2 ; eval echo $i(1 + {global.myVariable})' # returns 3
```

<!-- TOC --><a name="adding-features"></a>
## Adding Features

Nebulite offers clean expansions of its functionality through its DomainModules. 
Maintainers can create their own module classes and add them to a specific domain.

| Domain: Commands operating on... | Action                                                  | Info                                                                                                         |
|----------------------------------|---------------------------------------------------------|--------------------------------------------------------------------------------------------------------------|
| Global level                     | Extend `GSDM.hpp` by creating GlobalSpace DomainModules  | See `include/Interaction/Execution/GlobalSpace.h` and its modules `include/DomainModule/GlobalSpace/GSDM_*.h` |
| Specific RenderObjects           | Extend `RODM.hpp` by creating RenderObject DomainModules | See `include/Core/RenderObject.h` and its modules `include/DomainModule/RenderObject/RODM_*.h`                |
| Specific JSON-Documents          | Extend `JSDM.hpp` by creating JSON DomainModules         | See `include/Utility/JSON.h` and its modules `include/DomainModule/JSON/JSDM_*.h`                             |
| Specific Textures                | Extend `TXDM.hpp` by creating Texture DomainModules      | See `include/Core/Texture.h` and its modules `include/DomainModule/Texture/TXDM_*.h`                          |

Each DomainModule has access to a different domain through `domain->...` and a different set of functions through `domain->parseStr(command)` : 
- `GlobalSpace`  modules can access the global space
- `RenderObject` modules can access the attached RenderObject
- `JSON`         modules can access the attached JSON
- `Texture`      modules can access the attached Texture

Each DomainModule can make use of an update routine, allowing us to declutter classes by binding routines to specific modules:
- input-reading
- state-update
- lifetime management

and more. We then just insert each module into the class and its update function is automatically called.

<!-- TOC --><a name="function-collision-prevention"></a>
### Function Collision Prevention

- `GlobalSpace` automatically inherits all functions from `JSON`, which act on the global document
- `RenderObject` automatically inherits all functions from `JSON`, which act on the objects document

It is **not allowed** to overwrite already existing functions:
- If the function `set` was already declared, it is not possible to declare a new `set` function in that same tree
- If the function `set` was already declared for the subtree, it is not possible to declare a new `set` function in the Tree that inherits the function

Furthermore, it is not allowed to overwrite existing `subtrees` with functions and vice versa. 

If a function is bound inside a non-existing `subtree`, the program will exit:
```cpp
bindSubtree("MyModule","<Description>");
bindFunction(/**/,"MyModule foo","<Description>"); //<-- This will fail without bindSubTree()
```

<!-- TOC --><a name="example-adding-a-new-globalspace-feature"></a>
### Example: Adding a New GlobalSpace Feature

<!-- TOC --><a name="step-by-step-process"></a>
#### Step-by-Step Process

1. **Create expansion file:** `GSDM_MyModule.{hpp,cpp}`
2. **Inherit from DomainModule base class:** Create class inheriting from `Nebulite::Interaction::Execution::DomainModule<DomainClass>`
3. **Implement command methods:** Functions with `Nebulite::Constants::Error (int argc, char* argv[])` signature
4. **DomainModule init** inside `include/DomainModule/{GSDM,JSDM,RODM}.hpp`, initialize the DomainModule

<!-- TOC --><a name="complete-code-example"></a>
#### Complete Code Example

**Inside GSDM_MyModule.hpp:**

```cpp

/**
 * @file GSDM_MyModule.hpp
 * 
 * This file contains the DomainModule of the GlobalSpace for MyFeature functions.
 */

#pragma once

//------------------------------------------
// Includes

// Nebulite
#include "Constants/ErrorTypes.hpp"
#include "Interaction/Execution/DomainModule.hpp"

//------------------------------------------
// Forward declarations
namespace Nebulite{
    namespace Core{
        class GlobalSpace; // Forward declaration of domain class GlobalSpace
    }
}

//------------------------------------------
namespace Nebulite {
namespace DomainModule {
namespace GlobalSpace {
NEBULITE_DOMAINMODULE(Nebulite::Core::GlobalSpace, MyModule) {
public:
    /**
     * @brief Overridden update function.
     * 
     * For implementing internal update-procedures 
     * on each new frame
     */
    Nebulite::Constants::Error update();

    //----------------------------------------
    // Available Functions

    Nebulite::Constants::Error spawnCircle(int argc, char* argv[]);

    //------------------------------------------
    // Setup

    /**
     * @brief Initializes the module, binding functions and variables. 
     */
    NEBULITE_DOMAINMODULE_CONSTRUCTOR(Nebulite::Core::GlobalSpace, MyModule){
        //------------------------------------------
        // Binding functions to the Domain
        bindFunction(&MyModule::spawnCircle, "spawn-circle", "Spawn a circle");
        /*Bind more functions of MyModule here*/
        /*You can also implement sublevels to the command using the subtree feature:*/
        bindSubtree("spawn","Spawn functions");
        bindSubtree("spawn geometry", "Geometric forms");
        bindFunction(&MyModule::spawnCircle, "spawn geometry circle", "Spawn a circle");
    }
private:
    /*Add necessary variables here*/
};
}
}
```

**Inside GSDM_MyModule.cpp:**
```cpp
#include "DomainModule/GlobalSpace/GSDM_MyModule.hpp"
#include "Core/GlobalSpace.hpp"

void Nebulite::DomainModule::GlobalSpace::MyModule::update(){
    // If our expansion uses any internal values 
    // that need to be updated on each frame
    // We can update them here
}

Nebulite::Nebulite::Constants::Error Nebulite::DomainModule::GlobalSpace::MyModule::spawnCircle(int argc, char* argv[]){
    /*
    Implementation here.
    You can access domain and its members through: 
    `domain->...`
    */
}
```

**Then add the header file to include/DomainModule/GSDM.hpp and initialize:**

```cpp
/*..*/

//------------------------------------------
// Module includes 
#if GSDM_ENABLED
    /*...*/
    #include "DomainModule/GlobalSpace/GSDM_MyModule.hpp"
    /*...*/
#endif

//------------------------------------------
namespace Nebulite{
namespace DomainModule{
/**
 * @brief Inserts all DomainModules into the GlobalSpace domain.
 */
void GSDM_init(Nebulite::Core::GlobalSpace* target){
    #if GSDM_ENABLED
        // Initialize DomainModules
        using namespace Nebulite::DomainModule::GlobalSpace;
        /*...*/
        target->initModule<MyModule>("<Feature Description>");
        /*...*/
    #endif
}
}
}
```

<!-- TOC --><a name="feature-management"></a>
### Feature Management

If necessary, the entire feature can then be:
- **disabled** by commenting out `initModule` inside `{GSDM,JSDM,RODM,TXDM}.hpp`
- **removed** by undoing all changes inside `{GSDM,JSDM,RODM,TXDM}.hpp` and potentially deleting its files.

<!-- TOC --><a name="implementation-guidelines"></a>
### Implementation Guidelines

- It is recommended to implement unfinished functions inside the cpp file with a return of `Nebulite::Constants::ErrorTable::FUNCTIONAL::CRITICAL_FUNCTION_NOT_IMPLEMENTED()`
- Use filenames `GSDM_<ModuleName>.{hpp,cpp}` , `RODM_<ModuleName>.{hpp,cpp}` and `JSDM_<ModuleName>.{hpp,cpp}` for module files
- Use the recommended class naming schemes and namespaces for modules: `Nebulite::DomainModule::GlobalSpace::MyModule`

<!-- TOC --><a name="preview-editing-work-in-progress"></a>
## Preview Editing (Work in Progress)

Preview Editing is currently under development. The current plan is to use the headless rendering mode of Nebulite in combination with either a taskfile or a python-script to allow rendering snapshots while editing JSON files.

<!-- TOC --><a name="submitting-changes"></a>
## Submitting Changes

1. Ensure all tests pass: `./build.sh && python Scripts/TestingSuite.py`
2. Create a pull request with a clear description of your changes
3. Include test cases for new functionality
4. Update documentation as needed

<!-- TOC --><a name="code-style"></a>
## Code Style

- Follow the existing code style and conventions
- Use meaningful variable and function names
- Comment complex logic and algorithms
- Create helper classes if necessary
- Keep functions focused and modular

<!-- TOC --><a name="getting-help"></a>
## Getting Help

- Check existing issues and discussions
- Review the architecture documentation in the main README
- Look at existing expansion implementations for patterns
- Feel free to open an issue for questions or clarifications
