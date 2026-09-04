#ifndef NEBULITE_GRAPHICS_RMLUI_INTERFACE_HPP
#define NEBULITE_GRAPHICS_RMLUI_INTERFACE_HPP

//------------------------------------------
// Includes

// Standard library
#include <cstddef>
#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <unordered_set>
#include <utility>
#include <vector>

// External
#include <RmlUi/Core/DataModelHandle.h>
#include <RmlUi/Core/Element.h>
#include <RmlUi_Renderer_SDL.h>
#include <SDL3/SDL_events.h>
#include <SDL3/SDL_video.h>
#include <absl/container/flat_hash_map.h>

// Nebulite
#include "Nebulite/Graphics/RmlUi/DocumentManager.hpp"
#include "Nebulite/Graphics/RmlUi/ElementIdentifier.hpp"
#include "Nebulite/Graphics/RmlUi/SystemInterface.hpp"
#include "Nebulite/Interaction/Context.hpp"

//------------------------------------------
// Forward declarations

namespace Nebulite::Core {
class Renderer;
} // namespace Nebulite::Core

namespace Nebulite::Data {
class JsonScope;
} // namespace Nebulite::Data

namespace Nebulite::Module::Base {
class RmlUiModule;
} // namespace Nebulite::Module::Base

//------------------------------------------
namespace Nebulite::Graphics::RmlUi {

class Interface {
    Interface();
    ~Interface();

public:
    Interface(Interface const&) = delete;
    Interface& operator=(Interface const&) = delete;
    Interface(Interface&&) = delete;
    Interface& operator=(Interface&&) = delete;

    static auto constexpr contextName = "nebuliteRmlContext";
    static auto constexpr dataModelName = "nebuliteDataSync";

    /**
     * @brief Provides access to the singleton Interface instance
     * @return A reference to the Nebulite Interface
     */
    static Interface& instance();

    /**
     * @brief Initialize the Interface with a given Renderer and scope
     * @param renderer The Renderer to use for rendering RmlUi documents
     * @param width The width of the context
     * @param height The height of the context
     */
    void init(Core::Renderer& renderer, int width, int height);

    /**
     * @brief Close the Interface, cleaning up all resources and shutting down RmlUi.
     * @details This should be called before the application exits to ensure proper cleanup.
     */
    void close() const ;

    /**
     * @brief Translates and processes an SDL_Event
     * @param event The event to process
     */
    void processRmlUiEvent(SDL_Event const& event) const ;

    /**
     * @brief Update the Interface, including all open documents and registered modules.
     * @param mousePositionX The current X position of the mouse cursor, used for cursor management in the system interface
     * @param mousePositionY The current Y position of the mouse cursor, used for cursor management in the system interface
     */
    void update(int mousePositionX, int mousePositionY) const ;

    /**
     * @brief Call the provided postRenderUpdate function of each registered module.
     */
    void postRenderUpdate() const ;

    /**
     * @brief Render the RmlUi context to the screen. This should be called after all updates and before presenting the frame.
     */
    void render() const ;

    /**
     * @brief Set the dimensions of the RmlUi context
     * @param width The width of the context
     * @param height The height of the context
     */
    void setDimensions(int width, int height) const ;

    /**
     * @brief Checks if the focused element is a text input
     * @return True if a text input is active, false otherwise
     */
    [[nodiscard]] bool isTextInputFocused() const ;

    // Helper functions

    static void updateElement(Rml::Element* element, std::function<void(Rml::Element*, Rml::Element*)> const& updateFunc);
    static void updateElement(Rml::ElementDocument* element, std::function<void(Rml::Element*, Rml::Element*)> const& updateFunc);

    /**
     * @brief Get a list of all opened Rml Documents
     * @return An unordered set of all Documents
     */
    [[nodiscard]] std::unordered_set<Rml::ElementDocument*> const& getOpenedDocuments() const ;

    /**
     * @brief Get the count of currently opened Rml Documents
     * @return The count of currently opened documents
     */
    [[nodiscard]] std::size_t countOpenedDocuments() const ;

    /**
     * @brief Get a list of all opened Rml Documents with their associated owner domain id and document name
     * @return The list
     */
    [[nodiscard]] std::vector<std::pair<std::size_t, std::string>> listOpenedDocuments() const ;

    // Context Management

    /**
     * @brief Storage of an Elements/Documents Interaction Context and ContextScope.
     */
    struct ContextAndScope {
        Interaction::Context ctx;
        Interaction::ContextScope ctxScope;
    };

