/**
 * @file JsonRvalueTransformer.hpp
 * @brief Data class for modifying JSON return value keys.
 * @details JSON(key) -> value | transformation on value | transformation on value ... -> new value
 *          This allows for dynamic modification of JSON values during retrieval.
 */

#ifndef NEBULITE_DATA_JSON_RVALUE_TRANSFORMER_HPP
#define NEBULITE_DATA_JSON_RVALUE_TRANSFORMER_HPP

//------------------------------------------
// Includes

// Standard library
#include <memory>
#include <string>
#include <type_traits>
#include <vector>

// Nebulite
#include "Interaction/Execution/FuncTree.hpp"

//------------------------------------------
// Forward declarations

namespace Nebulite::Core {
class JsonScope;
} // namespace Nebulite::Core

namespace Nebulite::Data {
class JSON;
class TransformationModule;
} // namespace Nebulite::Data

//------------------------------------------
namespace Nebulite::Data {
/**
 * @brief The JsonRvalueTransformer class is responsible for applying transformations to JSON values during retrieval.
 * @details It uses a transformation function tree to apply modifications in sequence based on the provided arguments.
 *          Why is this not part of the JsonScope class as DomainModule? Because JSON-Transformations, for the most part, are very destructive as they modify the value of the key in-place.
 *          This is not ideal for a DomainModule, as these destructive transformations are then part of every Domain that inherits from the JSON domain.
 *          We do not want to allow destructive commands such as "length" to be directly available, otherwise users may accidentally overwrite all data from RenderObjects, Textures etc.
 *          Instead, JsonScope DomainModules focus on simple set/move/copy operations, debug utilities and interfaces to better encapsulate destructive actions such as JSON transformations.
 */
class JsonRvalueTransformer {
    /**
     * @brief The transformation tree is used to apply modifications to JSON values during getting
     * @details if the key includes the pipe '|' character, we apply the transformations in sequence.
     *          Example: get<int>('MyKey.subKey|strLength|add 1')
     *                   will get the length of the string at MyKey.subKey and add 1 to it.
     *          Takes in a JsonScope pointer as argument to modify.
     *          Returns true on success, false on failure.
     */
    std::shared_ptr<Interaction::Execution::FuncTree<bool, Core::JsonScope*>> transformationFuncTree;

    /**
     * @brief List of all initialized transformation modules
     */
    std::vector<std::unique_ptr<TransformationModule>> modules;

    /**
     * @brief Initializes a transformation module and adds it to the list of modules.
     * @tparam ModuleType The type of the transformation module to initialize. Must be a subclass of TransformationModule.
     */
    template<typename ModuleType> void initModule() {
        static_assert(std::is_base_of_v<TransformationModule, ModuleType>, "ModuleType must be a subclass of TransformationModule");
        modules.emplace_back(std::make_unique<ModuleType>(transformationFuncTree));
    }

public:
    JsonRvalueTransformer();

    /**
     * @brief Parses and applies JSON transformations from the given arguments.
     * @details Example: get<int>("MyKey|add 5|multiply 2")
     *          Calls: parse(["add 5", "multiply 2"], inputDocument)
     *          Input document: {"value": 10}
     *          After applying the transformation, the document will be: {"value": 30}
     * @param args A list of arguments representing the transformations to apply.
     * @param jsonDoc The JSON document to modify. Should hold the value from the get-operation
     *                in a specified key. On success, the modified value will be stored back in the same key.
     * @return true if the transformations were successfully applied, false otherwise.
     */
    bool parse(std::vector<std::string> const& args, Core::JsonScope* jsonDoc) const ;
    bool parse(std::vector<std::string> const& args, JSON* jsonDoc) const ;
};
} // namespace Nebulite::Data
#endif // NEBULITE_DATA_JSON_RVALUE_TRANSFORMER_HPP
