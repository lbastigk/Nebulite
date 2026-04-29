#include "Nebulite.hpp"
#include "Module/Domain/GlobalSpace/Physics.hpp"

namespace Nebulite::Module::Domain::GlobalSpace {

Constants::Event Physics::updateHook() {
    // No dynamic updates needed for constants
    return Constants::Event::Success;
}

void Physics::setupConstants() const {
    // Fundamental constants
    moduleScope.set(Key::Global::G, 6.67430e-11);
    moduleScope.set(Key::Global::c, 2.99792458e8);
    moduleScope.set(Key::Global::h, 6.62607015e-34);
    moduleScope.set(Key::Global::hbar, 1.054571817e-34);
    moduleScope.set(Key::Global::kB, 1.380649e-23);

    // Electromagnetic constants
    moduleScope.set(Key::Global::e, 1.602176634e-19);
    moduleScope.set(Key::Global::epsilon0, 8.8541878128e-12);
    moduleScope.set(Key::Global::mu0, 1.25663706212e-6);
    moduleScope.set(Key::Global::ke, 8.9875517923e9);

    // Particle masses
    moduleScope.set(Key::Global::me, 9.1093837015e-31);
    moduleScope.set(Key::Global::mp, 1.67262192369e-27);
    moduleScope.set(Key::Global::mn, 1.67492749804e-27);

    // Thermodynamics / chemistry
    moduleScope.set(Key::Global::NA, 6.02214076e23);
    moduleScope.set(Key::Global::R, 8.314462618);

    // Earth-related (useful approximations)
    moduleScope.set(Key::Global::g, 9.80665);
}

} // namespace Nebulite::Module::Domain::GlobalSpace
