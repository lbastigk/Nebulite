//------------------------------------------
// Includes

// External
#include <algorithm>
#include <cstddef>
#include <cstdint> // NOLINT
#include <functional>
#include <memory>
#include <optional>
#include <ranges>
#include <stdexcept>
#include <string>
#include <unordered_set>
#include <utility>
#include <vector>

// External
#include <RmlUi/Core/Context.h>
#include <RmlUi/Core/Core.h>
#include <RmlUi/Core/Element.h>
#include <RmlUi/Core/ElementDocument.h>
#include <RmlUi/Core/Input.h>
#include <RmlUi/Core/Variant.h>
#include <RmlUi_Platform_SDL.h>
#include <RmlUi_Renderer_SDL.h>
#include <SDL3/SDL_events.h>
#include <SDL3/SDL_keycode.h>
#include <SDL3/SDL_mouse.h>
#include <SDL3/SDL_scancode.h>
#include <absl/container/flat_hash_map.h>

// Nebulite
#include "Nebulite/Core/Renderer.hpp"
#include "Nebulite/Graphics/RmlUi/DocumentManager.hpp"
#include "Nebulite/Graphics/RmlUi/ElementIdentifier.hpp"
#include "Nebulite/Graphics/RmlUi/Interface.hpp"
#include "Nebulite/Graphics/RmlUi/SystemInterface.hpp"
#include "Nebulite/Interaction/Context.hpp"
#include "Nebulite/Utility/Io/FileManagement.hpp"

// Nebulite: RmlUi-Modules
#include "Nebulite/Module/RmlUi/Conditional.hpp"
#include "Nebulite/Module/RmlUi/ContextManager.hpp"
#include "Nebulite/Module/RmlUi/DataReference.hpp"
#include "Nebulite/Module/RmlUi/EventBridge.hpp"
#include "Nebulite/Module/RmlUi/ExpressionManager.hpp"
#include "Nebulite/Module/RmlUi/Reflection.hpp"

//------------------------------------------
// Due to lifetime issues, we need to keep track of the interface
// with an outside variable.
// An outside destructor might try to remove references to itself after the Interface was already destroyed

namespace {
struct StatusTracker {
    bool volatile rmlInterfaceInitialized = false;
} statusTracker;
} // namespace

