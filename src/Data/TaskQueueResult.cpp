//------------------------------------------
// Includes

// Standard library
#include <utility>

// Nebulite
#include "Nebulite/Constants/Event.hpp"
#include "Nebulite/Data/TaskQueueResult.hpp"

//------------------------------------------
namespace Nebulite::Data {

Constants::Event TaskQueueResult::worstEvent() {
    Constants::Event worst = Constants::Event::Success;
    for (auto const& event : events) {
        switch (event) {
        case Constants::Event::Success:
            break;
        case Constants::Event::Warning:
            if (worst == Constants::Event::Success) {
                worst = Constants::Event::Warning;
            }
            break;
        case Constants::Event::Error:
            worst = Constants::Event::Error;
            break;
        default:
            std::unreachable();
        }
    }
    return worst;
}

} // namespace Nebulite::Data
