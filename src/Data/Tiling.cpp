//------------------------------------------
// Includes

// Standard library
#include <algorithm>
#include <cstdlib>
#include <iterator>
#include <utility>
#include <vector>

// External
#include <SDL3/SDL_error.h>
#include <SDL3/SDL_pixels.h>
#include <SDL3/SDL_rect.h>
#include <SDL3/SDL_render.h>

// Nebulite
#include "Constants/Event.hpp"
#include "Core/GlobalSpace.hpp"
#include "Core/RenderObject.hpp"
#include "Data/Batch.hpp"
#include "Data/RenderObjectContainer.hpp"
#include "Data/Tiling.hpp"
#include "Nebulite.hpp"
#include "Utility/IO/Capture.hpp"

//------------------------------------------
namespace Nebulite::Data {

void Tile::deleteTexture(){
    if (texture != nullptr) {
        SDL_DestroyTexture(texture);
        texture = nullptr;
    }
}

std::vector<Batch> const& Tile::getBatches() const {
    return batches;
}

void Tile::appendBatch(Batch&& batch) {
    batches.push_back(std::move(batch));
}

void Tile::moveObjects(std::vector<Core::RenderObject*>& destination) {
    for (auto& [objects, _] : batches) {
        // Move all objects to trash
        std::ranges::move(objects.begin(), objects.end(), std::back_inserter(destination));
        objects.clear(); // Remove all objects from the batch
    }
    batches.clear();
}

bool Tile::insertIfCostGoalMatches(Core::RenderObject* toAppend) {
    auto const it = std::ranges::find_if(
        batches.begin(),
        batches.end(),
        // NOLINTNEXTLINE
        [&](Batch const& b) {
            if constexpr (batchCostGoal == 0) {
                // NOLINTNEXTLINE
                return true; // No cost goal, accept all batches
            }
            else {
                // NOLINTNEXTLINE
                return b.estimatedCost <= batchCostGoal;
            }
        }
    );
    if (it != batches.end()) {
        it->push(toAppend);
        deleteTexture();
        return true;
    }
    return false;
}

void Tile::update(std::vector<Core::RenderObject*>& to_move, std::vector<Core::RenderObject*>& to_delete, TilingInformation const& tilingInfo, TileCoordinate const& coordinate) {
    for (auto& batch : batches) {
        std::vector<Core::RenderObject*> to_move_local;
        std::vector<Core::RenderObject*> to_delete_local;

        for (auto* obj : batch.objects) {
            if ( auto const event = obj->update(); event != Constants::Event::Success) {
                Global::instance().notifyEvent(event);
            }
            if (!obj->flag.deleteFromScene) {
                if (RenderObjectContainer::getTilePos(obj->getPosition(), tilingInfo) != coordinate) {
                    to_move_local.push_back(obj);
                }
            } else {
                to_delete_local.push_back(obj);
            }
        }
        // All objects to move are collected in queue
        for (auto* ptr : to_move_local) {
            batch.removeObject(ptr);
        }

        // All objects to delete are collected in trash
        for (auto* ptr : to_delete_local) {
            batch.removeObject(ptr);
        }

        // Update batch cost
        batch.updateCost();

        // Invalidate texture
        if (!to_move_local.empty() || !to_delete_local.empty()) {
            deleteTexture();
        }

        std::ranges::move(to_move_local.begin(), to_move_local.end(), std::back_inserter(to_move));
        std::ranges::move(to_delete_local.begin(), to_delete_local.end(), std::back_inserter(to_delete));
    }
}

void Tile::render(
    Core::Renderer const& nebuliteRenderer,
    TileCoordinate const& coordinate,
    TilingInformation const& tilingInfo,
    Utility::IO::Capture& capture,
    int const dispPosX,
    int const dispPosY,
    int const windowScale
){
    auto* const renderer = nebuliteRenderer.getSdlRenderer();

    // Re-render background texture
    if (!texture) {
        texture = SDL_CreateTexture(
            renderer,
            SDL_PIXELFORMAT_RGBA8888,
            SDL_TEXTUREACCESS_TARGET,
            2*windowScale*tilingInfo.w,
            2*windowScale*tilingInfo.h
        );
        if (!texture) {
            capture.error.println("Failed to create render target texture.");
            std::abort();
        }
        SDL_SetRenderTarget(renderer, texture);
        for (auto const& objects : getBatchedObjects()) {
            for (auto const& obj : objects) {
                obj->draw(
                    nebuliteRenderer,
                    static_cast<float>(coordinate.x * tilingInfo.w),
                    static_cast<float>(coordinate.y * tilingInfo.h)
                );
            }
        }
    }

    // Render to screen
    SDL_SetRenderTarget(renderer, nullptr);
    SDL_FRect const destRect{
        .x = static_cast<float>(windowScale * (coordinate.x * tilingInfo.w - dispPosX)),
        .y = static_cast<float>(windowScale * (coordinate.y * tilingInfo.h - dispPosY)),
        .w = static_cast<float>(2 * windowScale * tilingInfo.w),
        .h = static_cast<float>(2 * windowScale * tilingInfo.h)
    };
    if (!SDL_RenderTexture(renderer, texture, nullptr, &destRect)) {
        capture.error.println("Failed to render background tile texture: ", SDL_GetError());
    }
}

} // namespace Nebulite::Data
