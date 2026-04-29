#ifndef NEBULITE_GRAPHICS_RML_INTERFACE_HPP
#define NEBULITE_GRAPHICS_RML_INTERFACE_HPP

//------------------------------------------
// Includes

#include <absl/container/flat_hash_map.h>
#include <RmlUi_Platform_SDL.h>
#include <RmlUi_Renderer_SDL.h>
#include <RmlUi/Core.h>

//------------------------------------------
// Forward declarations

namespace Nebulite::Core {
class Renderer;
} // namespace Nebulite::Core

namespace Nebulite::Module::Base {
class RmlUiModule;
} // namespace Nebulite::Module::Base

//------------------------------------------
namespace Nebulite::Graphics {

class RmlInterface {
public:
    void init(Core::Renderer& renderer, Data::JsonScope const& domainScope);

    void update() const ;

    void postRenderUpdate() const ;

    void render() const {
        context->Render();
    }

    void setDimensions(int const& width, int const& height) const ;

    bool isTextInputFocused() const {
        if (Rml::Element* el = context->GetFocusElement(); el){
            // Covers <input type="text"> and <textarea>
            if (Rml::String const tag = el->GetTagName(); tag == "input" || tag == "textarea"){
                // Optional: check type="text"
                if (tag == "input"){
                    if (Rml::Variant const* type = el->GetAttribute("type"); type && type->Get<Rml::String>() != "text")
                        return false;
                }
                return true;
            }
        }
        return false;
    }

    // Simpler identifiers based on element pointer did not work previously, maybe this isn't the case anymore?
    struct RmlElementIdentifier {
        Rml::Element* parent;
        size_t index; // Index of the element among its siblings, to uniquely identify it in case of multiple elements with the same tag
        int children;

        RmlElementIdentifier(Rml::Element* p, size_t const& i, Rml::Element const* e){
            // Using e->getParentNode is buggy, so we don't use it here
            parent = p;
            index = i;
            children = e->GetNumChildren();
        }

        bool operator==(const RmlElementIdentifier& other) const {
            return parent == other.parent && index == other.index;
        }

        template <typename H>
        friend H AbslHashValue(H h, const RmlElementIdentifier& id) {
            return H::combine(std::move(h), id.parent, id.index, id.children);
        }
    };

    struct ContextAndScope {
        Interaction::Context ctx;
        Interaction::ContextScope ctxScope;
    };

    size_t countOpenedDocuments() const {
        return documentToContext.size();
    }

    bool loadDocument(std::string_view const& name, std::string_view const& path, Interaction::Context const& ctx, Interaction::ContextScope const& ctxScope);
    bool removeDocument(size_t const& id, std::string_view const& name);
    bool removeDocument(Rml::ElementDocument* doc);

    std::optional<ContextAndScope> getRmlElementContextAndScope(RmlElementIdentifier const& element);
    std::optional<ContextAndScope> getRmlDocumentContextAndScope(Rml::ElementDocument* document);

    void setRmlElementContextAndScope(RmlElementIdentifier const& element, ContextAndScope const& ctxAndScope);
    void setRmlDocumentContextAndScope(Rml::ElementDocument* document, ContextAndScope const& ctxAndScope);


    static void updateElement(Rml::Element* element, std::function<void(Rml::Element*, Rml::Element*, size_t const&)> const& updateFunc);

    void processRmlUiEvent(const SDL_Event& event) const ;

    // TODO: Add a custom bind function for modules to use
    Rml::DataModelConstructor dataModelConstructor;
private:
    std::unique_ptr<RenderInterface_SDL> renderInterface;
    std::unique_ptr<SystemInterface_SDL> systemInterface; // TODO: Use custom SystemInterface derived from SDL and add toggle for Logging messages/Redirecting them to capture
    Rml::Context* context = nullptr;
    std::vector<std::unique_ptr<Module::Base::RmlUiModule>> modules;

    absl::flat_hash_map<Rml::ElementDocument*, ContextAndScope> documentToContext; // Map of document to its context and scope for expression evaluation
    absl::flat_hash_map<RmlElementIdentifier, ContextAndScope> elementToContext; // Map of element to its context and scope for expression evaluation

    absl::flat_hash_map<
        size_t, // owner domain id
        absl::flat_hash_map<
            std::string, // document name
            Rml::ElementDocument*
        >
    > ownerToDocument; // Map of owner domain id to its document for easy retrieval and management
};

} // namespace Nebulite::Graphics
#endif // NEBULITE_GRAPHICS_RML_INTERFACE_HPP
