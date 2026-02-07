#include "Data/Document/TransformationModule.hpp"

namespace Nebulite::Data {

TransformationModule::TransformationModule(std::shared_ptr<Interaction::Execution::FuncTree<bool, Core::JsonScope*>> funcTree)
    : transformationFuncTree(std::move(funcTree)){}

TransformationModule::~TransformationModule() = default; // out-of-line dtor (key function)

std::string TransformationModule::extractPotentiallyWrappedString(std::span<std::string const> const& args){
    if (args.empty()) {
        return "";
    }
    if (args.front().starts_with("{!") && args.back().ends_with('}')) {
        auto const full = Utility::StringHandler::recombineArgs(args);
        return std::string(full.data() + 2, full.size() - 3);
    }
    return Utility::StringHandler::recombineArgs(args);
}

} // namespace Nebulite::Data
