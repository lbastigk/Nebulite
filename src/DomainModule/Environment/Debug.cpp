#include "DomainModule/Environment/Debug.hpp"
#include "Core/Environment.hpp"

namespace Nebulite::DomainModule::Environment {

Constants::Error Debug::update(){
    size_t activeWorkers = 0;
    size_t totalWorkers = 0;
    for (auto const& layer : domain.getAllLayers()) {
        auto const& info = layer.getContainerInfo();
        activeWorkers += info.activeWorkers;
        totalWorkers += info.totalWorkers;
    }
    moduleScope.set<size_t>(Key::workersActive, activeWorkers);
    moduleScope.set<size_t>(Key::workersTotal, totalWorkers);
    return Constants::ErrorTable::NONE();
}

} // namespace Nebulite::DomainModule::Environment
