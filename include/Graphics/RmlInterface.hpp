#ifndef GRAPHICS_RMLINTERFACE_HPP
#define GRAPHICS_RMLINTERFACE_HPP

//------------------------------------------
// Includes

// External
#include <RmlUi/Core.h>
#include <RmlUi_Platform_SDL.h>
#include <RmlUi_Renderer_SDL.h>
#include <absl/container/flat_hash_map.h>

// Nebulite
#include "Graphics/RmlSystemInterface.hpp"
#include "Interaction/Context.hpp"

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
namespace Nebulite::Graphics {

class RmlInterface {
    RmlInterface();
    ~RmlInterface();

public:
    RmlInterface(RmlInterface const&) = delete;
    RmlInterface& operator=(RmlInterface const&) = delete;
    RmlInterface(RmlInterface&&) = delete;
    RmlInterface& operator=(RmlInterface&&) = delete;

    static auto constexpr contextName = "nebuliteRmlContext";
    static auto constexpr dataModelName = "nebuliteDataSync";

    /**
     * @brief Provides access to the singleton RmlInterface instance
     * @return A reference to the Nebulite RmlInterface
     */
    static RmlInterface& instance();

    /**
     * @brief Initialize the RmlInterface with a given Renderer and scope
     * @param renderer The Renderer to use for rendering RmlUi documents
     * @param width The width of the context
     * @param height The height of the context
     */
    void init(Core::Renderer& renderer, int const& width, int const& height);

    /**
     * @brief Translates and processes an SDL_Event
     * @param event The event to process
     */
    void processRmlUiEvent(SDL_Event event) const ;

    /**
     * @brief Update the RmlInterface, including all open documents and registered modules.
     * @param mousePositionX The current X position of the mouse cursor, used for cursor management in the system interface
     * @param mousePositionY The current Y position of the mouse cursor, used for cursor management in the system interface
     */
    void update(int const& mousePositionX, int const& mousePositionY) const ;

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
    void setDimensions(int const& width, int const& height) const ;

    /**
     * @brief Checks if the focused element is a text input
     * @return True if a text input is active, false otherwise
     */
    [[nodiscard]] bool isTextInputFocused() const ;

    /**
     * @brief Handles unique element identification for context management.
     * @details Uses the Rml Attribute functionality to set and retrieve unique identifications
     */
    class RmlElementIdentifier {
        static auto constexpr identifierAttribute = "element-identifier";

        size_t id; // This elements id

        static size_t idRoll(); // Get a new id

        static size_t& count(); // Get the current id count as reference

    public:
        /**
         * @brief Get the current count of assigned identifiers.
         * @return The count of assigned identifiers.
         */
        static size_t getCount();

        /**
         * @brief Forces an Element to have a certain identifier id
         * @details This should be used with caution and only used with ids that are known to not be registered elsewhere
         * @param element The element to manipulate
         * @param id The id to set
         * @todo Pass an RmlElementIdentifier instead?
         */
        static void forceElementIdentifier(Rml::Element* element, size_t const& id);

        /**
         * @brief Remove the identifier attribute from an element, effectively unregistering it.
         *        The freed id cannot be reused.
         * @param element The element to manipulate
         */
        static void removeElementIdentifier(Rml::Element* element);

        /**
         * @brief Checks if an element has an identifier.
         * @param element The element to check
         * @return True if it has an identifier, false otherwise.
         */
        static bool hasElementIdentifier(Rml::Element const* element);

        /**
         * @brief Construct an identifier for/from a given Rml::Element.
         * @details If the element already has an identifier, it will be used.
         *          Otherwise, a new identifier will be generated and assigned to the element.
         * @param e The element to construct the identifier for
         */
        explicit RmlElementIdentifier(Rml::Element* e);

