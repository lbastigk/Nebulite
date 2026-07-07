#ifndef NEBULITE_MODULE_RULESET_CAMERA_HPP
#define NEBULITE_MODULE_RULESET_CAMERA_HPP

//------------------------------------------
// Includes

// Standard library
#include <cstdint>
#include <string_view>
#include <vector>

// Nebulite
#include "Nebulite/Constants/KeyNames.hpp"
#include "Nebulite/Data/Document/ScopedKey.hpp"
#include "Nebulite/Module/Base/RulesetModule.hpp"

//------------------------------------------
// Forward declarations

namespace Nebulite::Interaction {
class Context;
} // namespace Nebulite::Interaction

//------------------------------------------
namespace Nebulite::Module::Ruleset {
class Camera : public Base::RulesetModule {
public:
    //------------------------------------------
    // Functions

    // Align camera

    void alignCenter(Interaction::Context const& context, double** slf, double** otr) const ;
    static std::string_view constexpr alignCenterName = "::camera::align::center";
    static std::string_view constexpr alignCenterDesc = "Aligns the camera view center to the center of the render object.";

    void alignTop(Interaction::Context const& context, double** slf, double** otr) const ;
    static std::string_view constexpr alignTopName = "::camera::align::top";
    static std::string_view constexpr alignTopDesc = "Aligns the camera view top edge to the center of the render object.";

    void alignBottom(Interaction::Context const& context, double** slf, double** otr) const ;
    static std::string_view constexpr alignBottomName = "::camera::align::bottom";
    static std::string_view constexpr alignBottomDesc = "Aligns the camera view bottom edge to the center of the render object.";

    void alignLeft(Interaction::Context const& context, double** slf, double** otr) const ;
    static std::string_view constexpr alignLeftName = "::camera::align::left";
    static std::string_view constexpr alignLeftDesc = "Aligns the camera view left edge to the center of the render object.";

    void alignRight(Interaction::Context const& context, double** slf, double** otr) const ;
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

    enum class Key : std::uint8_t {
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

    struct Position {
        double x = 0.0;
        double y = 0.0;
    };

    enum class Align : std::uint8_t {
        Center,
        Top,
        Bottom,
        Left,
        Right
    };

    // TODO: Second align parameter for object edge alignment
    //       Each camera align needs to know what part of the object to align to what part of the camera view
    //       e.g.: ::camera::align::right-top would align the camera's right edge to the object's top edge
    void setCameraPosition(const Position& pos, Align const& align) const ;

    static Position getAdjustedObjectPosition(double** baseValues, Align const& align);
};
} // namespace Nebulite::Module::Ruleset
#endif // NEBULITE_MODULE_RULESET_CAMERA_HPP
