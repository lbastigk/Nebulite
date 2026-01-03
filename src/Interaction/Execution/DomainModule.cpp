#include "Interaction/Execution/DomainModule.hpp"
#include "Data/Document/JsonScopeBase.hpp"

namespace Nebulite::Interaction::Execution {

DomainModuleBase::DomainModuleBase(std::shared_ptr<FuncTree<Constants::Error>> funcTreePtr, Data::JsonScopeBase* scope)
    : funcTree(std::move(funcTreePtr))
{
    docPtr = std::make_unique<Data::JsonScopeBase>(*scope);
}

Data::JsonScopeBase& DomainModuleBase::getDoc() const {
    return *docPtr.get();
}

} // namespace Nebulite::Interaction::Execution
