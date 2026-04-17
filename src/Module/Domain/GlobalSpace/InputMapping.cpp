//------------------------------------------
// Includes

// Standard library
#include <cmath>
#include <cfloat>

// Nebulite
#include "Nebulite.hpp"
#include "Module/Domain/GlobalSpace/InputMapping.hpp"
#include "Module/Domain/GlobalSpace/Settings.hpp"
#include "Module/Domain/Renderer/Input.hpp"
#include "Math/Equality.hpp"

//------------------------------------------
namespace Nebulite::DomainModule::GlobalSpace {

Constants::Event InputMapping::lockOnce(std::span<std::string const> const& args) {
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
            if (action.starts_with(args[1] + "::")) {
                entry.lockState = mapEntry::LockState::lockOnce;
            }
        }
        return Constants::Event::Success;
    }
    it->second.lockState = mapEntry::LockState::lockOnce;
    return Constants::Event::Success;
}

Constants::Event InputMapping::lockOn(std::span<std::string const> const& args) {
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
            if (action.starts_with(args[1] + "::")) {
                entry.lockState = mapEntry::LockState::lockOn;
            }
        }
        return Constants::Event::Success;
    }
    it->second.lockState = mapEntry::LockState::lockOn;
    return Constants::Event::Success;
}

Constants::Event InputMapping::unlock(std::span<std::string const> const& args) {
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
            if (action.starts_with(args[1] + "::")) {
                entry.lockState = mapEntry::LockState::unlocked;
            }
        }
        return Constants::Event::Success;
    }
    it->second.lockState = mapEntry::LockState::unlocked;
    return Constants::Event::Success;
}

//------------------------------------------

Constants::Event InputMapping::updateHook() {
    if (!Math::isZero(*sdlPolledInput)) {
        processMappings();
    }
    return Constants::Event::Success;
}

void InputMapping::reloadMappings() {
    mappings.clear();
    for (auto const& [member, key] : settingsScope.listAvailableMembersAndKeys(Settings::Key::inputMapping)) {
        mapEntry entry;
        entry.slotA.key = settingsScope.get<std::string>(key + "." + InputMappingSlot::associationA).value_or("");
        entry.slotA.type = stringToAssociationType(settingsScope.get<std::string>(key + "." + InputMappingSlot::actionA).value_or(""));
        entry.slotB.key = settingsScope.get<std::string>(key + "." + InputMappingSlot::associationB).value_or("");
        entry.slotB.type = stringToAssociationType(settingsScope.get<std::string>(key + "." + InputMappingSlot::actionB).value_or(""));
        entry.slotC.key = settingsScope.get<std::string>(key + "." + InputMappingSlot::associationC).value_or("");
        entry.slotC.type = stringToAssociationType(settingsScope.get<std::string>(key + "." + InputMappingSlot::actionC).value_or(""));
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
                    triggerCount += moduleScope.get<int>(Renderer::Input::Key::keyboardCurrent + key).value_or(0);
                    break;
                case association::action::onPress:
                    triggerCount += moduleScope.get<int>(Renderer::Input::Key::keyboardDelta + key) == 1;
                    break;
                case association::action::onRelease:
                    triggerCount += moduleScope.get<int>(Renderer::Input::Key::keyboardDelta + key) == -1;
                    break;
                case association::action::onChange:
                    triggerCount += abs(moduleScope.get<int>(Renderer::Input::Key::keyboardDelta + key).value_or(0)) == 1;
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

void InputMapping::addMappingToScope(Data::JsonScope& scope, std::string const& action, std::array<std::pair<std::string, std::string>,3> const& slots) {
    scope.set<std::string>(Settings::Key::inputMapping + "." + action + "." + InputMappingSlot::associationA, slots[0].first);
    scope.set<std::string>(Settings::Key::inputMapping + "." + action + "." + InputMappingSlot::actionA, slots[0].second);
    scope.set<std::string>(Settings::Key::inputMapping + "." + action + "." + InputMappingSlot::associationB, slots[1].first);
    scope.set<std::string>(Settings::Key::inputMapping + "." + action + "." + InputMappingSlot::actionB, slots[1].second);
    scope.set<std::string>(Settings::Key::inputMapping + "." + action + "." + InputMappingSlot::associationC, slots[2].first);
    scope.set<std::string>(Settings::Key::inputMapping + "." + action + "." + InputMappingSlot::actionC, slots[2].second);
}

void InputMapping::loadDefaultMappings(Data::JsonScope& scope) {
    // Combat
    addMappingToScope(scope, "combat::attack", {{{"space", "current"}, {"", "empty"}, {"", "empty"}}});

    // Movement
    addMappingToScope(scope, "movement::up", {{{"w", "onPress"}, {"up", "onPress"}, {"", "empty"}}});
    addMappingToScope(scope, "movement::down", {{{"s", "onPress"}, {"down", "onPress"}, {"", "empty"}}});
    addMappingToScope(scope, "movement::left", {{{"a", "onPress"}, {"left", "onPress"}, {"", "empty"}}});
    addMappingToScope(scope, "movement::right", {{{"d", "onPress"}, {"right", "onPress"}, {"", "empty"}}});
}

} // namespace Nebulite::DomainModule::GlobalSpace
