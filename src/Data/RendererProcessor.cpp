//------------------------------------------
// Includes

// Standard library
#include <cstddef>
#include <exception>
#include <mutex>
#include <optional>
#include <ranges>
#include <string>
#include <vector>

// Nebulite
#include "Nebulite/Constants/ThreadSettings.hpp"
#include "Nebulite/Core/RenderObject.hpp"
#include "Nebulite/Data/RenderObjectContainer.hpp"
#include "Nebulite/Data/RendererProcessor.hpp"
#include "Nebulite/Nebulite.hpp"
#include "Nebulite/Utility/Coordination/WorkDispatcher.hpp"
#include "Nebulite/Utility/Generate.hpp"

//------------------------------------------
namespace Nebulite::Data {

RendererProcessor& RendererProcessor::instance() {
    static RendererProcessor instance;
    return instance;
}

RendererProcessor::RendererProcessor()
    : batchWorkerPool(Utility::Generate::array<
          std::optional<Utility::Coordination::WorkDispatcher<DispatcherWorkspace>>,
          Constants::ThreadSettings::Maximum::rendererWorkerCount
      >([](std::size_t) {
          return std::nullopt;
    })){
    // Initialize worker pool
    for (std::size_t i = 0; i < Constants::ThreadSettings::getRendererWorkerCount(); i++) {
        batchWorkerPool[i].emplace(stopFlag, batchWorkerFunc);
    }
}

RendererProcessor::~RendererProcessor() {
    try {
        // 1.) Notify all workers to stop
        stopFlag = true;

        // 2.) Optionally, start work to wake any waiting threads
        for (auto& worker : batchWorkerPool | std::views::take(Constants::ThreadSettings::getRendererWorkerCount())) {
            if (worker.has_value()) {
                worker.value().startWork();  // wakes worker so it can check stopFlag
            }
            else {
                throw std::exception();
            }
        }
    } catch (std::exception const& e) {
        // Log the exception if needed
        Global::capture().error.println("Exception during RendererProcessor destruction: " + std::string(e.what()));
    }
}

void RendererProcessor::prepareForNewLayer(RenderObjectContainer* layer) {
    for (auto& worker : batchWorkerPool | std::views::take(Constants::ThreadSettings::getRendererWorkerCount())) {
        if (worker.has_value()) {
            worker.value().workspace.reinsertionProcess = &layer->reinsertionProcess;
            worker.value().workspace.deletionProcess = &layer->deletionProcess;
        }
        else {
            throw std::exception();
        }
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

void RendererProcessor::processPool(std::size_t const count) {
    for (auto& worker : batchWorkerPool | std::views::take(count)) {
        if (worker.has_value()) {
            worker.value().startWork();
        }
        else {
            throw std::exception();
        }
    }
    for (auto& worker : batchWorkerPool | std::views::take(count)) {
        if (worker.has_value()) {
            worker.value().waitForWorkFinished();
        }
        else {
            throw std::exception();
        }
    }
}

} // namespace Nebulite::Data
