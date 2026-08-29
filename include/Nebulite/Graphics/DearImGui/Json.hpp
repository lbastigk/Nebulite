#ifndef NEBULITE_GRAPHICS_DEARIMGUI_JSON_HPP
#define NEBULITE_GRAPHICS_DEARIMGUI_JSON_HPP

//------------------------------------------
// Forward declarations

namespace Nebulite::Data {
class Json;
class JsonScope;
class ScopedKeyView;
} // namespace Nebulite::Data

//------------------------------------------
namespace Nebulite::Graphics::DearImGui::Json {
/**
 * @brief Renders a JSON tree node in an ImGui window.
 * @param s The JSON scope to render.
 * @param root The root key for the JSON node.
 */
void renderScope(Data::JsonScope const& s, Data::ScopedKeyView const& root);

/**
 * @brief Renders a JSON tree node in an ImGui window.
 * @param doc The JSON document to render.
 * @param root The root key for the JSON node.
 */
void renderDocument(Data::Json& doc, Data::ScopedKeyView const& root);

} // namespace Nebulite::Graphics::DearImGui::Json
#endif // NEBULITE_GRAPHICS_DEARIMGUI_JSON_HPP

