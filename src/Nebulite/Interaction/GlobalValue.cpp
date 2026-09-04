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
    .gravitationalConstant = *doc.getStableDoublePointer(Module::Domain::GlobalSpace::Physics::Key::Global::G),
    .dt = *doc.getStableDoublePointer(Module::Domain::GlobalSpace::Time::Key::deltaTime),
    .t = *doc.getStableDoublePointer(Module::Domain::GlobalSpace::Time::Key::time),
} {}

GlobalValueCopy GlobalValue::copy() const {
    return GlobalValueCopy{
        .gravitationalConstant = gravitationalConstant,
        .dt = dt,
        .t = t,
    };
}

} // namespace Nebulite::Interaction