        /**
         * @brief Construct an identifier with a known id.
         * @details This should only be used if you are sure the id is not already assigned to another element,
         *          e.g. for the reflection module to assign a list of owned, pre-allocated, identifiers to newly generated elements
         * @param knownId The id to use
         */
        explicit RmlElementIdentifier(size_t const& knownId) : id(knownId) {}

        /**
         * @brief Get the identifier's id.
         * @return The id
         */
        [[nodiscard]] size_t getId() const noexcept {
            return id;
        }

        bool operator==(const RmlElementIdentifier& other) const {
            return id == other.id;
        }

        template <typename H>
        friend H AbslHashValue(H h, const RmlElementIdentifier& toHash) {
            return H::combine(std::move(h), toHash.id);
        }
    };

    // Helper functions

    static void updateElement(Rml::Element* element, std::function<void(Rml::Element*, Rml::Element*)> const& updateFunc);

    /**
     * @brief Get a list of all opened Rml Documents
     * @return An unordered set of all Documents
     */
    [[nodiscard]] std::unordered_set<Rml::ElementDocument*> const& getOpenedDocuments() const ;

    /**
     * @brief Get the count of currently opened Rml Documents
     * @return The count of currently opened documents
     */
    [[nodiscard]] size_t countOpenedDocuments() const ;

    /**
     * @brief Get a list of all opened Rml Documents with their associated owner domain id and document name
     * @return The list
     */
    [[nodiscard]] std::vector<std::pair<size_t, std::string>> listOpenedDocuments() const ;

    // Context Management

    /**
     * @brief Storage of an Elements/Documents Interaction Context and ContextScope.
     */
    struct ContextAndScope {
        Interaction::Context ctx;
        Interaction::ContextScope ctxScope;
    };

    bool loadDocument(std::string_view const& name, std::string_view const& path, Interaction::Context const& ctx, Interaction::ContextScope const& ctxScope);

    bool removeDocument(size_t const& id, std::string_view const& name);

    bool removeDocument(Rml::ElementDocument* doc);

    void removeReferencesToId(size_t const& domainId);

    std::optional<ContextAndScope> getRmlElementContextAndScope(RmlElementIdentifier const& element);

    std::optional<ContextAndScope> getRmlDocumentContextAndScope(Rml::ElementDocument* document);

    void setRmlElementContextAndScope(RmlElementIdentifier const& element, ContextAndScope const& ctxAndScope);

    void setRmlDocumentContextAndScope(Rml::ElementDocument* document, ContextAndScope const& ctxAndScope);

    // TODO: Add a custom bind function for modules to use
    Rml::DataModelConstructor dataModelConstructor;

private:
    std::unique_ptr<RenderInterface_SDL> renderInterface;
    std::unique_ptr<RmlSystemInterface> systemInterface;
    Rml::Context* context = nullptr;
    std::vector<std::unique_ptr<Module::Base::RmlUiModule>> modules;
    SDL_Window* window = nullptr;

    // Owner -> name -> document
    absl::flat_hash_map<
        size_t, // owner domain id
        absl::flat_hash_map<
            std::string, // document name
            Rml::ElementDocument*
        >
    > ownerToDocument; // Map of owner domain id to its document for easy retrieval and management
    absl::flat_hash_map<Rml::ElementDocument*, ContextAndScope> documentToContext; // Map of document to its context and scope for expression evaluation
    absl::flat_hash_map<RmlElementIdentifier, ContextAndScope> elementToContext; // Map of element to its context and scope for expression evaluation

    // Document manager
    class DocumentManager final : public Rml::Plugin {
    public:
        explicit DocumentManager();

        void OnDocumentLoad(Rml::ElementDocument* document) override ;

        void OnDocumentUnload(Rml::ElementDocument* document) override ;

        // Hashset of opened documents
        std::unordered_set<Rml::ElementDocument*> openedDocuments;
    };
    std::unique_ptr<DocumentManager> documentManager;
};
} // namespace Nebulite::Graphics
#endif // GRAPHICS_RMLINTERFACE_HPP
