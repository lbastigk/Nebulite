/**
 * @file JsonScope.hpp
 * @brief This file contains the definition of the JsonScope class, which provides a scoped interface
 *        for accessing and modifying JSON documents within the Nebulite engine.
 */

#ifndef NEBULITE_DATA_DOCUMENT_JSON_SCOPE_HPP
#define NEBULITE_DATA_DOCUMENT_JSON_SCOPE_HPP

//------------------------------------------
// Includes

// Nebulite
#include "Data/Document/JSON.hpp"

//------------------------------------------
namespace Nebulite::Data {

/**
 * @class Nebulite::Data::JsonScope
 * @brief The JsonScope class provides a scoped interface for accessing and modifying JSON documents.
 * @details It allows for temporary modifications to a JSON document within a specific scope,
 *          that is a key-prefixed section of the document.
 * @todo Proper overload order required. Prioritize string_view?
 * @todo Needs a proper implementation of domain-related stuff
 *       1.) Ensure domains have a JsonScope instead of direct JSON access
 *       2.) Ensure that the JSON domainModules are either part of the scope or part of both JSON and scope
 *       For now, JsonScope is not a Nebulite domain.
 *       A good idea would be to have JSON not be a domain, but JsonScope be one.
 *       This drastically simplifies things, such as construction of JSON documents,
 *       and having a nice wrapper for scoped access, that itself is a domain.
 */
class JsonScope {
    std::variant<std::shared_ptr<JSON>, std::reference_wrapper<JsonScope>> baseDocument;
    std::string scopePrefix;

    // Helper for std::visit with lambdas
    template<class... Ts> struct overloaded : Ts... { using Ts::operator()...; };
    template<class... Ts> overloaded(Ts...) -> overloaded<Ts...>;

    //------------------------------------------
    // Ordered double pointers system

    // TODO: To be implemented: Each JsonScope should have its own ordered double pointers system,
    //       so that expressions and rulesets working within a scope receive the correct pointers.

    /**
     * @brief Mapped ordered double pointers for expression references.
     */
    MappedOrderedDoublePointers expressionRefs[ORDERED_DOUBLE_POINTERS_MAPS];

    /**
     * @brief Super quick double cache based on unique IDs, no hash lookup.
     *        Used for the first few entries. It's recommended to reserve
     *        low value uids for frequently used keys.
     * @todo Add a reserve-mechanism in globalspace for ids, so they are low value.
     */
    std::array<double*, JSON::uidQuickCacheSize> uidDoubleCache{nullptr};

    //------------------------------------------
    // Valid prefix check and generation

    std::string generatePrefix(std::string const& givenPrefix) const {
        std::string fullPrefix = scopePrefix + givenPrefix;
        if (!fullPrefix.empty() && !fullPrefix.ends_with(".")) fullPrefix += ".";
        return fullPrefix;
    }

    std::string generateFullKey(std::string const& key) const {
        std::string full;
        full.reserve(scopePrefix.size() + key.size());
        full = scopePrefix;
        full.append(key);
        return full;
    }

    std::string generateFullKey(std::string_view const& key) const {
        std::string full;
        full.reserve(scopePrefix.size() + key.size());
        full = scopePrefix;
        full.append(key);
        return full;
    }

public:
    //------------------------------------------
    // Constructors

    // Constructing a JsonScope from a JSON document and a prefix
    JsonScope(JSON& doc, std::string const& prefix)
        // create a non-owning shared_ptr to the provided JSON (no delete on destruction)
        : baseDocument(std::shared_ptr<JSON>(&doc, [](JSON*){})), scopePrefix(generatePrefix(prefix)) {}

    // Constructing a JsonScope from another JsonScope and a sub-prefix
    JsonScope(JsonScope& other, std::string const& prefix)
        : baseDocument(std::ref(other)), scopePrefix(prefix) {
        if (!scopePrefix.empty() && !scopePrefix.ends_with(".")) scopePrefix += ".";
    }

