//------------------------------------------
// Includes

// Standard library
#include <cstdint> // NOLINT
#include <cstddef>
#include <memory>
#include <numeric>
#include <ranges>
#include <span>
#include <string>

// External
#include <SDL3/SDL_rect.h>
#include <SDL3/SDL_render.h>
#include <imgui.h>

// Nebulite
#include "Nebulite/Constants/Event.hpp"
#include "Nebulite/Constants/KeyNames.hpp"
#include "Nebulite/Constants/StandardCapture.hpp"
#include "Nebulite/Core/Environment.hpp"
#include "Nebulite/Core/Renderer.hpp"
#include "Nebulite/Module/Domain/Renderer/Tiling.hpp"
#include "Nebulite/Utility/Coordination/TimedRoutine.hpp"
#include "Nebulite/Utility/Ranges.hpp"

//------------------------------------------
namespace Nebulite::Module::Domain::Renderer {

[[nodiscard]] Constants::Event Tiling::updateHook() {
    if (!tileInfoRoutine) {
        tileInfoRoutine = std::make_unique<Utility::Coordination::TimedRoutine>(
            [&] {
                // NOLINTNEXTLINE
                auto const [wTile, hTile] = domain.tilingInformation();
                moduleScope.set<uint16_t>(Key::tileSizeW, wTile);
                moduleScope.set<uint16_t>(Key::tileSizeH, hTile);
                auto const visibleTiles = domain.visibleTiles();
                if (visibleTiles.size() < moduleScope.memberSize(Key::visibleTiles)) {
                    // Not all entries will be overwritten, remove entire array
                    moduleScope.removeMember(Key::visibleTiles);
                }
                for (auto [idx, tile] : visibleTiles | Utility::Ranges::enumerate) {
                    auto key = Key::visibleTiles.addIndex(idx);
                    auto keyX = key.addMember("x");
                    auto keyY = key.addMember("y");
                    moduleScope.set<int>(keyX, tile.x);
                    moduleScope.set<int>(keyY, tile.y);
                }
            },
            2000,
            Utility::Coordination::TimedRoutine::ConstructionMode::START_IMMEDIATELY
        );
    }
    tileInfoRoutine->update();

    if (gridOn) {
        domain.addRenderCallback([&] {
            auto* const renderer = domain.getSdlRenderer();

            // Camera pos
            auto const x = moduleScope.get<int>(Constants::KeyNames::Renderer::positionX).value_or(0);
            auto const y = moduleScope.get<int>(Constants::KeyNames::Renderer::positionY).value_or(0);

            // Size of tiles
            // NOLINTNEXTLINE
            auto const [wTile, hTile] = domain.tilingInformation();
            for (auto const& tilePosition : domain.visibleTiles()) {
                SDL_FRect rect;
                rect.x = static_cast<float>(tilePosition.x * wTile - x);
                rect.y = static_cast<float>(tilePosition.y * hTile - y);
                rect.w = wTile;
                rect.h = hTile;
                auto scaledRect = domain.scaleRectFromLogicalSize(rect);
                SDL_SetRenderDrawColor(renderer, 255, 0, 255, 255); // Magenta for tile borders
                SDL_RenderRect(renderer, &scaledRect);

                // Render object count and cost
                std::size_t tileCost = 0;
                std::size_t objectCount = 0;
                for (auto const& layer : Core::Environment::getAllLayerTypes()) {
                    auto const& tile = domain.getTile(layer, tilePosition);
                    tileCost += std::accumulate(
                        tile.getBatches().begin(), tile.getBatches().end(), std::size_t{0},
                        [](std::size_t const acc, auto const& batch) {
                            return acc + batch.estimatedCost;
                        }
                    );
                    objectCount += std::accumulate(
                        tile.getBatches().begin(), tile.getBatches().end(), std::size_t{0},
                        [](std::size_t const acc, auto const& batch) {
                            return acc + batch.objects.size();
                        }
                    );
                }

                // Render cost and count as text in the top-left corner of the tile
                std::string const metaInfoText = "Count: " + std::to_string(objectCount) + " Cost: " + std::to_string(tileCost);
                ImGui::GetBackgroundDrawList()->AddText(
                    ImVec2(scaledRect.x+2, scaledRect.y+2),
                    ImColor(255, 0, 255, 255),
                    metaInfoText.c_str()
                );
            }

            // Render current tile pos using ImGui
            auto const w = moduleScope.get<float>(Constants::KeyNames::Renderer::dispResXWindow).value_or(0.0);
            ImGui::SetNextWindowPos(ImVec2(w - 5.0f, 5.0f), ImGuiCond_Always, ImVec2(1.0f, 0.0f));
            ImGui::SetNextWindowBgAlpha(0.35f);

            // Make the window tighter: small padding and item spacing
            ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(6.0f, 2.0f));
            ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(4.0f, 2.0f));

            ImGui::Begin(
                "Tile pos",
                nullptr,
                ImGuiWindowFlags_NoDecoration |
                ImGuiWindowFlags_AlwaysAutoResize |
                ImGuiWindowFlags_NoFocusOnAppearing |
                ImGuiWindowFlags_NoNav
            );

            ImGui::Text("Tile: (%+05d, %+05d)",  domain.getTilePositionX(), domain.getTilePositionY());
            ImGui::End();
            ImGui::PopStyleVar(2);
        });
    }
    return Constants::Event::Success;
}

//------------------------------------------
// Available Functions

Constants::Event Tiling::gridToggle(std::span<std::string_view const> const& args) {
    if (args.size() > 2) return Constants::StandardCapture::Warning::Functional::tooManyArgs(domain.capture);
    if (args.empty()) {
        gridOn = !gridOn;
        return Constants::Event::Success;
    }
    auto const& arg = args[1];
    if (arg == "on") {
        gridOn = true;
        return Constants::Event::Success;
    }
    if (arg == "off") {
        gridOn = false;
        return Constants::Event::Success;
    }
    return Constants::StandardCapture::Warning::Functional::unknownArg(domain.capture);
}

Constants::Event Tiling::viewToggle(std::span<std::string_view const> const& args) const {
    if (args.size() < 2) return Constants::StandardCapture::Warning::Functional::tooFewArgs(domain.capture);
    if (args.size() > 2) return Constants::StandardCapture::Warning::Functional::tooManyArgs(domain.capture);
    if (args[1] == "high") {
        domain.setView(Core::Renderer::ViewSetting::high);
        return Constants::Event::Success;
    }
    if (args[1] == "low") {
        domain.setView(Core::Renderer::ViewSetting::low);
        return Constants::Event::Success;
    }
    if (args[1] == "lowest") {
        domain.setView(Core::Renderer::ViewSetting::lowest);
        return Constants::Event::Success;
    }
    return Constants::StandardCapture::Warning::Functional::unknownArg(domain.capture);
}

} // namespace Nebulite::Module::Domain::Renderer
