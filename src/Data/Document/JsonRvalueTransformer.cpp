//------------------------------------------
// Includes

#include "Nebulite.hpp"
#include "Data/Document/JsonScope.hpp"
#include "Data/Document/JSON.hpp"
#include "Data/Document/JsonRvalueTransformer.hpp"

//------------------------------------------
// Modules
#include "Module/Transformation/Arithmetic.hpp"
#include "Module/Transformation/Array.hpp"
#include "Module/Transformation/Assertions.hpp"
#include "Module/Transformation/Boolean.hpp"
#include "Module/Transformation/Casting.hpp"
#include "Module/Transformation/Collection.hpp"
#include "Module/Transformation/Debug.hpp"
#include "Module/Transformation/General.hpp"
#include "Module/Transformation/Requirements.hpp"
#include "Module/Transformation/Statistics.hpp"
#include "Module/Transformation/String.hpp"
#include "Module/Transformation/Types.hpp"

//------------------------------------------
namespace Nebulite::Data {

JsonRvalueTransformer::JsonRvalueTransformer() {
    transformationFuncTree = std::make_shared<Interaction::Execution::FuncTree<bool, JsonScope*>>("JSON rvalue transformation FuncTree", true, false, Global::capture()); // Pass to main capture

    //------------------------------------------
    // Initialize modules
    initModule<Nebulite::TransformationModule::Arithmetic>();
    initModule<Nebulite::TransformationModule::Array>();
    initModule<Nebulite::TransformationModule::Assertions>();
    initModule<Nebulite::TransformationModule::Boolean>();
    initModule<Nebulite::TransformationModule::Casting>();
    initModule<Nebulite::TransformationModule::Collection>();
    initModule<Nebulite::TransformationModule::Debug>();
    initModule<Nebulite::TransformationModule::General>();
    initModule<Nebulite::TransformationModule::Requirements>();
    initModule<Nebulite::TransformationModule::Statistics>();
    initModule<Nebulite::TransformationModule::String>();
    initModule<Nebulite::TransformationModule::Types>();

    //------------------------------------------
    // Bind all transformations
    for (auto const& module : modules) {
        module->bindTransformations();
    }
}

bool JsonRvalueTransformer::parse(std::vector<std::string> const& args, JsonScope* jsonDoc) const {
    static std::string const funcName = __FUNCTION__;
    if (args.empty()) [[unlikely]] {
        return false;
    }

    // Pre-allocate string to avoid reallocations in the loop
    std::string call;
    call.reserve(funcName.size() + 1 + 128); // funcName + space + typical transformation size

    return std::ranges::all_of(args, [&](std::string const& transformation) {
        call.clear();
        call.append(funcName);
        call.push_back(' ');
        call.append(transformation);
        return transformationFuncTree->parseStr(call, jsonDoc);
    });
}

bool JsonRvalueTransformer::parse(std::vector<std::string> const& args, JSON* jsonDoc) const {
    auto& scope = jsonDoc->fullScopeBase();
    static std::string const funcName = __FUNCTION__;
    if (args.empty()) [[unlikely]] {
        return false;
    }

    // Pre-allocate string to avoid reallocations in the loop
    std::string call;
    call.reserve(funcName.size() + 1 + 128); // funcName + space + typical transformation size

    return std::ranges::all_of(args, [&](std::string const& transformation) {
        call.clear();
        call.append(funcName);
        call.push_back(' ');
        call.append(transformation);
        return transformationFuncTree->parseStr(call, &scope);
    });
}


} // namespace Nebulite::Data