    // Default constructor, we create a self-owned empty JSON document
    JsonScope()
        : baseDocument(std::make_shared<JSON>()), scopePrefix("") {
    }

    //------------------------------------------
    // Danger territory: access to base document

    [[deprecated("Accessing the base document breaks the scope abstraction. This should only be used temporarly if certain JSON features aren't available in JsonScope. Use with caution.")]]
    JSON& getBaseDocument() {
        return std::visit(overloaded{
        [&](std::shared_ptr<JSON> const& p) -> JSON& {
                return *p;
        },
        [&](std::reference_wrapper<JsonScope> const& ref) -> JSON& {
                return ref.get().getBaseDocument();
        }
        }, baseDocument);
    }

    //------------------------------------------
    // Sharing a scope

    JsonScope shareScope(std::string const& subPrefix) {
        return JsonScope(*this, generateFullKey(subPrefix));
    }

    //------------------------------------------
    // Getter

    template<typename T>
    T get(std::string const& key, T const& defaultValue) const {
        return std::visit(overloaded{
            [&](std::shared_ptr<JSON> const& p) -> T {
                return p->template get<T>(generateFullKey(key), defaultValue);
            },
            [&](std::reference_wrapper<JsonScope> const& ref) -> T {
                return ref.get().template get<T>(generateFullKey(key), defaultValue);
            }
        }, baseDocument);
    }
    template<typename T>
    T get(std::string_view const& key, T const& defaultValue) const {
        return get<T>(std::string(key), defaultValue);
    }
    template<typename T>
    T get(char const* key, T const& defaultValue) const {
        return get<T>(std::string(key), defaultValue);
    }

    std::optional<RjDirectAccess::simpleValue> getVariant(std::string const& key) const {
        return std::visit(overloaded{
            [&](std::shared_ptr<JSON> const& p) -> std::optional<RjDirectAccess::simpleValue> {
                return p->getVariant(generateFullKey(key));
            },
            [&](std::reference_wrapper<JsonScope> const& ref) -> std::optional<RjDirectAccess::simpleValue> {
                return ref.get().getVariant(generateFullKey(key));
            }
        }, baseDocument);
    }
    std::optional<RjDirectAccess::simpleValue> getVariant(std::string_view const& key) const {
        return getVariant(std::string(key));
    }
    std::optional<RjDirectAccess::simpleValue> getVariant(char const* key) const {
        return getVariant(std::string(key));
    }

    JSON getSubDoc(std::string const& key) const {
        return std::visit(overloaded{
            [&](std::shared_ptr<JSON> const& p) -> JSON {
                return p->getSubDoc(generateFullKey(key));
            },
            [&](std::reference_wrapper<JsonScope> const& ref) -> JSON {
                return ref.get().getSubDoc(generateFullKey(key));
            }
        }, baseDocument);
    }
    JSON getSubDoc(std::string_view const& key) const {
        return getSubDoc(std::string(key));
    }
    JSON getSubDoc(char const* key) const {
        return getSubDoc(std::string(key));
    }

    double* getStableDoublePointer(std::string const& key) {
        return std::visit(overloaded{
            [&](std::shared_ptr<JSON> const& p) -> double* {
                return p->getStableDoublePointer(generateFullKey(key));
            },
            [&](std::reference_wrapper<JsonScope> const& ref) -> double* {
                return ref.get().getStableDoublePointer(generateFullKey(key));
            }
        }, baseDocument);
    }
    double* getStableDoublePointer(std::string_view const& key) {
        return getStableDoublePointer(std::string(key));
    }
    double* getStableDoublePointer(char const* key) {
        return getStableDoublePointer(std::string(key));
    }

    //------------------------------------------
    // Setter

