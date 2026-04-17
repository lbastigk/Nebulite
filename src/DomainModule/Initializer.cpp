#include "DomainModule/Initializer.hpp"

//------------------------------------------
// Include Nebulite main header for settings access
#include "Nebulite.hpp"

//------------------------------------------
// Domain includes
#include "Core/GlobalSpace.hpp"
#include "Core/Renderer.hpp"
#include "Core/Texture.hpp"
#include "Core/RenderObject.hpp"

//------------------------------------------
// DomainModules

// Common
#include "DomainModule/Common/ComplexData.hpp"
#include "DomainModule/Common/Debug.hpp"
#include "DomainModule/Common/General.hpp"
#include "DomainModule/Common/SimpleData.hpp"

// Environment
#include "DomainModule/Environment/Debug.hpp"

// GlobalSpace
#include "DomainModule/GlobalSpace/Clock.hpp"
#include "DomainModule/GlobalSpace/Debug.hpp"
#include "DomainModule/GlobalSpace/FunctionCollision.hpp"   // Special debugging utilities for domain collision detection
#include "DomainModule/GlobalSpace/FeatureTest.hpp"         // Feature testing module
#include "DomainModule/GlobalSpace/General.hpp"
#include "DomainModule/GlobalSpace/InputMapping.hpp"
#include "DomainModule/GlobalSpace/Physics.hpp"
#include "DomainModule/GlobalSpace/Ruleset.hpp"
#include "DomainModule/GlobalSpace/Settings.hpp"
#include "DomainModule/GlobalSpace/Time.hpp"

// RenderObject
#include "DomainModule/RenderObject/Logging.hpp"
#include "DomainModule/RenderObject/Mirror.hpp"
#include "DomainModule/RenderObject/Ruleset.hpp"
#include "DomainModule/RenderObject/StateUpdate.hpp"

// Renderer
#include "DomainModule/Renderer/Audio.hpp"
#include "DomainModule/Renderer/Console.hpp"
#include "DomainModule/Renderer/General.hpp"
#include "DomainModule/Renderer/Input.hpp"
#include "DomainModule/Renderer/RenderObjectDraft.hpp"
#include "DomainModule/Renderer/RmlUi.hpp"

// Texture
#include "DomainModule/Texture/General.hpp"
#include "DomainModule/Texture/Rotation.hpp"
#include "DomainModule/Texture/Fill.hpp"

//------------------------------------------
namespace Nebulite::DomainModule {

void Initializer::initCommon(Interaction::Execution::Domain* target) {
    target->initModule<Interaction::Execution::Domain, Common::ComplexData>(
        "Common Complex Data Functions",
        Global::settings(),
        *target
    );
    target->initModule<Interaction::Execution::Domain, Common::Debug>(
        "Common Debug Functions",
        Global::settings(),
        *target
    );
    target->initModule<Interaction::Execution::Domain, Common::General>(
        "Common General Functions",
        Global::settings(),
        *target
    );
    target->initModule<Interaction::Execution::Domain, Common::SimpleData>(
        "Common Simple Data Functions",
        Global::settings(),
        *target
    );
}

void Initializer::initEnvironment(Core::Environment* target) {
    using namespace Nebulite::DomainModule::Environment;

    target->initModule<Core::Environment, Debug>(
        "Environment Debug Functions",
        Global::settings(),
        *target
    );
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
    // Even though input is initialized in the Renderer module,
    // the input mapping module may be better suited to live in the GlobalSpace
    target->initModule<Core::GlobalSpace, InputMapping>(
        "Global Input Mapping Functions",
        Global::settings(),
        *target
    );

    //------------------------------------------
    // Misc
    target->initModule<Core::GlobalSpace, Physics>(
        "Global Physics Utilities",
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

void Initializer::initRenderObject(Core::RenderObject* target) {
    using namespace Nebulite::DomainModule::RenderObject;

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
    target->initModule<Core::Renderer, Audio>(
        "Renderer Audio Functions",
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
    target->initModule<Core::Renderer, RmlUi>(
        "Renderer RmlUi Functions",
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
