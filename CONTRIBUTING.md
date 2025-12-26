# Contributing to Nebulite

We welcome contributions! Nebulite's modular architecture makes it easy to add features 
in separate files and barely touching existing ones.

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
2. Install (see README.md)
3. Create a feature branch: `git checkout -b feature/my-feature`

<!-- TOC --><a name="testing"></a>
## Testing

You can add custom taskfiles to the test suite by extending the main test file `Tools/tests.jsonc`.
However, it is recommended to write the taskfiles inside the test json file itself for better portability:
```json
[
    {
      "command": [
        "set-fps 60",
        "echo 1234",
        "exit"
      ],
      "expected": { "cout": ["1234"], "cerr": [] }
    }
]
```

After adding your test cases in `Tools/Tests/*.json`, add the file to `Tools/tests.jsonc` like so:
```jsonc
{
    // ... existing test files
    "Tools/Tests/MyNewTestFile.json"
}
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

| Domain: Commands operating on...  | Action                                                            | Info                                                                                          |
|-----------------------------------|-------------------------------------------------------------------|-----------------------------------------------------------------------------------------------|
| Global level                      | Extend `Initializer.hpp` by creating GlobalSpace DomainModules    | See `include/Core/GlobalSpace.hpp` and its modules  `include/DomainModule/GlobalSpace/*.hpp`  |
| Renderer                          | Extend `Initializer.hpp` by creating Renderer DomainModules       | See `include/Core/Renderer.hpp` and its modules     `include/DomainModule/Renderer/*.hpp`     |
| Environment                       | Extend `Initializer.hpp` by creating Environment DomainModules    | See `include/Core/Environment.hpp` and its modules  `include/DomainModule/Environment/*.hpp`  |
| Specific RenderObjects            | Extend `Initializer.hpp` by creating RenderObject DomainModules   | See `include/Core/RenderObject.hpp` and its modules `include/DomainModule/RenderObject/*.hpp` |
| Specific JSON-Documents           | Extend `Initializer.hpp` by creating JSON DomainModules           | See `include/Utility/JSON.hpp` and its modules      `include/DomainModule/JSON/*.hpp`         |
| Specific Textures                 | Extend `Initializer.hpp` by creating Texture DomainModules        | See `include/Core/Texture.hpp` and its modules      `include/DomainModule/Texture/*.hpp`      |


Each DomainModule has access to a different domain workspace through `domain->...`,
as well as an update routine, allowing us to declutter classes by binding routines to specific modules:
- input-reading
- state-update
- lifetime management

and more. We then just insert each module into the class and its update function is automatically called.

<!-- TOC --><a name="function-collision-prevention"></a>
### Function Collision Prevention

Domains follow an inheritance tree structure for their functions:
- `GlobalSpace` automatically inherits all functions from `JSON` and `Renderer`
- `Renderer` inherits from `Environment`, but not from `JSON`, as it shares the same document as `GlobalSpace`
- `RenderObject` automatically inherits all functions from `JSON` and `Texture`
- `Texture` does not inherit from any other domain, but shares the same document as `RenderObject`

The difference between inheritance and shared documents is that functions parsed in the Domain 
are only redirected to another Domain if they inherit from it. Shared documents only allow access to the same variables
or manual function calls.

It is **not allowed** to overwrite already existing functions:
- If the function `set` was already declared, it is not possible to declare a new `set` function in that same tree
- If the function `set` was already declared for the category, it is not possible to declare a new `set` function in the Tree that inherits the function

Furthermore, it is not allowed to overwrite existing `categories` with functions and vice versa. 

If a function is bound inside a non-existing `category`, the program will exit:
```cpp
bindCategory("MyCategory","<Description of MyCategory>");
bindFunction(/**/,"MyCategory foo","<Description of foo>"); //<-- This would fail without bindCategory being called first
```

<!-- TOC --><a name="example-adding-a-new-globalspace-feature"></a>
### Example: Adding a New GlobalSpace Feature

<!-- TOC --><a name="step-by-step-process"></a>
#### Step-by-Step Process

1. **Create expansion file**
2. **Inherit from DomainModule base class:** Create class using the `NEBULITE_DOMAINMODULE(Domain,MyDomainModule)` macro
3. **Implement command methods:** Functions with `Nebulite::Constants::Error (std::span<std::string const> const& args)` signature
4. **Implement the update method:** Override `Nebulite::Constants::Error update()` for per-frame updates
5. **DomainModule init** inside `include/DomainModule/Initializer.hpp`, initialize the DomainModule

<!-- TOC --><a name="complete-code-example"></a>
#### Complete Code Example

**Inside MyModule.hpp:**

```cpp
/**
 * @file MyModule.hpp
 * @brief Contains the DomainModule of the GlobalSpace for MyFeature functions.
 */

