//------------------------------------------
// Includes

// Standard library
#include <random>

// External
#include <absl/container/flat_hash_map.h>
#include <imgui_impl_sdl3.h>
#include <imgui_impl_sdlrenderer3.h>
#include <RmlUi/Core/Input.h>
#include <SDL3_image/SDL_image.h>

// Nebulite
#include "Constants/KeyNames.hpp"
#include "Core/Environment.hpp"
#include "Core/Renderer.hpp"
#include "Core/RenderObject.hpp"
#include "DomainModule/GlobalSpace/Settings.hpp"
#include "DomainModule/Initializer.hpp"
#include "Interaction/Invoke.hpp"
#include "Nebulite.hpp"
#include "Utility/FileManagement.hpp"
#include "Utility/TimeKeeper.hpp"

//------------------------------------------
namespace Nebulite::Core {

Renderer::Renderer(Data::JsonScope& documentReference, bool* flag_headless, Utility::Capture& parentCapture)
    : Domain("Renderer", documentReference, parentCapture),
      env(documentReference, parentCapture){

    //------------------------------------------
    // Initialize internal variables

    // Window
    windowScale = 1;
    headless = flag_headless;

    // Position
    tilePositionX = 0;
    tilePositionY = 0;

    // Base directory
    baseDirectory = Utility::FileManagement::currentDir();

    //------------------------------------------
    // Start timers
    fps.controlTimer.start();
    fps.renderTimer.start();

    //------------------------------------------
    // Domain Modules
    DomainModule::Initializer::initRenderer(this);
}

Renderer::~Renderer() {

    // Clean up SDL resources
    if (renderer) {
        SDL_DestroyRenderer(renderer);
        renderer = nullptr;
    }
    if (window) {
        SDL_DestroyWindow(window);
        window = nullptr;
    }
    // Quit SDL subsystems
    if (!status.sdlInitialized)
        return;
    Rml::Shutdown();
    //IMG_Quit();
    TTF_Quit();
    SDL_Quit();
}

// TODO: Move all settings to workspace!
void Renderer::setupDisplayValues() {
    // Load from settings
    // Default values should never be used, as settings should always exist
    // Still, just in case, we set them to 1000x1000 @ 1x scaling and 60 FPS
    auto const X = Global::settings().get<uint16_t>(DomainModule::GlobalSpace::Settings::Key::resolutionX).value_or(1000);
    auto const Y = Global::settings().get<uint16_t>(DomainModule::GlobalSpace::Settings::Key::resolutionX).value_or(1000);
    windowScale = Global::settings().get<uint8_t>(DomainModule::GlobalSpace::Settings::Key::resolutionScaling).value_or(1);
    fps.target = Global::settings().get<uint16_t>(DomainModule::GlobalSpace::Settings::Key::targetFPS).value_or(60);

    // Set in workspace
    domainScope.set<unsigned int>(Constants::KeyNames::Renderer::dispResX, X*windowScale);
    domainScope.set<unsigned int>(Constants::KeyNames::Renderer::dispResY, Y*windowScale);
    domainScope.set<int>(Constants::KeyNames::Renderer::dispResXLogical, X);
    domainScope.set<int>(Constants::KeyNames::Renderer::dispResYLogical, Y);

    // Start position at 0|0
    // TODO: Move to environment?
    domainScope.set<unsigned int>(Constants::KeyNames::Renderer::positionX, 0);
    domainScope.set<unsigned int>(Constants::KeyNames::Renderer::positionY, 0);
}

Constants::Event Renderer::preParse() {
    // Initialize SDL and related subsystems
    initSDL();
    return Constants::Event::Success;
}

void Renderer::initImgui() const {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    // Pixel-friendly ImGui style for retro RPGs
    float const fullScale = SDL_GetDisplayContentScale(SDL_GetPrimaryDisplay()) * static_cast<float>(windowScale) * 0.6f; // adjust to taste

    ImGuiStyle &style = ImGui::GetStyle();

    // Rounding
    style.WindowRounding = 0.0f;
    style.ChildRounding = 0.0f;
    style.FrameRounding = 0.0f;
    style.PopupRounding = 0.0f;
    style.ScrollbarRounding = 0.0f;
    style.GrabRounding = 0.0f;

    // Borders
    style.WindowBorderSize = 1.0f;
    style.FrameBorderSize = 1.0f;
    style.TabBorderSize = 1.0f;

    // Spacing and padding: compact, consistent with retro UI
    style.WindowPadding = ImVec2(6.0f, 6.0f);
    style.FramePadding = ImVec2(6.0f, 2.0f);
    style.ItemSpacing = ImVec2(6.0f, 4.0f);
    style.ItemInnerSpacing = ImVec2(4.0f, 4.0f);
    style.CellPadding = ImVec2(4.0f, 4.0f);
    style.GrabMinSize = 8.0f;

    // Disable antialiasing for pixel-perfect rendering
    style.AntiAliasedLines = false;
    style.AntiAliasedFill  = false;

    // Scaling
    style.FontScaleDpi = fullScale;

    // Color palette: dark backgrounds, warm accent for UI (tweak hex to taste)
    auto constexpr bg      = ImVec4(0.05f, 0.07f, 0.10f, 1.00f); // deep navy
    auto constexpr panel   = ImVec4(0.10f, 0.13f, 0.16f, 1.00f); // slightly lighter
    auto constexpr accent  = ImVec4(0.92f, 0.70f, 0.16f, 1.00f); // golden accent
    auto constexpr accent2 = ImVec4(0.48f, 0.86f, 1.00f, 1.00f); // cyan for highlights
    auto constexpr textCol = ImVec4(0.92f, 0.92f, 0.92f, 1.00f); // bright text

    ImVec4* colors = style.Colors;
    colors[ImGuiCol_Text]                  = textCol;
    colors[ImGuiCol_WindowBg]              = bg;
    colors[ImGuiCol_ChildBg]               = panel;
    colors[ImGuiCol_PopupBg]               = panel;
    colors[ImGuiCol_Border]                = ImVec4(0.18f, 0.20f, 0.22f, 1.00f);
    colors[ImGuiCol_FrameBg]               = ImVec4(0.08f, 0.10f, 0.13f, 1.00f);
    colors[ImGuiCol_FrameBgHovered]        = ImVec4(0.16f, 0.18f, 0.20f, 1.00f);
    colors[ImGuiCol_FrameBgActive]         = ImVec4(0.22f, 0.24f, 0.26f, 1.00f);
    colors[ImGuiCol_Button]                = ImVec4(0.12f, 0.14f, 0.16f, 1.00f);
    colors[ImGuiCol_ButtonHovered]         = accent2;
    colors[ImGuiCol_ButtonActive]          = accent;
    colors[ImGuiCol_Header]                = ImVec4(0.12f, 0.14f, 0.16f, 1.00f);
    colors[ImGuiCol_HeaderHovered]         = accent2;
    colors[ImGuiCol_HeaderActive]          = accent;
    colors[ImGuiCol_Separator]             = ImVec4(0.14f, 0.16f, 0.18f, 1.00f);
    colors[ImGuiCol_ResizeGrip]            = ImVec4(0.12f, 0.14f, 0.16f, 1.00f);
    colors[ImGuiCol_Tab]                   = ImVec4(0.12f, 0.14f, 0.16f, 1.00f);
    colors[ImGuiCol_TabHovered]            = accent2;
    colors[ImGuiCol_TabActive]             = accent;
    colors[ImGuiCol_TitleBg]               = panel;
    colors[ImGuiCol_TitleBgActive]         = panel;

    // IO config
    ImGuiIO &io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;

    // Load a pixel font if available; fallback to default
    // Use a font config that disables oversampling and enables pixel snapping
    ImFontConfig font_cfg;
    font_cfg.OversampleH = 1;
    font_cfg.OversampleV = 1;
    font_cfg.PixelSnapH  = true;

    // Adjust the base font size to match pixel aesthetics (choose your font file & size)e
    if (Utility::FileManagement::fileExists(pixelFontPath)) {
        if (ImFont* f = io.Fonts->AddFontFromFileTTF(pixelFontPath, 40.0f * fullScale, &font_cfg, io.Fonts->GetGlyphRangesDefault()); f) io.FontDefault = f;
        else io.Fonts->AddFontDefault();
    } else {
        io.Fonts->AddFontDefault();
    }

    ImGui_ImplSDL3_InitForSDLRenderer(window, renderer);
    ImGui_ImplSDLRenderer3_Init(renderer);
}

void Renderer::initRmlUi() {
    Rml::Initialise();

    // Interfaces
    rml.renderInterface = std::make_unique<RenderInterface_SDL>(renderer);
    if (!rml.renderInterface) {
        throw std::runtime_error("Failed to create RmlUi render interface!");
    }
    SetRenderInterface(rml.renderInterface.get());
    rml.systemInterface = std::make_unique<SystemInterface_SDL>(window);
    if (!rml.systemInterface) {
        throw std::runtime_error("Failed to create system interface!");
    }

    // Context
    rml.context = Rml::CreateContext(
        "main", {
            static_cast<int>(
                domainScope.get<double>(Constants::KeyNames::Renderer::dispResX).value_or(800.0)
            ),
            static_cast<int>(
                domainScope.get<double>(Constants::KeyNames::Renderer::dispResY).value_or(600.0)
            )
        }
    );
    if (!rml.context) {
        throw std::runtime_error("Failed to create RmlUi context!");
    }

    if (!Rml::LoadFontFace(pixelFontPath)) {
        throw std::runtime_error("Failed to load font!");
    }

    // Data Model
    rml.dataModelConstructor = rml.context->CreateDataModel("renderer");
    rml.updateVariables(domainScope);

    // Demo Document
    auto const document = Utility::FileManagement::LoadFile("./Resources/Rml/alignment.rml");
    rml.demoDocument = rml.context->LoadDocumentFromMemory(document);
    if (!rml.demoDocument) {
        throw std::runtime_error("Failed to load RmlUi document from memory!");
    }
    rml.demoDocument->Show();
}

void Renderer::initSDL() {
    if (status.sdlInitialized)
        return;

    //------------------------------------------
    // Window
    setupDisplayValues();

    // Create SDL window
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        // SDL initialization failed
        capture.error.println("SDL_Init Video Error: ", SDL_GetError());
        std::abort();
    }
    // Define window via x|y|w|h
    int const w = domainScope.get<int>(Constants::KeyNames::Renderer::dispResX).value_or(0);
    int const h = domainScope.get<int>(Constants::KeyNames::Renderer::dispResY).value_or(0);

