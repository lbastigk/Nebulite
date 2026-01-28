//------------------------------------------
// Includes

#include "Nebulite.hpp"
#include "Core/JsonScope.hpp"
#include "Data/Document/JSON.hpp"
#include "Data/Document/JsonRvalueTransformer.hpp"

//------------------------------------------
// Modules
#include "Data/Document/TransformationModules/Arithmetic.hpp"
#include "Data/Document/TransformationModules/Array.hpp"
#include "Data/Document/TransformationModules/Assertions.hpp"
#include "Data/Document/TransformationModules/Casting.hpp"
#include "Data/Document/TransformationModules/Collection.hpp"
#include "Data/Document/TransformationModules/Debug.hpp"
#include "Data/Document/TransformationModules/Domain.hpp"
#include "Data/Document/TransformationModules/Statistics.hpp"
#include "Data/Document/TransformationModules/String.hpp"
#include "Data/Document/TransformationModules/Types.hpp"

//------------------------------------------
namespace Nebulite::Data {

JsonRvalueTransformer::JsonRvalueTransformer() {
    transformationFuncTree = std::make_shared<Interaction::Execution::FuncTree<bool, Core::JsonScope*>>("JSON rvalue transformation FuncTree", true, false);

    //------------------------------------------
    // Initialize modules
    initModule<TransformationModules::Arithmetic>();
    initModule<TransformationModules::Array>();
    initModule<TransformationModules::Assertions>();
    initModule<TransformationModules::Casting>();
    initModule<TransformationModules::Collection>();
    initModule<TransformationModules::Debug>();
    initModule<TransformationModules::Domain>();
    initModule<TransformationModules::Statistics>();
    initModule<TransformationModules::String>();
    initModule<TransformationModules::Types>();

    //------------------------------------------
    // Bind all transformations
    for (auto const& module : modules) {
        module->bindTransformations();
    }
}

bool JsonRvalueTransformer::parse(std::vector<std::string> const& args, Core::JsonScope* jsonDoc) const {
    static std::string const funcName = __FUNCTION__;
    if (args.empty()) {
        return false;
    }
    return std::ranges::all_of(args, [&](std::string const& transformation) {
        std::string const call = funcName + " " + transformation;
        return transformationFuncTree->parseStr(call, jsonDoc);
    });
}

bool JsonRvalueTransformer::parse(std::vector<std::string> const& args, JSON* jsonDoc) const {
    auto scope = jsonDoc->shareScope();
    static std::string const funcName = __FUNCTION__;
    if (args.empty()) {
        return false;
    }
    return std::ranges::all_of(args, [&](std::string const& transformation) {
        std::string const call = funcName + " " + transformation;
        return transformationFuncTree->parseStr(call, &scope);
    });
}

} // namespace Nebulite::Data
