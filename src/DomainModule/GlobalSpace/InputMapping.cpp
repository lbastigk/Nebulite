//------------------------------------------
// Includes

// Standard library
#include <cmath>
#include <cfloat>

// Nebulite
#include "Nebulite.hpp"
#include "DomainModule/GlobalSpace/InputMapping.hpp"
#include "DomainModule/GlobalSpace/Settings.hpp"
#include "DomainModule/Renderer/Input.hpp"

//------------------------------------------
namespace Nebulite::DomainModule::GlobalSpace {

Constants::Error InputMapping::lockOnce(std::span<std::string const> const& args) {
    if (args.size() > 2) {
        return Constants::ErrorTable::FUNCTIONAL::TOO_MANY_ARGS();
    }
    if (args.size() < 2) {
        return Constants::ErrorTable::FUNCTIONAL::TOO_FEW_ARGS();
    }

    auto const it = mappings.find(args[1]);
    if (it == mappings.end()) {
        // Find all actions that start with the given argument + "::"
        for (auto& [action, entry] : mappings) {
            if (action.starts_with(args[1] + "::")) {
                entry.lockState = mapEntry::LockState::lockOnce;
            }
        }
        return Constants::ErrorTable::NONE();
    }
    it->second.lockState = mapEntry::LockState::lockOnce;
    return Constants::ErrorTable::NONE();
}

Constants::Error InputMapping::lockOn(std::span<std::string const> const& args) {
    if (args.size() > 2) {
        return Constants::ErrorTable::FUNCTIONAL::TOO_MANY_ARGS();
    }
    if (args.size() < 2) {
        return Constants::ErrorTable::FUNCTIONAL::TOO_FEW_ARGS();
    }

    auto const it = mappings.find(args[1]);
    if (it == mappings.end()) {
        // Find all actions that start with the given argument + "::"
        for (auto& [action, entry] : mappings) {
            if (action.starts_with(args[1] + "::")) {
                entry.lockState = mapEntry::LockState::lockOn;
            }
        }
        return Constants::ErrorTable::NONE();
    }
    it->second.lockState = mapEntry::LockState::lockOn;
    return Constants::ErrorTable::NONE();
}

Constants::Error InputMapping::unlock(std::span<std::string const> const& args) {
    if (args.size() > 2) {
        return Constants::ErrorTable::FUNCTIONAL::TOO_MANY_ARGS();
    }
    if (args.size() < 2) {
        return Constants::ErrorTable::FUNCTIONAL::TOO_FEW_ARGS();
    }

    auto const it = mappings.find(args[1]);
    if (it == mappings.end()) {
        // Find all actions that start with the given argument + "::"
        for (auto& [action, entry] : mappings) {
            if (action.starts_with(args[1] + "::")) {
                entry.lockState = mapEntry::LockState::unlocked;
            }
        }
        return Constants::ErrorTable::NONE();
    }
    it->second.lockState = mapEntry::LockState::unlocked;
    return Constants::ErrorTable::NONE();
}

//------------------------------------------

Constants::Error InputMapping::update() {
    if (std::fabs(*sdlPolledInput) > DBL_EPSILON) {
        processMappings();
    }
    return Constants::ErrorTable::NONE();
}

void InputMapping::reloadMappings() {
    mappings.clear();
    for (auto const& [member, key] : settingsScope.listAvailableMembersAndKeys(Settings::Key::inputMapping)) {
        mapEntry entry;
        entry.slotA.key = settingsScope.get<std::string>(key + "." + InputMappingSlot::associationA);
        entry.slotA.type = stringToAssociationType(settingsScope.get<std::string>(key + "." + InputMappingSlot::actionA));
        entry.slotB.key = settingsScope.get<std::string>(key + "." + InputMappingSlot::associationB);
        entry.slotB.type = stringToAssociationType(settingsScope.get<std::string>(key + "." + InputMappingSlot::actionB));
        entry.slotC.key = settingsScope.get<std::string>(key + "." + InputMappingSlot::associationC);
        entry.slotC.type = stringToAssociationType(settingsScope.get<std::string>(key + "." + InputMappingSlot::actionC));
        mappings[member] = entry;
    }
}

void InputMapping::processMappings() {
    for (auto& [action, entry] : mappings) {
        static auto const mappingLocation = moduleScope.getRootScope() + "input.";

        // Process each mapping
        int triggerCount = 0;
        for(const auto& [key, type] : {entry.slotA, entry.slotB, entry.slotC}) {
            if (key.empty())
                continue;
            switch (type) {
                case association::action::current:
                    triggerCount += moduleScope.get<int>(Renderer::Input::Key::keyboardCurrent + key);
                    break;
                case association::action::onPress:
                    triggerCount += moduleScope.get<int>(Renderer::Input::Key::keyboardDelta + key) == 1;
                    break;
                case association::action::onRelease:
                    triggerCount += moduleScope.get<int>(Renderer::Input::Key::keyboardDelta + key) == -1;
                    break;
                case association::action::onChange:
                    triggerCount += abs(moduleScope.get<int>(Renderer::Input::Key::keyboardDelta + key)) == 1;
                    break;
                case association::action::empty:
                    break;
                default:
                    std::unreachable();
            }
        }

        // Process lock state and overwrite current value if necessary
        if (entry.lockState == mapEntry::LockState::lockOn) {
            triggerCount = 0;
        }
        if (entry.lockState == mapEntry::LockState::lockOnce) {
            triggerCount = 0;
            entry.lockState = mapEntry::LockState::unlocked;
        }

        // Now we write the state into our mapping location
        // We write the amount of actions triggered, may need to be normalized later on
        moduleScope.set<int>(mappingLocation + action, triggerCount);
    }
}

void InputMapping::addMappingToScope(Data::JsonScopeBase& scope, std::string const& action, std::array<std::pair<std::string, std::string>,3> const& slots) {
    scope.set<std::string>(Settings::Key::inputMapping + "." + action + "." + InputMappingSlot::associationA, slots[0].first);
    scope.set<std::string>(Settings::Key::inputMapping + "." + action + "." + InputMappingSlot::actionA, slots[0].second);
    scope.set<std::string>(Settings::Key::inputMapping + "." + action + "." + InputMappingSlot::associationB, slots[1].first);
    scope.set<std::string>(Settings::Key::inputMapping + "." + action + "." + InputMappingSlot::actionB, slots[1].second);
    scope.set<std::string>(Settings::Key::inputMapping + "." + action + "." + InputMappingSlot::associationC, slots[2].first);
    scope.set<std::string>(Settings::Key::inputMapping + "." + action + "." + InputMappingSlot::actionC, slots[2].second);
}

void InputMapping::loadDefaultMappings(Data::JsonScopeBase& scope) {
    // Combat
    addMappingToScope(scope, "combat::attack", {{{"space", "current"}, {"", "empty"}, {"", "empty"}}});

    // Movement
    addMappingToScope(scope, "movement::up", {{{"w", "onPress"}, {"up", "onPress"}, {"", "empty"}}});
    addMappingToScope(scope, "movement::down", {{{"s", "onPress"}, {"down", "onPress"}, {"", "empty"}}});
    addMappingToScope(scope, "movement::left", {{{"a", "onPress"}, {"left", "onPress"}, {"", "empty"}}});
    addMappingToScope(scope, "movement::right", {{{"d", "onPress"}, {"right", "onPress"}, {"", "empty"}}});
}

} // namespace Nebulite::DomainModule::GlobalSpace
