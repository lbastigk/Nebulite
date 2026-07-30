#ifndef NEBULITE_MODULE_DOMAIN_RENDERER_RENDEROBJECTDRAFT_HPP
#define NEBULITE_MODULE_DOMAIN_RENDERER_RENDEROBJECTDRAFT_HPP

//------------------------------------------
// Includes

// Standard library
#include <optional>
#include <span>
#include <string>
#include <string_view>

// Nebulite
#include "Nebulite/Constants/Event.hpp"
#include "Nebulite/Core/RenderObject.hpp"
#include "Nebulite/Data/Document/KeyGroup.hpp"
#include "Nebulite/Module/Base/DomainModule.hpp"

//------------------------------------------
// Forward declarations

namespace Nebulite::Core {
class Renderer;
} // namespace Nebulite::Core

namespace Nebulite::Interaction {
class Context;
class ContextScope;
} // namespace Nebulite::Interaction

namespace Nebulite::Utility::IO {
class Capture;
} // namespace Nebulite::Utility::IO

//------------------------------------------
namespace Nebulite::Module::Domain::Renderer {
/**
 * @class Nebulite::Module::Domain::Renderer::RenderObjectDraft
 * @brief Utilities for creating and manipulating RenderObjects
 * @details Allows for the creation and manipulation of RenderObjects in a draft state
 *          before spawning them into the Environment.
 */
class RenderObjectDraft final : public Base::DomainModule<Core::Renderer> {
public:
    [[nodiscard]] Constants::Event updateHook() override;
    void reinit() override {}

    //------------------------------------------
    // Available Functions

    [[nodiscard]] Constants::Event draft_parse(std::span<std::string_view const> const& args, Interaction::Context& ctx, Interaction::ContextScope& ctxScope);
    static auto constexpr draft_parseName = "draft parse";
    static auto constexpr draft_parseDesc = "Parse Renderobject-specific functions on the draft.\n"
        "\n"
        "Usage: draft parse <function> [args...]\n"
        "\n"
        "Use 'draft parse help' to see available functions.\n"
        "\n"
        "Examples:\n"
        "draft parse set text.str Hello World\n"
        "draft parse set posX 100\n"
        "draft parse set posY 200\n";

    [[nodiscard]] Constants::Event draft_spawn();
    static auto constexpr draft_spawnName = "draft spawn";
    static auto constexpr draft_spawnDesc = "Spawn the created draft object.\n"
        "\n"
        "Usage: draft spawn\n";

    [[nodiscard]] Constants::Event draft_reset();
    static auto constexpr draft_resetName = "draft reset";
    static auto constexpr draft_resetDesc = "Reset the draft object.\n"
        "\n"
        "This does not reset any spawned ones!\n"
        "\n"
        "Usage: draft reset\n";

    //------------------------------------------
    // Categories

    static auto constexpr draftName = "draft";
    static auto constexpr draftDesc = "Functions to manipulate and spawn RenderObjects in draft state";

    //------------------------------------------
    // Setup

    /**
     * @brief Initializes the module, binding functions and variables. 
     */
    explicit RenderObjectDraft(ConstructorParams const& params);

    struct Key : Data::KeyGroup<"renderer."> {
        // No keys for now
    };

private:
    /**
     * @class DraftHolder
     * @brief Protector struct for draft RenderObject
     * @details Ensuring the draft is only initialized when accessed through lazy-init
     */
    class DraftHolder {
        std::optional<Core::RenderObject> obj;

    public:
        DraftHolder() = default;

        Core::RenderObject& get(Utility::IO::Capture& capture) [[clang::lifetimebound]] {
            if (!obj) {
                obj.emplace(capture);
                obj->setName("Draft Object");
            }
            return *obj;
        }
    };

    /**
     * @brief The draft RenderObject instance
     */
    DraftHolder draft;
};
} // namespace Nebulite::Module::Domain::Renderer
#endif // NEBULITE_MODULE_DOMAIN_RENDERER_RENDEROBJECTDRAFT_HPP
