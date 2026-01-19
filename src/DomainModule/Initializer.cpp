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
#include "DomainModule/Renderer/GUI_test.hpp"
#include "DomainModule/Renderer/Input.hpp"
#include "DomainModule/Renderer/RenderObjectDraft.hpp"

// Texture
#include "DomainModule/Texture/General.hpp"
#include "DomainModule/Texture/Rotation.hpp"
#include "DomainModule/Texture/Fill.hpp"

//------------------------------------------
namespace Nebulite::DomainModule {

void Initializer::initEnvironment(Core::Environment* target) {
    // Currently, no DomainModules for Environment
    (void)target->parseStr(""); // Silence "can be made pointer to const" warning
}

void Initializer::initGlobalSpace(Core::GlobalSpace* target) {
    using namespace Nebulite::DomainModule::GlobalSpace;

    //------------------------------------------
    // Initialize DomainModules

    //------------------------------------------
    // Settings module should be initialized first to load settings for other modules
    target->initModule<Settings>(
        "Global Settings Functions",
        target->domainScope.shareScopeBase("settings."),
        Global::settings()
    );

    //------------------------------------------
    // Core modules
    target->initModule<General>(
        "Global General Functions",
        target->domainScope.shareDummyScopeBase(), // No workspace required.
        Global::settings()
    );
    target->initModule<Debug>(
        "Global Debug Functions",
        target->domainScope.shareScopeBase("debug."),
        Global::settings()
    );
    target->initModule<Ruleset>(
        "Global Ruleset Functions",
        target->domainScope.shareScopeBase("ruleset."),
        Global::settings()
    );

    //------------------------------------------
    // Special debugging / testing utilities
    target->initModule<FunctionCollision>(
        "Global Function Collision Detection utilities",
        target->domainScope.shareDummyScopeBase(), // No workspace required.
        Global::settings()
    );
    target->initModule<FeatureTest>(
        "Global Feature Test Functions",
        target->domainScope.shareDummyScopeBase(), // No workspace required.
        Global::settings()
    );

    //------------------------------------------
    // Time module relies on knowing if anything is locking the time
    // Since domainModules are updates in the order they are initialized,
    // we need to init time after most other modules.
    target->initModule<Time>(
        "Global Time Functions",
        target->domainScope.shareScopeBase("time."),
        Global::settings()
    );
    target->initModule<Clock>( // Clock relies on time, so init after time
        "Global Clock Functions",
        target->domainScope.shareScopeBase("time."),
        Global::settings()
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
    // Initialize DomainModules
    using namespace Nebulite::DomainModule::JsonScope;
    target->initModule<SimpleData>(
        "JSON Simple Data Functions",
        target->domainScope.shareDummyScopeBase(), // No workspace required.
        Global::settings()
    );
    target->initModule<ComplexData>(
        "JSON Complex Data Functions",
        target->domainScope.shareDummyScopeBase(), // No workspace required.
        Global::settings()
    );
    target->initModule<Debug>(
        "JSON Debug Functions",
        target->domainScope.shareDummyScopeBase(), // No workspace required.
        Global::settings()
    );
}

void Initializer::initRenderObject(Core::RenderObject* target) {
    // Initialize DomainModules
    using namespace Nebulite::DomainModule::RenderObject;
    target->initModule<Debug>(
        "RenderObject Debug Functions",
        target->domainScope.shareDummyScopeBase(),  // No workspace required.
        Global::settings()
    );
    target->initModule<Logging>(
        "RenderObject Logging Functions",
        target->domainScope.shareDummyScopeBase(), // No workspace required.
        Global::settings()
    );
    target->initModule<Mirror>(
        "RenderObject Mirror Functions",
        target->domainScope.shareScopeBase(""), // Requires full access to mirror entire object in update routine.
        Global::settings()
    );
    target->initModule<Ruleset>(
        "RenderObject Ruleset Functions",
        target->domainScope.shareScopeBase("ruleset."),
        Global::settings()
    );
    target->initModule<StateUpdate>(
        "RenderObject State Update Functions",
        target->domainScope.shareDummyScopeBase(),  // No workspace required.
        Global::settings()
    );
}

void Initializer::initRenderer(Core::Renderer* target) {
    // Initialize DomainModules
    using namespace Nebulite::DomainModule::Renderer;
    target->initModule<General>(
        "Renderer General Functions",
        target->domainScope.shareDummyScopeBase(),
        Global::settings()
    );
    target->initModule<Console>(
        "Renderer Console Functions",
        // Needs full access to redirect console commands to full global scope
        target->domainScope.shareScopeBase(""),
        Global::settings()
    );
    target->initModule<Input>(
        "Renderer Input Functions",
        target->domainScope.shareScopeBase("input."),
        Global::settings()
    );
    target->initModule<RenderObjectDraft>(
        "Renderer RenderObjectDraft Functions",
        // TODO: We could modify the RenderObject constructor to accept an optional scope.
        //       This way, we can directly store the draft data in the renderer scope.
        //       Then, we can modify the scope to "draft." and have the entire renderObject live in that scope at root level.
        target->domainScope.shareScopeBase(""),
        Global::settings()
    );
    target->initModule<GUI_test>(
        "Renderer GUI Test Functions",
        target->domainScope.shareDummyScopeBase(), // No workspace required.
        Global::settings()
    );
}

void Initializer::initTexture(Core::Texture* target) {
    // Initialize DomainModules
    using namespace Nebulite::DomainModule::Texture;
    target->initModule<General>(
        "Texture General Functions",
        target->domainScope.shareDummyScopeBase(), // No workspace required.
        Global::settings()
    );
    target->initModule<Rotation>(
        "Texture Rotation Functions",
        target->domainScope.shareDummyScopeBase(), // No workspace required.
        Global::settings()
    );
    target->initModule<Fill>(
        "Texture Fill Functions",
        target->domainScope.shareDummyScopeBase(), // No workspace required.
        Global::settings()
    );
}

} // namespace Nebulite::DomainModule
