/**
 * @file RmlUi.hpp
 * @brief This file contains the DomainModule of the Renderer to provide RmlUi management utilities.
 */

#ifndef MODULE_DOMAIN_RENDERER_RMLUI_HPP
#define MODULE_DOMAIN_RENDERER_RMLUI_HPP

//------------------------------------------
// Includes

// Nebulite
#include "Constants/KeyNames.hpp"
#include "Interaction/Execution/DomainModule.hpp"

//------------------------------------------
// Forward declarations

namespace Nebulite::Core {
class Renderer;
} // namespace Nebulite::Core

//------------------------------------------
namespace Nebulite::Module::Domain::Renderer {
/**
 * @class Nebulite::Module::Domain::Renderer::RmlUi
 * @brief DomainModule for RmlUi management functions
 */
class RmlUi final : public Interaction::Execution::DomainModule<Core::Renderer> {
public:
    [[nodiscard]] Constants::Event updateHook() override;
    void reinit() override {}

    //------------------------------------------
    // Available Functions

    [[nodiscard]] Constants::Event listDocuments(std::span<std::string const> const& args, Interaction::Context const& ctx, Interaction::ContextScope const& ctxScope) const ;
    static auto constexpr listDocuments_name = "rmlui document list";
    static auto constexpr listDocuments_desc = "Lists all currently loaded RmlUI documents in the renderer's context.\n"
        "Usage: rmlui document list\n";

    [[nodiscard]] Constants::Event loadDocument(std::span<std::string const> const& args, Interaction::Context const& ctx, Interaction::ContextScope const& ctxScope) const ;
    static auto constexpr loadDocument_name = "rmlui document load";
    static auto constexpr loadDocument_desc = "Loads an RmlUI document from a specified file path and adds it to the renderer's context.\n"
        "Usage: rmlui document load <name> <file_path>\n";

    [[nodiscard]] Constants::Event removeDocument(std::span<std::string const> const& args, Interaction::Context const& ctx, Interaction::ContextScope& ctxScope) const ;
    static auto constexpr removeDocument_name = "rmlui document remove";
    static auto constexpr removeDocument_desc = "Removes a loaded RmlUI document from the renderer's context by its name.\n"
        "Usage: rmlui document remove <name>\n";

    //------------------------------------------
    // Categories

    static auto constexpr ui_name = "rmlui";
    static auto constexpr ui_desc = "Functions for managing RmlUI elements.";

    static auto constexpr uiDocument_name = "rmlui document";
    static auto constexpr uiDocument_desc = "Functions for managing RmlUI documents.";

    //------------------------------------------
    // Setup

    /**
     * @brief Initializes the module, binding functions and variables.
     */
    explicit RmlUi(ConstructorParams const& params) : DomainModule(params) {
        bindCategory(ui_name, ui_desc);
        bindCategory(uiDocument_name, uiDocument_desc);
        bindFunction(&RmlUi::listDocuments, listDocuments_name, listDocuments_desc);
        bindFunction(&RmlUi::loadDocument, loadDocument_name, loadDocument_desc);
        bindFunction(&RmlUi::removeDocument, removeDocument_name, removeDocument_desc);
    }

    struct Key : Data::KeyGroup<"renderer.RmlUi."> {
        static auto constexpr openedDocuments = makeScoped("openedDocuments"); // Amount of opened documents
        static auto constexpr usedElementIds = makeScoped("usedElementIds");
    };
};
} // namespace Nebulite::Module::Domain::Renderer
#endif // MODULE_DOMAIN_RENDERER_RMLUI_HPP

