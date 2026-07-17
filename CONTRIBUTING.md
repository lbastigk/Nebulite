# Contributing to Nebulite

We welcome contributions! Nebulite's modular architecture makes it easy to add features 
in separate files and barely touching existing ones.

<!-- TOC start (generated with https://github.com/derlin/bitdowntoc) -->

- [Development Setup](#development-setup)
   * [Prerequisites](#prerequisites)
- [Testing](#testing)
   * [Quick Expression Testing](#quick-expression-testing)
- [Adding Features](#adding-features)
   * [Function Collision Prevention](#function-collision-prevention)
   * [Example: Adding a New GlobalSpace Feature](#example-adding-a-new-globalspace-feature)
      + [Step-by-Step Process](#step-by-step-process)
      + [Complete Code Example](#complete-code-example)
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

You can add custom tests to the test suite by extending the main test file `Tools/tests.jsonc` 
with references to your own test files in `Tools/Tests/.../*.json`. 
```jsonc
"tests": [
    // ... existing test files
    "Tools/Tests/MyNewTestFile.json"
]
```
Each test file should contain an array of test cases, where each test case specifies a taskfile to run and the expected output. For example:
```json
[
    {
      "command": "task path/to/myTestFile.nebs",
      "expected": { "cout": ["1234"], "cerr": [] }
    }
]
```

It is recommended to call taskFiles in your test files and validate the correctness inside the task file as much as possible:
```json
[
    {
      "command": "task TaskFiles/Tests/.../myTestFile.nebs",
      "expected": { "cout": [], "cerr": [] }
    }
]
```

```nebs
# Setup workspace values
set workspace.valueMember value
set workspace.arrayMember[0] 0
set workspace.arrayMember[1] 1
set workspace.arrayMember[2] 2
set workspace.objectMember.value0 0
set workspace.objectMember.value1 1
set workspace.objectMember.value2 2

# Bundle the members into an array and store the result in the workspace
assign global:workspace.result = {global:|bundleToArray workspace.valueMember workspace.arrayMember workspace.objectMember}

# Confirm the result is correct
eval nop {global:workspace.result[0]|typeAsString|assert equals string value:string:5}
eval nop {global:workspace.result[1]|typeAsString|assert equals string array:3}
eval nop {global:workspace.result[2]|typeAsString|assert equals string object:3}
```

<!-- TOC --><a name="adding-features"></a>
## Adding Features

Nebulite offers clean expansions of its functionality through its Modules for:
- Domains
- RmlUI (aka Plugins)
- Rulesets
- Transformations

See `include/Module/Base` for virtual base classes for each module type.

### DomainModules

Maintainers can create their own module classes and add them to a specific domain.
Either for specific Domains such as Renderer or for the GlobalSpace, 
or common functionality that is shared across multiple domains.

Each DomainModule has access to a different domain workspace through `domain....`,
as well as an update routine, allowing us to declutter classes by binding routines to specific modules:
- input-reading
- state-update
- lifetime management

and more. We then just insert each module into the class and its update function is automatically called.
See `include/Module/Domain/` for the existing DomainModules. Modules are added via the initializer class
`include/Module/Domain/Initializer.hpp`.

### RmlUI (aka Plugins)

RmlUi plugins allow you to easily add new UI element features, such as expression evaluation,
custom command execution etc. They are added inside the RmlInterface class 
`include/Graphics/RmlInterface.hpp` and are automatically initialized and updated.

Each module has access to core RmlUi Plugin functions such as `OnDocumentLoad` and `OnElementCreate`, 
as well as post-render callbacks, SDL event callbacks and update calls.
Each module has access to features from the RmlInterface such as context access,
unique Rml element identification, document management and more, 
allowing for easy implementation of new features.

### Rulesets

RulesetModules allow you to implement local and global hardcoded rules, such as:
- player movement
- physics
- game logic
- debugging output on certain conditions

and more.  They are added to the Ruleset Initializer class `Interaction/Rules/Construction/Initializer.hpp`.

For rulesets to work, you must generate a list of keys that the module will use for its rules,
and generate a baslist function on construction so that the Invoke class can generate the list of variables to pass to the ruleset on invocation. 

Example, using the camera movement ruleset:
```cpp
// Define the list of scoped keys
const std::vector<Data::ScopedKeyView> baseKeys = {
    Constants::KeyNames::RenderObject::positionX,
    Constants::KeyNames::RenderObject::positionY,
    Constants::KeyNames::RenderObject::sizeX,
    Constants::KeyNames::RenderObject::sizeX
};

// Shorthand enums to use
enum class Key : uint8_t {
    posX,
    posY,
    spriteSizeX,
    spriteSizeY
};
```
Inside the constructor, the function is generated and bound to each ruleset that needs it:
```cpp
auto const baseListFunc = generateBaseListFunction(baseKeys);
bind<alignCenterName>(Interaction::Rules::Ruleset::Type::Local, &Camera::alignCenter, alignCenterDesc, baseListFunc);
// ... bind other rulesets
```
On invocation, both slf and otr have the same list of variables. Use `baseVal(<enum>)` to access the variables in the ruleset function:
```cpp
void Camera::myCameraFunction(Interaction::Context const& /*context*/, double** slf, double** /*otr*/) {
    // Access the variables using the baseVal function and the enum
    double& posX = baseVal(Key::posX);
    double& posY = baseVal(Key::posY);
    double& spriteSizeX = baseVal(Key::spriteSizeX);
    double& spriteSizeY = baseVal(Key::spriteSizeY);

    // Implement the ruleset logic using the accessed variables
    // ...
}
```

### Transformations

TransformationModules allow you to implement custom data transformations such as:
- length of array
- listing members
- modifying specific members
- glob/regex matching

and more. Just like with DomainModules, implement a function and its name + description, and bind it.
Then, initialize the module in the Transformation Initializer class, and the function will be available for on value transformation.
Just add your custom module to the initializer class `include/Data/Document/JsonTransformer.hpp`.

<!-- TOC --><a name="function-collision-prevention"></a>
## Function Collision Prevention

Domains follow an inheritance tree structure for their functions:
- `GlobalSpace` automatically inherits all functions from `Renderer`
- `Renderer` inherits from `Environment`
- `RenderObject` inherits no other domain, but has floating texture Domains that live inside the same Document that need to be individually accessed.
   Inheritance is not feasible, as a RenderObject may have multiple textures.
- `Texture` does not inherit from any other domain.

The difference between inheritance and shared documents is that functions parsed in the Domain 
are only redirected to another Domain if they inherit from it. Shared documents only allow access to the same variables
or manual function calls.

It is **not allowed** to overwrite already existing functions, unless the function it points to is exactly the same.
This is to prevent accidental overwriting of functions and ensure that different modules can safely bind to the same function without worrying about collisions.

Furthermore, it is not allowed to overwrite existing `categories` with functions and vice versa. 

If a function is bound inside a non-existing `category`, the program will exit:
```cpp
bindCategory("MyCategory","<Description of MyCategory>");
bindFunction(/**/,"MyCategory foo","<Description of foo>"); //<-- This would fail without bindCategory being called first
```

**Then add the header file to `include/DomainModule/Initializer.hpp` and initialize in the cpp file.**

<!-- TOC --><a name="implementation-guidelines"></a>
### Implementation Guidelines

- It is recommended to return `Nebulite::Constants::StandardCapture::Error::Functional::functionNotImplemented(capture)` in unfinished DomainModule functions/features
- Use the recommended class and namespace naming schemes for DomainModules: 
`Nebulite::Module::Domain::<DomainName>::<ModuleName>`, where `<DomainName>` is `Common` if the module is for any Domain.
- Same for any other module type, just replace `Domain` with the module type name, such as `Ruleset` or `RmlUi`
- Add self-validating test scripts to `TaskFiles/Tests/...`, add a test file in `Tools/.../myTests.json` and reference them in `Tools/tests.jsonc`

<!-- TOC --><a name="preview-editing-work-in-progress"></a>
## Preview Editing (Work in Progress)

Preview Editing is currently under development. The current plan is to use the headless rendering mode of Nebulite 
in combination with a Golang-Backend to allow rendering snapshots to a web viewer UI while editing JSON files.
See `interface/web/` for the current progress.

<!-- TOC --><a name="submitting-changes"></a>
## Submitting Changes

1. Make sure you cover a wide range of test cases for your new feature. 
Verify the coverage with `make build-and-coverage-report`.
2. Ensure all tests pass: `make test` or `make build-and-test-available`
3. Update documentation: `make docs` as well as manual updates if necessary
4. Create a pull request with a clear description of your changes

<!-- TOC --><a name="code-style"></a>
## Code Style

- Follow the existing code style and conventions, using clang is highly recommended for formatting and linting
- Use meaningful variable and function names
- Comment why, not what!
- Keep functions focused and modular
- Keep file size friendly and try to reason if an organizer class is needed to declutter the code
- Stay curious regarding new c++ features and best practices, and apply them when possible:
  - Use `auto` when the type is obvious from the context
  - Use range-based for loops when iterating over containers
  - Use constexpr as much as possible
  - Use references, not pointers, when possible
  - Lambdas are your friend
  - While official support is still lacking, feel free to push for new features such as contracts and reflection
- `using namespace` in header files will get you banished to the shadow realm
- Use modern function signatures for Nebulite FuncTree bindings (span), not argc/argv!

<!-- TOC --><a name="getting-help"></a>
## Getting Help

- Check existing issues and discussions
- Review the architecture documentation in the main README
- Look at existing expansion implementations for patterns
- Feel free to open an issue/discussion for bugs, questions or clarifications
