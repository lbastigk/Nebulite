//------------------------------------------
// Includes

// Standard library
#include <cstdint>
#include <functional>

// Nebulite
#include "Utility/Coordination/TimedRoutine.hpp"

//------------------------------------------
namespace Nebulite::Utility::Coordination {

TimedRoutine::TimedRoutine(std::function<void()> const& routine, std::uint64_t const intervalMillis, ConstructionMode const mode)
        : foo(routine), interval(intervalMillis) {
    if (mode == ConstructionMode::START_IMMEDIATELY) {
        timer.start();
    }
    if (mode == ConstructionMode::START_IMMEDIATELY_AND_EXECUTE_ONCE) {
        timer.start();
        foo();
    }
}

void TimedRoutine::start() {
    timer.start();
}

void TimedRoutine::update() {
    if (timer.projected_dt() >= interval) {
        timer.update();
        foo();
    }
}

void TimedRoutine::forceExecute() const {
    foo();
}

} // namespace Nebulite::Utility::Coordination
