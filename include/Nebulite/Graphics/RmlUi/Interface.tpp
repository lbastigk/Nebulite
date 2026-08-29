#ifndef NEBULITE_GRAPHICS_RMLUI_INTERFACE_TPP
#define NEBULITE_GRAPHICS_RMLUI_INTERFACE_TPP

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

#ifndef NEBULITE_GRAPHICS_RMLUI_INTERFACE_HPP
#include "Nebulite/Graphics/RmlUi/Interface.hpp"
#endif // NEBULITE_GRAPHICS_RMLUI_INTERFACE_HPP

namespace Nebulite::Graphics::RmlUi {

template <typename Key, typename Container>
void Interface::determineNewContext(auto const& ctxAndScope, Key const& key, Container& container, std::size_t const domainId) {
    auto& oldCtx = ctxAndScope.ctx;
    auto& oldCtxScope = ctxAndScope.ctxScope;
    if (oldCtx.other.getId() != domainId && oldCtx.global.getId() != domainId) {
        return;
    }
    using NewDomainAndScope = std::pair<Interaction::Execution::Domain*, Data::JsonScope*>;
    NewDomainAndScope const newOther = oldCtx.other.getId() == domainId ? std::make_pair(&oldCtx.self, &oldCtxScope.self) : std::make_pair(&oldCtx.other, &oldCtxScope.other);
    NewDomainAndScope const newGlobal = oldCtx.global.getId() == domainId ? std::make_pair(&oldCtx.self, &oldCtxScope.self) : std::make_pair(&oldCtx.global, &oldCtxScope.global);
    ContextAndScope const newCtxAndScope = {
        {
            ctxAndScope.ctx.self,
            *newOther.first,
            *newGlobal.first,
        },
        {
            ctxAndScope.ctxScope.self,
            *newOther.second,
            *newGlobal.second,
        },
    };
    container.emplace_back(key, newCtxAndScope);
}

template<typename Container>
void Interface::removeContext(std::size_t const ownerId, Container& container) {
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

} // namespace Nebulite::Graphics::RmlUi
#endif // NEBULITE_GRAPHICS_RMLUI_INTERFACE_TPP
