#ifndef NEBULITE_INTERACTION_GLOBALVALUE_HPP
#define NEBULITE_INTERACTION_GLOBALVALUE_HPP

//------------------------------------------
// Forward declarations

namespace Nebulite::Data {
    class JsonScope;
} // namespace Nebulite::Data

//------------------------------------------
namespace Nebulite::Interaction {

template<typename T>
struct GlobalValueList {
    T G; // Gravitational constant
    T dt; // Simulation delta time
    T t; // Simulation time
    /* Add more global variables here as needed */
};

using GlobalValueCopy = GlobalValueList<double>;

struct GlobalValue : GlobalValueList<double&> {
    GlobalValue(Data::JsonScope const& doc);

    [[nodiscard]] GlobalValueCopy copy() const ;
};

} // namespace Nebulite::Interaction
#endif // NEBULITE_INTERACTION_GLOBALVALUE_HPP
