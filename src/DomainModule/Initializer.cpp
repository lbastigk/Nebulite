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
namespace Nebulite::DomainModule {

// TODO: Once CallerScope is implemented, we can restrict scope access to every DomainModule
//       no Module should have full access!

void Initializer::initGlobalSpace(Core::GlobalSpace* target) {

    //------------------------------------------
    // TODO: Add settings domainModule:
    //       - initialized first with scope "settings."
    //       - modify every DomainModule constructor to accept a settings scope
    //       - provide a reference to the settings scope in every DomainModule
    //       - perhaps we can even scope this? Instead of passing all settings to every DomainModule,
    //         we can only pass the relevant settings.

    //------------------------------------------
    // Initialize DomainModules
    using namespace Nebulite::DomainModule::GlobalSpace;
    target->initModule<General>(
        "Global General Functions",
        target->domainScope.shareScopeBase(""), // See above, later on we can restrict this
        Nebulite::globalDoc().settings()
    );
    target->initModule<Debug>(
        "Global Debug Functions",
        target->domainScope.shareScopeBase("debug."),
        Nebulite::globalDoc().settings()
    );
    target->initModule<Ruleset>(
        "Global Ruleset Functions",
        target->domainScope.shareScopeBase("ruleset."),
        Nebulite::globalDoc().settings()
    );

    //------------------------------------------
    // Special debugging / testing utilities
    target->initModule<FunctionCollision>(
        "Global Function Collision Detection utilities",
        target->domainScope.shareDummyScopeBase(),
        Nebulite::globalDoc().settings()
    );
    target->initModule<FeatureTest>(
        "Global Feature Test Functions",
        target->domainScope.shareDummyScopeBase(),
        Nebulite::globalDoc().settings()
    );

    //------------------------------------------
    // Time module relies on knowing if anything is locking the time
    // Since domainModules are updates in the order they are initialized,
    // we need to init time after most other modules.
    target->initModule<Time>(
        "Global Time Functions",
        target->domainScope.shareScopeBase("time."),
        Nebulite::globalDoc().settings()
    );
    target->initModule<Clock>( // Clock relies on time, so init after time
        "Global Clock Functions",
        target->domainScope.shareScopeBase("time."),
        Nebulite::globalDoc().settings()
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
        target->domainScope.shareScopeBase(""),  // See above, later on we can restrict this
        Nebulite::globalDoc().settings()
    );
    target->initModule<ComplexData>(
        "JSON Complex Data Functions",
        target->domainScope.shareScopeBase(""),  // See above, later on we can restrict this
        Nebulite::globalDoc().settings()
    );
    target->initModule<Debug>(
        "JSON Debug Functions",
        target->domainScope.shareScopeBase(""), // See above, later on we can restrict this
        Nebulite::globalDoc().settings()
    );
}

void Initializer::initRenderObject(Core::RenderObject* target) {
    // Initialize DomainModules
    using namespace Nebulite::DomainModule::RenderObject;
    target->initModule<Debug>(  // TODO: Move eval function to a General DomainModule, so we can restruct this modules access
        "RenderObject Debug Functions",
        target->domainScope.shareScopeBase(""),  // See above, later on we can restrict this
        Nebulite::globalDoc().settings()
    );
    target->initModule<Logging>(
        "RenderObject Logging Functions",
        target->domainScope.shareScopeBase(""), // See above, later on we can restrict this
        Nebulite::globalDoc().settings()
    );
    target->initModule<Mirror>(
        "RenderObject Mirror Functions",
        target->domainScope.shareScopeBase(""), // See above, later on we can restrict this
        Nebulite::globalDoc().settings()
    );
    target->initModule<Ruleset>(
        "RenderObject Ruleset Functions",
        target->domainScope.shareScopeBase("ruleset."),
        Nebulite::globalDoc().settings()
    );
    target->initModule<StateUpdate>(
        "RenderObject State Update Functions",
        target->domainScope.shareDummyScopeBase(),
        Nebulite::globalDoc().settings()
    );
}

void Initializer::initRenderer(Core::Renderer* target) {
    // Initialize DomainModules
    using namespace Nebulite::DomainModule::Renderer;
    target->initModule<General>(
        "Renderer General Functions",
        target->domainScope.shareScopeBase(""), // See above, later on we can restrict this
        Nebulite::globalDoc().settings()
    );
    target->initModule<Console>(
        "Renderer Console Functions",
        // Needs full access to redirect console commands to full global scope
        target->domainScope.shareScopeBase(""),
        Nebulite::globalDoc().settings()
    );
    target->initModule<Input>(
        "Renderer Input Functions",
        target->domainScope.shareScopeBase("input."),
        Nebulite::globalDoc().settings()
    );
    target->initModule<RenderObjectDraft>(
        "Renderer RenderObjectDraft Functions",
        // TODO: We could modify the RenderObject constructor to accept an optional scope.
        //       This way, we can directly store the draft data in the renderer scope.
        //       Then, we can modify the scope to "draft." and have the entire renderObject live in that scope at root level.
        target->domainScope.shareScopeBase(""),
        Nebulite::globalDoc().settings()
    );
}

void Initializer::initTexture(Core::Texture* target) {
    // Initialize DomainModules
    using namespace Nebulite::DomainModule::Texture;

    // So far, any texture domainModules are basic enough to have full access
    target->initModule<General>(
        "Texture General Functions",
        target->domainScope.shareScopeBase(""),
        Nebulite::globalDoc().settings()
    );
    target->initModule<Rotation>(
        "Texture Rotation Functions",
        target->domainScope.shareScopeBase(""),
        Nebulite::globalDoc().settings()
    );
    target->initModule<Fill>(
        "Texture Fill Functions",
        target->domainScope.shareScopeBase(""),
        Nebulite::globalDoc().settings()
    );
}

} // namespace Nebulite::DomainModule
