#include "Nebulite.hpp"
#include "DomainModule/GlobalSpace/InputMapping.hpp"
#include "DomainModule/Renderer/Input.hpp"

namespace Nebulite::DomainModule::GlobalSpace {

Constants::Error InputMapping::update() {
    if (static_cast<bool>(*sdlPolledInput)) {
        processMappings();
    }
    return Constants::ErrorTable::NONE();
}

void InputMapping::processMappings() {
    for (auto const& [action, entry] : mappings) {
        // Process each mapping
        int current = 0;

        static auto const baseLocation = moduleScope.getRootScope() + "renderer.input.keyboard.";
        static auto const mappingLocation = moduleScope.getRootScope() + "input.";

        for(const auto& [key, type] : {entry.slotA, entry.slotB, entry.slotC}) {
            if (key.empty())
                continue;
            switch (type) {
            case association::type::current:
                    current +=     moduleScope.get<int>(baseLocation + "current." + key);
                    break;
                case association::type::onPress:
                    current += abs(moduleScope.get<int>(baseLocation + "delta." + key)) == 1;
                    break;
                case association::type::onRelease:
                    current +=     moduleScope.get<int>(baseLocation + "delta." + key) == -1;
                    break;
                case association::type::empty:
                    break;
                default:
                    std::unreachable();
            }
        }

        // Now we write the state into our mapping location
        moduleScope.set<int>(mappingLocation + action, current);
    }
}

} // namespace Nebulite::DomainModule::GlobalSpace
