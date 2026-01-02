#include "Data/Document/JsonScope.hpp"
#include "DomainModule/Initializer.hpp"

namespace Nebulite::Data {

JsonScope::JsonScope(JsonScope&& other) noexcept
    : Domain("JsonScope", *this, getBaseDocument()), // TODO: change to *this once JsonScope is used in domains
      baseDocument(std::move(other.baseDocument)),
      scopePrefix(std::move(other.scopePrefix)) {
    // TODO: activate once available:
    //DomainModule::Initializer::initJsonScope(this);
}

JsonScope& JsonScope::operator=(JsonScope&& other) noexcept {
    if (this != &other) {
        baseDocument = std::move(other.baseDocument);
        scopePrefix = std::move(other.scopePrefix);
    }
    return *this;
}

JsonScope::~JsonScope() {
    // Nothing special to do here
}

} // namespace Nebulite::Data
