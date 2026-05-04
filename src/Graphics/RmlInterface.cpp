//------------------------------------------
// Includes

// External
#include <RmlUi/Core/Input.h>

// Nebulite
#include "Core/Renderer.hpp"
#include "Graphics/RmlInterface.hpp"
#include "Utility/IO/FileManagement.hpp"

// Nebulite: RmlUi-Modules
#include "Nebulite.hpp"
#include "Module/RmlUi/ContextManager.hpp"
#include "Module/RmlUi/DataReference.hpp"
#include "Module/RmlUi/ExpressionManager.hpp"
#include "Module/RmlUi/Reflection.hpp"
#include "Module/RmlUi/Ruleset.hpp"

//------------------------------------------
// Due to lifetime issues, we need to keep track of the interface
// with an outside variable.

namespace {
struct StatusTracker {
    bool rmlInterfaceInitialized = false;
} statusTracker;
} // namespace

//------------------------------------------
namespace Nebulite::Graphics {

// RmlInterface::RmlElementIdentifier

size_t& RmlInterface::RmlElementIdentifier::count() {
    static size_t rollingIdentifier = 0;
    return rollingIdentifier;
}

size_t RmlInterface::RmlElementIdentifier::idRoll() {
    return count()++;
}

size_t RmlInterface::RmlElementIdentifier::getCount() {
    return count();
}

void RmlInterface::RmlElementIdentifier::forceElementIdentifier(Rml::Element* element, size_t const& id) {
    element->SetAttribute(identifierAttribute, id);
}

void RmlInterface::RmlElementIdentifier::removeElementIdentifier(Rml::Element* element) {
    element->RemoveAttribute(identifierAttribute);
}

bool RmlInterface::RmlElementIdentifier::hasElementIdentifier(Rml::Element const* element){
    return element->HasAttribute(identifierAttribute);
}

RmlInterface::RmlElementIdentifier::RmlElementIdentifier(Rml::Element* e){
    // See if element has attribute
    if (e->GetAttribute(identifierAttribute)) {
        id = e->GetAttribute(identifierAttribute)->Get<size_t>();
    }
    else {
        id = idRoll();
        e->SetAttribute(identifierAttribute, id);
    }
}

// RmlInterface

// Lifetime of RmlInterface must be longer than any domain
RmlInterface& RmlInterface::instance() {
    static RmlInterface instance;
    return instance;
}

RmlInterface::RmlInterface() = default;

RmlInterface::~RmlInterface() {
    if (statusTracker.rmlInterfaceInitialized) {
        Rml::Shutdown();
        statusTracker.rmlInterfaceInitialized = false;
    }
}

void RmlInterface::init(Core::Renderer& renderer, int const& width, int const& height){
    Rml::Initialise();
    statusTracker.rmlInterfaceInitialized = true;

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

    // Core document manager plugin
    documentManager = std::make_unique<DocumentManager>();
    RegisterPlugin(documentManager.get());

    // Plugins
    modules.emplace_back(std::make_unique<Module::RmlUi::Ruleset>(renderer.capture, *this));
    modules.emplace_back(std::make_unique<Module::RmlUi::ContextManager>(renderer.capture, *this));
    modules.emplace_back(std::make_unique<Module::RmlUi::DataReference>(renderer.capture, *this));
    modules.emplace_back(std::make_unique<Module::RmlUi::Reflection>(renderer.capture, *this));
    modules.emplace_back(std::make_unique<Module::RmlUi::ExpressionManager>(renderer.capture, *this)); // Must be registered after Reflection module!
    for (auto& module : modules) {
        RegisterPlugin(module.get());
    }

    // Context
    context = Rml::CreateContext(contextName,{width,height});
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
    dataModelConstructor = context->CreateDataModel(dataModelName);
    update();
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

    if (modifier & SDL_KMOD_ALT)
        result |= Rml::Input::KM_ALT;

    if (modifier & SDL_KMOD_CAPS)
        result |= Rml::Input::KM_CAPSLOCK;

    if (modifier & SDL_KMOD_CTRL)
        result |= Rml::Input::KM_CTRL;

    if (modifier & SDL_KMOD_GUI)
        result |= Rml::Input::KM_META;

    if (modifier & SDL_KMOD_SHIFT)
        result |= Rml::Input::KM_SHIFT;

    return result;
}

bool isTextSdlScancode(SDL_Scancode const& scancode) {
    // Covers letters, numbers, and common symbols
    return (scancode >= SDL_SCANCODE_A && scancode <= SDL_SCANCODE_Z) ||
           (scancode >= SDL_SCANCODE_1 && scancode <= SDL_SCANCODE_0) ||
           (scancode >= SDL_SCANCODE_MINUS && scancode <= SDL_SCANCODE_RIGHTBRACKET);
}

} // namespace

