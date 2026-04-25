/**
 * @file Camera.hpp
 * @brief This file defines the Align ruleset module, containing static rulesets related to camera alignment.
 */

#ifndef NEBULITE_RULESET_MODULE_CAMERA_HPP
#define NEBULITE_RULESET_MODULE_CAMERA_HPP

//------------------------------------------
// Includes

// Standard library

// External

// Nebulite
#include "Constants/KeyNames.hpp"
#include "Interaction/Rules/RulesetModule.hpp"

//------------------------------------------
namespace Nebulite::Module::Ruleset {
class Camera : public Interaction::Rules::RulesetModule {
public:
    //------------------------------------------
    // Functions

    // Align camera

    void alignCenter(Interaction::Context const& context, double**& slf, double**& otr) const ;
    static std::string_view constexpr alignCenterName = "::camera::align::center";
    static std::string_view constexpr alignCenterDesc = "Aligns the camera view center to the center of the render object.";

    void alignTop(Interaction::Context const& context, double**& slf, double**& otr) const ;
    static std::string_view constexpr alignTopName = "::camera::align::top";
    static std::string_view constexpr alignTopDesc = "Aligns the camera view top edge to the center of the render object.";

    void alignBottom(Interaction::Context const& context, double**& slf, double**& otr) const ;
    static std::string_view constexpr alignBottomName = "::camera::align::bottom";
    static std::string_view constexpr alignBottomDesc = "Aligns the camera view bottom edge to the center of the render object.";

    void alignLeft(Interaction::Context const& context, double**& slf, double**& otr) const ;
    static std::string_view constexpr alignLeftName = "::camera::align::left";
    static std::string_view constexpr alignLeftDesc = "Aligns the camera view left edge to the center of the render object.";

    void alignRight(Interaction::Context const& context, double**& slf, double**& otr) const ;
    static std::string_view constexpr alignRightName = "::camera::align::right";
    static std::string_view constexpr alignRightDesc = "Aligns the camera view right edge to the center of the render object.";

    //------------------------------------------
    // Constructor
    Camera();

    static std::string_view constexpr moduleName = "::camera";
private:
    //------------------------------------------
    // Base value caching for camera alignment

    const std::vector<Data::ScopedKeyView> baseKeys = {
        Constants::KeyNames::RenderObject::positionX,
        Constants::KeyNames::RenderObject::positionY,
        Constants::KeyNames::RenderObject::sizeX,
        Constants::KeyNames::RenderObject::sizeX
    };

    enum class Key : std::size_t {
        posX,
        posY,
        spriteSizeX,
        spriteSizeY
    };

    //------------------------------------------
    // Global values

    struct GlobalVal {
        double* camPosX;
        double* camPosY;
        double* dispResX;
        double* dispResY;
    } globalVal = {};

    //------------------------------------------
    // Position

    struct position {
        double x = 0.0;
        double y = 0.0;
    };

    enum class Align {
        Center,
        Top,
        Bottom,
        Left,
        Right
    };

    // TODO: Second align parameter for object edge alignment
    //       Each camera align needs to know what part of the object to align to what part of the camera view
    //       e.g.: ::camera::align::right-top would align the camera's right edge to the object's top edge
    void setCameraPosition(const position& pos, Align const& align) const {
        switch (align) {
            case Align::Top:
                *globalVal.camPosY = pos.y ;
                break;
            case Align::Bottom:
                *globalVal.camPosY = pos.y - *globalVal.dispResY;
                break;
            case Align::Left:
                *globalVal.camPosX = pos.x;
                break;
            case Align::Right:
                *globalVal.camPosX = pos.x - *globalVal.dispResX;
                break;
            case Align::Center:
            default: // Fallback to center
                *globalVal.camPosX = pos.x - *globalVal.dispResX / 2.0;
                *globalVal.camPosY = pos.y - *globalVal.dispResY / 2.0;
                break;
        }
    }

    static position getAdjustedObjectPosition(double** baseValues, Align const& align) {
        // Adjust based on object size
        position pos;
        switch (align) {
            case Align::Top:
                pos.x = baseVal(baseValues, Key::posX) + baseVal(baseValues, Key::spriteSizeX) / 2.0;
                pos.y = baseVal(baseValues, Key::posY) + baseVal(baseValues, Key::spriteSizeY);
                break;
            case Align::Bottom:
                pos.x = baseVal(baseValues, Key::posX) + baseVal(baseValues, Key::spriteSizeX) / 2.0;
                pos.y = baseVal(baseValues, Key::posY);
                break;
            case Align::Left:
                pos.x = baseVal(baseValues, Key::posX);
                pos.y = baseVal(baseValues, Key::posY) + baseVal(baseValues, Key::spriteSizeY) / 2.0;
                break;
            case Align::Right:
                pos.x = baseVal(baseValues, Key::posX) + baseVal(baseValues, Key::spriteSizeX);
                pos.y = baseVal(baseValues, Key::posY) + baseVal(baseValues, Key::spriteSizeY) / 2.0;
                break;
            case Align::Center:
            default: // Fallback to center
                pos.x = baseVal(baseValues, Key::posX) + baseVal(baseValues, Key::spriteSizeX) / 2.0;
                pos.y = baseVal(baseValues, Key::posY) + baseVal(baseValues, Key::spriteSizeY) / 2.0;
                break;
        }
        return pos;
    }
};
} // namespace Nebulite::Module::Ruleset
#endif // NEBULITE_RULESET_MODULE_CAMERA_HPP
