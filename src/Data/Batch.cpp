//------------------------------------------
// Includes

// Standard library
#include <algorithm>
#include <vector>

// Nebulite
#include "Core/RenderObject.hpp"
#include "Data/Batch.hpp"
#include "Data/RenderObjectContainer.hpp"

//------------------------------------------
namespace Nebulite::Data {

void Batch::updateCost(){
    estimatedCost = 0;
    for (auto const* obj : objects) {
        estimatedCost += obj->estimateComputationalCost();
    }
}

Core::RenderObject* Batch::pop() {
    if (objects.empty()) return nullptr;

    Core::RenderObject* obj = objects.back(); // Get last element
    objects.pop_back(); // Remove from vector
    updateCost();
    return obj;
}

void Batch::push(Core::RenderObject* obj) {
    estimatedCost += obj->estimateComputationalCost();
    objects.push_back(obj);
}

bool Batch::removeObject(Core::RenderObject* obj) {
    if (auto const it = std::ranges::find(objects.begin(), objects.end(), obj); it != objects.end()) {
        estimatedCost -= obj->estimateComputationalCost();
        objects.erase(it);
        return true;
    }
    return false;
}

} // namespace Nebulite::Data
