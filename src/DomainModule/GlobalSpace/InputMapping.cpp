#include <cmath>
#include <cfloat>

#include "Nebulite.hpp"
#include "DomainModule/GlobalSpace/InputMapping.hpp"
#include "DomainModule/Renderer/Input.hpp"

namespace Nebulite::DomainModule::GlobalSpace {

Constants::Error InputMapping::update() {
    if (std::fabs(*sdlPolledInput) > DBL_EPSILON) {
        processMappings();
    }
    return Constants::ErrorTable::NONE();
}

void InputMapping::reloadMappings() {
    mappings.clear();
    // TODO: implement this function to reload mappings from settings scope
}

void InputMapping::processMappings() {
    for (auto const& [action, entry] : mappings) {
        static auto const mappingLocation = moduleScope.getRootScope() + "input.";

        // Process each mapping
        int current = 0;
        for(const auto& [key, type] : {entry.slotA, entry.slotB, entry.slotC}) {
            if (key.empty())
                continue;
            switch (type) {
                case association::type::current:
                    current += moduleScope.get<int>(Renderer::Input::Key::keyboardCurrent + key);
                    break;
                case association::type::onPress:
                    current += abs(moduleScope.get<int>(Renderer::Input::Key::keyboardDelta + key)) == 1;
                    break;
                case association::type::onRelease:
                    current += moduleScope.get<int>(Renderer::Input::Key::keyboardDelta + key) == -1;
                    break;
                case association::type::empty:
                    break;
                default:
                    std::unreachable();
            }
        }

        // Now we write the state into our mapping location
        // We write the amount of actions triggered, may need to be normalized later on
        moduleScope.set<int>(mappingLocation + action, current);
    }
}

} // namespace Nebulite::DomainModule::GlobalSpace
