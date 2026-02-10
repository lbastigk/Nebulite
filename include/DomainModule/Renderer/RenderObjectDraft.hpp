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
#include "Constants/KeyNames.hpp"
#include "Core/RenderObject.hpp"
#include "Interaction/Execution/DomainModule.hpp"


//------------------------------------------
// Forward declarations

namespace Nebulite::Core {
class Renderer; // Forward declaration of domain class Renderer
} // namespace Nebulite::Core

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

    Constants::Error draft_parse(std::span<std::string const> const& args);
    static auto constexpr draft_parse_name = "draft parse";
    static auto constexpr draft_parse_desc = "Parse Renderobject-specific functions on the draft.\n"
        "\n"
        "Usage: draft parse <function> [args...]\n"
        "\n"
        "Use 'draft parse help' to see available functions.\n"
        "\n"
        "Examples:\n"
        "draft parse set text.str Hello World\n"
        "draft parse set posX 100\n"
        "draft parse set posY 200\n";

    Constants::Error draft_spawn();
    static auto constexpr draft_spawn_name = "draft spawn";
    static auto constexpr draft_spawn_desc = "Spawn the created draft object.\n"
        "\n"
        "Usage: draft spawn\n";

    Constants::Error draft_reset();
    static auto constexpr draft_reset_name = "draft reset";
    static auto constexpr draft_reset_desc = "Reset the draft object.\n"
        "\n"
        "This does not reset any spawned ones!\n"
        "\n"
        "Usage: draft reset\n";

    //------------------------------------------
    // Category names

    static auto constexpr draft_name = "draft";
    static auto constexpr draft_desc = "Functions to manipulate and spawn RenderObjects in draft state";

    //------------------------------------------
    // Setup

    /**
     * @brief Initializes the module, binding functions and variables. 
     */
    NEBULITE_DOMAINMODULE_CONSTRUCTOR(Nebulite::Core::Renderer, RenderObjectDraft) {
        // Bind functions
        bindCategory(draft_name, draft_desc);
        BIND_FUNCTION(&RenderObjectDraft::draft_parse, draft_parse_name, draft_parse_desc);
        BIND_FUNCTION(&RenderObjectDraft::draft_spawn, draft_spawn_name, draft_spawn_desc);
        BIND_FUNCTION(&RenderObjectDraft::draft_reset, draft_reset_name, draft_reset_desc);
    }

    struct Key {
        // TODO: We could modify the RenderObject constructor to accept an optional scope.
        //       This way, we can directly store the draft data in the renderer scope.
        //       Then, we can modify the scope to "renderer.draft." and have the entire renderObject live in that scope at root level.
        DECLARE_SCOPE("renderer.")
    };

private:
    /**
     * @class DraftHolder
     * @brief Protector struct for draft RenderObject
     * @details Ensuring the draft is only initialized when accessed through lazy-init
     */
    class DraftHolder {
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
