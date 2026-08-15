//------------------------------------------
// Includes

// Nebulite
#include "Nebulite/Data/Document/JsonScope.hpp"
#include "Nebulite/Interaction/GlobalValue.hpp"
#include "Nebulite/Module/Domain/GlobalSpace/Physics.hpp"
#include "Nebulite/Module/Domain/GlobalSpace/Time.hpp"

//------------------------------------------
namespace Nebulite::Interaction {

GlobalValue::GlobalValue(Data::JsonScope const& doc) : GlobalValueList{
    .G = *doc.getStableDoublePointer(Module::Domain::GlobalSpace::Physics::Key::Global::G), // Gravitational constant
    .dt = *doc.getStableDoublePointer(Module::Domain::GlobalSpace::Time::Key::deltaTime), // Simulation delta time
    .t = *doc.getStableDoublePointer(Module::Domain::GlobalSpace::Time::Key::time), // Simulation time
} {}

GlobalValueCopy GlobalValue::copy() const {
    return GlobalValueCopy{
        .G = G,
        .dt = dt,
        .t = t
    };
}

} // namespace Nebulite::Interaction