//------------------------------------------
namespace Nebulite::Graphics::RmlUi {

// Lifetime of Interface must be longer than any domain
Interface& Interface::instance() {
    static Interface instance;
    return instance;
}

Interface::Interface() = default;

Interface::~Interface() {
    // NOLINTBEGIN
    if (statusTracker.rmlInterfaceInitialized) {
        statusTracker.rmlInterfaceInitialized = false;
    }
    // NOLINTEND
}

void Interface::init(Core::Renderer& renderer, int const width, int const height){
    window = renderer.getSdlWindow();
    Rml::Initialise();
    statusTracker.rmlInterfaceInitialized = true;

    // Interfaces
    renderInterface = std::make_unique<RenderInterface_SDL>(renderer.getSdlRenderer());
    if (!renderInterface) {
        throw std::runtime_error("Failed to create RmlUi render interface!");
    }
    SetRenderInterface(renderInterface.get());
    systemInterface = std::make_unique<RmlSystemInterface>(window, renderer.capture);
    if (!systemInterface) {
        throw std::runtime_error("Failed to create system interface!");
    }
    SetSystemInterface(systemInterface.get());

    // Core document manager plugin
    documentManager = std::make_unique<DocumentManager>();
    RegisterPlugin(documentManager.get());

    // Plugins
    modules.emplace_back(std::make_unique<Module::RmlUi::EventBridge>(renderer.capture, *this));
    modules.emplace_back(std::make_unique<Module::RmlUi::ContextManager>(renderer.capture, *this));
    modules.emplace_back(std::make_unique<Module::RmlUi::DataReference>(renderer.capture, *this));
    modules.emplace_back(std::make_unique<Module::RmlUi::Reflection>(renderer.capture, *this));
    modules.emplace_back(std::make_unique<Module::RmlUi::ExpressionManager>(renderer.capture, *this)); // Must be registered after Reflection module!
    modules.emplace_back(std::make_unique<Module::RmlUi::Conditional>(renderer.capture, *this)); // Must be registered after Reflection module!
    for (auto& module : modules) {
        RegisterPlugin(module.get());
    }

    // Context
    context = Rml::CreateContext(contextName,{width,height});
    if (!context) {
        throw std::runtime_error("Failed to create RmlUi context!");
    }

    for (auto constexpr fontDirectory = "./Resources/Fonts/"; auto& fontFile : Utility::Io::FileManagement::listFilesInDirectory(fontDirectory)) {
        if (fontFile.ends_with(".ttf")) {
            if (auto const fontPath = fontDirectory + fontFile; !Rml::LoadFontFace(fontPath)) {
                throw std::runtime_error("Failed to load font face for RmlUi from path: " + fontPath);
            }
        }
    }

    // Data Model used for data-value sync
    dataModelConstructor = context->CreateDataModel(dataModelName);
    update(0,0);
}

void Interface::close() const {
    context->Update();
    Rml::Shutdown();
}

namespace {

Rml::Input::KeyIdentifier sdlKeyToRmlKey(SDL_Keycode const& keycode) {
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

int sdlModifierToRmlModifier(std::uint32_t const modifier) {
    int result = 0;

    // Flags are seen as unsigned but guaranteed to be >= 0, so we can or them together

    if (modifier & SDL_KMOD_ALT)
        result |= Rml::Input::KM_ALT; // NOLINT

    if (modifier & SDL_KMOD_CAPS)
        result |= Rml::Input::KM_CAPSLOCK; // NOLINT

    if (modifier & SDL_KMOD_CTRL)
        result |= Rml::Input::KM_CTRL; // NOLINT

    if (modifier & SDL_KMOD_GUI)
        result |= Rml::Input::KM_META; // NOLINT

    if (modifier & SDL_KMOD_SHIFT)
        result |= Rml::Input::KM_SHIFT; // NOLINT

    return result;
}

bool isTextSdlScancode(SDL_Scancode const& scancode) {
    // Covers letters, numbers, and common symbols
    return (scancode >= SDL_SCANCODE_A && scancode <= SDL_SCANCODE_Z) ||
           (scancode >= SDL_SCANCODE_1 && scancode <= SDL_SCANCODE_0) ||
           (scancode >= SDL_SCANCODE_MINUS && scancode <= SDL_SCANCODE_RIGHTBRACKET);
}

} // namespace

void Interface::processMouseButtonEvent(SDL_Event const& event, int const modifiers) const {
    int const button = [&event] {
        if (event.button.button == SDL_BUTTON_LEFT) return 0;
        if (event.button.button == SDL_BUTTON_RIGHT) return 1;
        if (event.button.button == SDL_BUTTON_MIDDLE) return 2;
        return 0;
    }();

    if (event.type == SDL_EVENT_MOUSE_BUTTON_DOWN){
        // We assume the mouse click unfocused the element.
        // If the click was at the elements position, ProcessMouseButtonDown will refocus the element.
        if (context->GetFocusElement()) context->GetFocusElement()->Blur();
        context->ProcessMouseButtonDown(button, modifiers);
    }
    else {
        context->ProcessMouseButtonUp(button, modifiers);
    }
}

void Interface::processKeyEvent(SDL_Event const& event, int const modifiers) const {
    // Skip keys that generate text input
    if (isTextSdlScancode(event.key.scancode)) {
        return;
    }

    auto const rmlKey = sdlKeyToRmlKey(event.key.scancode);
    if (event.type == SDL_EVENT_KEY_DOWN) {
        context->ProcessKeyDown(rmlKey, modifiers);
    }
    else {
        context->ProcessKeyUp(rmlKey, modifiers);
    }
}

void Interface::processRmlUiEvent(SDL_Event const& event) const {
    // Just forwarding all events to RmlSDL::InputEventHandler has a ton of issues...
    // Instead, we manually translate the events and cherrypick what to process with RmlSDL::InputEventHandler
    // Click-activating text input fields is still a nightmare ...
    if (!context) return;

    // Get common data for event processing
    auto const modifiers = sdlModifierToRmlModifier(event.key.mod);

    // Core events
    switch (event.type) {
    case SDL_EVENT_MOUSE_BUTTON_DOWN:
    case SDL_EVENT_MOUSE_BUTTON_UP:
        processMouseButtonEvent(event, modifiers);
        break;
    case SDL_EVENT_MOUSE_WHEEL: {
        context->ProcessMouseWheel(
            {event.wheel.x, event.wheel.y},
            sdlModifierToRmlModifier(event.key.mod)
        );
        break;
    }
    case SDL_EVENT_KEY_DOWN:
    case SDL_EVENT_KEY_UP:
        processKeyEvent(event, modifiers);
        break;
    case SDL_EVENT_TEXT_INPUT:
        context->ProcessTextInput(event.text.text);
        break;
    case SDL_EVENT_MOUSE_MOTION:
    default:
        auto eventCopy = event;
        RmlSDL::InputEventHandler(context, window, eventCopy);
        break;
    }

    // Handle Ctrl + A/C/V/X for text input fields.
    if (modifiers & Rml::Input::KM_CTRL && event.type == SDL_EVENT_KEY_DOWN && isTextInputFocused()) { // NOLINT
        auto eventCopy = event;
        RmlSDL::InputEventHandler(context, window, eventCopy);
    }

    // Pass event to all modules
    for (auto const& module : modules) {
        module->processRmlUiEvent(event, modifiers, context->GetFocusElement());
    }
}

void Interface::update(int const mousePositionX, int const mousePositionY) const {
    // Update SystemInterface
    systemInterface->update(mousePositionX, mousePositionY);

    // Update Documents
    for (auto const& doc : ownershipManager.documentToContext | std::views::keys) {
        doc->UpdateDocument();
    }

    for (auto const& module : modules) {
        module->update();
    }
    context->Update();
}

void Interface::postRenderUpdate() const {
    for (auto const& module : modules) {
        module->postRenderUpdate();
    }
}

void Interface::render() const {
    context->Render();
}

void Interface::setDimensions(int const width, int const height) const {
    context->SetDimensions({width, height});
}

bool Interface::isTextInputFocused() const {
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

// Helper functions

void Interface::updateElement(Rml::Element* element, std::function<void(Rml::Element*, Rml::Element*)> const& updateFunc) {
    auto const numChildren = static_cast<size_t>(element->GetNumChildren());
    for (std::size_t i = 0; i < numChildren; ++i) {
        if (auto* const child = element->GetChild(static_cast<int>(i)); child) {
            updateFunc(child, element);
            updateElement(child, updateFunc);
        }
    }
}

void Interface::updateElement(Rml::ElementDocument* element, std::function<void(Rml::Element*, Rml::Element*)> const& updateFunc){
    updateElement(static_cast<Rml::Element*>(element), updateFunc);
}

std::unordered_set<Rml::ElementDocument*> const& Interface::getOpenedDocuments() const{
    return documentManager->openedDocuments;
}

size_t Interface::countOpenedDocuments() const {
    return documentManager->openedDocuments.size();
}

std::vector<std::pair<size_t, std::string>> Interface::listOpenedDocuments() const{
    std::vector<std::pair<size_t, std::string>> documents;
    for (auto const& [ownerId, nameToDoc] : ownershipManager.ownerToDocument) {
        for (auto const& name : nameToDoc | std::views::keys) {
            documents.emplace_back(ownerId, name);
        }
    }
    return documents;
}

// Context Management

bool Interface::loadDocument(std::string_view const name, std::string_view const path, Interaction::Context const& ctx, Interaction::ContextScope const& ctxScope) {
    auto const document = Utility::Io::FileManagement::loadFile(path);
    Rml::ElementDocument* doc = context->LoadDocumentFromMemory(document);
    if (!doc) return false;

    auto const& id = ctx.self.getId();
    auto ctxAndScope = ContextAndScope{.ctx=ctx, .ctxScope=ctxScope};

    if (ownershipManager.ownerToDocument[id].contains(name)) {
        return false; // Document with this name already exists for this owner
    }
    ownershipManager.ownerToDocument[id][std::string(name)] = doc;
    ownershipManager.documentToContext.emplace(doc, ctxAndScope);
    doc->Show();
    return true;
}

bool Interface::removeDocument(std::size_t const id, std::string_view const name) {
    auto const it = std::ranges::find(ownershipManager.ownerToDocument[id], name, [](auto const& pair) { return pair.first; });
    if (it == ownershipManager.ownerToDocument[id].end()) {
        return false; // No document with this name for this owner
    }
    auto* const doc = it->second;
    doc->Close();
    ownershipManager.ownerToDocument[id].erase(name);
    ownershipManager.documentToContext.erase(doc);
    return true;
}

bool Interface::removeDocument(Rml::ElementDocument* doc) {
    bool foundInOwnerMap = false;
    for (auto& documents : ownershipManager.ownerToDocument | std::views::values) {
        if (auto const it = std::ranges::find_if(documents, [doc](auto const& pair) { return pair.second == doc; }); it != documents.end()) {
            documents.erase(it);
            foundInOwnerMap = true;
            break;
        }
    }
    bool const foundInContextMap = ownershipManager.documentToContext.contains(doc);
    if (foundInContextMap) {
        ownershipManager.documentToContext.erase(doc);
    }
    context->UnloadDocument(doc);
    return foundInOwnerMap && foundInContextMap;
}

void Interface::removeReferencesToId(std::size_t const domainId){
    // This function might be called after the interface is already deleted... So we keep track of the singleton
    if (!statusTracker.rmlInterfaceInitialized) return;
    if (!ownershipManager.ownerToDocument.contains(domainId)) return;

    // 1.) Close and remove all documents owned by the id, and remove them from the context map
    for (auto const& doc : ownershipManager.ownerToDocument[domainId] | std::views::values) {
        doc->Close();
        ownershipManager.documentToContext.erase(doc);
        context->UnloadDocument(doc);
    }
    ownershipManager.ownerToDocument.erase(domainId);

    // 2.) Erase all references to the domain from the element to context map
    absl::erase_if(ownershipManager.elementToContext, [&](auto const& pair) {
        auto const& [element, ctxAndScope] = pair;
        return ctxAndScope.ctx.self.getId() == domainId;
    });

    // 3.) See if the context is referenced as other or global in any document or element contexts, and if so replace it with self
    removeContext(domainId, ownershipManager.elementToContext);
    removeContext(domainId, ownershipManager.documentToContext);
}

std::optional<Interface::ContextAndScope> Interface::getRmlElementContextAndScope(ElementIdentifier const& elementId) {
    if (auto const it = ownershipManager.elementToContext.find(elementId); it != ownershipManager.elementToContext.end()) {
        return it->second;
    }
    return std::nullopt;
}

std::optional<Interface::ContextAndScope> Interface::getRmlDocumentContextAndScope(Rml::ElementDocument* document){
    if (!document) return std::nullopt;
    if (auto const it = ownershipManager.documentToContext.find(document); it != ownershipManager.documentToContext.end()) {
        return it->second;
    }
    return std::nullopt;
}

void Interface::setRmlElementContextAndScope(ElementIdentifier const& elementId, ContextAndScope const& ctxAndScope) {
    ownershipManager.elementToContext.emplace(elementId, ctxAndScope);
}

void Interface::setRmlDocumentContextAndScope(Rml::ElementDocument* document, ContextAndScope const& ctxAndScope) {
    if (!document) return;
    ownershipManager.documentToContext.emplace(document, ctxAndScope);
}

} // namespace Nebulite::Graphics::RmlUi
