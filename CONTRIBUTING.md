# Contributing to Nebulite

We welcome contributions! Nebulite's modular architecture makes it easy to add features in separate files and barely touching existing ones.

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
./build.sh && ./Scripts/Tests.sh
```

## Testing

- Use `./Scripts/Tests.sh` for preconfigured tests
- Use `./Scripts/CrashDebug.sh` for debugging crashes with predefined taskfiles
- Use `./Scripts/MemLeakTest.sh` for memory leak testing using `valgrind` and `massif-visualizer`
- Use the VSCode debugger and its existing tasks

You can add custom taskfiles to the test suite by extending the `tests` variable inside `./Scripts/Tests.sh`, or run them independently:
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

| Domain: Commands operating on... | Action                                                  | Info                                                                    |
|------------------------------|-------------------------------|-------------------------------------------------------------------------|
| Global level                     | Extend `GDM.hpp` by creating GlobalSpace DomainModules  | See `include/Interaction/Execution/GlobalSpace.h` and its modules `include/DomainModule/GlobalSpace/GTE_*.h` |
| Specific RenderObjects           | Extend `RDM.hpp` by creating RenderObject DomainModules | See `include/Core/RenderObject.h` and its modules `include/DomainModule/RenderObject/RTE_*.h` |
| Specific JSON-Documents          | Extend `JDM.hpp` by creating JSON DomainModules         | See `include/Utility/JSON.h` and its modules `include/DomainModule/JSON/JTE_*.h` |

Each DomainModule has access to a different set of functions through `funcTree->...` and a different domain through `domain->...`: 
- `GlobalSpace` modules can access the global space
- `RenderObject` modules can access the attached RenderObject
- `JSON` modules can access the attached JSON

Each DomainModule can make use of an update routine, allowing us to declutter classes:
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

- `GlobalSpace` automatically inherits all functions from `JSON`, which act on the global document
- `RenderObject` automatically inherits all functions from `JSON`, which act on the objects document

It is **not allowed** to overwrite already existing functions:
- If the function `set` was already declared, it is not possible to declare a new `set` function in that same tree
- If the function `set` was already declared for the subtree, it is not possible to declare a new `set` function in the Tree that inherits the function

## Example: Adding a New GlobalSpace Feature

### Step-by-Step Process

1. **Create expansion file:** `GDM_MyModule.{hpp,cpp}`
2. **Inherit from DomainModule base class:** Create class inheriting from `Nebulite::Interaction::Execution::DomainModule<DomainClass>`
3. **Implement command methods:** Functions with `ERROR_TYPE (int argc, char* argv[])` signature
4. **DomainModule init** inside `include/DomainModule/{GDM,JDM,RDM}.hpp`, initialize the DomainModule

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
    MyModule(std::string moduleName, Nebulite::Core::GlobalSpace* domain, Nebulite::Interaction::Execution::FuncTree<Nebulite::Constants::ERROR_TYPE>* funcTreePtr) 
    : DomainModule(moduleName, domain, funcTreePtr) {
        //------------------------------------------
        // Binding functions to the FuncTree
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

As well as the method implementations in `src/DomainModule/GlobalSpace/MyFeature.hpp`

**Inside GDM_MyModule.cpp:**
```cpp
#include "DomainModule/GlobalSpace/GDM_MyModule.hpp"
#include "Core/GlobalSpace.hpp"

void Nebulite::DomainModule::GlobalSpace::MyModule::update(){
    // If our expansion uses any internal values 
    // that need to be updated on each frame
    // We can update them here
}

Nebulite::ERROR_TYPE Nebulite::DomainModule::GlobalSpace::MyModule::spawnCircle(int argc, char* argv[]){
    /*
    Implementation here.
    You can access domain and its members through: 
    `domain->...`
    As well as the funcTree through: 
    `funcTree->...`
    */
}
```

**Then add the header file to include/DomainModule/GDM.hpp and initialize:**

```cpp
/*..*/

//------------------------------------------
// Module includes 
#if GDM_ENABLED
    /*...*/
    #include "DomainModule/GlobalSpace/GDM_MyFeature.hpp"
    /*...*/
#endif

//------------------------------------------
namespace Nebulite{
namespace DomainModule{
/**
 * @brief Inserts all DomainModules into the GlobalSpace domain.
 */
void GDM_init(Nebulite::Core::GlobalSpace* target){
    #if GDM_ENABLED
        // Initialize DomainModules
        using namespace Nebulite::DomainModule::GlobalSpace;
        /*...*/
        target->initModule<MyFeature>("<Feature Description>");
        /*...*/
    #endif
}
}
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
