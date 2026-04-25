/**
 * @file TransformationModule.hpp
 * @brief Holds specific functions for transforming document values during retrieval.
 */

#ifndef NEBULITE_DATA_DOCUMENT_TRANSFORMATION_MODULE_HPP
#define NEBULITE_DATA_DOCUMENT_TRANSFORMATION_MODULE_HPP

//------------------------------------------
// Includes

// Standard library
#include <memory>

// Nebulite
#include "Data/Document/ScopedKey.hpp"
#include "Interaction/Execution/FuncTree.hpp"
#include "Interaction/Execution/DomainModuleBase.hpp"

//------------------------------------------
// Forward declarations

namespace Nebulite::Data {
class JSON;
} // namespace Nebulite::Data

//------------------------------------------
namespace Nebulite::Data {
class TransformationModule {
public:
    explicit TransformationModule(std::shared_ptr<Interaction::Execution::FuncTree<bool, JsonScope*>> funcTree);

    virtual ~TransformationModule();

    /**
     * @brief Binds a static function to the transformation funcTree
     * @tparam Func The function type to bind
     * @param functionPtr The function to bind
     * @param name The name of the function
     * @param helpDescription The help description of the function
     */
    template <typename Func>
    void bindTransformation(Func functionPtr, std::string_view const& name, std::string_view const& helpDescription) const {
        Interaction::Execution::DomainModuleBase::bindFunctionStatic(transformationFuncTree.get(), functionPtr, name, helpDescription);
    }

    /**
     * @brief Binds a category to the transformation funcTree
     * @param name The name of the category
     * @param helpDescription The help description of the category
     */
    void bindCategory(std::string_view const& name, std::string_view const& helpDescription) const {
        transformationFuncTree->bindCategory(name, helpDescription);
    }

    /**
     * @brief Add all functions to bind here
     */
    virtual void bindTransformations() {}

    /**
     * @brief The key string used to store and retrieve the value being modified.
     * @details Uses an empty string as key, so the entire JSON document is the value used.
     */
    static std::string_view constexpr rootKeyStr = "";

    /**
     * @brief The key used to store and retrieve the value being modified.
     * @details Uses an empty string as key, so the entire JSON document is the value used.
     * @note DO NOT CHANGE THIS KEY!
     */
    static auto constexpr rootKey = ScopedKeyView(rootKeyStr);

private:
    static_assert(rootKeyStr.empty(), "The rootKeyStr must be an empty string for correct operation of the transformation module.");

    std::shared_ptr<Interaction::Execution::FuncTree<bool, JsonScope*>> transformationFuncTree;
};
} // namespace Nebulite::Data
#endif // NEBULITE_DATA_DOCUMENT_TRANSFORMATION_MODULE_HPP