    //------------------------------------------
    // Window and renderer
    uint32_t const flags = *headless ? SDL_WINDOW_HIDDEN : 0
    | SDL_WINDOW_HIGH_PIXEL_DENSITY // Add more necessary flags here
    ;

    if (!SDL_CreateWindowAndRenderer("Nebulite", w*windowScale, h*windowScale, flags, &window, &renderer)) {
        capture.error.println("SDL_CreateWindowAndRenderer Error: ", SDL_GetError());
        std::abort();
    }
    SDL_SetWindowPosition(window, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);

    //------------------------------------------
    // UI
    initImgui();
    initRmlUi();

    //------------------------------------------
    // Cursor
    static auto const cursorPath = "./Resources/Cursor/Drakensang.png";

    // See if cursor file exists
    if (Utility::FileManagement::fileExists(cursorPath)) {
        // Load pixel data
        if (SDL_Surface* cursorSurface = IMG_Load(cursorPath); cursorSurface) {
            // Create cursor
            if (SDL_Cursor* cursor = SDL_CreateColorCursor(cursorSurface, 0, 0); cursor) {
                SDL_SetCursor(cursor);
            } else {
                capture.error.println("Failed to create cursor: ", SDL_GetError());
                // No quit, just use default cursor
            }
        }
    }

