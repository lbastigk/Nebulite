#include "DomainModule/GlobalSpace/InputMapping.hpp"

namespace Nebulite::DomainModule::GlobalSpace {

Constants::Error InputMapping::update() {
    // TODO: Implement input mapping update logic
    if (inputMapperTimer == nullptr) {
        auto routine = [] {
            // TODO: Implement the logic to update input mappings based on the current state of inputs
            //       and turn this into a local lambda function so we don't have to define it here...
        };

        inputMapperTimer = std::make_unique<Utility::TimedRoutine>(routine, 100); // Update every 100 ms (example value)
    }
    inputMapperTimer->update();
    return Constants::ErrorTable::NONE();
}

} // namespace Nebulite::DomainModule::GlobalSpace