    /**
     * @brief Loads a document with a given context and scope
     * @param name A name to give to the document
     * @param path The path to the .rml file
     * @param ctx The context of the document
     * @param ctxScope The scope of the document
     * @return True if the document was loaded successfully, false otherwise
     */
    bool loadDocument(std::string_view name, std::string_view path, Interaction::Context const& ctx, Interaction::ContextScope const& ctxScope);

    /**
     * @brief Removes a document from a given owner id
     * @param id The owner
     * @param name The name of the document
     * @return True if a document was found and removed, false otherwise
     */
    bool removeDocument(std::size_t id, std::string_view name);

    /**
     * @brief Removes a document from a given document pointer
     * @param doc The pointer of the document
     * @return True if the document was removed successfully, false otherwise
     */
    bool removeDocument(Rml::ElementDocument* doc);

    /**
     * @brief Replaces all references to a given owner id, so that no context interaction has access to the associated domain
     * @param domainId The id of the domain
     */
    void removeReferencesToId(std::size_t domainId);

    /**
     * @brief Gets a context and scope from a given RML element id
     * @param elementId The unique Rml element identifier
     * @return The context and scope if it was found, or nullopt if not
     */
    std::optional<ContextAndScope> getRmlElementContextAndScope(ElementIdentifier const& elementId);

    /**
     * @brief Gets a context and scope from a given RML document
     * @param document The document
     * @return The context and scope if it was found, or nullopt if not
     */
    std::optional<ContextAndScope> getRmlDocumentContextAndScope(Rml::ElementDocument* document);

    /**
     * @brief Sets a context and scope for an RML element
     * @param elementId The unique Rml element identifier
     * @param ctxAndScope The context and scope to set
     */
    void setRmlElementContextAndScope(ElementIdentifier const& elementId, ContextAndScope const& ctxAndScope);

    /**
     * @brief Sets a context and scope for an RML document
     * @param document The Rml document
     * @param ctxAndScope The context and scope to set
     */
    void setRmlDocumentContextAndScope(Rml::ElementDocument* document, ContextAndScope const& ctxAndScope);

    /**
     * @brief Binds a variable to the RmlUi data model, allowing it to be accessed and modified from RmlUi documents.
     * @tparam T The type of variable to add
     * @param name The unique name of the variable
     * @param value The address of the variable
     */
    template <typename T>
    void bindVariable(std::string const& name, T* value) {
        dataModelConstructor.Bind(name, value);
    }

private:
    std::unique_ptr<RenderInterface_SDL> renderInterface;
    std::unique_ptr<RmlSystemInterface> systemInterface;
    Rml::DataModelConstructor dataModelConstructor;
    Rml::Context* context = nullptr;
    std::vector<std::unique_ptr<Module::Base::RmlUiModule>> modules;
    SDL_Window* window = nullptr;

    using NameToDocumentMap = absl::flat_hash_map<std::string, Rml::ElementDocument*>; // Map of document name to document pointer
    using OwnerToDocumentMap = absl::flat_hash_map<std::size_t, NameToDocumentMap>; // Map of owner domain id to its documents

    // Ownership management

    struct OwnershipManager {
        OwnerToDocumentMap ownerToDocument;
        absl::flat_hash_map<Rml::ElementDocument*, ContextAndScope> documentToContext;
        absl::flat_hash_map<ElementIdentifier, ContextAndScope> elementToContext;
    } ownershipManager;

    std::unique_ptr<DocumentManager> documentManager;

    /**
     * @brief Checks a container for any references to a given domainId in context 'other' or 'global' and replaces them with context 'self'
     * @tparam Key The identifier type of the element/document
     * @tparam Container The temporary container for the new context
     * @param ctxAndScope The current context and context scope
     * @param key The identifier of the element/document
     * @param container The container to check
     * @param domainId The domain id to check for
     */
    template <typename Key, typename Container>
    static void determineNewContext(auto const& ctxAndScope, Key const& key, Container& container, std::size_t domainId);
    /**
     * @brief Purges a given domain id from any context 'other' and 'global', replacing them with 'self'
     * @tparam Container The container type to modify
     * @param ownerId The identifier of the domain to remove
     * @param container The container to modify
     */
    template<typename Container>
    static void removeContext(std::size_t ownerId, Container& container);

    // Event processing

    void processMouseButtonEvent(SDL_Event const& event, int modifiers) const ;

    void processKeyEvent(SDL_Event const& event, int modifiers) const ;
};
} // namespace Nebulite::Graphics::RmlUi
#include "Nebulite/Graphics/RmlUi/Interface.tpp" // NOLINT(misc-include-cleaner)
#endif // NEBULITE_GRAPHICS_RMLUI_INTERFACE_HPP
