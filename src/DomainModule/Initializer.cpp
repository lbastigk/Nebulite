#include "DomainModule/Initializer.hpp"

//------------------------------------------
// Domain includes
#include "Core/GlobalSpace.hpp"
#include "Core/Renderer.hpp"
#include "Core/Texture.hpp"
#include "Core/RenderObject.hpp"
#include "Data/Document/JSON.hpp"

//------------------------------------------
// DomainModules

// GlobalSpace
#include "DomainModule/GlobalSpace/Clock.hpp"               // Clock management functions
#include "DomainModule/GlobalSpace/Debug.hpp"               // Debugging and logging functions
#include "DomainModule/GlobalSpace/FunctionCollision.hpp"   // Special debugging utilities for domain collision detection
#include "DomainModule/GlobalSpace/FeatureTest.hpp"         // Feature testing module
#include "DomainModule/GlobalSpace/General.hpp"             // General functions like eval, exit, wait, etc.
#include "DomainModule/GlobalSpace/Input.hpp"               // Input handling
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
#include "DomainModule/Renderer/General.hpp"
#include "DomainModule/Renderer/Console.hpp"
#include "DomainModule/Renderer/RenderObjectDraft.hpp"

// Texture
#include "DomainModule/Texture/General.hpp"
#include "DomainModule/Texture/Rotation.hpp"
#include "DomainModule/Texture/Fill.hpp"

//------------------------------------------
namespace Nebulite::DomainModule {

void Initializer::initGlobalSpace(Core::GlobalSpace* target) {
    //------------------------------------------
    // Initialize DomainModules
    using namespace Nebulite::DomainModule::GlobalSpace;
    target->initModule<General>("Global General Functions", &target->shareDocumentScopeBase(""));
    target->initModule<Debug>("Global Debug Functions", &target->shareDocumentScopeBase(""));
    target->initModule<Input>("Global Input Functions", &target->shareDocumentScopeBase(""));
    target->initModule<Ruleset>("Global Ruleset Functions", &target->shareDocumentScopeBase(""));

    //------------------------------------------
    // Special debugging utilities
    target->initModule<FunctionCollision>("Global Function Collision Detection utilities", &target->shareDocumentScopeBase(""));

    //------------------------------------------
    // Feature Test Modules
    target->initModule<FeatureTest>("Global Feature Test Functions", &target->shareDocumentScopeBase(""));

    //------------------------------------------
    // Time module relies on knowing if anything is locking the time
    // So we need to initialize it last
    // Example: Console might want to halt time while open
    //          if we initialize time first, it will update before console
    //          thus ignoring the console's halt request being sent to renderer
    target->initModule<Time>("Global Time Functions", &target->shareDocumentScopeBase(""));
    target->initModule<Clock>("Global Clock Functions", &target->shareDocumentScopeBase("")); // Clock relies on time, so init after time

    //------------------------------------------
    // Initialize Variable Bindings
    target->bindVariable(&target->cmdVars.headless, "headless", "Set headless mode (no renderer)");
    target->bindVariable(&target->cmdVars.recover, "recover", "Enable recoverable error mode");
}

void Initializer::initJsonScope(Data::JsonScope* target) {
    // Initialize DomainModules
    using namespace Nebulite::DomainModule::JsonScope;
    target->initModule<SimpleData>("JSON Simple Data Functions", &target->shareScopeBase(""));
    target->initModule<ComplexData>("JSON Complex Data Functions", &target->shareDocumentScopeBase(""));
    target->initModule<Debug>("JSON Debug Functions", &target->shareDocumentScopeBase(""));
}

void Initializer::initRenderObject(Core::RenderObject* target) {
    // Initialize DomainModules
    using namespace Nebulite::DomainModule::RenderObject;
    target->initModule<Debug>("RenderObject Debug Functions", &target->shareDocumentScopeBase(""));
    target->initModule<Logging>("RenderObject Logging Functions", &target->shareDocumentScopeBase(""));
    target->initModule<Mirror>("RenderObject Mirror Functions", &target->shareDocumentScopeBase(""));
    target->initModule<Ruleset>("RenderObject Ruleset Functions", &target->shareDocumentScopeBase(""));
    target->initModule<StateUpdate>("RenderObject State Update Functions", &target->shareDocumentScopeBase(""));
}

void Initializer::initRenderer(Core::Renderer* target) {
    // Initialize DomainModules
    using namespace Nebulite::DomainModule::Renderer;
    target->initModule<General>("Renderer General Functions", &target->shareDocumentScopeBase(""));
    target->initModule<Console>("Renderer Console Functions", &target->shareDocumentScopeBase(""));
    target->initModule<RenderObjectDraft>("Renderer RenderObjectDraft Functions", &target->shareDocumentScopeBase(""));
}

void Initializer::initTexture(Core::Texture* target) {
    // Initialize DomainModules
    using namespace Nebulite::DomainModule::Texture;
    target->initModule<General>("Texture General Functions", &target->shareDocumentScopeBase(""));
    target->initModule<Rotation>("Texture Rotation Functions", &target->shareDocumentScopeBase(""));
    target->initModule<Fill>("Texture Fill Functions", &target->shareDocumentScopeBase(""));
}

} // namespace Nebulite::DomainModule