#ifndef NEBULITE_RODM_MYMODULE_HPP
#define NEBULITE_RODM_MYMODULE_HPP

//------------------------------------------
// Includes

// Nebulite
#include "Constants/ErrorTypes.hpp"
#include "Interaction/Execution/DomainModule.hpp"

//------------------------------------------
// Forward declarations
namespace Nebulite::Core {
class RenderObject;
}

//------------------------------------------
namespace Nebulite::DomainModule::RenderObject {
/**
 * @class Nebulite::DomainModule::RenderObject::MyModule
 * @brief Example module for RenderObject domain using the Debug.hpp conventions.
 */
NEBULITE_DOMAINMODULE(Nebulite::Core::RenderObject, MyModule) {
public:
    Constants::Error update() override;
    void reinit() override {}

    //------------------------------------------
    // Available Functions

    Constants::Error exampleCommand(std::span<std::string const> const& args);
    static std::string_view constexpr exampleCommand_name = "example do-something";
    static std::string_view constexpr exampleCommand_desc = "Performs an example action on the current RenderObject.\n"
        "\n"
        "Usage: example do-something [args]\n";

    Constants::Error anotherCmd(std::span<std::string const> const& args);
    static std::string_view constexpr anotherCmd_name = "example another-cmd";
    static std::string_view constexpr anotherCmd_desc = "Another example command demonstrating binding and descriptions.\n"
        "\n";
        "Usage: example another-cmd [args]\n";

    //------------------------------------------
    // Category names
    static std::string_view constexpr example_name = "example";
    static std::string_view constexpr example_desc = "Example functions for RenderObject domain";

    //------------------------------------------
    // Setup

    /**
     * @brief Initializes the module, binding functions and variables.
     */
    NEBULITE_DOMAINMODULE_CONSTRUCTOR(Nebulite::Core::RenderObject, MyModule) {
        // Ensure category exists before binding functions that include it
        (void)bindCategory(example_name, example_desc);

        // Bind functions using the name/description constants above
        bindFunction(&MyModule::exampleCommand, exampleCommand_name, exampleCommand_desc);
        bindFunction(&MyModule::anotherCmd, anotherCmd_name, anotherCmd_desc);
    }
};
} // namespace Nebulite::DomainModule::RenderObject

#endif // NEBULITE_RODM_MYMODULE_HPP
```

**Then add the header file to `include/DomainModule/Initializer.hpp` and initialize in the cpp file.**

<!-- TOC --><a name="implementation-guidelines"></a>
### Implementation Guidelines

- It is recommended to implement unfinished functions inside the cpp file with a return of 
`Nebulite::Constants::ErrorTable::FUNCTIONAL::CRITICAL_FUNCTION_NOT_IMPLEMENTED()`
- Use the recommended class naming schemes and namespaces for modules: 
`Nebulite::DomainModule::<Domain>::<ModuleName>`

<!-- TOC --><a name="preview-editing-work-in-progress"></a>
## Preview Editing (Work in Progress)

Preview Editing is currently under development. The current plan is to use the headless rendering mode of Nebulite 
in combination with either a taskfile or a python-script to allow rendering snapshots while editing JSON files.

<!-- TOC --><a name="submitting-changes"></a>
## Submitting Changes

1. Make sure you cover a wide range of test cases for your new feature. 
Verify the coverage with `make build-and-coverage-report`.
2. Ensure all tests pass: `make test`
3. Update documentation: `make docs` as well as manual updates if necessary
4. Create a pull request with a clear description of your changes

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
