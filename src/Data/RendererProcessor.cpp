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
#include "Data/Batch.hpp"
#include "Data/RenderObjectContainer.hpp"
#include "Data/RendererProcessor.hpp"
#include "Utility/Coordination/WorkDispatcher.hpp"
#include "Utility/Generate.hpp"

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

RendererProcessor::RendererProcessor()
    : batchWorkerPool(Utility::Generate::array<
          std::optional<Utility::Coordination::WorkDispatcher<DispatcherWorkspace>>,
          Constants::ThreadSettings::Maximum::rendererWorkerCount
      >([](std::size_t) {
          return std::nullopt;
    })){
    // Ensure that this is a singleton class
    static bool instanceExists = false;
    if (instanceExists) {
        throw std::runtime_error("RendererProcessor instance already exists!");
    }
    instanceExists = true;

    // Initialize worker pool
    for (std::size_t i = 0; i < Constants::ThreadSettings::getRendererWorkerCount(); i++) {
        batchWorkerPool[i].emplace(stopFlag, batchWorkerFunc);
    }
}

RendererProcessor::~RendererProcessor() {
    // 1.) Notify all workers to stop
    stopFlag = true;

    // 2.) Optionally, start work to wake any waiting threads
    for (auto& worker : batchWorkerPool | std::views::take(Constants::ThreadSettings::getRendererWorkerCount())) {
        worker.value().startWork();  // wakes worker so it can check stopFlag
    }
}

void RendererProcessor::prepareForNewLayer(RenderObjectContainer* layer) {
    for (auto& worker : batchWorkerPool | std::views::take(Constants::ThreadSettings::getRendererWorkerCount())) {
        worker.value().workspace.reinsertionProcess = &layer->reinsertionProcess;
        worker.value().workspace.deletionProcess = &layer->deletionProcess;
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

void RendererProcessor::processPool() {
    processPool(Constants::ThreadSettings::getRendererWorkerCount());
}

void RendererProcessor::processPool(std::size_t count) {
    for (auto& worker : batchWorkerPool | std::views::take(count)) {
        worker.value().startWork();
    }
    for (auto& worker : batchWorkerPool | std::views::take(count)) {
        worker.value().waitForWorkFinished();
    }
}

} // namespace Nebulite::Data
