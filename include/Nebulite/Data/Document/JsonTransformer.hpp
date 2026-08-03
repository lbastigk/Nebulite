#ifndef NEBULITE_DATA_DOCUMENT_JSONTRANSFORMER_HPP
#define NEBULITE_DATA_DOCUMENT_JSONTRANSFORMER_HPP

//------------------------------------------
// Includes

// Standard library
#include <memory>
#include <span>
#include <string_view>
#include <vector>

// Nebulite
#include "Nebulite/Utility/Args/FuncTree.hpp"

//------------------------------------------
// Forward declarations

namespace Nebulite::Data {
class Json;
class JsonScope;
} // namespace Nebulite::Data

namespace Nebulite::Module::Base {
class TransformationModule;
} // namespace Nebulite::Module::Base

//------------------------------------------
namespace Nebulite::Data {
/**
 * @brief The JsonTransformer class is responsible for applying transformations to JSON values during retrieval.
 * @details It uses a transformation function tree to apply modifications in sequence based on the provided arguments.
 *          Why is this not part of the JsonScope class as DomainModule? Because JSON-Transformations, for the most part, are very destructive as they modify the value of the key in-place.
 *          This is not ideal for a DomainModule, as these destructive transformations are then part of every Domain that inherits from the JSON domain.
 *          We do not want to allow destructive commands such as "length" to be directly available, otherwise users may accidentally overwrite all data from RenderObjects, Textures etc.
 *          Instead, JsonScope DomainModules focus on simple set/move/copy operations, debug utilities and interfaces to better encapsulate destructive actions such as JSON transformations.
 */
class JsonTransformer {
    /**
     * @brief The transformation tree is used to apply modifications to JSON values during getting
     * @details if the key includes the pipe '|' character, we apply the transformations in sequence.
     *          Example: get<int>('MyKey.subKey|strLength|add 1')
     *                   will get the length of the string at MyKey.subKey and add 1 to it.
     *          Takes in a JsonScope pointer as argument to modify.
     *          Returns true on success, false on failure.
     */
    std::shared_ptr<Utility::Args::FuncTree<bool, JsonScope&>> transformationFuncTree;

    /**
     * @brief List of all initialized transformation modules
     */
    std::vector<std::unique_ptr<Module::Base::TransformationModule>> modules;

    /**
     * @brief Initializes a transformation module and adds it to the list of modules.
     * @tparam ModuleType The type of the transformation module to initialize. Must be a subclass of TransformationModule.
     */
    template<typename ModuleType>
    void initModule();

    // Is singleton, no public construction is allowed.
    JsonTransformer();

public:
    // Singleton-Instance
    static JsonTransformer& instance();

    /**
     * @brief Parses and applies JSON transformations from the given arguments.
     * @param transformationList A list of strings representing the transformations to apply.
     * @param jsonDoc The JSON scoped document to modify.
     * @return true if the transformations were successfully applied, false otherwise.
     *         If the value is false, the document should still be considered modified, but in an unknown state.
     */
    bool parse(std::span<std::string_view const> const& transformationList, JsonScope& jsonDoc) const ;

    /**
     * @brief Parses and applies JSON transformations from the given arguments.
     * @param transformationList A list of strings representing the transformations to apply.
     * @param jsonDoc The JSON document to modify.
     * @return true if the transformations were successfully applied, false otherwise.
     *         If the value is false, the document should still be considered modified, but in an unknown state.
     */
    bool parse(std::span<std::string_view const> const& transformationList, Json& jsonDoc) const ;


    /**
     * @brief Parse a single transformation with already separated arguments.
     * @param args The arguments to pass to the function
     * @param jsonDoc The document to manipulate
     * @return true if the transformations were successfully applied, false otherwise.
     */
    bool parseSingleTransformation(std::span<std::string_view const> const& args, JsonScope& jsonDoc) const ;
};
} // namespace Nebulite::Data
#include "Nebulite/Data/Document/JsonTransformer.tpp" // NOLINT(misc-include-cleaner)
#endif // NEBULITE_DATA_DOCUMENT_JSONTRANSFORMER_HPP