    //------------------------------------------
    // Fonts

    // Initialize SDL_ttf
    if (!TTF_Init()) {
        // Handle SDL_ttf initialization error
        capture.error.println("TTF_Init Error: ", SDL_GetError());
        std::abort();
    }
    loadFonts();

    //------------------------------------------
    // Check for remaining errors in SDL

    /**
     * @todo On wine, it says: "Device not found", but seems to work anyway. Investigate further.
     *       For now, we just log the error and continue
     */
    if (SDL_GetError()[0] != '\0') {
        capture.error.println("SDL Error during initialization: ", SDL_GetError());
        SDL_ClearError(); // Clear error after reporting
    }

    // All done
    status.sdlInitialized = true;
}

void Renderer::loadFonts() {
    //------------------------------------------
    // Sizes
    uint32_t constexpr FontSizeGeneral = 60; // Does not need to scale

    //------------------------------------------
    // Font location
    std::string const sep(1, Utility::FileManagement::preferredSeparator());
    std::string const fontDir = std::string("Resources") + sep + std::string("Fonts") + sep + std::string("Arimo-Regular.ttf");
    std::string const fontPath = Utility::FileManagement::CombinePaths(baseDirectory, fontDir);

    //------------------------------------------
    // Load general font
    font = TTF_OpenFont(fontPath.c_str(), FontSizeGeneral); // Adjust size as needed
    if (font == nullptr) {
        // Handle font loading error
        capture.error.println("Failed to load font: ", fontPath);
        std::abort();
    }
}

