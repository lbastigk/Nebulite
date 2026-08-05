//------------------------------------------
// Includes

// Standard library
#include <cstdint>
#include <functional>

// Nebulite
#include "Nebulite/Utility/Coordination/TimedRoutine.hpp"

//------------------------------------------
namespace Nebulite::Utility::Coordination {

TimedRoutine::TimedRoutine(std::function<void()> const& routine, std::uint64_t const intervalMillis, ConstructionMode const mode)
        : foo(routine), interval(intervalMillis) {
    if (mode == ConstructionMode::startImmediately) {
        timer.start();
    }
    if (mode == ConstructionMode::startImmediatelyAndExecuteOnce) {
        timer.start();
        foo();
    }
}

void TimedRoutine::start() {
    timer.start();
}

void TimedRoutine::update() {
    if (timer.dtProjected() >= interval) {
        timer.update();
        foo();
    }
}

void TimedRoutine::forceExecute() const {
    foo();
}

} // namespace Nebulite::Utility::Coordination