    template<typename T>
    void set(std::string const& key, T const& value) {
        std::visit(overloaded{
            [&](std::shared_ptr<JSON> const& p) {
                p->template set<T>(generateFullKey(key), value);
            },
            [&](std::reference_wrapper<JsonScope> const& ref) {
                ref.get().template set<T>(generateFullKey(key), value);
            }
        }, baseDocument);
    }
    template<typename T>
    void set(std::string_view const& key, T const& value) {
        set<T>(std::string(key), value);
    }
    template<typename T>
    void set(char const* key, T const& value) {
        set<T>(std::string(key), value);
    }

    void setVariant(std::string const& key, RjDirectAccess::simpleValue const& value) {
        std::visit(overloaded{
            [&](std::shared_ptr<JSON> const& p) {
                p->setVariant(generateFullKey(key), value);
            },
            [&](std::reference_wrapper<JsonScope> const& ref) {
                ref.get().setVariant(generateFullKey(key), value);
            }
        }, baseDocument);
    }
    void setVariant(std::string_view const& key, RjDirectAccess::simpleValue const& value) {
        setVariant(std::string(key), value);
    }
    void setVariant(char const* key, RjDirectAccess::simpleValue const& value) {
        setVariant(std::string(key), value);
    }

    void setSubDoc(std::string const& key, JSON& subDoc) {
        std::visit(overloaded{
            [&](std::shared_ptr<JSON> const& p) {
                p->setSubDoc(generateFullKey(key), subDoc);
            },
            [&](std::reference_wrapper<JsonScope> const& ref) {
                ref.get().setSubDoc(generateFullKey(key), subDoc);
            }
        }, baseDocument);
    }
    void setSubDoc(std::string_view const& key, JSON& subDoc) {
        setSubDoc(std::string(key), subDoc);
    }
    void setSubDoc(char const* key, JSON& subDoc) {
        setSubDoc(std::string(key), subDoc);
    }

    void setEmptyArray(std::string const& key) {
        std::visit(overloaded{
            [&](std::shared_ptr<JSON> const& p) {
                p->setEmptyArray(generateFullKey(key));
            },
            [&](std::reference_wrapper<JsonScope> const& ref) {
                ref.get().setEmptyArray(generateFullKey(key));
            }
        }, baseDocument);
    }
    void setEmptyArray(std::string_view const& key) {
        setEmptyArray(std::string(key));
    }
    void setEmptyArray(char const* key) {
        setEmptyArray(std::string(key));
    }

    //------------------------------------------
    // Special sets for threadsafe maths operations

    void set_add(std::string_view const& key, double const& val) {
        std::visit(overloaded{
            [&](std::shared_ptr<JSON> const& p) {
                p->set_add(generateFullKey(key), val);
            },
            [&](std::reference_wrapper<JsonScope> const& ref) {
                ref.get().set_add(generateFullKey(key), val);
            }
        }, baseDocument);
    }

    void set_multiply(std::string_view const& key, double const& val) {
        std::visit(overloaded{
            [&](std::shared_ptr<JSON> const& p) {
                p->set_multiply(generateFullKey(key), val);
            },
            [&](std::reference_wrapper<JsonScope> const& ref) {
                ref.get().set_multiply(generateFullKey(key), val);
            }
        }, baseDocument);
    }

    void set_concat(std::string_view const& key, std::string const& valStr) {
        std::visit(overloaded{
            [&](std::shared_ptr<JSON> const& p) {
                p->set_concat(generateFullKey(key), valStr);
            },
            [&](std::reference_wrapper<JsonScope> const& ref) {
                ref.get().set_concat(generateFullKey(key), valStr);
            }
        }, baseDocument);
    }

    //------------------------------------------
    // Locking

    std::scoped_lock<std::recursive_mutex> lock() {
        return std::visit(overloaded{
            [&](std::shared_ptr<JSON> const& p) -> std::scoped_lock<std::recursive_mutex> {
                return p->lock();
            },
            [&](std::reference_wrapper<JsonScope> const& ref) -> std::scoped_lock<std::recursive_mutex> {
                return ref.get().lock();
            }
        }, baseDocument);
    }