//------------------------------------------
// Getting

int Renderer::getResX() const { return domainScope.get<int>(Constants::KeyNames::Renderer::dispResX).value_or(0); }

int Renderer::getResY() const { return domainScope.get<int>(Constants::KeyNames::Renderer::dispResY).value_or(0); }

int Renderer::getPosX() const { return domainScope.get<int>(Constants::KeyNames::Renderer::positionX).value_or(0); }

int Renderer::getPosY() const { return domainScope.get<int>(Constants::KeyNames::Renderer::positionY).value_or(0); }

//------------------------------------------
// Serialization / Deserialization

std::string Renderer::serialize() {
    return env.serialize();
}

void Renderer::deserialize(std::string const& serialOrLink) noexcept {
    env.deserialize(
        serialOrLink,
        domainScope.get<uint16_t>(Constants::KeyNames::Renderer::dispResX).value_or(0),
        domainScope.get<uint16_t>(Constants::KeyNames::Renderer::dispResY).value_or(0)
    );
}

//------------------------------------------
// Pipeline

void Renderer::renderInit() {
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255); // RGB values (black)
    SDL_RenderClear(renderer);
    if (!status.firstFrameRendered) {
        ImGui_ImplSDLRenderer3_NewFrame();
        ImGui_ImplSDL3_NewFrame();
        ImGui::NewFrame();
        status.firstFrameRendered = true;
    }
}

void Renderer::renderFPS() const {
    ImGui::SetNextWindowPos(ImVec2(5.0f, 5.0f), ImGuiCond_Always);
    ImGui::SetNextWindowBgAlpha(0.35f);

    // Make the window tighter: small padding and item spacing
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(6.0f, 2.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(4.0f, 2.0f));

    // Optional: constrain maximum size (min 0, max 150x50)
    //ImGui::SetNextWindowSizeConstraints(ImVec2(0, 0), ImVec2(150.0f, 50.0f));

    ImGui::Begin(
        "FPS Overlay",
        nullptr,
        ImGuiWindowFlags_NoDecoration |
        ImGuiWindowFlags_AlwaysAutoResize |
        ImGuiWindowFlags_NoFocusOnAppearing |
        ImGuiWindowFlags_NoNav
    );

    ImGui::Text("FPS: %04d", fps.real);
    ImGui::End();
    ImGui::PopStyleVar(2); // pop ItemSpacing and WindowPadding
}

