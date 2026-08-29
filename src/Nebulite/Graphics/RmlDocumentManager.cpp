//------------------------------------------
// Includes

// Standard library
#include <unordered_set>

// External
#include <RmlUi/Core/ElementDocument.h>

// Nebulite
#include "Nebulite/Graphics/RmlDocumentManager.hpp"

//------------------------------------------
namespace Nebulite::Graphics {

RmlDocumentManager::RmlDocumentManager() = default;

RmlDocumentManager::~RmlDocumentManager() {
    clearDocuments();
}

void RmlDocumentManager::clearDocuments(){
    for (auto const& doc : openedDocuments) {
        doc->Close();
    }
    openedDocuments.clear();
}

void RmlDocumentManager::OnDocumentLoad(Rml::ElementDocument* document){
    openedDocuments.insert(document);
}

void RmlDocumentManager::OnDocumentUnload(Rml::ElementDocument* document) {
    openedDocuments.erase(document);
}

} // namespace Nebulite::Graphics
