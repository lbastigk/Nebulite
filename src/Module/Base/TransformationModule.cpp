//------------------------------------------
// Includes

// Standard library
#include <memory>
#include <string_view>
#include <utility>

// Nebulite
#include "Interaction/Execution/FuncTree.hpp"
#include "Interaction/Logic/Expression.hpp"
#include "Module/Base/TransformationModule.hpp"

//------------------------------------------
namespace Nebulite::Module::Base {

TransformationModule::TransformationModule(std::shared_ptr<Interaction::Execution::FuncTree<bool, Data::JsonScope*>> funcTree)
    : transformationFuncTree(std::move(funcTree)){}

TransformationModule::~TransformationModule() = default;

void TransformationModule::bindCategory(std::string_view const name, std::string_view const helpDescription) const {
    transformationFuncTree->bindCategory(name, helpDescription);
}

} // namespace Nebulite::Module::Base
