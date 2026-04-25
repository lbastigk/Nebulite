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
    transformationFuncTree = std::make_shared<Interaction::Execution::FuncTree<bool, JsonScope*>>(
        "JSON rvalue transformation FuncTree",
        true,
        false,
        Global::capture()
    );

    //------------------------------------------
    // Initialize modules
    initModule<Module::Transformation::Arithmetic>();
    initModule<Module::Transformation::Array>();
    initModule<Module::Transformation::Assertions>();
    initModule<Module::Transformation::Boolean>();
    initModule<Module::Transformation::Casting>();
    initModule<Module::Transformation::Collection>();
    initModule<Module::Transformation::Debug>();
    initModule<Module::Transformation::General>();
    initModule<Module::Transformation::Requirements>();
    initModule<Module::Transformation::Statistics>();
    initModule<Module::Transformation::String>();
    initModule<Module::Transformation::Types>();

    //------------------------------------------
    // Bind all transformations
    for (auto const& module : modules) {
        module->bindTransformations();
    }
}

JsonRvalueTransformer& JsonRvalueTransformer::instance() {
    static JsonRvalueTransformer instance;
    return instance;
}

bool JsonRvalueTransformer::parse(std::vector<std::string> const& transformationList, JsonScope* jsonDoc) const {
    static std::string constexpr funcName = __FUNCTION__;
    if (transformationList.empty()) [[unlikely]] {
        return false;
    }

    // Pre-allocate string to avoid reallocations in the loop
    std::string call;
    call.reserve(funcName.size() + 1 + 128); // funcName + space + typical transformation size

    return std::ranges::all_of(transformationList, [&](std::string const& transformation) {
        call.clear();
        call.append(funcName);
        call.push_back(' ');
        call.append(transformation);
        return transformationFuncTree->parseStr(call, jsonDoc);
    });
}

bool JsonRvalueTransformer::parseSingleTransformation(std::span<std::string const> const& args, JsonScope* jsonDoc) const {
    return transformationFuncTree->parse(args, jsonDoc);
}

bool JsonRvalueTransformer::parse(std::vector<std::string> const& transformationList, JSON* jsonDoc) const {
    auto& scope = jsonDoc->fullScopeBase();
    return parse(transformationList, &scope);
}


} // namespace Nebulite::Data
