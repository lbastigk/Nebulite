#include "Graphics/SdlPrimitive.hpp"
#include <algorithm>
#include <math.h>
#include <cfloat>

namespace Nebulite::Graphics {

void SdlPrimitive::drawFilledCircle(SDL_Renderer* renderer, SDL_Texture* texture, SDL_Color const& color, int const& radius) {
    SDL_SetRenderTarget(renderer, texture);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
    SDL_RenderClear(renderer);
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
    for (int dy = -radius; dy <= radius; dy++) {
        int const dx = static_cast<int>(sqrt(radius * radius - dy * dy));
        SDL_RenderLine(
            renderer,
            static_cast<float>(radius - dx), static_cast<float>(radius + dy),
            static_cast<float>(radius + dx), static_cast<float>(radius + dy)
        );
    }
    SDL_SetRenderTarget(renderer, nullptr);
}

void SdlPrimitive::drawLines(SDL_Renderer* renderer, SDL_Texture* texture, SDL_Color const& color, std::vector<SDL_FPoint> const& points) {
    SDL_SetRenderTarget(renderer, texture);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
    SDL_RenderClear(renderer);
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
    for (size_t i = 0; i < points.size() - 1; ++i) {
        SDL_RenderLine(
            renderer,
            points[i].x, points[i].y,
            points[(i + 1)].x, points[(i + 1)].y
        );
    }
    SDL_SetRenderTarget(renderer, nullptr);
}

void SdlPrimitive::drawEmptyPolygon(SDL_Renderer* renderer, SDL_Texture* texture, SDL_Color const& color, std::vector<SDL_FPoint> const& points) {
    if (points.size() < 2) { // Need at least 2 points to draw a polygon
        return;
    }
    std::vector<SDL_FPoint> closedPoints = points;
    if (std::fabs(points.front().x - points.back().x) > FLT_EPSILON || std::fabs(points.front().y - points.back().y) > FLT_EPSILON) {
        closedPoints.push_back(points.front());
    }
    drawLines(renderer, texture, color, closedPoints);
}

void SdlPrimitive::drawFilledPolygon(SDL_Renderer* renderer, SDL_Texture* texture, SDL_Color const& color, std::vector<SDL_FPoint> const& points) {
    if (points.size() < 3) {
        return; // Need at least a triangle
    }

    SDL_SetRenderTarget(renderer, texture);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
    SDL_RenderClear(renderer);
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);

    // Find Y bounds
    float minY = points[0].y;
    float maxY = points[0].y;
    for (const auto& p : points) {
        minY = std::min(minY, p.y);
        maxY = std::max(maxY, p.y);
    }

    // Scanline loop
    for (int y = static_cast<int>(std::ceil(minY)); y <= static_cast<int>(std::floor(maxY)); ++y) {
        std::vector<float> intersections;

        // Walk polygon edges
        for (size_t i = 0; i < points.size(); ++i) {
            const SDL_FPoint& p1 = points[i];
            const SDL_FPoint& p2 = points[(i + 1) % points.size()];

            // Skip horizontal edges
            if (std::fabs(p1.y - p2.y) < FLT_EPSILON) {
                continue;
            }

            float yMin = std::min(p1.y, p2.y);
            float yMax = std::max(p1.y, p2.y);

            // Check if scanline intersects edge
            if (y >= yMin && y < yMax) {
                float t = (y - p1.y) / (p2.y - p1.y);
                float x = p1.x + t * (p2.x - p1.x);
                intersections.push_back(x);
            }
        }

        // Sort x intersections
        std::ranges::sort(intersections.begin(), intersections.end());

        // Fill between pairs
        for (size_t i = 0; i + 1 < intersections.size(); i += 2) {
            int x1 = static_cast<int>(std::ceil(intersections[i]));
            int x2 = static_cast<int>(std::floor(intersections[i + 1]));
            SDL_RenderLine(renderer, x1, y, x2, y);
        }
    }

    SDL_SetRenderTarget(renderer, nullptr);
}

} // namespace Nebulite::Graphics