    //------------------------------------------
    // Getters: Unique id based retrieval

    // TODO: Probably best to have an own map for scopes?
    //       otherwise key clashes may happen, as MappedOrderedDoublePointers
    //       itself manages the keys internally.
    //       MappedOrderedDoublePointers* probably needs its own prefixing system!!!
    //       maybe best to pass on construction, more research needed.

    // WARNING: This is not yet implemented!
    [[deprecated("JsonScope::getOrderedCacheListMap is not yet implemented!")]]
    MappedOrderedDoublePointers* getOrderedCacheListMap() {
        return nullptr; // To be implemented
    }

    //------------------------------------------
    // Key Types, Sizes

    JSON::KeyType memberType(std::string const& key) {
        return std::visit(overloaded{
            [&](std::shared_ptr<JSON> const& p) -> JSON::KeyType {
                return p->memberType(generateFullKey(key));
            },
            [&](std::reference_wrapper<JsonScope> const& ref) -> JSON::KeyType {
                return ref.get().memberType(generateFullKey(key));
            }
        }, baseDocument);
    }
    JSON::KeyType memberType(std::string_view key) {
        return memberType(std::string(key));
    }
    JSON::KeyType memberType(char const* key) {
        return memberType(std::string(key));
    }

    size_t memberSize(std::string const& key) {
        return std::visit(overloaded{
            [&](std::shared_ptr<JSON> const& p) -> size_t {
                return p->memberSize(generateFullKey(key));
            },
            [&](std::reference_wrapper<JsonScope> const& ref) -> size_t {
                return ref.get().memberSize(generateFullKey(key));
            }
        }, baseDocument);
    }
    size_t memberSize(std::string_view key) {
        return memberSize(std::string(key));
    }
    size_t memberSize(char const* key) {
        return memberSize(std::string(key));
    }

    void removeKey(std::string_view key) {
        std::visit(overloaded{
            [&](std::shared_ptr<JSON> const& p) {
                p->removeKey(generateFullKey(key).c_str());
            },
            [&](std::reference_wrapper<JsonScope> const& ref) {
                ref.get().removeKey(generateFullKey(key).c_str());
            }
        }, baseDocument);
    }
    void removeKey(std::string const& key) {
        removeKey(static_cast<std::string_view>(key));
    }

    void removeKey(char const* key) {
        removeKey(static_cast<std::string_view>(key));
    }

    //------------------------------------------
    // Serialize/Deserialize

    std::string serialize(std::string const& key = "") {
        return std::visit(overloaded{
            [&](std::shared_ptr<JSON> const& p) -> std::string {
                return p->serialize(generateFullKey(key));
            },
            [&](std::reference_wrapper<JsonScope> const& ref) -> std::string {
                return ref.get().serialize(generateFullKey(key));
            }
        }, baseDocument);
    }
    void deserialize(std::string const& serialOrLink) {
        JSON tmp;
        tmp.deserialize(serialOrLink);
        auto scopePrefixWithoutDot = scopePrefix;
        if (!scopePrefixWithoutDot.empty() && scopePrefixWithoutDot.ends_with(".")) {
            scopePrefixWithoutDot = scopePrefixWithoutDot.substr(0, scopePrefixWithoutDot.size() - 1);
        }
        std::visit(overloaded{
            [&](std::shared_ptr<JSON> const& p) {
                p->setSubDoc(scopePrefixWithoutDot, tmp);
            },
            [&](std::reference_wrapper<JsonScope> const& ref) {
                ref.get().setSubDoc(scopePrefixWithoutDot, tmp);
            }
        }, baseDocument);
    }

};
} // namespace Nebulite::Data
#endif // NEBULITE_DATA_DOCUMENT_JSON_SCOPE_HPP
