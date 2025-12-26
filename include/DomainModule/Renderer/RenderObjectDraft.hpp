/**
 * @file RenderObjectDraft.hpp
 * @brief Provides RenderObject creation utilities
 */

#ifndef NEBULITE_RRDM_RENDER_OBJECT_DRAFT_HPP
#define NEBULITE_RRDM_RENDER_OBJECT_DRAFT_HPP

//------------------------------------------
// Includes

// Standard library
#include <string>
#include <memory>

// Nebulite
#include "Constants/ErrorTypes.hpp"
#include "Interaction/Execution/DomainModule.hpp"
#include "Core/RenderObject.hpp"

//------------------------------------------
// Forward declarations

namespace Nebulite::Core {
class Renderer; // Forward declaration of domain class Renderer
}

//------------------------------------------
namespace Nebulite::DomainModule::Renderer {
/**
 * @class Nebulite::DomainModule::Renderer::RenderObjectDraft
 * @brief Utilities for creating and manipulating RenderObjects
 * @details Allows for the creation and manipulation of RenderObjects in a draft state
 *          before spawning them into the Environment.
 */
NEBULITE_DOMAINMODULE(Nebulite::Core::Renderer, RenderObjectDraft) {
public:
    Constants::Error update() override;
    void reinit() override {}

    //------------------------------------------
    // Available Functions

    Constants::Error draft_parse(int argc, char** argv);
    static std::string_view constexpr draft_parse_name = "draft parse";
    static std::string_view constexpr draft_parse_desc = "Parse Renderobject-specific functions on the draft.\n"
        "\n"
        "Usage: draft parse <function> [args...]\n"
        "\n"
        "Use 'draft parse help' to see available functions.\n"
        "\n"
        "Examples:\n"
        "draft parse set text.str Hello World\n"
        "draft parse set posX 100\n"
        "draft parse set posY 200\n";

    Constants::Error draft_spawn(int argc, char** argv);
    static std::string_view constexpr draft_spawn_name = "draft spawn";
    static std::string_view constexpr draft_spawn_desc = "Spawn the created draft object.\n"
        "\n"
        "Usage: draft spawn\n";

    Constants::Error draft_reset(int argc, char** argv);
    static std::string_view constexpr draft_reset_name = "draft reset";
    static std::string_view constexpr draft_reset_desc = "Reset the draft object.\n"
        "\n"
        "This does not reset any spawned ones!\n"
        "\n"
        "Usage: draft reset\n";

    //------------------------------------------
    // Category names
    
    static std::string_view constexpr draft_name = "draft";
    static std::string_view constexpr draft_desc = "Functions to manipulate and spawn RenderObjects in draft state";

    //------------------------------------------
    // Setup

    /**
     * @brief Initializes the module, binding functions and variables. 
     */
    NEBULITE_DOMAINMODULE_CONSTRUCTOR(Nebulite::Core::Renderer, RenderObjectDraft) {
        // Bind functions
        (void)bindCategory(draft_name, draft_desc);
        bindFunction(&RenderObjectDraft::draft_parse, draft_parse_name, draft_parse_desc);
        bindFunction(&RenderObjectDraft::draft_spawn, draft_spawn_name, draft_spawn_desc);
        bindFunction(&RenderObjectDraft::draft_reset, draft_reset_name, draft_reset_desc);
    }

private:
    /**
     * @class DraftHolder
     * @brief Protector struct for draft RenderObject
     * @details Ensuring the draft is only initialized when accessed through lazy-init
     */
    class DraftHolder {
    private:
        std::unique_ptr<Core::RenderObject> ptr;

    public:
        DraftHolder() = default;

        std::unique_ptr<Core::RenderObject>& get() {
            if (!ptr) {
                ptr = std::make_unique<Core::RenderObject>();
            }
            return ptr;
        }
    };

    /**
     * @brief The draft RenderObject instance
     */
    DraftHolder draft;
};
} // namespace Nebulite::DomainModule::Renderer
#endif // NEBULITE_RRDM_RENDER_OBJECT_DRAFT_HPP
