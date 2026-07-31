//------------------------------------------
// Includes

// Standard library
#include <utility>

// Nebulite
#include "Nebulite/Constants/Event.hpp"
#include "Nebulite/Data/TaskQueueResult.hpp"

//------------------------------------------
namespace Nebulite::Data {

Constants::Event TaskQueueResult::worstEvent() const {
    auto worst = Constants::Event::success;
    for (auto const& event : events) {
        switch (event) {
        case Constants::Event::success:
            break;
        case Constants::Event::warning:
            if (worst == Constants::Event::success) {
                worst = Constants::Event::warning;
            }
            break;
        case Constants::Event::error:
            worst = Constants::Event::error;
            break;
        default:
            std::unreachable();
        }
    }
    return worst;
}

} // namespace Nebulite::Data
