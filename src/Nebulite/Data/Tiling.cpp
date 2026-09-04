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
#include "Nebulite/Constants/Event.hpp"
#include "Nebulite/Core/GlobalSpace.hpp"
#include "Nebulite/Core/RenderObject.hpp"
#include "Nebulite/Data/Batch.hpp"
#include "Nebulite/Data/RenderObjectContainer.hpp"
#include "Nebulite/Data/Tiling.hpp"
#include "Nebulite/Graphics/Sdl/Canvas.hpp"
#include "Nebulite/Nebulite.hpp"
#include "Nebulite/Utility/Io/Capture.hpp"

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
    std::ranges::for_each(
        batches,
        [&](auto& batch) {
            std::ranges::move(batch.objects, std::back_inserter(destination));
            batch.objects.clear();
        }
    );
    batches.clear();
}

bool Tile::insertIfCostGoalMatches(Core::RenderObject* toAppend) {
    auto const it = std::ranges::find_if(
        batches.begin(),
        batches.end(),
        [&](Batch const& b) {
            // NOLINTBEGIN
            if constexpr (batchCostGoal == 0) {
                return true; // No cost goal, accept all batches
            }
            // NOLINTEND
            else {
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

void Tile::update(std::vector<Core::RenderObject*>& toMove, std::vector<Core::RenderObject*>& toDelete, TilingInformation const& tilingInfo, TileCoordinate const& coordinate) {
    for (auto& batch : batches) {
        std::vector<Core::RenderObject*> toMoveLocal;
        std::vector<Core::RenderObject*> toDeleteLocal;

        for (auto* obj : batch.objects) {
            if ( auto const event = obj->update(); event != Constants::Event::success) {
                Global::instance().notifyEvent(event);
            }
            if (!obj->flag.deleteFromScene) {
                if (RenderObjectContainer::getTilePos(obj->getPosition(), tilingInfo) != coordinate) {
                    toMoveLocal.push_back(obj);
                }
            } else {
                toDeleteLocal.push_back(obj);
            }
        }

        for (auto* ptr : toMoveLocal) {
            batch.removeObject(ptr);
        }
        for (auto* ptr : toDeleteLocal) {
            batch.removeObject(ptr);
        }
        if (!toMoveLocal.empty() || !toDeleteLocal.empty()) {
            deleteTexture();
        }

        std::ranges::move(toMoveLocal.begin(), toMoveLocal.end(), std::back_inserter(toMove));
        std::ranges::move(toDeleteLocal.begin(), toDeleteLocal.end(), std::back_inserter(toDelete));
        batch.updateCost();
    }
}

void Tile::render(
    Core::Renderer& nebuliteRenderer,
    TileCoordinate const& coordinate,
    TilingInformation const& tilingInfo,
    Utility::Io::Capture& capture,
    int const dispPosX,
    int const dispPosY,
    int const windowScale
){
    auto* const renderer = nebuliteRenderer.getSdlRenderer();

    // Create texture, if missing
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

        Graphics::Sdl::Canvas::drawOnTexture(renderer, texture, [&] {
            for (auto const& objects : getBatchedObjects()) {
                for (auto const& obj : objects) {
                    obj->draw(
                        nebuliteRenderer,
                        static_cast<float>(coordinate.x * tilingInfo.w),
                        static_cast<float>(coordinate.y * tilingInfo.h)
                    );
                }
            }
        });
    }

    // Render tile to screen
    SDL_SetRenderTarget(renderer, nullptr);
    SDL_FRect const destRect{
        .x = static_cast<float>(windowScale * (coordinate.x * tilingInfo.w - dispPosX)),
        .y = static_cast<float>(windowScale * (coordinate.y * tilingInfo.h - dispPosY)),
        .w = static_cast<float>(2 * windowScale * tilingInfo.w),
        .h = static_cast<float>(2 * windowScale * tilingInfo.h),
    };
    if (!SDL_RenderTexture(renderer, texture, nullptr, &destRect)) {
        capture.error.println("Failed to render background tile texture: ", SDL_GetError());
    }
}

} // namespace Nebulite::Data
