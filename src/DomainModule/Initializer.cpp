#include "DomainModule/Initializer.hpp"

//------------------------------------------
// Domain includes
#include "Core/GlobalSpace.hpp"
#include "Core/Renderer.hpp"
#include "Core/Texture.hpp"
#include "Core/RenderObject.hpp"
#include "Utility/JSON.hpp"

//------------------------------------------
// DomainModules

// GlobalSpace
#include "DomainModule/GlobalSpace/FeatureTest.hpp"              // Feature testing module
#include "DomainModule/GlobalSpace/GSDM_Time.hpp"                // Basic Time management functions
#include "DomainModule/GlobalSpace/GSDM_Clock.hpp"               // Clock management functions
#include "DomainModule/GlobalSpace/GSDM_General.hpp"             // General functions like eval, exit, wait, etc.
#include "DomainModule/GlobalSpace/GSDM_Debug.hpp"               // Debugging and logging functions
#include "DomainModule/GlobalSpace/GSDM_Input.hpp"               // Input handling
#include "DomainModule/GlobalSpace/GSDM_Debug_Domain_Collision_Detection.hpp" // Special debugging utilities for domain collision detection

// JSON
#include "DomainModule/JSON/JSDM_SimpleData.hpp"
#include "DomainModule/JSON/JSDM_ComplexData.hpp"
#include "DomainModule/JSON/JSDM_Debug.hpp"

// RenderObject
#include "DomainModule/RenderObject/RODM_Debug.hpp"
#include "DomainModule/RenderObject/RODM_Logging.hpp"
#include "DomainModule/RenderObject/RODM_StateUpdate.hpp"
#include "DomainModule/RenderObject/RODM_Mirror.hpp"

// Renderer
#include "DomainModule/Renderer/RRDM_General.hpp"
#include "DomainModule/Renderer/RRDM_Console.hpp"
#include "DomainModule/Renderer/RRDM_RenderObjectDraft.hpp"

// Texture
#include "DomainModule/Texture/TXDM_General.hpp"
#include "DomainModule/Texture/TXDM_Rotation.hpp"
#include "DomainModule/Texture/TXDM_Fill.hpp"

//------------------------------------------
namespace Nebulite::DomainModule {

static std::string const headless_desc = "Set headless mode (no renderer)";
static std::string const recover_desc  = "Enable recoverable error mode";

void Initializer::initGlobalSpace(Core::GlobalSpace* target) {
    //------------------------------------------
    // Initialize DomainModules
    using namespace Nebulite::DomainModule::GlobalSpace;
    target->initModule<General>("Global General Functions");
    target->initModule<Debug>("Global Debug Functions");
    target->initModule<Input>("Global Input Functions");

    //------------------------------------------
    // Special debugging utilities
    target->initModule<Debug_Domain_Collision_Detection>("Global Debug Domain Collision Detection");

    //------------------------------------------
    // Feature Test Modules
    target->initModule<FeatureTest>("Global Feature Test Functions");

    //------------------------------------------
    // Time module relies on knowing if anything is locking the time
    // So we need to initialize it last
    // Example: Console might want to halt time while open
    //          if we initialize time first, it will update before console
    //          thus ignoring the console's halt request being sent to renderer
    target->initModule<Time>("Global Time Functions");
    target->initModule<Clock>("Global Clock Functions"); // Clock relies on time, so init after time
    
    //------------------------------------------
    // Initialize Variable Bindings
    target->bindVariable(&target->cmdVars.headless, "headless", &headless_desc);
    target->bindVariable(&target->cmdVars.recover,  "recover",  &recover_desc);
}

void Initializer::initJSON(Utility::JSON* target) {
    // Initialize DomainModules
    using namespace Nebulite::DomainModule::JSON;
    target->initModule<SimpleData>("JSON Simple Data Functions");
    target->initModule<ComplexData>("JSON Complex Data Functions");
    target->initModule<Debug>("JSON Debug Functions");
}

void Initializer::initRenderObject(Core::RenderObject* target) {
    // Initialize DomainModules
    using namespace Nebulite::DomainModule::RenderObject;
    target->initModule<Debug>("RenderObject Debug Functions");
    target->initModule<Logging>("RenderObject Logging Functions");
    target->initModule<StateUpdate>("RenderObject State Update Functions");
    target->initModule<Mirror>("RenderObject Mirror Functions");
}

void Initializer::initRenderer(Core::Renderer* target) {
    // Initialize DomainModules
    using namespace Nebulite::DomainModule::Renderer;
    target->initModule<General>("Renderer General Functions");
    target->initModule<Console>("Renderer Console Functions");
    target->initModule<RenderObjectDraft>("Renderer RenderObjectDraft Functions");
}

void Initializer::initTexture(Core::Texture* target) {
    // Initialize DomainModules
    using namespace Nebulite::DomainModule::Texture;
    target->initModule<General>("Texture General Functions");
    target->initModule<Rotation>("Texture Rotation Functions");
    target->initModule<Fill>("Texture Fill Functions");
}

} // namespace Nebulite::DomainModule