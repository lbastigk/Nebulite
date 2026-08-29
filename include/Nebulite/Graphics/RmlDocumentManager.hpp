#ifndef NEBULITE_GRAPHICS_RMLDOCUMENTMANAGER_HPP
#define NEBULITE_GRAPHICS_RMLDOCUMENTMANAGER_HPP

//------------------------------------------
// Includes

// Standard library
#include <unordered_set>

// External
#include <RmlUi/Core/Plugin.h>

//------------------------------------------
namespace Nebulite::Graphics {
/**
 * @brief A plugin for RmlUi that tracks opened documents and provides a mechanism to clear them.
 */
class RmlDocumentManager final : public Rml::Plugin {
public:
    explicit RmlDocumentManager();

    ~RmlDocumentManager() override;

    void clearDocuments();

    RmlDocumentManager(RmlDocumentManager const&) = delete;
    RmlDocumentManager& operator=(RmlDocumentManager const&) = delete;
    RmlDocumentManager(RmlDocumentManager&&) = delete;
    RmlDocumentManager& operator=(RmlDocumentManager&&) = delete;

    void OnDocumentLoad(Rml::ElementDocument* document) override ;

    void OnDocumentUnload(Rml::ElementDocument* document) override ;

    // Hashset of opened documents
    std::unordered_set<Rml::ElementDocument*> openedDocuments;
};

} // namespace Nebulite::Graphics
#endif // NEBULITE_GRAPHICS_RMLDOCUMENTMANAGER_HPP
