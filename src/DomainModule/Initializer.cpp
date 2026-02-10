#include "DomainModule/Initializer.hpp"

//------------------------------------------
// Include Nebulite main header for settings access
#include "Nebulite.hpp"

//------------------------------------------
// Domain includes
#include "Core/GlobalSpace.hpp"
#include "Core/JsonScope.hpp"
#include "Core/Renderer.hpp"
#include "Core/Texture.hpp"
#include "Core/RenderObject.hpp"

//------------------------------------------
// DomainModules

// GlobalSpace
#include "DomainModule/GlobalSpace/Clock.hpp"               // Clock management functions
#include "DomainModule/GlobalSpace/Debug.hpp"               // Debugging and logging functions
#include "DomainModule/GlobalSpace/FunctionCollision.hpp"   // Special debugging utilities for domain collision detection
#include "DomainModule/GlobalSpace/FeatureTest.hpp"         // Feature testing module
#include "DomainModule/GlobalSpace/General.hpp"             // General functions like eval, exit, wait, etc.
#include "DomainModule/GlobalSpace/InputMapping.hpp"        // Input mapping and processing functions
#include "DomainModule/GlobalSpace/Ruleset.hpp"             // Ruleset management
#include "DomainModule/GlobalSpace/Settings.hpp"            // Settings management
#include "DomainModule/GlobalSpace/Time.hpp"                // Basic Time management functions

// JSON
#include "DomainModule/JsonScope/SimpleData.hpp"
#include "DomainModule/JsonScope/ComplexData.hpp"
#include "DomainModule/JsonScope/Debug.hpp"

// RenderObject
#include "DomainModule/RenderObject/Debug.hpp"
#include "DomainModule/RenderObject/Logging.hpp"
#include "DomainModule/RenderObject/Mirror.hpp"
#include "DomainModule/RenderObject/Ruleset.hpp"
#include "DomainModule/RenderObject/StateUpdate.hpp"

// Renderer
#include "DomainModule/Renderer/Console.hpp"
#include "DomainModule/Renderer/General.hpp"
#include "DomainModule/Renderer/Input.hpp"
#include "DomainModule/Renderer/RenderObjectDraft.hpp"

// Texture
#include "DomainModule/Texture/General.hpp"
#include "DomainModule/Texture/Rotation.hpp"
#include "DomainModule/Texture/Fill.hpp"

//------------------------------------------
// TODO: Some way to reference the scope of a module as static member,
//       perhaps by making it part of the module itself, and using it in the initializer class?
//       This may actually be necessary later on, with dll/so loading of modules
//       Then we can call module.getPrefix() inside the initModule function

//------------------------------------------
namespace Nebulite::DomainModule {

void Initializer::initEnvironment(Core::Environment* target) {
    // Currently, no DomainModules for Environment
    (void)target->parseStr(""); // Silence "can be made pointer to const" warning
}

void Initializer::initGlobalSpace(Core::GlobalSpace* target) {
    using namespace Nebulite::DomainModule::GlobalSpace;

    //------------------------------------------
    // Settings module should be initialized first to load settings for other modules
    target->initModule<Core::GlobalSpace, Settings>(
        "Global Settings Functions",
        Global::settings(),
        *target
    );

    //------------------------------------------
    // Core modules
    target->initModule<Core::GlobalSpace, General>(
        "Global General Functions",
        Global::settings(),
        *target
    );
    target->initModule<Core::GlobalSpace, Debug>(
        "Global Debug Functions",
        Global::settings(),
        *target
    );
    target->initModule<Core::GlobalSpace, Ruleset>(
        "Global Ruleset Functions",
        Global::settings(),
        *target
    );

    //------------------------------------------
    // Special debugging / testing utilities
    target->initModule<Core::GlobalSpace, FunctionCollision>(
        "Global Function Collision Detection utilities",
        Global::settings(),
        *target
    );
    target->initModule<Core::GlobalSpace, FeatureTest>(
        "Global Feature Test Functions",
        Global::settings(),
        *target
    );

    //------------------------------------------
    // Time module relies on knowing if anything is locking the time
    // Since domainModules are updates in the order they are initialized,
    // we need to init time after most other modules.
    target->initModule<Core::GlobalSpace, Time>(
        "Global Time Functions",
        Global::settings(),
        *target
    );
    target->initModule<Core::GlobalSpace, Clock>( // Clock relies on time, so init after time
        "Global Clock Functions",
        Global::settings(),
        *target
    );

    //------------------------------------------
    // Input mapping
    // Even though input is initialized in the Renderer module, the input mapping module may be better suited to live in the GlobalSpace
    target->initModule<Core::GlobalSpace, InputMapping>(
        "Global Input Mapping Functions",
        Global::settings(),
        *target
    );

    //------------------------------------------
    // Initialize Variable Bindings
    target->bindVariable(
        &target->cmdVars.headless,
        "headless",
        "Set headless mode (no renderer)"
    );
    target->bindVariable(
        &target->cmdVars.recover,
        "recover",
        "Enable recoverable error mode"
    );
}

void Initializer::initJsonScope(Core::JsonScope* target) {
    using namespace Nebulite::DomainModule::JsonScope;

    target->initModule<Core::JsonScope, SimpleData>(
        "JSON Simple Data Functions",
        Global::settings(),
        *target
    );
    target->initModule<Core::JsonScope, ComplexData>(
        "JSON Complex Data Functions",
        Global::settings(),
        *target
    );
    target->initModule<Core::JsonScope, Debug>(
        "JSON Debug Functions",
        Global::settings(),
        *target
    );
}

void Initializer::initRenderObject(Core::RenderObject* target) {
    using namespace Nebulite::DomainModule::RenderObject;

    target->initModule<Core::RenderObject, Debug>(
        "RenderObject Debug Functions",
        Global::settings(),
        *target
    );
    target->initModule<Core::RenderObject, Logging>(
        "RenderObject Logging Functions",
        Global::settings(),
        *target
    );
    target->initModule<Core::RenderObject, Mirror>(
        "RenderObject Mirror Functions",
        Global::settings(),
        *target
    );
    target->initModule<Core::RenderObject, Ruleset>(
        "RenderObject Ruleset Functions",
        Global::settings(),
        *target
    );
    target->initModule<Core::RenderObject, StateUpdate>(
        "RenderObject State Update Functions",
        Global::settings(),
        *target
    );
}

void Initializer::initRenderer(Core::Renderer* target) {
    using namespace Nebulite::DomainModule::Renderer;

    target->initModule<Core::Renderer, General>(
        "Renderer General Functions",
        Global::settings(),
        *target
    );
    target->initModule<Core::Renderer, Console>(
        "Renderer Console Functions",
        Global::settings(),
        *target
    );
    target->initModule<Core::Renderer, Input>(
        "Renderer Input Functions",
        Global::settings(),
        *target
    );
    target->initModule<Core::Renderer, RenderObjectDraft>(
        "Renderer RenderObjectDraft Functions",
        Global::settings(),
        *target
    );
}

void Initializer::initTexture(Core::Texture* target) {
    using namespace Nebulite::DomainModule::Texture;

    target->initModule<Core::Texture, General>(
        "Texture General Functions",
        Global::settings(),
        *target
    );
    target->initModule<Core::Texture, Rotation>(
        "Texture Rotation Functions",
        Global::settings(),
        *target
    );
    target->initModule<Core::Texture, Fill>(
        "Texture Fill Functions",
        Global::settings(),
        *target
    );
}

} // namespace Nebulite::DomainModule
