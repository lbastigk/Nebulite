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

Each Class has access to a different tree through `funcTree->...` and a different domain through `self->...`: 
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
- Use filenames `GDM_<ModuleName>.{h,cpp}` , `RDM_<ModuleName>.{h,cpp}` and `JDM_<ModuleName>.{h,cpp}` for module files
- Use the recommended class naming schemes and namespaces, e.g.: `Nebulite::DomainModule::GlobalSpace::MyFeature`


### Function Collision Prevention

- The `GlobalSpaceTree` automatically inherits all functions from `JSONTree`, which act on the global document
- The `RenderObjectTree` automatically inherits all functions from `JSONTree`, which act on the objects document

It is **not allowed** to overwrite already existing functions:
- If the function `set` was already declared, it is not possible to declare a new `set` function in that same tree
- If the function `set` was already declared for the subtree, it is not possible to declare a new `set` function in the Tree that inherits the function

## Example: Adding a New GlobalSpaceTree Feature

### Step-by-Step Process

1. **Create expansion file:** `GDM_MyFeature.{h,cpp}`
2. **Inherit from wrapper:** Create class inheriting from `Nebulite::Interaction::Execution::DomainModuleWrapper<DomainClass, MyFeatureClass>` and using its Constructor
3. **Implement command methods:** Functions with `ERROR_TYPE (int argc, char* argv[])` signature
4. **setupBindings():** Bind your commands to the FuncTree
5. **update** Add all necessary update-procedures within the tree
6. **Add to GlobalSpaceTree:** Include in `include/Interaction/Execution/GlobalSpaceTree.h`, add to the update and initialize in constructor
7. **Command line Variables** are more difficult to implement, as they require the full domain definition. Bind them in `src/Interaction/Execution/GlobalSpaceTree.cpp`

### Complete Code Example

**Inside GDM_MyFeature.h:**

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
class MyFeature : public Nebulite::Interaction::Execution::DomainModuleWrapper<Nebulite::Core::GlobalSpace, MyFeature> {
public:
    using DomainModuleWrapper<Nebulite::Core::GlobalSpace, MyFeature>::DomainModuleWrapper; // Templated constructor from Wrapper, call this->setupBindings()

    void update();  // For implementing internal update-procedures on each new frame

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

**Inside GDM_MyFeature.cpp:**
```cpp
#include "DomainModule/GlobalSpace/GDM_MyFeature.h"
#include "Core/GlobalSpace.h"

void Nebulite::DomainModule::GlobalSpace::MyFeature::update(){
    // If our expansion uses any internal values that need to be updated on each frame
    // We can update them here
}

Nebulite::ERROR_TYPE Nebulite::DomainModule::GlobalSpace::MyFeature::spawnCircle(int argc, char* argv[]){
    /*
    Implementation here.
    You can access the global space and its members through: self->...
    As well as the funcTree through: funcTree->...
    */
}
```

**Then add to include/GlobalSpaceTree.h:**
```cpp
#include "DomainModule/GlobalSpace/GDM_MyFeature.h"
namespace Nebulite{
namespace Interaction{
namespace Execution{
/**/
class GlobalSpaceTree : public FuncTree<Nebulite::ERROR_TYPE>{
    /*...*/
private:
    /*...*/
    std::unique_ptr<Nebulite::DomainModule::GlobalSpace::MyFeature> myFeature;
};
}   // namespace Interaction
}   // namespace Execution
}   // namespace Nebulite
```

**And initialize and update in GlobalSpaceTree.cpp:**
```cpp
Nebulite::Interaction::Execution::GlobalSpaceTree::GlobalSpaceTree(/*...*/) : /*...*/
{
    // Initialize all expansions
    /*...*/
    myFeature = createExpansionOfType<Nebulite::DomainModule::GlobalSpace::MyFeature>();

    // Initialize Variable Bindings here, due to circular dependency issues
    /*...*/
    bindVariable(&domain->myVariable, "myVariable", "This is a variable inside globalSpace");
}

void Nebulite::Interaction::Execution::GlobalSpaceTree::update(){
    /*...*/
    myFeature->update();
}

```

### Feature Management

If necessary, the entire feature can then be:
- **disabled** by commenting out `createExpansionOfType` in the GlobalSpaceTree Constructor and removing the update call in `GlobalSpaceTree::update()`
- **removed** by undoing all changes inside `GlobalSpaceTree.{h,cpp}`

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
