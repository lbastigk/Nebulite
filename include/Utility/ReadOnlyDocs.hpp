/**
 * @file ReadOnlyDocs.hpp
 * @brief Defines structures for managing read-only documents.
 */
#ifndef NEBULITE_UTILITY_READONLYDOCS_HPP
#define NEBULITE_UTILITY_READONLYDOCS_HPP

//------------------------------------------
// Includes

// Standard library
#include <stdexcept>
#include <random>

// Nebulite
#include "Utility/JSON.hpp"
#include "Utility/FileManagement.hpp"
#include "Utility/TimeKeeper.hpp"
#include "Utility/Capture.hpp"

//------------------------------------------
namespace Nebulite::Utility {

/**
 * @struct ReadOnlyDoc
 * @brief Represents a read-only document with its associated metadata.
 */
struct __attribute__((aligned(2*sizeof(uint64_t)))) ReadOnlyDoc {
    Nebulite::Utility::JSON document; // The actual JSON document
    Nebulite::Utility::TimeKeeper lastUsed;

    explicit ReadOnlyDoc(Nebulite::Core::GlobalSpace* globalSpace) : document(globalSpace), lastUsed(){}
};

/**
 * @brief Map of document paths to their corresponding ReadOnlyDoc instances.
 * 
 * Takes care of loading and unloading documents as needed.
 */
struct ReadOnlyDocs{
    private:
        /**
         * @brief Time in milliseconds after which unused documents are unloaded.
         * 
         * Documents that have not been accessed within this time frame will be removed from the cache to free up memory.
         */
        const uint64_t unloadTime = 5 * Nebulite::Utility::Time::Conversion::millisecondsPerMinute; // Unload documents after 5 minutes of inactivity

        /**
         * @brief Contains the cached documents mapped by their file paths.
         */
        absl::flat_hash_map<std::string, ReadOnlyDoc> docs;

        /**
         * @brief Reference to the global space for document creation.
         */
        Nebulite::Core::GlobalSpace* globalSpace;

    public:
        /**
         * @brief Constructor that takes a GlobalSpace pointer.
         */
        explicit ReadOnlyDocs(Nebulite::Core::GlobalSpace* global) : globalSpace(global){
            // Validate that globalSpace is not null
            if (globalSpace == nullptr){
                throw std::invalid_argument("DocumentCache: GlobalSpace pointer cannot be null");
            }
        }

        /**
         * @brief Updates the cache by checking a random document for its last usage time,
         * and unloading it if it has been unused for too long.
         */
        void update(){
            if(docs.empty()){
                return; // No documents to check
            }

            // Check the last used time of a random document
            auto it = docs.begin();
            thread_local std::mt19937_64 rng{std::random_device{}()};
            std::uniform_int_distribution<std::size_t> dist(0, docs.size() - 1);
            std::advance(it, dist(rng));
            ReadOnlyDoc* docPtr = &it->second;

            // If the document has not been used recently, unload it
            if (docPtr->lastUsed.projected_dt() > unloadTime){
                docs.erase(it);
            }
        }

        /**
         * @brief Proper retrieval of a document, loading it if not already cached.
         * And updating its metadata.
         * 
         * @param doc The link to the document.
         * @return Pointer to the ReadOnlyDoc, or nullptr if loading fails.
         */
        ReadOnlyDoc* getDocument(std::string const& doc){
            // Validate inputs and state
            if (doc.empty()){
                return nullptr;
            }
            if (globalSpace == nullptr){
                return nullptr;
            }
            
            // Check if the document exists in the cache
            auto it = docs.find(doc);
            if (it == docs.end()){
                // Load the document if it doesn't exist
                std::string const serial = Nebulite::Utility::FileManagement::LoadFile(doc);
                if (serial.empty()){
                    return nullptr; // Return nullptr if document loading fails
                }
                auto result = docs.emplace(doc, ReadOnlyDoc(globalSpace));
                if (!result.second){
                    // Emplace failed for some reason
                    return nullptr;
                }
                result.first->second.document.deserialize(serial);
                it = result.first;
            }
            ReadOnlyDoc* docPtr = &it->second;
            docPtr->lastUsed.update();
            return docPtr;
        }
};
} // namespace Nebulite::Utility
#endif // NEBULITE_UTILITY_READONLYDOCS_HPP