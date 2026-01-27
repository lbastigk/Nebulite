#include "Data/Document/TransformationModule.hpp"

namespace Nebulite::Data {

TransformationModule::TransformationModule(std::shared_ptr<Interaction::Execution::FuncTree<bool, Core::JsonScope*>> funcTree)
    : transformationFuncTree(std::move(funcTree)){}

TransformationModule::~TransformationModule() = default; // out-of-line dtor (key function)

} // namespace Nebulite::Data
