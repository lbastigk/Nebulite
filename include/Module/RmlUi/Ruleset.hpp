#ifndef NEBULITE_MODULE_RMLUI_RULESET_HPP
#define NEBULITE_MODULE_RMLUI_RULESET_HPP

//------------------------------------------
// Includes

// External
#include <RmlUi/Core.h>

// Nebulite
#include "Utility/IO/Capture.hpp"
#include "Module/Base/RmlUiModule.hpp"


//------------------------------------------
namespace Nebulite::Module::RmlUi {
class Ruleset final : public Base::RmlUiModule {
public:
    explicit Ruleset(Utility::IO::Capture& c, Graphics::RmlInterface& i);

    void update() override ;

    void OnElementDestroy(Rml::Element* element) override ;

    static auto constexpr rulesetAttributeOnDestroy = "ruleset-invoke-on-destroy";
    static auto constexpr parseOnDestroy = "parse-on-destroy";
private:

    struct DeletedElement {
        std::variant<
            Graphics::RmlInterface::RmlElementIdentifier,
            Rml::ElementDocument*
        > identifier = nullptr;

        std::optional<std::string> rulesetLink = std::nullopt;
        std::optional<std::string> stringToParse = std::nullopt;

        void applyRuleset(Utility::IO::Capture& capture, Graphics::RmlInterface& interface);
    };

    std::vector<DeletedElement> rulesetsToApply;
};
} // namespace Nebulite::Module::RmlUi

#endif // NEBULITE_MODULE_RMLUI_RULESET_HPP