void Renderer::pollEvents() {
    SDL_Event event{};
    events.clear();
    while (SDL_PollEvent(&event)) {
        // Store events for other processes, e.g. domain modules
        events.push_back(event);

        // Handle quit event
        if (event.type == SDL_EVENT_QUIT) { status.quit = true; }
    }
}

namespace {
Rml::Input::KeyIdentifier SDLKeyToRmlKey(SDL_Keycode const& keycode) {
    using KI = Rml::Input::KeyIdentifier;
    switch (keycode) {
    case SDLK_BACKSPACE: return KI::KI_BACK;
    case SDLK_TAB:       return KI::KI_TAB;
    case SDLK_RETURN:    return KI::KI_RETURN;
    case SDLK_ESCAPE:    return KI::KI_ESCAPE;
    case SDLK_SPACE:     return KI::KI_SPACE;
    case SDLK_DELETE:    return KI::KI_DELETE;
    default:             return KI::KI_UNKNOWN;
    }
}

int SdlModifierToRmlModifier(uint32_t const& modifier) {
    return static_cast<int>(0 * modifier); // TODO: implement
}

} // namespace

void Renderer::processRmlUiEvent(const SDL_Event& event) const {
    if (!rml.context) return;

    switch (event.type) {

    case SDL_EVENT_MOUSE_MOTION:
        rml.context->ProcessMouseMove(
            static_cast<int>(event.motion.x),
            static_cast<int>(event.motion.y),
            0
        );
        break;

    case SDL_EVENT_MOUSE_BUTTON_DOWN:
    case SDL_EVENT_MOUSE_BUTTON_UP: {
        int button = 0;
        if (event.button.button == SDL_BUTTON_LEFT) button = 0;
        else if (event.button.button == SDL_BUTTON_RIGHT) button = 1;
        else if (event.button.button == SDL_BUTTON_MIDDLE) button = 2;

        if (event.type == SDL_EVENT_MOUSE_BUTTON_DOWN)
            rml.context->ProcessMouseButtonDown(button, 0);
        else
            rml.context->ProcessMouseButtonUp(button, 0);
        break;
    }

    case SDL_EVENT_MOUSE_WHEEL: {
        Rml::Vector2f const wheel = {event.wheel.x, event.wheel.y};
        auto const mods = SdlModifierToRmlModifier(event.key.mod);
        rml.context->ProcessMouseWheel(wheel, mods);
        break;
    }

    case SDL_EVENT_KEY_DOWN:
    case SDL_EVENT_KEY_UP: {
        auto const rmlKey = SDLKeyToRmlKey(event.key.scancode);
        auto const mods = SdlModifierToRmlModifier(event.key.mod);

        if (event.type == SDL_EVENT_KEY_DOWN)
            rml.context->ProcessKeyDown(rmlKey, mods);
        else
            rml.context->ProcessKeyUp(rmlKey, mods);
        break;
    }

    case SDL_EVENT_TEXT_INPUT:
        rml.context->ProcessTextInput(event.text.text);
        break;

    default:
        break;
    }
}

void Renderer::render() {
    //---------------------------------------
    // Pre-render processing

    // Event polling and processing
    pollEvents();
    for (auto const& event : events) {
        ImGui_ImplSDL3_ProcessEvent(&event);
        processRmlUiEvent(event);
    }

    //---------------------------------------
    // Frame rendering

    // Core
    renderInit();
    renderFrame();
    status.skippedUpdateLastFrame = status.skipUpdate;
    status.skipUpdate = false;
    updateModules(); // Update domain modules, potentially adding ImGui elements

    // RML
    // Update variables
    rml.updateVariables(domainScope);
    rml.context->Update();
    rml.context->Render();

    // Imgui
    if (status.showFps) renderFPS();
    ImGui::Render();
    ImGui_ImplSDLRenderer3_RenderDrawData(ImGui::GetDrawData(), renderer);

    // Present frame
    SDL_RenderPresent(renderer);

    //---------------------------------------
    // Post-render processing

    // Custom callback functions
    for (auto const& callback : postRenderCallback) {
        callback();
    }
    postRenderCallback.clear();

    // Start new imgui frame instantly, so that modules can render to it
    ImGui_ImplSDLRenderer3_NewFrame();
    ImGui_ImplSDL3_NewFrame();
    ImGui::NewFrame();
}

