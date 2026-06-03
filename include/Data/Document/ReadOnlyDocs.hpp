#ifndef DATA_DOCUMENT_READONLYDOCS_HPP
#define DATA_DOCUMENT_READONLYDOCS_HPP

//------------------------------------------
// Includes

// Standard library
#include <cstdint> // NOLINT
#include <string>

// Nebulite
#include "Data/Document/JSON.hpp"
#include "Utility/Coordination/SharedMutex.hpp"
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

    mutable Utility::Coordination::SharedMutex docsMutex; // Mutex to protect access to the docs map

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
#endif // DATA_DOCUMENT_READONLYDOCS_HPP