void RmlInterface::processRmlUiEvent(SDL_Event const& event) const {
    if (!context) return;

    auto const modifiers = SdlModifierToRmlModifier(event.key.mod);

    // Core events
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
            if (auto const el = context->GetFocusElement(); el) el->Blur();
            context->ProcessMouseButtonDown(button, modifiers);
        }
        else {
            context->ProcessMouseButtonUp(button, modifiers);
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
        if (event.type == SDL_EVENT_KEY_DOWN)
            context->ProcessKeyDown(rmlKey, modifiers);
        else
            context->ProcessKeyUp(rmlKey, modifiers);
        break;
    }
    case SDL_EVENT_TEXT_INPUT:
        context->ProcessTextInput(event.text.text);
        break;
    default:
        break;
    }

    // Handle Ctrl + A/C/V/X for text input fields
    if (modifiers & Rml::Input::KM_CTRL && event.type == SDL_EVENT_KEY_DOWN && isTextInputFocused()) {
        if (event.key.key == SDLK_A) {
            // TODO: highlight entire text input field
            Global::capture().warning.println("Ctrl + A pressed - TODO: implement select all functionality for text input fields");
        }
        else if (event.key.key == SDLK_C) {
            // TODO: copy highlighted text
            //       - how to extract highlighted portion?
            Global::capture().warning.println("Ctrl + C pressed - TODO: implement copy functionality for text input fields");
        }
        else if (event.key.key == SDLK_V) {
            std::string const input(SDL_GetClipboardText());
            context->ProcessTextInput(input);
        }
        else if (event.key.key == SDLK_X) {
            // TODO: extract highlighted text
            //       - how to extract highlighted portion?
            //       - how to remove highlighted portion?
            Global::capture().warning.println("Ctrl + X pressed - TODO: implement cut functionality for text input fields");
        }
    }
}

void RmlInterface::update() const {
    for (auto const& doc : documentToContext | std::views::keys) {
        doc->UpdateDocument();
    }

    for (auto const& module : modules) {
        module->update();
    }
    context->Update();
}

void RmlInterface::postRenderUpdate() const {
    for (auto& module : modules) {
        module->postRenderUpdate();
    }
}

void RmlInterface::render() const {
    context->Render();
}

void RmlInterface::setDimensions(int const& width, int const& height) const {
    context->SetDimensions({width, height});
}