Constants::Event Renderer::update() {
    if (!status.skipUpdate) { // Skip update if flagged
        // Update environment
        Global::instance().notifyEvent(env.update());
        env.updateObjects(
            tilePositionX,
            tilePositionY,
            domainScope.get<uint16_t>(Constants::KeyNames::Renderer::dispResX).value_or(0),
            domainScope.get<uint16_t>(Constants::KeyNames::Renderer::dispResY).value_or(0),
            rendererProcessor
        );
    }
    if (SDL_GetError()[0] != '\0') {
        capture.error.println("SDL Error during rendering: ", SDL_GetError());
        SDL_ClearError();
        return Constants::Event::Warning;
    }
    return Constants::Event::Success;
}

bool Renderer::timeToRender() {
    // Goal: projected_dt() == target
    // Issue: target might be fractional, projected_dt() is integer milliseconds
    // Solution: use probabilistic rounding
    // Example: target = 16.67 ms
    // set target to 16, remainder = 0.67
    // so do 16 for 67% of the time, and 17 for 33% of the time
    static std::uniform_real_distribution distribution(0.0, 1.0);
    static std::hash<std::string> hashString;
    static std::mt19937 randNum(hashString("RNG for FPS control"));

    double const target = 1000.0 / static_cast<double>(fps.target);
    double const remainder = target - static_cast<double>(static_cast<uint32_t>(target)); // between 0.0 and 1.0
    uint32_t const adjustedTarget = static_cast<uint32_t>(target) + (distribution(randNum) < remainder ? 1 : 0);
    return fps.controlTimer.projected_dt() >= adjustedTarget;
}

void Renderer::append(RenderObject* toAppend) {
    // Add domain id to map
    indexToIdMap[indexCounter] = toAppend->getId();
    indexCounter++;

    // Append to environment, based on layer
    env.append(
        toAppend,
        domainScope.get<uint16_t>(Constants::KeyNames::Renderer::dispResX).value_or(0),
        domainScope.get<uint16_t>(Constants::KeyNames::Renderer::dispResY).value_or(0),
        toAppend->domainScope.get<uint8_t>(Constants::KeyNames::RenderObject::layer).value_or(0)
    );
}

void Renderer::reinsertAllObjects() {
    env.reinsertAllObjects(
    domainScope.get<uint16_t>(Constants::KeyNames::Renderer::dispResX).value_or(0),
    domainScope.get<uint16_t>(Constants::KeyNames::Renderer::dispResY).value_or(0)
    );
}

//------------------------------------------
// Purge

void Renderer::purgeObjects() {
    env.purgeObjects();
}

void Renderer::purgeTextures() {
    // Release resources for TextureContainer
    for (auto const& texture : std::views::values(TextureContainer)) {
        destroyTexture(texture);
    }
    TextureContainer.clear(); // Clear the map to release resources
}

void Renderer::destroy() {
    if (!status.sdlInitialized)
        return;
    if (window) {
        SDL_DestroyWindow(window);
        window = nullptr;
    }
    if (renderer) {
        SDL_DestroyRenderer(renderer);
        renderer = nullptr;
    }
    if (font) {
        TTF_CloseFont(font);
        font = nullptr;
    }
}

//------------------------------------------
// Manipulation

