#ifndef NEBULITE_DATA_DOCUMENT_JSONTRANSFORMER_TPP
#define NEBULITE_DATA_DOCUMENT_JSONTRANSFORMER_TPP

//------------------------------------------
// Includes

// Standard library
#include <memory>
#include <type_traits>
#include <vector>

//------------------------------------------
// Conditional includes

#ifndef NEBULITE_DATA_DOCUMENT_JSONTRANSFORMER_HPP
#include "JsonTransformer.hpp"
#endif // NEBULITE_DATA_DOCUMENT_JSONTRANSFORMER_HPP

//------------------------------------------
namespace Nebulite::Data {
template<typename ModuleType>
void JsonTransformer::initModule() {
    static_assert(std::is_base_of_v<Module::Base::TransformationModule, ModuleType>, "ModuleType must be a subclass of TransformationModule");
    modules.emplace_back(std::make_unique<ModuleType>(transformationFuncTree));
}
} // namespace Nebulite::Data
#endif // NEBULITE_DATA_DOCUMENT_JSONTRANSFORMER_TPP
