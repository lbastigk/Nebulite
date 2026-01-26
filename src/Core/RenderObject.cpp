//------------------------------------------
// Includes

// Nebulite
#include "Nebulite.hpp"
#include "Constants/KeyNames.hpp"
#include "Core/RenderObject.hpp"
#include "DomainModule/Initializer.hpp"
#include "DomainModule/JsonScope/SimpleData.hpp"
#include "Interaction/Rules/Ruleset.hpp"
#include "Interaction/Rules/Construction/RulesetCompiler.hpp"

//------------------------------------------
namespace Nebulite::Core {

//------------------------------------------
// Special member Functions

namespace {
// Helper function to initialize RenderObject in constructor
void setStandardValues(JsonScope& document) {
    // General
    document.set(Constants::KeyNames::RenderObject::id, 0);    // Initialize to 0, Renderer itself sets proper id, which starts at 1
    document.set(Constants::KeyNames::RenderObject::positionX, 0);
    document.set(Constants::KeyNames::RenderObject::positionY, 0);
    document.set(Constants::KeyNames::RenderObject::layer, 0);

    // Create a basic drawcall
    Graphics::Drawcall::ApplyDefault::Sprite(document.shareScope(Constants::KeyNames::RenderObject::draw + ".exampleSprite"));
    Graphics::Drawcall::ApplyDefault::Text(document.shareScope(Constants::KeyNames::RenderObject::draw + ".exampleText"));

    // Set default size
    document.set(Constants::KeyNames::RenderObject::pixelSizeX, 32);
    document.set(Constants::KeyNames::RenderObject::pixelSizeY, 32);

    // Ruleset
    document.setEmptyArray(Constants::KeyNames::RenderObject::Ruleset::broadcast);
    document.setEmptyArray(Constants::KeyNames::RenderObject::Ruleset::listen);
    document.set(Constants::KeyNames::RenderObject::Ruleset::listen + "[0]", std::string("all"));
}
} // namespace

RenderObject::RenderObject() : Domain("RenderObject", *this, document) {
    //------------------------------------------
    // Set standard values
    setStandardValues(document);

    //------------------------------------------
    // Flags
    flag.deleteFromScene = false;
    flag.calculateText = true; // In order to calculate text texture on first update

    //------------------------------------------
    // Initialize Linkages, References and DomainModules and object itself
    init();
}

void RenderObject::init() {
    // Inherit functions from child objects
    inherit(&document);

    // Link frequently used values
    linkFrequentRefs();

    // Initialize Domain Modules
    DomainModule::Initializer::initRenderObject(this);

    // Initialize Drawcalls
    initDrawcalls();

    // Update once to initialize
    update();
}

RenderObject::~RenderObject() = default;

//------------------------------------------
// Draw

void RenderObject::sortDrawcalls() {
    // re-generate drawcall order (alphabetical for now)
    drawcallOrder.clear();
    for (auto const& [member, _] : document.listAvailableMembersAndKeys(Constants::KeyNames::RenderObject::draw)) {
        drawcallOrder.push_back(member);
    }
    std::ranges::sort(drawcallOrder.begin(), drawcallOrder.end());
}

void RenderObject::reinitDrawcalls() {
    // Clear existing drawcalls
    drawcalls.clear();

    // Get list of drawcalls from document
    for (auto const& [member, key] : document.listAvailableMembersAndKeys(Constants::KeyNames::RenderObject::draw)) {
        // Initialize drawcall with its own scope
        drawcalls[member] = std::make_unique<Graphics::Drawcall>(document.shareScope(key.view()));
    }
    sortDrawcalls();
}

void RenderObject::initDrawcalls() {
    // Get list of drawcalls from document
    for (auto const& [member, key] : document.listAvailableMembersAndKeys(Constants::KeyNames::RenderObject::draw)) {
        // Initialize drawcall with its own scope
        if (drawcalls.find(member) == drawcalls.end()) {
            drawcalls[member] = std::make_unique<Graphics::Drawcall>(document.shareScope(key.view()));
        }
    }
    sortDrawcalls();
}

void RenderObject::reInitDrawcall(std::string const& drawcallName) {
    // Reinitialize a specific drawcall from document
    auto const key = Constants::KeyNames::RenderObject::draw + drawcallName;
    drawcalls[drawcallName] = std::make_unique<Graphics::Drawcall>(document.shareScope(key.view()));
}

void RenderObject::updateDrawcalls() {
    for (auto const& member : drawcallOrder) {
        drawcalls[member]->update();
    }
}

//------------------------------------------
// Marshalling

std::string RenderObject::serialize() const {
    return document.serialize();
}

void RenderObject::deserialize(std::string const& serialOrLink) {
    baseDeserialization(serialOrLink);

    // Re-Establish frequent references
    linkFrequentRefs();

    //------------------------------------------
    // Reinitialize everything
    reinitModules();
    reinitDrawcalls();

    //------------------------------------------
    // Update once to initialize
    update();
}

void RenderObject::linkFrequentRefs() {
    // Identity
    refs.id = document.getStableDoublePointer(Constants::KeyNames::RenderObject::id);

    // Position and Size
    refs.posX = document.getStableDoublePointer(Constants::KeyNames::RenderObject::positionX);
    refs.posY = document.getStableDoublePointer(Constants::KeyNames::RenderObject::positionY);
}

//------------------------------------------
// Outside communication with invoke for updating and estimation

Constants::Error RenderObject::update() {
    //------------------------------------------
    // Update modules and all inner domains
    updateModules();
    updateDrawcalls();
    document.update();
    return Constants::ErrorTable::NONE();
}

// TODO: Improve estimation by somehow leveraging a generated value from DomainModule Ruleset!
//       Current implementation generates mock Rulesets again here, which is not optimal
uint64_t RenderObject::estimateComputationalCost(bool const& onlyInternal) {
    std::vector<std::shared_ptr<Interaction::Rules::Ruleset>> rulesetsGlobal;
    std::vector<std::shared_ptr<Interaction::Rules::Ruleset>> rulesetsLocal;
    Interaction::Rules::Construction::RulesetCompiler::parse(rulesetsGlobal, rulesetsLocal, *this);

    //------------------------------------------
    // Count number of $ and { in logical Arguments
    uint64_t cost = 0;

    // Local entries
    cost = std::accumulate(
        rulesetsLocal.begin(), rulesetsLocal.end(), cost,
        [](uint64_t const acc, std::shared_ptr<Interaction::Rules::Ruleset> const& entry) {
            return acc + entry->getEstimatedCost();
        }
    );

    // Global entries
    if (!onlyInternal) {
        cost = std::accumulate(
            rulesetsGlobal.begin(), rulesetsGlobal.end(), cost,
            [](uint64_t const acc, std::shared_ptr<Interaction::Rules::Ruleset> const& entry) {
                return acc + entry->getEstimatedCost();
            }
        );
    }

    return cost;
}

} // namespace Nebulite::Core
