#include "Data/Document/TransformationModule.hpp"
#include "Interaction/Logic/Expression.hpp"

namespace Nebulite::Data {

TransformationModule::TransformationModule(std::shared_ptr<Interaction::Execution::FuncTree<bool, JsonScope*>> funcTree)
    : transformationFuncTree(std::move(funcTree)){}

TransformationModule::~TransformationModule() = default;

} // namespace Nebulite::Data
