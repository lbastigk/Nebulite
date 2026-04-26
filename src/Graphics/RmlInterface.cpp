#include <RmlUi/Core/Input.h>

#include "Core/Renderer.hpp"
#include "Constants/KeyNames.hpp"
#include "Graphics/RmlInterface.hpp"
#include "Utility/IO/FileManagement.hpp"

#include "Module/RmlUi/ContextManager.hpp"
#include "Module/RmlUi/DataReference.hpp"
#include "Module/RmlUi/ExpressionManager.hpp"
#include "Module/RmlUi/Reflection.hpp"


namespace Nebulite::Graphics {

void RmlInterface::init(Core::Renderer& renderer, Data::JsonScope const& domainScope){
    Rml::Initialise();

    // Interfaces
    renderInterface = std::make_unique<RenderInterface_SDL>(renderer.getSdlRenderer());
    if (!renderInterface) {
        throw std::runtime_error("Failed to create RmlUi render interface!");
    }
    SetRenderInterface(renderInterface.get());
    systemInterface = std::make_unique<SystemInterface_SDL>(renderer.getSdlWindow());
    if (!systemInterface) {
        throw std::runtime_error("Failed to create system interface!");
    }

    // Plugins
    modules.emplace_back(std::make_unique<Module::RmlUi::ContextManager>(renderer.capture, renderer));
    modules.emplace_back(std::make_unique<Module::RmlUi::DataReference>(renderer.capture, renderer));
    modules.emplace_back(std::make_unique<Module::RmlUi::Reflection>(renderer.capture, renderer));
    modules.emplace_back(std::make_unique<Module::RmlUi::ExpressionManager>(renderer.capture, renderer)); // Must come after Reflection module!


    for (auto& module : modules) {
        RegisterPlugin(module.get());
    }

    // Context
    context = Rml::CreateContext(
        "main", {
            static_cast<int>(
                domainScope.get<double>(Constants::KeyNames::Renderer::dispResX).value_or(800.0)
            ),
            static_cast<int>(
                domainScope.get<double>(Constants::KeyNames::Renderer::dispResY).value_or(600.0)
            )
        }
    );
    if (!context) {
        throw std::runtime_error("Failed to create RmlUi context!");
    }

    for (auto constexpr fontDirectory = "./Resources/Fonts/"; auto& fontFile : Utility::IO::FileManagement::listFilesInDirectory(fontDirectory)) {
        if (fontFile.ends_with(".ttf")) {
            if (auto const fontPath = fontDirectory + fontFile; !Rml::LoadFontFace(fontPath)) {
                throw std::runtime_error("Failed to load font face for RmlUi from path: " + fontPath);
            }
        }
    }

    // Data Model used for data-value sync
    dataModelConstructor = context->CreateDataModel("nebuliteDataSync");
    update();
}

bool RmlInterface::loadDocument(std::string_view const& name, std::string_view const& path, Interaction::Context const& ctx, Interaction::ContextScope const& ctxScope) {
    auto const document = Utility::IO::FileManagement::LoadFile(path);
    Rml::ElementDocument* doc = context->LoadDocumentFromMemory(document);
    if (!doc) return false;

    auto& id = ctx.self.getId();
    auto ctxAndScope = ContextAndScope{ctx, ctxScope};

    if (ownerToDocument[id].contains(name)) {
        return false; // Document with this name already exists for this owner
    }
    ownerToDocument[id][std::string(name)] = doc;
    documentToContext.emplace(doc, ctxAndScope);
    doc->Show();
    return true;
}

bool RmlInterface::removeDocument(size_t const& id, std::string_view const& name) {
    auto const it = std::ranges::find(ownerToDocument[id], name, [](auto const& pair) { return pair.first; });
    if (it == ownerToDocument[id].end()) {
        return false; // No document with this name for this owner
    }
    auto const doc = it->second;
    doc->Close();
    ownerToDocument[id].erase(name);
    documentToContext.erase(doc);
    return true;
}

bool RmlInterface::removeDocument(Rml::ElementDocument* doc) {
    bool foundInOwnerMap = false;
    for (auto& documents : ownerToDocument | std::views::values) {
        if (auto const it = std::ranges::find_if(documents, [doc](auto const& pair) { return pair.second == doc; }); it != documents.end()) {
            documents.erase(it);
            foundInOwnerMap = true;
            break;
        }
    }
    bool const foundInContextMap = documentToContext.contains(doc);
    if (foundInContextMap) {
        documentToContext.erase(doc);
    }
    context->UnloadDocument(doc);
    return foundInOwnerMap && foundInContextMap;
}

void RmlInterface::updateElement(Rml::Element* element, std::function<void(Rml::Element*, Rml::Element*, size_t const&)> const& updateFunc) {
    size_t const numChildren = static_cast<size_t>(element->GetNumChildren());
    for (size_t i = 0; i < numChildren; ++i) {
        if (auto const child = element->GetChild(static_cast<int>(i)); child) {
            updateFunc(child, element, i);
            updateElement(child, updateFunc);
        }
    }
}


std::optional<RmlInterface::ContextAndScope> RmlInterface::getRmlElementContextAndScope(RmlElementIdentifier const& element) {
    if (auto const it = elementToContext.find(element); it != elementToContext.end()) {
        return it->second;
    }
    return std::nullopt;
}

std::optional<RmlInterface::ContextAndScope> RmlInterface::getRmlDocumentContextAndScope(Rml::ElementDocument* document){
    if (!document) return std::nullopt;
    if (auto const it = documentToContext.find(document); it != documentToContext.end()) {
        return it->second;
    }
    return std::nullopt;
}

void RmlInterface::setRmlElementContextAndScope(RmlElementIdentifier const& element, ContextAndScope const& ctxAndScope) {
    elementToContext.emplace(element, ctxAndScope);
}

void RmlInterface::setRmlDocumentContextAndScope(Rml::ElementDocument* document, ContextAndScope const& ctxAndScope) {
    if (!document) return;
    documentToContext.emplace(document, ctxAndScope);
}

void RmlInterface::update() const {
    for (auto const& doc : documentToContext | std::views::keys) {
        doc->UpdateDocument();
    }

    for (auto const& module : modules) {
        module->update();
    }
}

namespace {

Rml::Input::KeyIdentifier SDLKeyToRmlKey(SDL_Keycode const& keycode) {
    switch (keycode) {
        // Basic text editing keys
    case SDL_SCANCODE_BACKSPACE: return Rml::Input::KI_BACK;
    case SDL_SCANCODE_TAB:       return Rml::Input::KI_TAB;
    case SDL_SCANCODE_RETURN:    return Rml::Input::KI_RETURN;
    case SDL_SCANCODE_SPACE:     return Rml::Input::KI_SPACE;
    case SDL_SCANCODE_DELETE:    return Rml::Input::KI_DELETE;
        // Arrow
    case SDL_SCANCODE_LEFT:      return Rml::Input::KI_LEFT;
    case SDL_SCANCODE_RIGHT:     return Rml::Input::KI_RIGHT;
    case SDL_SCANCODE_UP:        return Rml::Input::KI_UP;
    case SDL_SCANCODE_DOWN:      return Rml::Input::KI_DOWN;
        // Other
    case SDL_SCANCODE_ESCAPE:    return Rml::Input::KI_ESCAPE;
    default:                     return Rml::Input::KI_UNKNOWN;
    }
}

int SdlModifierToRmlModifier(uint32_t const& modifier) {
    int result = 0;

    if (modifier & SDL_KMOD_SHIFT)
        result |= Rml::Input::KM_SHIFT;

    if (modifier & SDL_KMOD_CTRL)
        result |= Rml::Input::KM_CTRL;

    if (modifier & SDL_KMOD_ALT)
        result |= Rml::Input::KM_ALT;

    if (modifier & SDL_KMOD_GUI)
        result |= Rml::Input::KM_META; // Windows key / Cmd key

    return result;
}

bool isTextSdlScancode(SDL_Scancode const& scancode) {
    // Covers letters, numbers, and common symbols
    return (scancode >= SDL_SCANCODE_A && scancode <= SDL_SCANCODE_Z) ||
           (scancode >= SDL_SCANCODE_1 && scancode <= SDL_SCANCODE_0) ||
           (scancode >= SDL_SCANCODE_MINUS && scancode <= SDL_SCANCODE_RIGHTBRACKET);
}

} // namespace

void RmlInterface::processRmlUiEvent(const SDL_Event& event) const {
    if (!context) return;

    switch (event.type) {

    case SDL_EVENT_MOUSE_MOTION:
        context->ProcessMouseMove(
            static_cast<int>(event.motion.x),
            static_cast<int>(event.motion.y),
            SdlModifierToRmlModifier(event.key.mod)
        );
        break;

    case SDL_EVENT_MOUSE_BUTTON_DOWN:
    case SDL_EVENT_MOUSE_BUTTON_UP: {
        int button = 0;
        if (event.button.button == SDL_BUTTON_LEFT) button = 0;
        else if (event.button.button == SDL_BUTTON_RIGHT) button = 1;
        else if (event.button.button == SDL_BUTTON_MIDDLE) button = 2;

        if (event.type == SDL_EVENT_MOUSE_BUTTON_DOWN){
            // We assume the mouse click unfocused the element.
            // If the click was at the elements position, ProcessMouseButtonDown will refocus the element.
            context->GetFocusElement()->Blur();
            context->ProcessMouseButtonDown(button, SdlModifierToRmlModifier(event.key.mod));
        }
        else {
            context->ProcessMouseButtonUp(button, SdlModifierToRmlModifier(event.key.mod));
        }
        break;
    }

    case SDL_EVENT_MOUSE_WHEEL: {
        context->ProcessMouseWheel(
            {event.wheel.x, event.wheel.y},
            SdlModifierToRmlModifier(event.key.mod)
        );
        break;
    }

    case SDL_EVENT_KEY_DOWN:
    case SDL_EVENT_KEY_UP: {
        // Skip keys that generate text input
        if (isTextSdlScancode(event.key.scancode)) {
            break;
        }

        auto const rmlKey = SDLKeyToRmlKey(event.key.scancode);
        auto const mods = SdlModifierToRmlModifier(event.key.mod);
        if (event.type == SDL_EVENT_KEY_DOWN)
            context->ProcessKeyDown(rmlKey, mods);
        else
            context->ProcessKeyUp(rmlKey, mods);
        break;
    }

    case SDL_EVENT_TEXT_INPUT:
        context->ProcessTextInput(event.text.text);
        break;

    default:
        break;
    }
}

} // namespace Nebulite::Graphics
