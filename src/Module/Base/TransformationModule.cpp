//------------------------------------------
// Includes

// Standard library
#include <memory>
#include <string_view>

// Nebulite
#include "Nebulite/Interaction/Logic/Expression.hpp"
#include "Nebulite/Module/Base/TransformationModule.hpp"
#include "Nebulite/Utility/Args/FuncTree.hpp"

//------------------------------------------
namespace Nebulite::Module::Base {

TransformationModule::TransformationModule(std::shared_ptr<Utility::Args::FuncTree<bool, Data::JsonScope&>> const& funcTree)
    : transformationFuncTree(funcTree){}

TransformationModule::~TransformationModule() = default;

void TransformationModule::bindCategory(std::string_view const name, std::string_view const helpDescription) const {
    transformationFuncTree->bindCategory(name, helpDescription);
}

} // namespace Nebulite::Module::Base
