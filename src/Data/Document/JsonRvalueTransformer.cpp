//------------------------------------------
// Includes

#include "Nebulite.hpp"
#include "Core/JsonScope.hpp"
#include "Data/Document/JSON.hpp"
#include "Data/Document/JsonRvalueTransformer.hpp"

//------------------------------------------
// Modules
#include "TransformationModule/Arithmetic.hpp"
#include "TransformationModule/Array.hpp"
#include "TransformationModule/Assertions.hpp"
#include "TransformationModule/Boolean.hpp"
#include "TransformationModule/Casting.hpp"
#include "TransformationModule/Collection.hpp"
#include "TransformationModule/Debug.hpp"
#include "TransformationModule/Domain.hpp"
#include "TransformationModule/Statistics.hpp"
#include "TransformationModule/String.hpp"
#include "TransformationModule/Types.hpp"

//------------------------------------------
namespace Nebulite::Data {

JsonRvalueTransformer::JsonRvalueTransformer() {
    transformationFuncTree = std::make_shared<Interaction::Execution::FuncTree<bool, Core::JsonScope*>>("JSON rvalue transformation FuncTree", true, false);

    //------------------------------------------
    // Initialize modules
    initModule<Nebulite::TransformationModule::Arithmetic>();
    initModule<Nebulite::TransformationModule::Array>();
    initModule<Nebulite::TransformationModule::Assertions>();
    initModule<Nebulite::TransformationModule::Boolean>();
    initModule<Nebulite::TransformationModule::Casting>();
    initModule<Nebulite::TransformationModule::Collection>();
    initModule<Nebulite::TransformationModule::Debug>();
    initModule<Nebulite::TransformationModule::Domain>();
    initModule<Nebulite::TransformationModule::Statistics>();
    initModule<Nebulite::TransformationModule::String>();
    initModule<Nebulite::TransformationModule::Types>();

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
