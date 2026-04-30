/**
 * @file ReadOnlyDocs.hpp
 * @brief Defines structures for managing read-only documents.
 */
#ifndef NEBULITE_DATA_DOCUMENT_READ_ONLY_DOCS_HPP
#define NEBULITE_DATA_DOCUMENT_READ_ONLY_DOCS_HPP

//------------------------------------------
// Includes

// Standard library
#include <random>

// Nebulite
#include "Data/Document/JSON.hpp"
#include "Utility/TimeKeeper.hpp"

//------------------------------------------
namespace Nebulite::Data {

/**
 * @struct ReadOnlyDoc
 * @brief Represents a read-only document with its associated metadata.
 */
struct ReadOnlyDoc {
    JSON document;          // The actual JSON document
    Utility::TimeKeeper lastUsed; // TimeKeeper to track last access time
    std::string serial;
    ReadOnlyDoc() = default;
};

/**
 * @brief Map of document paths to their corresponding ReadOnlyDoc instances.
 * @details Takes care of loading and unloading documents as needed.
 */
struct ReadOnlyDocs{
private:
    /**
     * @brief Time in milliseconds after which unused documents are unloaded.
     * @details Documents that have not been accessed within this time frame will be removed from the cache to free up memory.
     *          Unload documents after 5 minutes of inactivity
     */
    static constexpr uint64_t unloadTime = 5U * Utility::Time::Conversion::millisecondsPerMinute;

    /**
     * @brief Contains the cached documents mapped by their file paths.
     */
    mutable absl::flat_hash_map<std::string, ReadOnlyDoc> docs;

public:
    /**
     * @brief Constructor that takes a GlobalSpace pointer.
     */
    ReadOnlyDocs() = default;

    /**
     * @brief Updates the cache by checking a random document for its last usage time,
     *        and unloading it if it has been unused for too long.
     */
    void update() const ;

    /**
     * @brief Proper retrieval of a document, loading it if not already cached and updating its metadata.
     * @param doc The link to the document.
     * @return Pointer to the ReadOnlyDoc, or nullptr if loading fails.
     *         If loading fails, writes an error message to the console.
     */
    ReadOnlyDoc* getDocument(std::string_view const& doc) const ;
};
} // namespace Nebulite::Data
#endif // NEBULITE_DATA_DOCUMENT_READ_ONLY_DOCS_HPP
