//------------------------------------------
// Includes

// Standard library
#include <algorithm>
#include <cstddef>
#include <memory>
#include <mutex>
#include <ranges>
#include <stdexcept>
#include <vector>

// Nebulite
#include "Constants/ThreadSettings.hpp"
#include "Core/RenderObject.hpp"
#include "Data/RenderObjectContainer.hpp"
#include "Data/RendererProcessor.hpp"
#include "Nebulite.hpp"
#include "Utility/Coordination/WorkDispatcher.hpp"

//------------------------------------------
namespace Nebulite::Data {

void Batch::updateCost(){
    estimatedCost = 0;
    for (auto const* obj : objects) {
        estimatedCost += obj->estimateComputationalCost();
    }
}

Core::RenderObject* Batch::pop() {
    if (objects.empty())
        return nullptr;

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

RendererProcessor::RendererProcessor() {
    // Ensure that this is a singleton class
    static bool instanceExists = false;
    if (instanceExists) {
        throw std::runtime_error("RendererProcessor instance already exists!");
    }
    instanceExists = true;

    // Initialize worker pool
    for (size_t i = 0; i < Constants::ThreadSettings::getRendererWorkerCount(); i++) {
        batchWorkerPool[i] = std::make_unique<Utility::Coordination::WorkDispatcher<DispatcherWorkspace>>(stopFlag, batchWorkerFunc);
    }
}

RendererProcessor::~RendererProcessor() {
    // 1.) Notify all workers to stop
    stopFlag = true;

    // 2.) Optionally, start work to wake any waiting threads
    for (auto const& worker : batchWorkerPool | std::views::take(Constants::ThreadSettings::getRendererWorkerCount())) {
        worker->startWork();  // wakes worker so it can check stopFlag
    }
}

void RendererProcessor::prepareForNewLayer(RenderObjectContainer* layer) const {
    for (auto const& worker : batchWorkerPool | std::views::take(Constants::ThreadSettings::getRendererWorkerCount())) {
        worker->workspace.reinsertionProcess = &layer->reinsertionProcess;
        worker->workspace.deletionProcess = &layer->deletionProcess;
    }
}


void RendererProcessor::batchWorkerFunc(DispatcherWorkspace const& workspace){
    // Process
    // We update each object and check if it needs to be moved or deleted
    // Every batch worker has potential objects to move or delete
    std::vector<Core::RenderObject*> to_move;
    std::vector<Core::RenderObject*> to_delete;

    workspace.work->update(to_move, to_delete, workspace.tilingInformation, workspace.pos);

    // All objects to move are collected in queue
    for (auto* ptr : to_move) {
        std::scoped_lock const lock(workspace.reinsertionProcess->reinsertMutex);
        workspace.reinsertionProcess->queue.push_back(ptr);
    }

    // All objects to delete are collected in trash
    for (auto* ptr : to_delete) {
        std::scoped_lock const lock(workspace.deletionProcess->deleteMutex);
        workspace.deletionProcess->trash.push_back(ptr);
    }
}

void RendererProcessor::processPool() const {
    processPool(Constants::ThreadSettings::getRendererWorkerCount());
}

void RendererProcessor::processPool(size_t count) const {
    for (auto const& worker : batchWorkerPool | std::views::take(count)) {
        worker->startWork();
    }
    for (auto const& worker : batchWorkerPool | std::views::take(count)) {
        worker->waitForWorkFinished();
    }
}

} // namespace Nebulite::Data
