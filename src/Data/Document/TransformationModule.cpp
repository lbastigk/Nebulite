#include "Data/Document/TransformationModule.hpp"
#include "Interaction/Logic/Expression.hpp"

namespace Nebulite::Data {

TransformationModule::TransformationModule(std::shared_ptr<Interaction::Execution::FuncTree<bool, JsonScope*>> funcTree)
    : transformationFuncTree(std::move(funcTree)){}

TransformationModule::~TransformationModule() = default;

std::string TransformationModule::handlePotentiallyWrappedString(std::span<std::string const> const& args){
    // Remove all outer anti-eval wrappers
    auto str = Interaction::Logic::Expression::removeOuterAntiEvalWrapper(args);

    // If the resulting string is still wrapped in braces, remove them as well
    if (str.starts_with("{") && str.ends_with("}")) {
        str = str.substr(1, str.size() - 2); // Remove outer braces if present
    }
    return str;
}

} // namespace Nebulite::Data