// This does not change the settings file, only the current session
void Renderer::changeWindowSize(int const& w, int const& h, uint8_t const& scalar) {
    // Validate resolution and scalar
    if (w < 240 || w > 16384) {
        capture.error.println("Selected resolution is not supported:", w, "x", h);
        return;
    }
    if (h < 240 || h > 16384) {
        capture.error.println("Selected resolution is not supported:", w, "x", h);
        return;
    }
    if ( scalar < 1 || scalar > 8) {
        capture.error.println("Selected window scaling is not supported:", static_cast<int>(scalar), "x");
        return;
    }

    // Set new scalar
    windowScale = scalar;

    // Set the new resolution in the workspace
    domainScope.set<int>(Constants::KeyNames::Renderer::dispResX, w * windowScale);
    domainScope.set<int>(Constants::KeyNames::Renderer::dispResY, h * windowScale);
    domainScope.set<int>(Constants::KeyNames::Renderer::dispResXLogical, w);
    domainScope.set<int>(Constants::KeyNames::Renderer::dispResYLogical, h);
    domainScope.set<uint8_t>(Constants::KeyNames::Renderer::windowScale, windowScale);

    // Set the physical window size
    SDL_SetWindowSize(window, w * windowScale, h * windowScale);

    // Rescale rml context
    rml.context->SetDimensions({
        w * windowScale,
        h * windowScale
    });

    // Reinsert objects
    // TODO: Once fixed tiles are implemented, this isn't needed anymore
    reinsertAllObjects();
}

void Renderer::moveCam(int const& dX, int const& dY) const {
    domainScope.set<int>(
        Constants::KeyNames::Renderer::positionX,
        domainScope.get<int>(Constants::KeyNames::Renderer::positionX).value_or(0) + dX
    );
    domainScope.set<int>(
        Constants::KeyNames::Renderer::positionY,
        domainScope.get<int>(Constants::KeyNames::Renderer::positionY).value_or(0) + dY
    );
}

void Renderer::setCam(int const& X, int const& Y, bool const& isMiddle) const {
    int newPosX = X;
    int newPosY = Y;
    if (isMiddle) {
        newPosX -= domainScope.get<int>(Constants::KeyNames::Renderer::dispResX).value_or(0) / 2;
        newPosY -= domainScope.get<int>(Constants::KeyNames::Renderer::dispResY).value_or(0) / 2;
    }
    domainScope.set<int>(Constants::KeyNames::Renderer::positionX, newPosX);
    domainScope.set<int>(Constants::KeyNames::Renderer::positionY, newPosY);
}

//------------------------------------------
// Setting

void Renderer::setTargetFPS(uint16_t const& targetFps) {
    fps.target = targetFps;
}

//------------------------------------------
// Renderer::tick Functions

