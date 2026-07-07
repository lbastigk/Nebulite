//------------------------------------------
// Includes

// Standard library
#include <array>
#include <cmath>
#include <span>
#include <string>
#include <utility>

// Nebulite
#include "Nebulite/Constants/Event.hpp"
#include "Nebulite/Constants/StandardCapture.hpp"
#include "Nebulite/Core/GlobalSpace.hpp"
#include "Nebulite/Math/Equality.hpp"
#include "Nebulite/Module/Base/DomainModule.hpp"
#include "Nebulite/Module/Domain/GlobalSpace/InputMapping.hpp"
#include "Nebulite/Module/Domain/GlobalSpace/Settings.hpp"
#include "Nebulite/Module/Domain/Renderer/Input.hpp"
#include "Nebulite/Utility/StringHandler.hpp"

//------------------------------------------
namespace Nebulite::Module::Domain::GlobalSpace {

InputMapping::InputMapping(ConstructorParams const& params)
    : DomainModule(params)
    , sdlPolledInput(moduleScope.getStableDoublePointer(Renderer::Input::Key::routineActivated)){ // Setup pointer to polled input key in Renderer::Input module, to sync our updates with it and avoid missing deltas
    // Load initial mappings from settings
    reloadMappings();

    // Bind functions
    bindCategory(inputMappingName, inputMappingDesc);
    bindCategory(inputMappingLockName, inputMappingLockDesc);
    bindFunction(&InputMapping::lockOnce, lockOnceName, lockOnceDesc);
    bindFunction(&InputMapping::lockOn, lockOnName, lockOnDesc);
    bindFunction(&InputMapping::unlock, unlockName, unlockDesc);
}

Constants::Event InputMapping::lockOnce(std::span<std::string_view const> const& args) {
    if (args.size() > 2) {
        return Constants::StandardCapture::Warning::Functional::tooManyArgs(domain.capture);
    }
    if (args.size() < 2) {
        return Constants::StandardCapture::Warning::Functional::tooFewArgs(domain.capture);
    }

    auto const it = mappings.find(args[1]);
    if (it == mappings.end()) {
        // Find all actions that start with the given argument + "::"
        for (auto& [action, entry] : mappings) {
            if (Utility::StringHandler::startsWithSequence(action, args[1], "::")) {
                entry.lockState = MapEntry::LockState::lockOnce;
            }
        }
        return Constants::Event::Success;
    }
    it->second.lockState = MapEntry::LockState::lockOnce;
    return Constants::Event::Success;
}

Constants::Event InputMapping::lockOn(std::span<std::string_view const> const& args) {
    if (args.size() > 2) {
        return Constants::StandardCapture::Warning::Functional::tooManyArgs(domain.capture);
    }
    if (args.size() < 2) {
        return Constants::StandardCapture::Warning::Functional::tooFewArgs(domain.capture);
    }

    auto const it = mappings.find(args[1]);
    if (it == mappings.end()) {
        // Find all actions that start with the given argument + "::"
        for (auto& [action, entry] : mappings) {
            if (Utility::StringHandler::startsWithSequence(action, args[1], "::")) {
                entry.lockState = MapEntry::LockState::lockOn;
            }
        }
        return Constants::Event::Success;
    }
    it->second.lockState = MapEntry::LockState::lockOn;
    return Constants::Event::Success;
}

Constants::Event InputMapping::unlock(std::span<std::string_view const> const& args) {
    if (args.size() > 2) {
        return Constants::StandardCapture::Warning::Functional::tooManyArgs(domain.capture);
    }
    if (args.size() < 2) {
        return Constants::StandardCapture::Warning::Functional::tooFewArgs(domain.capture);
    }

    auto const it = mappings.find(args[1]);
    if (it == mappings.end()) {
        // Find all actions that start with the given argument + "::"
        for (auto& [action, entry] : mappings) {
            if (Utility::StringHandler::startsWithSequence(action, args[1], "::")) {
                entry.lockState = MapEntry::LockState::unlocked;
            }
        }
        return Constants::Event::Success;
    }
    it->second.lockState = MapEntry::LockState::unlocked;
    return Constants::Event::Success;
}

//------------------------------------------

Constants::Event InputMapping::updateHook() {
    if (sdlPolledInput && !Math::isZero(*sdlPolledInput)) {
        processMappings();
    }
    return Constants::Event::Success;
}

void InputMapping::reloadMappings() {
    mappings.clear();
    for (auto const& [member, key] : settingsScope.listAvailableMembersAndKeys(Settings::Key::inputMapping)) {
        MapEntry entry;
        entry.slotA.key = settingsScope.get<std::string>(key.addMember(InputMappingSlot::associationA)).value_or("");
        entry.slotA.type = stringToAssociationType(settingsScope.get<std::string>(key.addMember(InputMappingSlot::actionA)).value_or(""));
        entry.slotB.key = settingsScope.get<std::string>(key.addMember(InputMappingSlot::associationB)).value_or("");
        entry.slotB.type = stringToAssociationType(settingsScope.get<std::string>(key.addMember(InputMappingSlot::actionB)).value_or(""));
        entry.slotC.key = settingsScope.get<std::string>(key.addMember(InputMappingSlot::associationC)).value_or("");
        entry.slotC.type = stringToAssociationType(settingsScope.get<std::string>(key.addMember(InputMappingSlot::actionC)).value_or(""));
        mappings[member] = entry;
    }
}

void InputMapping::processMappings() {
    for (auto& [action, entry] : mappings) {
        static auto const mappingLocation = moduleScope.getRootScope().addMember("input");

        // Process each mapping
        int triggerCount = 0;
        for(const auto& [key, type] : {entry.slotA, entry.slotB, entry.slotC}) {
            if (key.empty())
                continue;
            switch (type) {
                case association::Action::current:
                    triggerCount += moduleScope.get<int>(Renderer::Input::Key::keyboardCurrent.addMember(key)).value_or(0);
                    break;
                case association::Action::onPress:
                    triggerCount += moduleScope.get<int>(Renderer::Input::Key::keyboardDelta.addMember(key)) == 1;
                    break;
                case association::Action::onRelease:
                    triggerCount += moduleScope.get<int>(Renderer::Input::Key::keyboardDelta.addMember(key)) == -1;
                    break;
                case association::Action::onChange:
                    triggerCount += abs(moduleScope.get<int>(Renderer::Input::Key::keyboardDelta.addMember(key)).value_or(0)) == 1;
                    break;
                case association::Action::empty:
                    break;
                default:
                    std::unreachable();
            }
        }

        // Process lock state and overwrite current value if necessary
        if (entry.lockState == MapEntry::LockState::lockOn) {
            triggerCount = 0;
        }
        if (entry.lockState == MapEntry::LockState::lockOnce) {
            triggerCount = 0;
            entry.lockState = MapEntry::LockState::unlocked;
        }

        // Now we write the state into our mapping location
        // We write the amount of actions triggered, may need to be normalized later on
        moduleScope.set<int>(mappingLocation.addMember(action), triggerCount);
    }
}

void InputMapping::addMappingToScope(Data::JsonScope& scope, std::string const& action, std::array<std::pair<std::string, std::string>,3> const& slots) {
    scope.set<std::string>(Settings::Key::inputMapping.addMember(action).addMember(InputMappingSlot::associationA), slots[0].first);
    scope.set<std::string>(Settings::Key::inputMapping.addMember(action).addMember(InputMappingSlot::actionA), slots[0].second);
    scope.set<std::string>(Settings::Key::inputMapping.addMember(action).addMember(InputMappingSlot::associationB), slots[1].first);
    scope.set<std::string>(Settings::Key::inputMapping.addMember(action).addMember(InputMappingSlot::actionB), slots[1].second);
    scope.set<std::string>(Settings::Key::inputMapping.addMember(action).addMember(InputMappingSlot::associationC), slots[2].first);
    scope.set<std::string>(Settings::Key::inputMapping.addMember(action).addMember(InputMappingSlot::actionC), slots[2].second);
}

void InputMapping::loadDefaultMappings(Data::JsonScope& scope) {
    // Combat
    addMappingToScope(scope, "combat::attack", {{{"space", "current"}, {"", "empty"}, {"", "empty"}}});

    // Movement
    addMappingToScope(scope, "movement::up", {{{"w", "current"}, {"up", "current"}, {"", "empty"}}});
    addMappingToScope(scope, "movement::down", {{{"s", "current"}, {"down", "current"}, {"", "empty"}}});
    addMappingToScope(scope, "movement::left", {{{"a", "current"}, {"left", "current"}, {"", "empty"}}});
    addMappingToScope(scope, "movement::right", {{{"d", "current"}, {"right", "current"}, {"", "empty"}}});
}

} // namespace Nebulite::Module::Domain::GlobalSpace
