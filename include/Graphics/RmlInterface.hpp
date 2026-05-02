#ifndef NEBULITE_GRAPHICS_RML_INTERFACE_HPP
#define NEBULITE_GRAPHICS_RML_INTERFACE_HPP

//------------------------------------------
// Includes

// External
#include <absl/container/flat_hash_map.h>
#include <RmlUi_Platform_SDL.h>
#include <RmlUi_Renderer_SDL.h>
#include <RmlUi/Core.h>

// Nebulite
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
    static auto constexpr contextName = "nebuliteRmlContext";
    static auto constexpr dataModelName = "nebuliteDataSync";

    static RmlInterface& instance();

    void init(Core::Renderer& renderer, Data::JsonScope const& domainScope);

    void update() const ;

    void postRenderUpdate() const ;

    void render() const {
        context->Render();
    }

    void setDimensions(int const& width, int const& height) const ;

    [[nodiscard]] bool isTextInputFocused() const ;

    class RmlElementIdentifier {
        size_t id;
        static auto constexpr identifierAttribute = "element-identifier";
        static size_t idRoll();

        static size_t& count();
    public:
        static size_t getCount() ;

        static void forceElementIdentifier(Rml::Element* element, size_t const& id);

        static void removeElementIdentifier(Rml::Element* element);

        static bool hasElementIdentifier(Rml::Element const* element);

        explicit RmlElementIdentifier(Rml::Element* e);

        explicit RmlElementIdentifier(size_t const& knownId) : id(knownId) {}

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

    struct ContextAndScope {
        Interaction::Context ctx;
        Interaction::ContextScope ctxScope;
    };

    [[nodiscard]] std::unordered_set<Rml::ElementDocument*> const& getOpenedDocuments() const ;

    [[nodiscard]] size_t countOpenedDocuments() const ;

    bool loadDocument(std::string_view const& name, std::string_view const& path, Interaction::Context const& ctx, Interaction::ContextScope const& ctxScope);
    bool removeDocument(size_t const& id, std::string_view const& name);
    bool removeDocument(Rml::ElementDocument* doc);

    void removeAllDocumentsOfOwner(size_t const& domainId);

    std::optional<ContextAndScope> getRmlElementContextAndScope(RmlElementIdentifier const& element);
    std::optional<ContextAndScope> getRmlDocumentContextAndScope(Rml::ElementDocument* document);

    void setRmlElementContextAndScope(RmlElementIdentifier const& element, ContextAndScope const& ctxAndScope);
    void setRmlDocumentContextAndScope(Rml::ElementDocument* document, ContextAndScope const& ctxAndScope);

    static void updateElement(Rml::Element* element, std::function<void(Rml::Element*, Rml::Element*)> const& updateFunc);

    void processRmlUiEvent(const SDL_Event& event) const ;

    // TODO: Add a custom bind function for modules to use
    Rml::DataModelConstructor dataModelConstructor;
private:

    std::unique_ptr<RenderInterface_SDL> renderInterface;
    std::unique_ptr<SystemInterface_SDL> systemInterface; // TODO: Use custom SystemInterface derived from SDL and add toggle for Logging messages/Redirecting them to capture
    Rml::Context* context = nullptr;
    std::vector<std::unique_ptr<Module::Base::RmlUiModule>> modules;

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
#endif // NEBULITE_GRAPHICS_RML_INTERFACE_HPP