void Renderer::renderFrame() {
    //------------------------------------------
    // Store for faster access

    // Get camera position
    auto const dispPosX = domainScope.get<int16_t>(Constants::KeyNames::Renderer::positionX).value_or(0);
    auto const dispPosY = domainScope.get<int16_t>(Constants::KeyNames::Renderer::positionY).value_or(0);

    // Depending on position, set tiles to render
    auto const dispResX = domainScope.get<int16_t>(Constants::KeyNames::Renderer::dispResX).value_or(0);
    auto const dispResY = domainScope.get<int16_t>(Constants::KeyNames::Renderer::dispResY).value_or(0);
    if (dispResX == 0 || dispResY == 0) {
        // Avoid division by zero
        capture.error.println("Display resolution is zero, cannot render frame.");
        std::abort();
    }
    tilePositionX = static_cast<int16_t>(dispPosX / dispResX);
    tilePositionY = static_cast<int16_t>(dispPosY / dispResY);

    //------------------------------------------
    // FPS Count and Control

    //Calculate fps every second
    fps.realCounter++;
    if (fps.renderTimer.projected_dt() >= 1000) {
        fps.real = fps.realCounter;
        fps.realCounter = 0;
        fps.renderTimer.update();
    }

    // Control framerate
    fps.controlTimer.update();

    //------------------------------------------
    // Rendering
    if (renderer) {
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255); // Black background
    }

    //Render Objects
    //For all layers, starting at 0
    for (auto const& layer : env.getAllLayerTypes()) {
        // Get all tile positions to render
        std::vector<std::pair<int16_t, int16_t>> tilesToRender;
        for (int dX = tilePositionX == 0 ? 0 : -1; dX <= 1; dX++) {
            for (int dY = tilePositionY == 0 ? 0 : -1; dY <= 1; dY++) {
                if (env.isValidPosition(tilePositionX + dX, tilePositionY + dY, layer)) {
                    tilesToRender.emplace_back(tilePositionX + dX, tilePositionY + dY);
                }
            }
        }

        // For all tiles to render
        for (auto const& [tileX, tileY] : tilesToRender) {
            // For all batches inside
            for (auto const& [objectsInThisBatch, _] : env.getContainerAt(tileX, tileY, layer)) {
                // For all objects in batch
                for (auto const& obj : objectsInThisBatch) {
                    renderObjectToScreen(obj, dispPosX, dispPosY);
                }
            }
        }

        // Render all textures that were attached from outside processes
        for (auto const& [texture, rect] : std::views::values(BetweenLayerTextures[layer])) {
            if (!texture) {
                continue; // Skip if texture is null
            }
            // We assume the rect was already scaled correctly when added
            SDL_FRect const rectF = {
                static_cast<float>(rect->x),
                static_cast<float>(rect->y),
                static_cast<float>(rect->w),
                static_cast<float>(rect->h)
            };
            if (!SDL_RenderTexture(renderer, texture, nullptr, &rectF)) {
                capture.error.println("Failed to render between-layer texture: ", SDL_GetError());
            }
        }
    }
}

void Renderer::renderObjectToScreen(RenderObject* obj, int const& dispPosX, int const& dispPosY) {
    obj->draw(
        static_cast<float>(dispPosX),
        static_cast<float>(dispPosY)
    );
}

//------------------------------------------
// Texture-Related

SDL_Texture* Renderer::getTexture(std::string const& link) {
    // Check if texture is already loaded
    if (auto const it = TextureContainer.find(link); it != TextureContainer.end()) {
        return it->second;
    }

    // Load texture if not found
    SDL_Texture* texture = loadTextureToMemory(link);
    if (texture != nullptr) {
        TextureContainer[link] = texture;
    }
    return texture;
}

void Renderer::loadTexture(std::string const& link) {
    if (auto const t = loadTextureToMemory(link); t != nullptr) TextureContainer[link] = t;
}

SDL_Texture* Renderer::loadTextureToMemory(std::string const& link) {
    std::string const path = Utility::FileManagement::CombinePaths(baseDirectory, link);

    // Get file extension, based on last dot
    std::string extension;
    if (size_t const dotPos = path.find_last_of('.'); dotPos != std::string::npos) {
        extension = path.substr(dotPos + 1);
    } else {
        capture.error.println("Failed to load image '", path, "': No file extension found.");
        return nullptr;
    }

    // turn to lowercase
    std::ranges::transform(extension.begin(), extension.end(), extension.begin(), tolower);

    // Check for known image formats
    SDL_Surface* surface = nullptr;
    if (extension == "bmp") {
        surface = SDL_LoadBMP(path.c_str());
    } else if (extension == "png" || extension == "jpg" || extension == "jpeg" || extension == "tif" || extension == "tiff" || extension == "webp" || extension == "gif") {
        surface = IMG_Load(path.c_str());
    }

    // Unknown format or other issues with surface
    if (surface == nullptr) {
        capture.error.println("Failed to load image '", path, "': ", SDL_GetError());
        return nullptr;
    }

    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_DestroySurface(surface); // Free the surface after creating texture

    // Check for texture issues
    if (!texture) {
        capture.error.println("Failed to create texture from image '", path, "': ", SDL_GetError());
        return nullptr;
    }

    SDL_SetTextureScaleMode(texture, SDL_SCALEMODE_NEAREST);
    return texture;
}

} // namespace Nebulite::Core
