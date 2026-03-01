//------------------------------------------
// Includes

// External
#include "imgui.h"

// Nebulite
#include "Graphics/ImguiHelper.hpp"

//------------------------------------------
namespace Nebulite::Graphics {

void ImguiHelper::renderJsonScope(Data::JsonScopeBase const& scope, std::string const& name) {
    ImGui::Begin(name.c_str());

    std::function<void(Data::JsonScopeBase const&, Data::ScopedKey const&)> traverseObject;
    traverseObject = [&traverseObject](Data::JsonScopeBase const& s, Data::ScopedKey const& root) {
        for (auto const& key : s.listAvailableKeys(root)) {
            std::string const rootPath = root.view().toString();
            std::string const fullPath = key.view().toString(); // stable ID
            std::string keyPath = fullPath;                   // visible label
            if (rootPath != fullPath) keyPath = fullPath.substr(rootPath.length());
            if (!keyPath.empty() && keyPath.front() == '.') keyPath.erase(0, 1);
            if (auto const type = s.memberType(key); type == Data::KeyType::object || type == Data::KeyType::array) {
                // use fullPath as the ID (first arg) and keyPath as the visible text (format)
                if (ImGui::TreeNode(fullPath.c_str(), "%s", keyPath.c_str())) {
                    traverseObject(s, key);
                    ImGui::TreePop();
                }
            } else if (type == Data::KeyType::value) {
                // fetch value every frame and display
                std::string const valueStr = s.get<std::string>(key).value_or("<unavailable>");
                ImGui::Text("%s : %s", keyPath.c_str(), valueStr.c_str());
            } else {
                ImGui::TextDisabled("%s : null", keyPath.c_str());
            }
        }
    };

    traverseObject(scope, scope.getRootScope().toScopedKey());
    ImGui::End();
}

} // namespace Nebulite::Graphics
