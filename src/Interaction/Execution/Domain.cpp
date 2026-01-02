#include "Interaction/Execution/Domain.hpp"
#include "Data/Document/JsonScope.hpp"

namespace Nebulite::Interaction::Execution {

DomainBase& DomainBase::operator=(DomainBase const& other) {
    if (this == &other) return *this;
    if (&document != &other.document) {
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
    return document.getScopePrefix();
}

} // namespace Nebulite::Interaction::Execution
