#include "Nebulite.hpp"
#include "Graphics/Drawcall.hpp"

namespace Nebulite::Graphics {

void Drawcall::draw() const {
    updaterRoutine.update(); // Update the drawcall data periodically

    // TODO: Add SDL drawing logic here based on type...
}

void Drawcall::updateDrawcallData() {
    // TODO: Add more complicated update logic if needed
    if (auto const t = drawcallScope.get<std::string>(Key::type, "sprite"); t == "sprite") {
        type = SPRITE;
    }
    else if (t == "text") {
        type = TEXT;
    }
}

Constants::Error Drawcall::parseStr(std::string const& str) { return texture.parseStr(str); }

} // namespace Nebulite::Graphics
