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

    // Get ordered cache lists for both entities
    auto self = ensureOrderedCacheList(*context.self.getDoc(), id, keys)->data();
    auto othr = ensureOrderedCacheList(*context.other.getDoc(), id, keys)->data();

    // Calculate distance components
    double distanceX = *self[posX] - *othr[posX];
    double distanceY = *self[posY] - *othr[posY];
    double denominator = std::pow((distanceX * distanceX + distanceY * distanceY),1.5);

    // Avoid division by zero or NaN
    // denominator is always positive due to squaring
    if (denominator < std::numeric_limits<double>::epsilon() || std::isnan(denominator))
        return;

    // Calculate aX and aY of other
    *othr[physics_aX] += *G * *self[physics_mass] * distanceX / (denominator);
    *othr[physics_aY] -= *G * *self[physics_mass] * distanceY / (denominator);
}

}