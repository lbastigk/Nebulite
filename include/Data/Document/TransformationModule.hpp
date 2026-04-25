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

    template <typename Func>
    void bindTransformation(Func functionPtr, std::string_view const& name, std::string_view const& helpDescription) const {
        Interaction::Execution::DomainModuleBase::bindFunctionStatic(transformationFuncTree.get(), functionPtr, name, helpDescription);
    }

    void bindCategory(std::string_view const& name, std::string_view const& helpDescription) const {
        transformationFuncTree->bindCategory(name, helpDescription);
    }

    virtual void bindTransformations() {
        // Basic example of how to bind transformations
        bindTransformation(&TransformationModule::bar, "test1", "Example bind of a static function.\n");
    }

    /**
     * @brief The key string used to store and retrieve the value being modified.
     * @details Uses an empty string as key, so the entire JSON document is the value used.
     * @note DO NOT CHANGE THIS KEY! Needs to stay empty for correct operation.
     */
    static auto constexpr rootKeyStr = "";

    /**
     * @brief The key used to store and retrieve the value being modified.
     * @details Uses an empty string as key, so the entire JSON document is the value used.
     * @note DO NOT CHANGE THIS KEY!
     */
    static auto constexpr rootKey = ScopedKeyView(rootKeyStr);

private:
    std::shared_ptr<Interaction::Execution::FuncTree<bool, JsonScope*>> transformationFuncTree;

    // Example function for binding

    // NOLINTNEXTLINE
    static bool bar(Data::JsonScope* scope) {
        (void)scope;
        return true;
    }
};
} // namespace Nebulite::Data
#endif // NEBULITE_DATA_DOCUMENT_TRANSFORMATION_MODULE_HPP
