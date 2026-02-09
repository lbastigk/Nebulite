#include "Nebulite.hpp"
#include "Data/Document/ReadOnlyDocs.hpp"

namespace {
void docLoadingFailedMessage(std::string const& doc) {
    Nebulite::Error::println("Nebulite failed to load document: " + doc);
    Nebulite::Error::println("Please check if the file exists and is accessible.");
    Nebulite::Error::println("If the string wasn't mean to be a file path, perhaps you forgot to wrap the string in an anti-evaluation wrapper?");
    Nebulite::Error::println("Instead of writing: {...} in transformations or other nested expressions, try writing: {!...}");
    Nebulite::Error::println("This prevents the variable from being evaluated, and is passed further down the line as a string.");
    Nebulite::Error::println("Example:");
    Nebulite::Error::println("eval echo {global.test|parse json set size {!./Resources/Renderobjects/standard.jsonc:size}|get size.X|toInt}");
    Nebulite::Error::println("The anti-evaluation wrapper makes sure the link is evaluated at the right time.");
    Nebulite::Error::println("Without the anti-evaluation wrapper, eval would try to evaluate a JSON object 'size' as string, which would fail");
    Nebulite::Error::println("Another example that triggers this error:");
    // NOLINTNEXTLINE
    Nebulite::Error::println("eval nop {global.arr|filterRegex {!\[(1\d*)\]}|filterNulls|print}");
    Nebulite::Error::println("The anti-evaluation wrapper prevents the regex from being parsed incorrectly due to quotes, brackets etc");
    Nebulite::Error::println("and makes sure that eval doesn't try to evaluate the regex, which would fail and trigger this error message as no file named '\[(1\d*)\]' exists.");
}
} // anonymous namespace


namespace Nebulite::Data {

void ReadOnlyDocs::update(){
    if(docs.empty()){
        return; // No documents to check
    }

    // Check the last used time of a random document
    auto it = docs.begin();
    thread_local std::mt19937_64 rng{std::random_device{}()};
    std::uniform_int_distribution<std::size_t> dist(0, docs.size() - 1);
    std::advance(it, dist(rng));

    // If the document has not been used recently, unload it
    if (ReadOnlyDoc* docPtr = &it->second; docPtr->lastUsed.projected_dt() > unloadTime){
        docs.erase(it);
    }
}

ReadOnlyDoc* ReadOnlyDocs::getDocument(std::string const& doc){
    // Validate inputs and state
    if (doc.empty()){
        docLoadingFailedMessage(doc);
        return nullptr;
    }
    // Check if the document exists in the cache
    auto it = docs.find(doc);
    if (it == docs.end()){
        // Load the document if it doesn't exist
        std::string const serial = Utility::FileManagement::LoadFile(doc);
        if (serial.empty()){
            docLoadingFailedMessage(doc);
            return nullptr; // Return nullptr if document loading fails
        }
        auto [newIt, result] = docs.emplace(doc, ReadOnlyDoc());
        if (!result){
            // Emplace failed for some reason
            docLoadingFailedMessage(doc);
            return nullptr;
        }
        newIt->second.document.deserialize(serial);
        newIt->second.serial = serial;
        it = newIt;
    }
    ReadOnlyDoc* docPtr = &it->second;
    docPtr->lastUsed.update();
    return docPtr;
}

} // namespace Nebulite::Data
