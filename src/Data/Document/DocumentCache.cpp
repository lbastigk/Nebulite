//------------------------------------------
// Includes

// Nebulite
#include "Data/Document/DocumentCache.hpp"

//------------------------------------------
namespace Nebulite::Data {

void DocumentCache::update() const {
    readOnlyDocs.update();
}

double const* DocumentCache::getStableDoublePointer(std::string const& doc_key) const {
    auto [doc, key] = splitDocKey(doc_key);

    ReadOnlyDoc const* docPtr = readOnlyDocs.getDocument(doc);
    if (docPtr == nullptr) {
        // Return a pointer to internal zero if document loading fails
        return &zero;
    }

    // Update the cache (unload old documents)
    update();

    // Return pointer to double value inside the document
    return docPtr->document.getStableDoublePointer(key);
}

} // namespace Nebulite::Data
