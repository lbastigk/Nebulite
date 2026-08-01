#ifndef NEBULITE_GRAPHICS_RMLINTERFACE_TPP
#define NEBULITE_GRAPHICS_RMLINTERFACE_TPP

//------------------------------------------
// Includes

// Standard library
#include <cstddef>
#include <utility>
#include <vector>

//------------------------------------------
// Forward declarations

namespace Nebulite::Interaction::Execution {
class Domain;
} // namespace Nebulite::Interaction::Execution

//------------------------------------------
// Conditional includes

#ifndef NEBULITE_GRAPHICS_RMLINTERFACE_HPP
#include "Nebulite/Graphics/RmlInterface.hpp"
#endif // NEBULITE_GRAPHICS_RMLINTERFACE_HPP

namespace Nebulite::Graphics {

template <typename Key, typename Container>
void RmlInterface::determineNewContext(auto const& ctxAndScope, Key const& key, Container& container, std::size_t const domainId) {
    auto& oldCtx = ctxAndScope.ctx;
    auto& oldCtxScope = ctxAndScope.ctxScope;
    if (oldCtx.other.getId() != domainId && oldCtx.global.getId() != domainId) {
        return;
    }
    using newDomainAndScope = std::pair<Interaction::Execution::Domain*, Data::JsonScope*>;
    newDomainAndScope const newOther = oldCtx.other.getId() == domainId ? std::make_pair(&oldCtx.self, &oldCtxScope.self) : std::make_pair(&oldCtx.other, &oldCtxScope.other);
    newDomainAndScope const newGlobal = oldCtx.global.getId() == domainId ? std::make_pair(&oldCtx.self, &oldCtxScope.self) : std::make_pair(&oldCtx.global, &oldCtxScope.global);
    ContextAndScope const newCtxAndScope = {
        {
            ctxAndScope.ctx.self,
            *newOther.first,
            *newGlobal.first
        },
        {
            ctxAndScope.ctxScope.self,
            *newOther.second,
            *newGlobal.second
        },
    };
    container.emplace_back(key, newCtxAndScope);
}

template<typename Container>
void RmlInterface::removeContext(std::size_t const ownerId, Container& container) {
    // Should be possible in one loop: check ctxAndScope for id, replace instantly
    using Key = Container::key_type;
    std::vector<std::pair<Key, ContextAndScope>> updated;
    updated.reserve(container.size());
    for (auto& [key, ctxAndScope] : container) {
        determineNewContext(ctxAndScope, key, updated, ownerId);
    }
    for (auto& [key, newCtxAndScope] : updated) {
        container.emplace(key, newCtxAndScope);
    }
}

} // namespace Nebulite::Graphics
#endif // NEBULITE_GRAPHICS_RMLINTERFACE_TPP