bool RmlInterface::isTextInputFocused() const {
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

void RmlInterface::updateElement(Rml::Element* element, std::function<void(Rml::Element*, Rml::Element*)> const& updateFunc) {
    auto const numChildren = static_cast<size_t>(element->GetNumChildren());
    for (size_t i = 0; i < numChildren; ++i) {
        if (auto const child = element->GetChild(static_cast<int>(i)); child) {
            updateFunc(child, element);
            updateElement(child, updateFunc);
        }
    }
}

std::unordered_set<Rml::ElementDocument*> const& RmlInterface::getOpenedDocuments() const{
    return documentManager->openedDocuments;
}

size_t RmlInterface::countOpenedDocuments() const {
    return documentManager->openedDocuments.size();
}

std::vector<std::pair<size_t, std::string>> RmlInterface::listOpenedDocuments() const{
    std::vector<std::pair<size_t, std::string>> documents;
    for (auto const& [ownerId, nameToDoc] : ownerToDocument) {
        for (auto const& name : nameToDoc | std::views::keys) {
            documents.emplace_back(ownerId, name);
        }
    }
    return documents;
}

// Context Management

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

void RmlInterface::removeReferencesToId(size_t const& domainId){
    // This function might be called after the interface is already deleted... So we keep track of the singleton
    // NOLINTBEGIN
    if (!statusTracker.rmlInterfaceInitialized) return;
    if (!ownerToDocument.contains(domainId)) return;

    // 1.) Close and remove all documents owned by the id, and remove them from the context map
    for (auto const& doc : ownerToDocument[domainId] | std::views::values) {
        doc->Close();
        documentToContext.erase(doc);
        context->UnloadDocument(doc);
    }
    ownerToDocument.erase(domainId);

    // 2.) Erase all references to the domain from the element to context map
    absl::erase_if(elementToContext, [&](auto const& pair) {
        auto const& [element, ctxAndScope] = pair;
        return ctxAndScope.ctx.self.getId() == domainId;
    });

    // 3.) See if the context is referenced as other or global in any document or element contexts, and if so replace it with self

    // Templated helper lambda for updating any context
    auto determineNewContext = [&]<typename Key, typename UpdatedElementContainer>(auto const& ctxAndScope, Key const& key, UpdatedElementContainer& container) {
        auto& oldCtx = ctxAndScope.ctx;
        auto& oldCtxScope = ctxAndScope.ctxScope;
        if (oldCtx.other.getId() != domainId && oldCtx.global.getId() != domainId) {
            return;
        }
        using newDomainAndScope = std::pair<Interaction::Execution::Domain*, Data::JsonScope*>;
        newDomainAndScope newOther = oldCtx.other.getId() == domainId ? std::make_pair(&oldCtx.self, &oldCtxScope.self) : std::make_pair(&oldCtx.other, &oldCtxScope.other);
        newDomainAndScope newGlobal = oldCtx.global.getId() == domainId ? std::make_pair(&oldCtx.self, &oldCtxScope.self) : std::make_pair(&oldCtx.global, &oldCtxScope.global);
        ContextAndScope newCtxAndScope = {
            {
                ctxAndScope.ctx.self,
                *newOther.first,
                *newGlobal.first
            },
            {
                ctxAndScope.ctxScope.self,
                *newOther.second,
                *newGlobal.second
            }
        };
        container.emplace_back(key, newCtxAndScope);
    };

    // Replace
    std::vector<std::pair<RmlElementIdentifier, ContextAndScope>> updatedElements;
    for (auto& [element, ctxAndScope] : elementToContext) {
        determineNewContext(ctxAndScope, element, updatedElements);
    }
    for (auto& [element, newCtxAndScope] : updatedElements) {
        elementToContext.emplace(element, newCtxAndScope);
    }
    std::vector<std::pair<Rml::ElementDocument*, ContextAndScope>> updatedDocuments;
    for (auto& [document, ctxAndScope] : documentToContext) {
        determineNewContext(ctxAndScope, document, updatedDocuments);
    }
    for (auto& [document, newCtxAndScope] : updatedDocuments) {
        elementToContext.emplace(document, newCtxAndScope);
    }

    // NOLINTEND
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

// Plugin for document tracking

RmlInterface::DocumentManager::DocumentManager() = default;

void RmlInterface::DocumentManager::OnDocumentLoad(Rml::ElementDocument* document){
    openedDocuments.insert(document);
}

void RmlInterface::DocumentManager::OnDocumentUnload(Rml::ElementDocument* document) {
    openedDocuments.erase(document);
}

} // namespace Nebulite::Graphics
