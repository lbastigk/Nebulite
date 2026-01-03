#include "Interaction/Execution/Domain.hpp"
#include "Data/Document/JsonScope.hpp"

namespace Nebulite::Interaction::Execution {

DocumentAccessor::~DocumentAccessor() = default;

Data::JsonScope& DocumentAccessor::getDoc() const {
    return documentScope;
}

DomainBase& DomainBase::operator=(DomainBase const& other) {
    if (this == &other) return *this;
    if (&documentScope != &other.documentScope) {
        throw std::invalid_argument("DomainBase::operator=: cannot assign from object with different document reference");
    }

    domainName = other.domainName;
    funcTree = other.funcTree; // shared ownership
    if (funcTree) {
        // Rebind preParse to this object (avoid using other's bound callback)
        funcTree->setPreParse([this] { return preParse(); });
    }
    return *this;
}

DomainBase& DomainBase::operator=(DomainBase&& other) noexcept {
    if (this == &other) return *this;
    domainName = std::move(other.domainName);
    funcTree = std::move(other.funcTree);
    if (funcTree) {
        funcTree->setPreParse([this] { return preParse(); });
    }
    return *this;
}

DomainBase::~DomainBase() = default;

std::string const& DomainBase::scopePrefix() const {
    return documentScope.getScopePrefix();
}

Data::MappedOrderedDoublePointers* DomainBase::getDocumentCacheMap() const {
    return documentScope.getOrderedCacheListMap();
}

Data::JsonScopeBase& DomainBase::shareDocumentScopeBase(std::string const& prefix) const {
    return documentScope.shareScopeBase(prefix);
}

std::scoped_lock<std::recursive_mutex> DomainBase::lockDocument() const {
    return documentScope.lock();
}

} // namespace Nebulite::Interaction::Execution
