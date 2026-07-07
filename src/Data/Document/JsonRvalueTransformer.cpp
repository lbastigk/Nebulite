//------------------------------------------
// Includes

// Standard library
#include <algorithm>
#include <memory>
#include <span>
#include <string>
#include <string_view>
#include <vector>

// Nebulite
#include "Nebulite/Data/Document/JSON.hpp"
#include "Nebulite/Data/Document/JsonRvalueTransformer.hpp"
#include "Nebulite/Data/Document/JsonScope.hpp"
#include "Nebulite/Interaction/Execution/FuncTree.hpp"
#include "Nebulite/Nebulite.hpp"

// Nebulite: Transformation modules
#include "Nebulite/Module/Transformation/Arithmetic.hpp"
#include "Nebulite/Module/Transformation/Array.hpp"
#include "Nebulite/Module/Transformation/Assertions.hpp"
#include "Nebulite/Module/Transformation/Boolean.hpp"
#include "Nebulite/Module/Transformation/Casting.hpp"
#include "Nebulite/Module/Transformation/Collection.hpp"
#include "Nebulite/Module/Transformation/Debug.hpp"
#include "Nebulite/Module/Transformation/Domain.hpp"
#include "Nebulite/Module/Transformation/Filter.hpp"
#include "Nebulite/Module/Transformation/General.hpp"
#include "Nebulite/Module/Transformation/Requirements.hpp"
#include "Nebulite/Module/Transformation/Sort.hpp"
#include "Nebulite/Module/Transformation/Statistics.hpp"
#include "Nebulite/Module/Transformation/String.hpp"
#include "Nebulite/Module/Transformation/Types.hpp"

//------------------------------------------
namespace Nebulite::Data {

JsonRvalueTransformer::JsonRvalueTransformer() {
    transformationFuncTree = std::make_shared<Interaction::Execution::FuncTree<bool, JsonScope&>>(
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
    initModule<Module::Transformation::Domain>();
    initModule<Module::Transformation::Filter>();
    initModule<Module::Transformation::General>();
    initModule<Module::Transformation::Requirements>();
    initModule<Module::Transformation::Sort>();
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

bool JsonRvalueTransformer::parse(std::vector<std::string_view> const& transformationList, JsonScope& jsonDoc) const {
    static std::string constexpr funcName = __FUNCTION__;
    if (transformationList.empty()) [[unlikely]] {
        return false;
    }

    // Pre-allocate string to avoid reallocations in the loop
    std::string call;
    call.reserve(funcName.size() + 1 + 128); // funcName + space + typical transformation size

    return std::ranges::all_of(transformationList, [&](std::string_view const transformation) {
        call.clear();
        call.append(funcName);
        call.push_back(' ');
        call.append(transformation);
        return transformationFuncTree->parseStr(call, jsonDoc);
    });
}

bool JsonRvalueTransformer::parse(std::vector<std::string_view> const& transformationList, JSON& jsonDoc) const {
    auto& scope = jsonDoc.fullScope();
    return parse(transformationList, scope);
}

bool JsonRvalueTransformer::parseSingleTransformation(std::span<std::string_view const> const& args, JsonScope& jsonDoc) const {
    return transformationFuncTree->parse(args, jsonDoc);
}

} // namespace Nebulite::Data
