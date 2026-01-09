#include "Nebulite.hpp"
#include "Data/Document/JsonScopeBase.hpp"
#include "Interaction/Execution/DomainModule.hpp"

namespace Nebulite::Interaction::Execution {

DomainModuleBase::DomainModuleBase(std::shared_ptr<FuncTree<Constants::Error>> funcTreePtr, Data::JsonScopeBase& w, std::string const& s)
    : funcTree(std::move(funcTreePtr)), moduleScope(w), settingsPrefix(s) {}

Data::JsonScopeBase& DomainModuleBase::getDoc() const {
    return moduleScope;
}

Data::JsonScopeBase const& DomainModuleBase::settings() const {
    static Data::JsonScopeBase const& settingsScopeConst = global().shareSettingsScope(settingsPrefix);
    return settingsScopeConst;
}

} // namespace Nebulite::Interaction::Execution
