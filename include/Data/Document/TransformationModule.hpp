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
#include "Constants/Asserts.hpp"
#include "Data/Document/ScopedKey.hpp"
#include "Interaction/Execution/FuncTree.hpp"
#include "Interaction/Execution/DomainModuleBase.hpp"

//------------------------------------------
// Forward declarations

namespace Nebulite::Core {
class JsonScope;
} // namespace Nebulite::Core

namespace Nebulite::Data {
class JSON;
} // namespace Nebulite::Data

//------------------------------------------
// Binding helper macro

// Bind non-static member function
#define BIND_TRANSFORMATION_MEMBER(foo, name, desc) \
static_assert(::Nebulite::Constants::Assert::endsWithNewline(desc), "Description must end with a newline character"); \
Interaction::Execution::DomainModuleBase::bindFunctionStatic(transformationFuncTree.get(), this, foo, name, desc)

// Bind static/free function
#define BIND_TRANSFORMATION_STATIC(foo, name, desc) \
static_assert(::Nebulite::Constants::Assert::endsWithNewline(desc), "Description must end with a newline character"); \
Interaction::Execution::DomainModuleBase::bindFunctionStatic(transformationFuncTree.get(), foo, name, desc)

//------------------------------------------
namespace Nebulite::Data {
class TransformationModule {
public:
    explicit TransformationModule(std::shared_ptr<Interaction::Execution::FuncTree<bool, Core::JsonScope*>> funcTree);

    virtual ~TransformationModule();

    virtual void bindTransformations() {
        // Basic example of how to bind transformations
        BIND_TRANSFORMATION_STATIC(&TransformationModule::bar, "test1", "Example bind of a static function.\n");
        BIND_TRANSFORMATION_MEMBER(&TransformationModule::baz, "test2", "Example bind of a member function.\n");
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

protected:
    std::shared_ptr<Interaction::Execution::FuncTree<bool, Core::JsonScope*>> transformationFuncTree;

    // Example functions for binding

private:
    // NOLINTNEXTLINE
    static bool bar(Core::JsonScope* scope) {
        (void)scope;
        return true;
    }

    // NOLINTNEXTLINE
    bool baz(Core::JsonScope* scope) {
        (void)scope;
        i++;
        return true;
    }
    int i = 0;
};
} // namespace Nebulite::Data
#endif // NEBULITE_DATA_DOCUMENT_TRANSFORMATION_MODULE_HPP
