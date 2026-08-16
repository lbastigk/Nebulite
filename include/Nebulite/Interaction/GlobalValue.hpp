#ifndef NEBULITE_INTERACTION_GLOBALVALUE_HPP
#define NEBULITE_INTERACTION_GLOBALVALUE_HPP

//------------------------------------------
// Forward declarations

namespace Nebulite::Data {
class JsonScope;
} // namespace Nebulite::Data

//------------------------------------------
namespace Nebulite::Interaction {

/**
 * @brief Struct to hold basic global values used in various rulesets.
 * @details Only use values that stay constant in one frame, as they are not synchronized during the frame,
 *          but copied at the beginning of the frame.
 *          If you need to use a value that changes during the frame,
 *          consider using a different mechanism to pass it to the rulesets.
 * @tparam T The type of the global values. Can be a reference type to allow for direct access to the original values.
 */
template<typename T>
struct GlobalValueList {
    T gravitationalConstant; // [G] = m^3gk^-1s^-2
    T dt; // Simulation delta time. [dt] = s
    T t; // Simulation time. [t] = s
    /* Add more global variables here as needed */
};

using GlobalValueCopy = GlobalValueList<double>;

struct GlobalValue : GlobalValueList<double&> {
    explicit GlobalValue(Data::JsonScope const& doc);

    [[nodiscard]] GlobalValueCopy copy() const ;
};

} // namespace Nebulite::Interaction
#endif // NEBULITE_INTERACTION_GLOBALVALUE_HPP
