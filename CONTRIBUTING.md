# Contributing to Nebulite

We welcome contributions! Nebulite's modular architecture makes it easy to add features in separate files.

## Development Setup

### Prerequisites

1. Fork the repository
2. Install dependencies: `./install.sh`
3. Create feature branch: `git checkout -b feature/my-feature`

### Recommended Environment

Using VSCode is recommended for an optimal workflow. The project includes preconfigured tasks:
- `CTRL + SHIFT + T` for test options
- `CTRL + SHIFT + B` for build options

### Building

Build and test your changes:
```bash
./build.sh && cd Application && ./Tests.sh
```

## Testing

Go into the Application directory first: `cd Application/`

- Use `Tests.sh` for preconfigured tests
- Use `CrashDebug.sh` for debugging crashes with predefined taskfiles
- Use `MemLeakTest.sh` for memory leak testing using `valgrind` and `massif-visualizer`

You can add custom taskfiles to the test suite by extending the `tests` variable, or run them independently:
```bash
./bin/Nebulite task TaskFiles/.../your_test.txt
```

### Quick Expression Testing

You can quickly verify the correctness of an expression with the command line:
```bash
./bin/Nebulite 'set myVariable 2 ; eval echo $i(1 + {global.myVariable})' # returns 3
```

## Adding Features

Nebulite offers clean expansions of its functionality through its DomainModules. 
Maintainers can create their own module classes and add them to a specific domain.

| New commands operating on... | Action                        | Info                                                                    |
|------------------------------|-------------------------------|-------------------------------------------------------------------------|
| global level                 | Extend the `GlobalSpaceTree`  | See `include/Interaction/Execution/GlobalSpaceTree.h` and its modules `include/DomainModule/GlobalSpace/GTE_*.h`    |
| specific RenderObjects       | Extend the `RenderObjectTree` | See `include/Interaction/Execution/RenderObjectTree.h` and its modules `include/DomainModule/RenderObject/RTE_*.h`   |
| specific JSON-Documents      | Extend the `JSONTree`         | See `include/Interaction/Execution/JSONTree.h` and its modules `include/DomainModule/JSON/JTE_*.h`           |

Each Class has access to a different tree through `funcTree->...` and a different domain through `domain->...`: 
- `GlobalSpaceTree` can access the global space
- `RenderObjectTree` can access the attached RenderObject
- `JSONTree` can access the attached JSON

Each Tree extension can make use of an update routine, allowing us to declutter classes:
- input-reading
- state-update
- lifetime management
and more may be associated with specific tree-extensions instead of the base class.
We then just insert each extensions update inside the base tree and call `Tree->update()`
from the base class.

### Implementation Guidelines

- It is recommended to implement unfinished functions inside the cpp file via a return of `Nebulite::ERROR_TYPE::CRITICAL_FUNCTION_NOT_IMPLEMENTED`
- Use filenames `GDM_<ModuleName>.{hpp,cpp}` , `RDM_<ModuleName>.{hpp,cpp}` and `JDM_<ModuleName>.{hpp,cpp}` for module files
- Use the recommended class naming schemes and namespaces for modules: `Nebulite::DomainModule::GlobalSpace::MyModule`


### Function Collision Prevention

- The `GlobalSpaceTree` automatically inherits all functions from `JSONTree`, which act on the global document
- The `RenderObjectTree` automatically inherits all functions from `JSONTree`, which act on the objects document

It is **not allowed** to overwrite already existing functions:
- If the function `set` was already declared, it is not possible to declare a new `set` function in that same tree
- If the function `set` was already declared for the subtree, it is not possible to declare a new `set` function in the Tree that inherits the function

## Example: Adding a New GlobalSpaceTree Feature

### Step-by-Step Process

1. **Create expansion file:** `GDM_MyModule.{hpp,cpp}`
2. **Inherit from DomainModule base class:** Create class inheriting from `Nebulite::Interaction::Execution::DomainModule<DomainClass>`
3. **Implement command methods:** Functions with `ERROR_TYPE (int argc, char* argv[])` signature
4. **Constructor:** Initialize the base class, setup variables and bind functions/variables to the domain
5. **update** Add all necessary update-procedures within the tree
6. **Add to GlobalSpaceTree:** Include in `include/Interaction/Execution/GlobalSpaceTree.h`, add to the module vector in the constructor inside `src/Interaction/Execution/GlobalSpaceTree.cpp`
7. **Command line Variables** are added inside the constructor as well

