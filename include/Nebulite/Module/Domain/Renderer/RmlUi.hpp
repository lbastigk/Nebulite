#ifndef NEBULITE_MODULE_DOMAIN_RENDERER_RMLUI_HPP
#define NEBULITE_MODULE_DOMAIN_RENDERER_RMLUI_HPP

//------------------------------------------
// Includes

// Standard library
#include <span>
#include <string_view>

// Nebulite
#include "Nebulite/Constants/Event.hpp"
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

//------------------------------------------
namespace Nebulite::Module::Domain::Renderer {
/**
 * @class Nebulite::Module::Domain::Renderer::RmlUi
 * @brief DomainModule for RmlUi management functions
 */
class RmlUi final : public Base::DomainModule<Core::Renderer> {
public:
    [[nodiscard]] Constants::Event updateHook() override;
    void reinit() override {}

    //------------------------------------------
    // Available Functions

    [[nodiscard]] Constants::Event listDocuments(std::span<std::string_view const> const& args, Interaction::Context const& ctx, Interaction::ContextScope const& ctxScope) const ;
    static auto constexpr listDocumentsName = "rmlui document list";
    static auto constexpr listDocumentsDesc = "Lists all currently loaded RmlUI documents in the renderer's context.\n"
        "Usage: rmlui document list\n";

    [[nodiscard]] Constants::Event loadDocument(std::span<std::string_view const> const& args, Interaction::Context const& ctx, Interaction::ContextScope const& ctxScope) const ;
    static auto constexpr loadDocumentName = "rmlui document load";
    static auto constexpr loadDocumentDesc = "Loads an RmlUI document from a specified file path and adds it to the renderer's context.\n"
        "Usage: rmlui document load <name> <file_path>\n";

    [[nodiscard]] Constants::Event removeDocument(std::span<std::string_view const> const& args, Interaction::Context const& ctx, Interaction::ContextScope& ctxScope) const ;
    static auto constexpr removeDocumentName = "rmlui document remove";
    static auto constexpr removeDocumentDesc = "Removes a loaded RmlUI document from the renderer's context by its name.\n"
        "Usage: rmlui document remove <name>\n";

    //------------------------------------------
    // Categories

    static auto constexpr uiName = "rmlui";
    static auto constexpr uiDesc = "Functions for managing RmlUI elements.";

    static auto constexpr uiDocumentName = "rmlui document";
    static auto constexpr uiDocumentDesc = "Functions for managing RmlUI documents.";

    //------------------------------------------
    // Setup

    /**
     * @brief Initializes the module, binding functions and variables.
     */
    explicit RmlUi(ConstructorParams const& params);

    struct Key : Data::KeyGroup<"renderer.RmlUi."> {
        static auto constexpr openedDocuments = makeScoped("openedDocuments"); // Amount of opened documents
        static auto constexpr usedElementIds = makeScoped("usedElementIds");
    };
};
} // namespace Nebulite::Module::Domain::Renderer
#endif // NEBULITE_MODULE_DOMAIN_RENDERER_RMLUI_HPP
