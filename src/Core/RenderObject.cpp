//------------------------------------------
// Includes

// Standard library
#include <algorithm>
#include <cstdint> // NOLINT
#include <memory>
#include <string>
#include <vector>

// Nebulite
#include "Constants/Event.hpp"
#include "Constants/KeyNames.hpp"
#include "Core/RenderObject.hpp"
#include "Data/Document/JsonScope.hpp"
#include "Graphics/Drawcall.hpp"
#include "Interaction/Rules/Ruleset.hpp"
#include "Module/Domain/Initializer.hpp"
#include "Nebulite.hpp"
#include "Utility/IO/Capture.hpp"

//------------------------------------------
namespace Nebulite::Core {

//------------------------------------------
// Special member Functions

namespace {
// Helper function to initialize RenderObject in constructor
void setStandardValues(Data::JsonScope& document) {
    // General
    document.set(Constants::KeyNames::RenderObject::positionX, 0);
    document.set(Constants::KeyNames::RenderObject::positionY, 0);
    document.set(Constants::KeyNames::RenderObject::layer, 1);

    // Create a basic drawcall
    Graphics::Drawcall::ApplyDefault::Sprite(document.shareScope(Constants::KeyNames::RenderObject::draw.addMember("exampleSprite")));
    Graphics::Drawcall::ApplyDefault::Text(document.shareScope(Constants::KeyNames::RenderObject::draw.addMember("exampleText")));

    // Set default size
    document.set(Constants::KeyNames::RenderObject::sizeX, 32);
    document.set(Constants::KeyNames::RenderObject::sizeY, 32);
}
} // namespace

RenderObject::RenderObject(Utility::IO::Capture& parentCapture)
    : Domain("RenderObject", parentCapture)
    , RenderObjectDocumentAccessor(domainScope){
    //------------------------------------------
    // Set standard values
    setStandardValues(domainScope);

    //------------------------------------------
    // Flags
    flag.deleteFromScene = false;

    //------------------------------------------
    // Initialize Linkages, References and DomainModules and object itself
    init();
}

void RenderObject::init() {
    // Inherit functions from child objects
    // None so far

    // Link frequently used values
    linkFrequentRefs();

    // Initialize Domain Modules
    Module::Domain::Initializer::initRenderObject(this);

    // Initialize Drawcalls
    initDrawcalls();

    // Update once to initialize
    Global::instance().notifyEvent(update());
}

RenderObject::~RenderObject() = default;

//------------------------------------------
// Drawcalls

void RenderObject::draw(Renderer const& renderer, float const& offsetX, float const& offsetY) {
    for (auto const& member : drawcallOrder) {
        drawcalls[member]->draw(
            renderer,
            static_cast<float>(*refs.posX) - offsetX,
            static_cast<float>(*refs.posY) - offsetY
        );
    }
}

void RenderObject::reinitDrawcalls() {
    // Clear existing drawcalls
    drawcalls.clear();

    // Get list of drawcalls from document
    for (auto const& [member, key] : domainScope.listAvailableMembersAndKeys(Constants::KeyNames::RenderObject::draw)) {
        // Initialize drawcall with its own scope
        drawcalls[member] = std::make_unique<Graphics::Drawcall>(domainScope.shareScope(key.view()), capture);
    }
    sortDrawcalls();
}

void RenderObject::initDrawcalls() {
    // Get list of drawcalls from document
    for (auto const& [member, key] : domainScope.listAvailableMembersAndKeys(Constants::KeyNames::RenderObject::draw)) {
        // Initialize drawcall with its own scope
        if (drawcalls.find(member) == drawcalls.end()) {
            drawcalls[member] = std::make_unique<Graphics::Drawcall>(domainScope.shareScope(key.view()), capture);
        }
    }
    sortDrawcalls();
}

void RenderObject::reInitDrawcall(std::string const& drawcallName) {
    // Reinitialize a specific drawcall from document
    auto const key = Constants::KeyNames::RenderObject::draw.addMember(drawcallName);
    drawcalls[drawcallName] = std::make_unique<Graphics::Drawcall>(domainScope.shareScope(key.view()), capture);
}

void RenderObject::sortDrawcalls() {
    // re-generate drawcall order (alphabetical for now)
    drawcallOrder.clear();
    for (auto const& [member, _] : domainScope.listAvailableMembersAndKeys(Constants::KeyNames::RenderObject::draw)) {
        drawcallOrder.push_back(member);
    }
    std::ranges::sort(drawcallOrder.begin(), drawcallOrder.end());
}

void RenderObject::updateDrawcalls() {
    for (auto const& member : drawcallOrder) {
        drawcalls[member]->update();
    }
}

//------------------------------------------
// Marshalling

std::string RenderObject::serialize() const {
    return domainScope.serialize();
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
    Global::instance().notifyEvent(update());
}

//------------------------------------------
// Position/Layer

[[nodiscard]] RenderObject::Position RenderObject::getPosition() const {
    return {
        .x=static_cast<int32_t>(std::lround(*refs.posX)),
        .y=static_cast<int32_t>(std::lround(*refs.posY))
    };
}

[[nodiscard]] uint8_t RenderObject::getLayer() const {
    return domainScope.get<uint8_t>(Constants::KeyNames::RenderObject::layer).value_or(0);
}

//------------------------------------------
// Outside communication with invoke for updating and estimation

Constants::Event RenderObject::update() {
    //------------------------------------------
    // Update modules and all inner domains
    updateModules();
    parseTaskQueues(true);
    updateDrawcalls();
    return Constants::Event::Success;
}

//------------------------------------------
// References

void RenderObject::linkFrequentRefs() {
    // Position and Size
    refs.posX = domainScope.getStableDoublePointer(Constants::KeyNames::RenderObject::positionX);
    refs.posY = domainScope.getStableDoublePointer(Constants::KeyNames::RenderObject::positionY);
}

} // namespace Nebulite::Core
