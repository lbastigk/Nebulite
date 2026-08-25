//------------------------------------------
// Includes

// Standard library
#include <algorithm>
#include <cstddef>
#include <string>
#include <string_view>
#include <utility>

// External
#include <rapidjson/document.h>

// Nebulite
#include "Nebulite/Data/Document/DocumentCache.hpp"
#include "Nebulite/Data/Document/KeyType.hpp"
#include "Nebulite/Data/Document/ReadOnlyDocs.hpp"
#include "Nebulite/Data/Document/RjDirectAccess.hpp"
#include "Nebulite/Interaction/Context.hpp"
#include "Nebulite/Utility/StringHandler.hpp"

//------------------------------------------
namespace Nebulite::Data {

// Basic value retrieval: type,size,serial, etc.

double const* DocumentCache::getStableDoublePointer(std::string_view const docAndKey) const {
    return getValueFromCache<double const*>(docAndKey, &zero, [](ReadOnlyDoc const* docPtr, std::string_view const key) {
        return static_cast<double const*>(docPtr->document.getStableDoublePointer(key));
    });
}

KeyType DocumentCache::memberType(std::string_view const docAndKey) const {
    return getValueFromCache<KeyType>(docAndKey, KeyType::null, [](ReadOnlyDoc const* docPtr, std::string_view const key) {
        return docPtr->document.memberType(key);
    });
}

size_t DocumentCache::memberSize(std::string_view const docAndKey) const {
    return getValueFromCache<size_t>(docAndKey, 0, [](ReadOnlyDoc const* docPtr, std::string_view const key) {
        return docPtr->document.memberSize(key);
    });
}

std::string DocumentCache::serialize(std::string_view const docAndKey) const {
    return getValueFromCache<std::string>(docAndKey, "{}", [](ReadOnlyDoc const* docPtr, std::string_view const key) {
        if (key.empty()) {
            return docPtr->serial;
        }
        Json const subDoc = docPtr->document.getSubDoc(key);
        return subDoc.serialize();
    });
}

// Document serialization

Json DocumentCache::getSubDoc(std::string_view const docAndKey) const {
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

void DocumentCache::copy(rapidjson::Document& dest, std::string_view link) const {
    ReadOnlyDoc const* docPtr = readOnlyDocs.getDocument(link);

    // Check if the document exists in the cache
    if (docPtr == nullptr) {
        dest.SetObject(); // Return empty JSON if document loading fails
        return;
    }

    RjDirectAccess::deserializeFromJson(dest, docPtr->serial);

    // Update the cache (unload old documents)
    readOnlyDocs.update();
}

std::pair<std::string_view, std::string_view> DocumentCache::splitDocKey(std::string_view docAndKey) {
    Utility::StringHandler::strip(docAndKey, ' '); // Remove whitespace for more forgiving input handling

    auto const barPos = docAndKey.find(Json::SpecialCharacter::transformationPipe);
    auto const colonPos = docAndKey.find(Interaction::ContextDeriver::contextKeySeparator);

    // Choose the first occurring separator
    auto const pos = std::min(colonPos, barPos);

    if (pos == std::string::npos) {
        // No colon found, meaning the entire string is document name/link
        return {docAndKey, ""};
    }

    if (pos == barPos) {
        // Return with transformation bar
        // path/to/doc|transform
        // or
        // |transform
        return {
            docAndKey.substr(0, pos),
            docAndKey.substr(pos)
        };
    }
    return {
        // path/to/doc:key
        // or
        // path/to/doc:key|transform
        docAndKey.substr(0, pos), // Doc
        docAndKey.substr(pos + 1) // Key
    };
}

} // namespace Nebulite::Data
