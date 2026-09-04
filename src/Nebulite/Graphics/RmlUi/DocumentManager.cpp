//------------------------------------------
// Includes

// Standard library
#include <unordered_set>

// External
#include <RmlUi/Core/ElementDocument.h>

// Nebulite
#include "Nebulite/Graphics/RmlUi/DocumentManager.hpp"

//------------------------------------------
namespace Nebulite::Graphics::RmlUi {

DocumentManager::DocumentManager() = default;

DocumentManager::~DocumentManager() {
    clearDocuments();
}

void DocumentManager::clearDocuments(){
    for (auto const& doc : openedDocuments) {
        doc->Close();
    }
    openedDocuments.clear();
}

void DocumentManager::OnDocumentLoad(Rml::ElementDocument* document){
    openedDocuments.insert(document);
}

void DocumentManager::OnDocumentUnload(Rml::ElementDocument* document) {
    openedDocuments.erase(document);
}

} // namespace Nebulite::Graphics::RmlUi
