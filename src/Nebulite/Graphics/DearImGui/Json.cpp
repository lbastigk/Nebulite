//------------------------------------------
// Includes

// Standard Library
#include <string>

// External
#include <imgui.h>

// Nebulite
#include "Nebulite/Data/Document/Json.hpp"
#include "Nebulite/Data/Document/JsonScope.hpp"
#include "Nebulite/Data/Document/KeyType.hpp"
#include "Nebulite/Data/Document/ScopedKeyView.hpp"
#include "Nebulite/Graphics/DearImGui/Json.hpp"

//------------------------------------------
namespace Nebulite::Graphics::DearImGui::Json {

void renderScope(Data::JsonScope const& s, Data::ScopedKeyView const& root) {
    for (auto const& key : s.listAvailableKeys(root)) {
        std::string const rootPath = root.toString();
        std::string const fullPath = key.view().toString();
        std::string keyPath = fullPath;
        if (rootPath != fullPath) keyPath = fullPath.substr(rootPath.length());
        if (!keyPath.empty() && keyPath.front() == Data::Json::SpecialCharacter::dot) keyPath.erase(0, 1);
        if (auto const type = s.memberType(key); type == Data::KeyType::object || type == Data::KeyType::array) {
            // use fullPath as the ID (first arg) and keyPath as the visible text (format)
            if (ImGui::TreeNode(fullPath.c_str(), "%s", keyPath.c_str())) {
                renderScope(s, key.view());
                ImGui::TreePop();
            }
        } else if (type == Data::KeyType::value) {
            if (auto const stringValue = s.get<std::string>(key); stringValue.has_value()) {
                ImGui::Text("%s : %s", keyPath.c_str(), stringValue.value().c_str());
            }
            else {
                ImGui::TextDisabled("%s : <err: failed to convert value to string>", keyPath.c_str());
            }
        } else {
            ImGui::TextDisabled("%s : null", keyPath.c_str());
        }
    }
}

void renderDocument(Data::Json& doc, Data::ScopedKeyView const& root){
    auto const& scope = doc.shareManagedScope(root.toString());
    renderScope(scope, root);
}

} // namespace Nebulite::Graphics::DearImGui::Json