### Complete Code Example

**Inside GDM_MyModule.hpp:**

```cpp
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
class MyModule : public Nebulite::Interaction::Execution::DomainModule<Nebulite::Core::GlobalSpace> {
public:
    /**
     * @brief Overridden update function.
     * 
     * For implementing internal update-procedures 
     * on each new frame
     */
    void update();

    //----------------------------------------
    // Available Functions

    ERROR_TYPE spawnCircle(int argc, char* argv[]);

    //------------------------------------------
    // Setup

    /**
     * @brief Initializes references to the domain and FuncTree, 
     * and binds functions to the FuncTree.
     */
    MyModule(Nebulite::Core::GlobalSpace* domain, Nebulite::Interaction::Execution::FuncTree<Nebulite::Constants::ERROR_TYPE>* funcTreePtr) 
    : DomainModule(domain, funcTreePtr) {
        bindFunction(&MyModule::spawnCircle, "spawn-circle", "Spawn a circle");
        /*Bind more functions of MyModule here*/
    }
};
}
}
```

**Inside GDM_MyModule.cpp:**
```cpp
#include "DomainModule/GlobalSpace/GDM_MyModule.hpp"
#include "Core/GlobalSpace.hpp"

void Nebulite::DomainModule::GlobalSpace::MyModule::update(){
    // If our expansion uses any internal values that need to be updated on each frame
    // We can update them here
}

Nebulite::ERROR_TYPE Nebulite::DomainModule::GlobalSpace::MyModule::spawnCircle(int argc, char* argv[]){
    /*
    Implementation here.
    You can access the global space and its members through: self->...
    As well as the funcTree through: funcTree->...
    */
}
```

**Then add the header file to include/GlobalSpaceTree.hpp:**
```cpp
/*..*/

//------------------------------------------
// Includes

/**/

// Nebulite DomainModules of GlobalSpaceTree
/**/
#include "DomainModule/GlobalSpace/GDM_MyModule.hpp"

/**/
```

**And initialize in GlobalSpaceTree.cpp:**
```cpp
#include "Interaction/Execution/GlobalSpaceTree.hpp"
#include "Core/GlobalSpace.hpp"       // Global Space for Nebulite

//------------------------------------------
// Linking ALL Functions to GlobalSpaceTree
Nebulite::Interaction::Execution::GlobalSpaceTree::GlobalSpaceTree(Nebulite::Core::GlobalSpace* domain, Nebulite::Interaction::Execution::JSONTree* jsonTree)
    : FuncTree<Nebulite::Constants::ERROR_TYPE>("Nebulite", Nebulite::Constants::ERROR_TYPE::NONE, Nebulite::Constants::ERROR_TYPE::CRITICAL_FUNCTIONCALL_INVALID, jsonTree), domain(domain) 
{
  // Initialize DomainModules
  /*...*/
  createDomainModuleOfType<Nebulite::DomainModule::GlobalSpace::MyFeature>();


  // Initialize Variable Bindings here, due to circular dependency issues
  bindVariable(&domain->cmdVars.headless, "headless", "Set headless mode (no renderer)");
  bindVariable(&domain->cmdVars.recover,  "recover",  "Enable recoverable error mode");
}
```

### Feature Management

If necessary, the entire feature can then be:
- **disabled** by commenting out `createExpansionOfType` in the Domains FuncTree Constructor`
- **removed** by undoing all changes inside Domains FuncTree

## Preview Editing (Work in Progress)

Preview Editing is currently under development. The current plan is to use the headless rendering mode of Nebulite in combination with either a taskfile or a python-script to allow rendering snapshots while editing JSON files.

## Submitting Changes

1. Ensure all tests pass: `./build.sh && cd Application && ./Tests.sh`
2. Create a pull request with a clear description of your changes
3. Include test cases for new functionality
4. Update documentation as needed

## Code Style

- Follow the existing code style and conventions
- Use meaningful variable and function names
- Comment complex logic and algorithms
- Create helper classes if necessary
- Keep functions focused and modular

## Getting Help

- Check existing issues and discussions
- Review the architecture documentation in the main README
- Look at existing expansion implementations for patterns
- Feel free to open an issue for questions or clarifications
