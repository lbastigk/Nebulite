//------------------------------------------
// Includes

// Standard library
#include <algorithm>
#include <cstddef>
#include <string>
#include <string_view>
#include <utility>

// Nebulite
#include "Nebulite/Data/Document/DocumentCache.hpp"
#include "Nebulite/Data/Document/KeyType.hpp"
#include "Nebulite/Data/Document/ReadOnlyDocs.hpp"
#include "Nebulite/Interaction/Context.hpp"
#include "Nebulite/Utility/StringHandler.hpp"

//------------------------------------------
namespace Nebulite::Data {

// Basic value retrieval: type,size,serial, etc.

double const* DocumentCache::getStableDoublePointer(std::string const& docAndKey) const {
    return getValueFromCache<double const*>(docAndKey, &zero, [](ReadOnlyDoc const* docPtr, std::string_view const key) {
        return docPtr->document.getStableDoublePointer(key);
    });
}

KeyType DocumentCache::memberType(std::string const& docAndKey) const {
    return getValueFromCache<KeyType>(docAndKey, KeyType::null, [](ReadOnlyDoc const* docPtr, std::string_view const key) {
        return docPtr->document.memberType(key);
    });
}

size_t DocumentCache::memberSize(std::string const& docAndKey) const {
    return getValueFromCache<size_t>(docAndKey, 0, [](ReadOnlyDoc const* docPtr, std::string_view const key) {
        return docPtr->document.memberSize(key);
    });
}

std::string DocumentCache::serialize(std::string const& docAndKey) const {
    return getValueFromCache<std::string>(docAndKey, "{}", [](ReadOnlyDoc const* docPtr, std::string_view const key) {
        if (key.empty()) {
            return docPtr->serial;
        }
        Json const subDoc = docPtr->document.getSubDoc(key);
        return subDoc.serialize();
    });
}

// Document serialization

Json DocumentCache::getSubDoc(std::string const& docAndKey) const {
    auto [doc, key] = splitDocKey(docAndKey);

    ReadOnlyDoc const* docPtr = readOnlyDocs.getDocument(doc);
    if (!docPtr) {
        return Json{};
    }

    // Check if the document exists in the cache
    Json data = docPtr->document.getSubDoc(key);

    // Update the cache (unload old documents) and return the size
    readOnlyDocs.update();
    return data;
}

std::string DocumentCache::getDocString(std::string_view const link) const {
    ReadOnlyDoc const* docPtr = readOnlyDocs.getDocument(link);

    // Check if the document exists in the cache
    if (docPtr == nullptr) {
        return Json().serialize(); // Return empty JSON if document loading fails
    }

    // Return string of document:
    std::string serial = docPtr->serial;

    // Update the cache (unload old documents)
    readOnlyDocs.update();

    return serial;
}

std::pair<std::string, std::string> DocumentCache::splitDocKey(std::string const& docAndKey) {
    std::string_view docAndKeyView(docAndKey);
    Utility::StringHandler::strip(docAndKeyView, ' '); // Remove whitespace for more forgiving input handling

    auto const barPos = docAndKeyView.find(Json::SpecialCharacter::transformationPipe);
    auto const colonPos = docAndKeyView.find(Interaction::ContextDeriver::contextKeySeparator);

    // Choose the first occurring separator
    auto const pos = std::min(colonPos, barPos);

    if (pos == std::string::npos) {
        // No colon found, meaning the entire string is document name/link
        return {std::string(docAndKeyView), ""};
    }
    auto const doc = docAndKeyView.substr(0, pos);
    auto const key = docAndKeyView.substr(pos + 1);

    // Add back the transform part if needed
    if (pos == barPos) {
        return {std::string(doc), Json::SpecialCharacter::transformationPipe + std::string(key)};
    }
    return {std::string(doc), std::string(key)};
}

} // namespace Nebulite::Data
