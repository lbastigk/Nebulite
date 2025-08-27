/**
 * @file InvokeJSONParser.h
 * 
 * This file contains the static InvokeJSONParser class, which is responsible for parsing
 * JSON documents into InvokeEntry objects.
 */

#pragma once
#include "Utility/JSON.h"
#include "Core/RenderObject.h"

namespace Nebulite{
namespace Interaction{

class Invoke;

/**
 * @class Nebulite::Interaction::InvokeJSONParser
 * 
 * @brief A utility class for parsing JSON documents into InvokeEntry objects.
 * 
 * @todo Idea for Invoke ruleset overwrites:
 * In addition, add the field "overwrites" to the JSON doc.
 * Then, on parsing, the overwrites are applied:
 * $(overwrites.key1) would be replaced by "value1"
 * If, however, an overwrite is not found:
 * $(overwrites.key3) would be replaced by $(global.key3)
 * This allows us to flexibly overwrite values in the invoke without changing the original JSON file.
 * Also, the behavior is well-defined, as it defaults to the global value if no overwrite is defined.
 * Note: retrieval of overwrites in a type object might be difficult. Instead, perhaps:
 * myInvoke.jsonc|push-back overwrites 'key1 -> value1'
 * Example JSON:
 * ```json
 * {
 * "overwrites": {
 *     "key1": "value1",
 *     "key2": "value2"
 * }
 * ```
 * This makes subkey-overwrites easier to parse, e.g.: `"overwrites" [ "physics.G -> 9.81" ]` 
 * turns an `$(overwrites.physics.G)` into `9.81` and 
 * defaults to `{global.physics.G}` if not overwritten.
 */
class InvokeJSONParser{
public:

    /**
     * @brief Parses a JSON encoded set of Invoke Entries inside a RenderObject into InvokeEntry objects.
     * 
     * @param entries_global The global InvokeEntry objects.
     * @param entries_local The local InvokeEntry objects.
     * @param self The RenderObject instance associated with the entries.
     * @param docCache The DocumentCache instance to use for parsing expressions.
     * @param global The global JSON document to use for parsing expressions.
     */
    static void parse(
        std::vector<std::shared_ptr<Nebulite::Interaction::Logic::ParsedEntry>>& entries_global, 
        std::vector<std::shared_ptr<Nebulite::Interaction::Logic::ParsedEntry>>& entries_local, 
        Nebulite::Core::RenderObject* self,
        Nebulite::Utility::DocumentCache* docCache,
        Nebulite::Utility::JSON* global
    );
private:
    /**
     * @brief Extracts function calls from a JSON entry document.
     * 
     * @param entryDoc The JSON document containing the entry.
     * @param invokeEntry The InvokeEntry object to populate with function calls.
     * @param self The RenderObject instance associated with the entry.
     * @param docCache The DocumentCache instance to use for parsing expressions.
     * @param global The global JSON document to use for parsing expressions.
     */
    static void getFunctionCalls(
        Nebulite::Utility::JSON& entryDoc,
        Nebulite::Interaction::Logic::ParsedEntry& invokeEntry, 
        Nebulite::Core::RenderObject* self,
        Nebulite::Utility::DocumentCache* docCache,
        Nebulite::Utility::JSON* global
    );

    /**
     * @brief Extract an expression from a JSON entry document
     * 
     * @param assignmentExpr The assignment expression to populate.
     * @param entry The JSON entry document to extract the expression from.
     * @param index The index of the expression in the entry document.
     * 
     * @return True if the expression was successfully extracted, false otherwise.
     */
    static bool getExpression(
        Nebulite::Interaction::Logic::Assignment& assignmentExpr, 
        Nebulite::Utility::JSON& entry, 
        int index
    );

    /**
     * @brief Extracts a logical argument from a JSON entry document.
     * 
     * @param entry The JSON entry document to extract the argument from.
     * @return The extracted logical argument as a string.
     */
    static std::string getLogicalArg(Nebulite::Utility::JSON& entry);

    /**
     * @brief Extracts an InvokeEntry object from a JSON entry document.
     * 
     * @param doc The JSON document containing the entry.
     * @param entry The JSON document to populate with the entry.
     * @param index The index of the entry in the document.
     * @return True if the InvokeEntry was successfully extracted, false otherwise.
     */
    static bool getInvokeEntry(
        Nebulite::Utility::JSON& doc, 
        Nebulite::Utility::JSON& entry, 
        int index
    );
};
} // namespace Interaction
} // namespace Nebulite

