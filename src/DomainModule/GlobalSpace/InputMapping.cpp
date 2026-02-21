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
        return Constants::ErrorTable::FUNCTIONAL::UNKNOWN_ARG();
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
        return Constants::ErrorTable::FUNCTIONAL::UNKNOWN_ARG();
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
        return Constants::ErrorTable::FUNCTIONAL::UNKNOWN_ARG();
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
                    triggerCount += abs(moduleScope.get<int>(Renderer::Input::Key::keyboardDelta + key)) == 1;
                    break;
                case association::action::onRelease:
                    triggerCount += moduleScope.get<int>(Renderer::Input::Key::keyboardDelta + key) == -1;
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

void InputMapping::loadDefaultMappings(Data::JsonScopeBase& scope) {
    // Jump
    scope.set<std::string>(Settings::Key::inputMapping + ".jump." + InputMappingSlot::associationA, "space");
    scope.set<std::string>(Settings::Key::inputMapping + ".jump." + InputMappingSlot::actionA, "current");
    scope.set<std::string>(Settings::Key::inputMapping + ".jump." + InputMappingSlot::associationB, "");
    scope.set<std::string>(Settings::Key::inputMapping + ".jump." + InputMappingSlot::actionB, "empty");
    scope.set<std::string>(Settings::Key::inputMapping + ".jump." + InputMappingSlot::associationC, "");
    scope.set<std::string>(Settings::Key::inputMapping + ".jump." + InputMappingSlot::actionC, "empty");

    // Movement: up
    scope.set<std::string>(Settings::Key::inputMapping + ".up." + InputMappingSlot::associationA, "w");
    scope.set<std::string>(Settings::Key::inputMapping + ".up." + InputMappingSlot::actionA, "current");
    scope.set<std::string>(Settings::Key::inputMapping + ".up." + InputMappingSlot::associationB, "up");
    scope.set<std::string>(Settings::Key::inputMapping + ".up." + InputMappingSlot::actionB, "current");
    scope.set<std::string>(Settings::Key::inputMapping + ".up." + InputMappingSlot::associationC, "");
    scope.set<std::string>(Settings::Key::inputMapping + ".up." + InputMappingSlot::actionC, "empty");

    // Movement: down
    scope.set<std::string>(Settings::Key::inputMapping + ".down." + InputMappingSlot::associationA, "s");
    scope.set<std::string>(Settings::Key::inputMapping + ".down." + InputMappingSlot::actionA, "current");
    scope.set<std::string>(Settings::Key::inputMapping + ".down." + InputMappingSlot::associationB, "down");
    scope.set<std::string>(Settings::Key::inputMapping + ".down." + InputMappingSlot::actionB, "current");
    scope.set<std::string>(Settings::Key::inputMapping + ".down." + InputMappingSlot::associationC, "");
    scope.set<std::string>(Settings::Key::inputMapping + ".down." + InputMappingSlot::actionC, "empty");

    // Movement: left
    scope.set<std::string>(Settings::Key::inputMapping + ".left." + InputMappingSlot::associationA, "a");
    scope.set<std::string>(Settings::Key::inputMapping + ".left." + InputMappingSlot::actionA, "current");
    scope.set<std::string>(Settings::Key::inputMapping + ".left." + InputMappingSlot::associationB, "left");
    scope.set<std::string>(Settings::Key::inputMapping + ".left." + InputMappingSlot::actionB, "current");
    scope.set<std::string>(Settings::Key::inputMapping + ".left." + InputMappingSlot::associationC, "");
    scope.set<std::string>(Settings::Key::inputMapping + ".left." + InputMappingSlot::actionC, "empty");

    // Movement: right
    scope.set<std::string>(Settings::Key::inputMapping + ".right." + InputMappingSlot::associationA, "d");
    scope.set<std::string>(Settings::Key::inputMapping + ".right." + InputMappingSlot::actionA, "current");
    scope.set<std::string>(Settings::Key::inputMapping + ".right." + InputMappingSlot::associationB, "right");
    scope.set<std::string>(Settings::Key::inputMapping + ".right." + InputMappingSlot::actionB, "current");
    scope.set<std::string>(Settings::Key::inputMapping + ".right." + InputMappingSlot::associationC, "");
    scope.set<std::string>(Settings::Key::inputMapping + ".right." + InputMappingSlot::actionC, "empty");
}

} // namespace Nebulite::DomainModule::GlobalSpace
