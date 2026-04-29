//------------------------------------------
// Includes

// Nebulite
#include "Core/RenderObject.hpp"
#include "Data/RendererProcessor.hpp"
#include "Data/RenderObjectContainer.hpp"
#include "Nebulite.hpp"

//------------------------------------------
// Helper
namespace {
size_t usedWorkerCount() {
    static size_t usedWorkerCount = Nebulite::Constants::ThreadSettings::getRendererWorkerCount();
    return usedWorkerCount;
}
} // namespace

//------------------------------------------
namespace Nebulite::Data {

Core::RenderObject* Batch::pop() {
    if (objects.empty())
        return nullptr;

    Core::RenderObject* obj = objects.back(); // Get last element
    estimatedCost -= obj->estimateComputationalCost(); // Adjust cost
    objects.pop_back(); // Remove from vector

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
    for (size_t i = 0; i < usedWorkerCount(); i++) {
        batchWorkerPool[i] = std::make_unique<Utility::Coordination::WorkDispatcher<DispatcherWorkspace>>(stopFlag, batchWorkerFunc);
    }
}

RendererProcessor::~RendererProcessor() {
    // 1.) Notify all workers to stop
    stopFlag = true;

    // 2.) Optionally, start work to wake any waiting threads
    for (auto const& worker : batchWorkerPool | std::views::take(usedWorkerCount())) {
        worker->startWork();  // wakes worker so it can check stopFlag
    }
}

void RendererProcessor::prepareForNewLayer(RenderObjectContainer* layer) const {
    for (auto const& worker : batchWorkerPool | std::views::take(usedWorkerCount())) {
        worker->workspace.reinsertionProcess = &layer->reinsertionProcess;
        worker->workspace.deletionProcess = &layer->deletionProcess;
    }
}


void RendererProcessor::batchWorkerFunc(DispatcherWorkspace const& workspace){
    // Process
    // We update each object and check if it needs to be moved or deleted
    // Every batch worker has potential objects to move or delete
    for (auto& batch : workspace.work) {
        std::vector<Core::RenderObject*> to_move_local;
        std::vector<Core::RenderObject*> to_delete_local;

        for (auto obj : batch->objects) {
            Global::instance().notifyEvent(obj->update());


            if (!obj->flag.deleteFromScene) {
                if (RenderObjectContainer::getTilePos(obj, workspace.dispResX, workspace.dispResY) != workspace.pos) {
                    to_move_local.push_back(obj);
                }
            } else {
                to_delete_local.push_back(obj);
            }
        }

        // All objects to move are collected in queue
        for (auto ptr : to_move_local) {
            batch->removeObject(ptr);
            std::scoped_lock lock(workspace.reinsertionProcess->reinsertMutex);
            workspace.reinsertionProcess->queue.push_back(ptr);
        }

        // All objects to delete are collected in trash
        for (auto ptr : to_delete_local) {
            batch->removeObject(ptr);
            std::scoped_lock lock(workspace.deletionProcess->deleteMutex);
            workspace.deletionProcess->trash.push_back(ptr);
        }
    }
}

void RendererProcessor::processPool() const {
    for (auto const& worker : batchWorkerPool | std::views::take(usedWorkerCount())) {
        worker->startWork();
    }
    for (auto const& worker : batchWorkerPool | std::views::take(usedWorkerCount())) {
        worker->waitForWorkFinished();
    }
    for (auto const& worker : batchWorkerPool | std::views::take(usedWorkerCount())) {
        worker->workspace.work.clear();
        worker->workspace.cost = 0;
    }
}

} // namespace Nebulite::Data
