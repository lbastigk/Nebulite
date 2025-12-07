#include "Interaction/Rules/RulesetModules/Physics.hpp"
#include "Interaction/Rules/StaticRulesets.hpp"

namespace Nebulite::Interaction::Rules::RulesetModules {

void Physics::gravity(Context const& context) {
    // Static variables
    static uint64_t id = Nebulite::global().getUniqueId("::gravity", Core::GlobalSpace::UniqueIdType::expression);
    static std::vector<std::string> keys = {
        "physics.aX",
        "physics.aY",
        "physics.mass",
        "posX",
        "posY"
    };
    enum key {
        physics_aX,
        physics_aY,
        physics_mass,
        posX,
        posY
    };
    static double* G = Nebulite::global().getDoc()->getStableDoublePointer("physics.G");
    static double invMaxAccel = 1.0 / 1e4; // To prevent extreme accelerations

    // Get ordered cache lists for both entities
    auto slf = ensureOrderedCacheList(*context.self.getDoc(), id, keys)->data();
    auto otr = ensureOrderedCacheList(*context.other.getDoc(), id, keys)->data();

    // Calculate distance components
    double const distanceX = *slf[posX] - *otr[posX];
    double const distanceY = *slf[posY] - *otr[posY];

    // Avoid division by zero by adding a small epsilon
    double const denominator = std::pow((distanceX * distanceX + distanceY * distanceY),1.5) + invMaxAccel;

    // Calculate aX and aY of other
    *otr[physics_aX] += *G * *slf[physics_mass] * distanceX / denominator;
    *otr[physics_aY] += *G * *slf[physics_mass] * distanceY / denominator;
}

}