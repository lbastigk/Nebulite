#ifndef NEBULITE_GRAPHICS_RML_INTERFACE_HPP
#define NEBULITE_GRAPHICS_RML_INTERFACE_HPP

#include <absl/container/flat_hash_map.h>

#include <RmlUi_Platform_SDL.h>
#include <RmlUi_Renderer_SDL.h>
#include <RmlUi/Core.h>

namespace Nebulite::Core {
class Renderer;
} // namespace Nebulite::Core

namespace Nebulite::Graphics {

struct RmlInterface {

    void init(Core::Renderer& renderer, Data::JsonScope const& domainScope);

    std::unique_ptr<RenderInterface_SDL> renderInterface;
    std::unique_ptr<SystemInterface_SDL> systemInterface; // TODO: Use custom SystemInterface derived from SDL and add toggle for Logging messages/Redirecting them to capture
    Rml::Context* context;
    Rml::DataModelConstructor dataModelConstructor;
    std::vector<std::unique_ptr<Module::Base::RmlUiModule>> modules;

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

    absl::flat_hash_map<Rml::ElementDocument*, ContextAndScope> documentContext; // Map of document to its context and scope for expression evaluation
    absl::flat_hash_map<RmlElementIdentifier, ContextAndScope> elementContext; // Map of element to its context and scope for expression evaluation

    static void updateElement(Rml::Element* element, std::function<void(Rml::Element*, Rml::Element*, size_t const&)> const& updateFunc);

    void update() const ;

    void processRmlUiEvent(const SDL_Event& event) const ;
};

} // namespace Nebulite::Graphics
#endif // NEBULITE_GRAPHICS_RML_INTERFACE_HPP
