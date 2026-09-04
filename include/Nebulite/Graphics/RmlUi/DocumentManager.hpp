#ifndef NEBULITE_GRAPHICS_RMLUI_DOCUMENTMANAGER_HPP
#define NEBULITE_GRAPHICS_RMLUI_DOCUMENTMANAGER_HPP

//------------------------------------------
// Includes

// Standard library
#include <unordered_set>

// External
#include <RmlUi/Core/Plugin.h>

//------------------------------------------
namespace Nebulite::Graphics::RmlUi {
/**
 * @brief A plugin for RmlUi that tracks opened documents and provides a mechanism to clear them.
 */
class DocumentManager final : public Rml::Plugin {
public:
    explicit DocumentManager();

    ~DocumentManager() override;

    void clearDocuments();

    DocumentManager(DocumentManager const&) = delete;
    DocumentManager& operator=(DocumentManager const&) = delete;
    DocumentManager(DocumentManager&&) = delete;
    DocumentManager& operator=(DocumentManager&&) = delete;

    void OnDocumentLoad(Rml::ElementDocument* document) override ;

    void OnDocumentUnload(Rml::ElementDocument* document) override ;

    // Hashset of opened documents
    std::unordered_set<Rml::ElementDocument*> openedDocuments;
};

} // namespace Nebulite::Graphics::RmlUi
#endif // NEBULITE_GRAPHICS_RMLUI_DOCUMENTMANAGER_HPP
