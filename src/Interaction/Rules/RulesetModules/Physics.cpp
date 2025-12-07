#include "Interaction/Rules/RulesetModules/Physics.hpp"
#include "Interaction/Rules/StaticRulesets.hpp"

namespace Nebulite::Interaction::Rules::RulesetModules {

void Physics::gravity(Context const& context) {
    //----------------------------------------------
    // Workspace Setup

    // Unique identifier for caching
    static const uint64_t id = Nebulite::global().getUniqueId(gravityName, Core::GlobalSpace::UniqueIdType::expression);

    // Variable context Keys being used for both entities
    static const std::vector<std::string> keys = {
        "physics.aX",
        "physics.aY",
        "physics.mass",
        "posX",
        "posY"
    };
    enum class Key : std::size_t {
        physics_aX,
        physics_aY,
        physics_mass,
        posX,
        posY
    };
    auto val = [](double** v, Key k) noexcept -> double& {
        return *v[static_cast<std::size_t>(k)];
    };

    // Variables from globalspace and constants
    static const double* G = Nebulite::global().getDoc()->getStableDoublePointer("physics.G");
    static double invMaxAccel = 1.0 / 1e0; // To prevent extreme accelerations

    //----------------------------------------------
    // Main Logic

    // Get ordered cache lists for both entities
    // No locks needed, as ensureOrderedCacheList handles that internally
    double** slf = ensureOrderedCacheList(*context.self.getDoc(), id, keys)->data();
    double** otr = ensureOrderedCacheList(*context.other.getDoc(), id, keys)->data();

    // Calculate distance components
    double const distanceX = val(slf,Key::posX) - val(otr,Key::posX);
    double const distanceY = val(slf,Key::posY) - val(otr,Key::posY);

    // Avoid division by zero by adding a small epsilon
    double const denominator = std::pow((distanceX * distanceX + distanceY * distanceY),1.5) + invMaxAccel;

    // Calculate aX and aY of other
    auto otrLock = context.other.getDoc()->lock();
    val(otr, Key::physics_aX) += *G * val(slf, Key::physics_mass) * distanceX / denominator;
    val(otr, Key::physics_aY) += *G * val(slf, Key::physics_mass) * distanceY / denominator;
}

std::string Physics::gravityName = "::physics::gravity";

}