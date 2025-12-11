/**
 * @file Camera.hpp
 * @brief This file defines the Align ruleset module, containing static rulesets related to camera alignment.
 */

#ifndef NEBULITE_INTERACTION_RULES_RULESET_MODULES_CAMERA_HPP
#define NEBULITE_INTERACTION_RULES_RULESET_MODULES_CAMERA_HPP

//------------------------------------------
// Includes

// Standard library

// External

// Nebulite
#include "Constants/KeyNames.hpp"
#include "Interaction/Rules/RulesetModule.hpp"

//------------------------------------------
// Forward declarations
namespace Nebulite::Interaction::Rules {
struct Context;
} // namespace Interaction::Rules

//------------------------------------------
namespace Nebulite::Interaction::Rules::RulesetModules {
class Camera : public RulesetModule {
public:
    //------------------------------------------
    // Functions

    // Align camera

    void alignCenter(Context const& context);
    static constexpr std::string_view alignCenterName = "::camera::align::center";

    void alignTop(Context const& context);
    static constexpr std::string_view alignTopName = "::camera::align::top";

    void alignBottom(Context const& context);
    static constexpr std::string_view alignBottomName = "::camera::align::bottom";

    void alignLeft(Context const& context);
    static constexpr std::string_view alignLeftName = "::camera::align::left";

    void alignRight(Context const& context);
    static constexpr std::string_view alignRightName = "::camera::align::right";

    //------------------------------------------
    // Constructor
    Camera() {
        // Bind Camera-related static rulesets here
        bind(RulesetType::Local, alignCenterName, &Camera::alignCenter);
        bind(RulesetType::Local, alignTopName, &Camera::alignTop);
        bind(RulesetType::Local, alignBottomName, &Camera::alignBottom);
        bind(RulesetType::Local, alignLeftName, &Camera::alignLeft);
        bind(RulesetType::Local, alignRightName, &Camera::alignRight);
    }

private:
    static constexpr std::string_view moduleName = "::camera";

    // Unique identifier for ordered cache list
    uint64_t const id = Nebulite::global().getUniqueId(std::string(moduleName), Core::GlobalSpace::UniqueIdType::expression);

    //------------------------------------------
    // Base value caching for camera alignment

    const std::vector<std::string> keys = {
        Nebulite::Constants::keyName.renderObject.positionX,
        Nebulite::Constants::keyName.renderObject.positionY,
        Nebulite::Constants::keyName.renderObject.pixelSizeX,
        Nebulite::Constants::keyName.renderObject.pixelSizeY
    };

    enum class Key : std::size_t {
        posX,
        posY,
        spriteSizeX,
        spriteSizeY
    };

    /**
     * @brief Retrieves a base value from the ordered cache list for the given key.
     * @param v The ordered cache list of base values.
     * @param k The key corresponding to the desired base value.
     * @return A reference to the base value associated with the specified key.
     */
    inline static double& baseVal(double** v, Key k) noexcept {
        return *v[static_cast<std::size_t>(k)];
    }

    /**
     * @brief Retrieves the ordered cache list of base values for the given render object context.
     *        Instead of retrieving each value individually, this function fetches all required values in a single call.
     *        This reduces lookup overhead and improves performance when accessing multiple base values.
     * @param ctx The render object context from which to retrieve the base values.
     * @return A pointer to an array of double pointers, each pointing to a base value.
     */
    double** getBaseList(Nebulite::Core::RenderObject& ctx) {
        return ensureOrderedCacheList(*ctx.getDoc(), id, keys)->data();
    }

    //------------------------------------------
    // Global values

    struct GlobalVal {
        double* camPosX = Nebulite::global().getDoc()->getStableDoublePointer(Nebulite::Constants::keyName.renderer.positionX);
        double* camPosY = Nebulite::global().getDoc()->getStableDoublePointer(Nebulite::Constants::keyName.renderer.positionY);
        double* dispResX = Nebulite::global().getDoc()->getStableDoublePointer(Nebulite::Constants::keyName.renderer.dispResX);
        double* dispResY = Nebulite::global().getDoc()->getStableDoublePointer(Nebulite::Constants::keyName.renderer.dispResY);
    } globalVal;

    //------------------------------------------
    // Position

    struct position {
        double x = 0.0;
        double y = 0.0;

        // Addition operator
        position operator+(const position& other) const {
            return position{x + other.x, y + other.y};
        };

        position& operator+=(const position& other) {
            x += other.x;
            y += other.y;
            return *this;
        };
    };

    enum class Align {
        Center,
        Top,
        Bottom,
        Left,
        Right
    };

    void setCameraPosition(const position& pos, Align align) const {
        switch (align) {
            case Align::Center:
                *globalVal.camPosX = pos.x - (*globalVal.dispResX / 2.0);
                *globalVal.camPosY = pos.y - (*globalVal.dispResY / 2.0);
                break;
            case Align::Top:
                *globalVal.camPosY = pos.y ;
                break;
            case Align::Bottom:
                *globalVal.camPosY = pos.y - (*globalVal.dispResY);
                break;
            case Align::Left:
                *globalVal.camPosX = pos.x;
                break;
            case Align::Right:
                *globalVal.camPosX = pos.x - (*globalVal.dispResX);
                break;
        }
    }

    position getAdjustedObjectPosition(double** baseValues, Align align) const {
        // Adjust based on object size
        position pos;
        switch (align) {
            case Align::Center:
                pos.x = baseVal(baseValues, Key::posX) + (baseVal(baseValues, Key::spriteSizeX) / 2.0);
                pos.y = baseVal(baseValues, Key::posY) + (baseVal(baseValues, Key::spriteSizeY) / 2.0);
                break;
            case Align::Top:
                pos.x = baseVal(baseValues, Key::posX) + (baseVal(baseValues, Key::spriteSizeX) / 2.0);
                pos.y = baseVal(baseValues, Key::posY) + baseVal(baseValues, Key::spriteSizeY);
                break;
            case Align::Bottom:
                pos.x = baseVal(baseValues, Key::posX) + (baseVal(baseValues, Key::spriteSizeX) / 2.0);
                pos.y = baseVal(baseValues, Key::posY);
                break;
            case Align::Left:
                pos.x = baseVal(baseValues, Key::posX);
                pos.y = baseVal(baseValues, Key::posY) + (baseVal(baseValues, Key::spriteSizeY) / 2.0);
                break;
            case Align::Right:
                pos.x = baseVal(baseValues, Key::posX) + baseVal(baseValues, Key::spriteSizeX);
                pos.y = baseVal(baseValues, Key::posY) + (baseVal(baseValues, Key::spriteSizeY) / 2.0);
                break;
        }
        return pos;
    }
};
} // namespace Nebulite::Interaction::Rules::RulesetModules
#endif // NEBULITE_INTERACTION_RULES_RULESET_MODULES_CAMERA_HPP