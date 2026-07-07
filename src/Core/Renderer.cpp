//------------------------------------------
// Includes

// Standard library
#include <algorithm>
#include <cctype>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <functional>
#include <optional>
#include <random>
#include <ranges>
#include <type_traits>
#include <utility>
#include <vector>

// External
#include <SDL3/SDL_error.h>
#include <SDL3/SDL_events.h>
#include <SDL3/SDL_init.h>
#include <SDL3/SDL_keyboard.h>
#include <SDL3/SDL_mouse.h>
#include <SDL3/SDL_rect.h>
#include <SDL3/SDL_render.h>
#include <SDL3/SDL_scancode.h>
#include <SDL3/SDL_surface.h>
#include <SDL3/SDL_video.h>
#include <SDL3_image/SDL_image.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <imgui.h>
#include <imgui_impl_sdl3.h>
#include <imgui_impl_sdlrenderer3.h>

// Nebulite
#include "Nebulite/Constants/Event.hpp"
#include "Nebulite/Constants/KeyNames.hpp"
#include "Nebulite/Core/GlobalSpace.hpp"
#include "Nebulite/Core/RenderObject.hpp"
#include "Nebulite/Core/Renderer.hpp"
#include "Nebulite/Data/RenderObjectContainer.hpp"
#include "Nebulite/Data/RendererProcessor.hpp"
#include "Nebulite/Data/Tiling.hpp"
#include "Nebulite/Graphics/RmlInterface.hpp"
#include "Nebulite/Module/Domain/GlobalSpace/Settings.hpp"
#include "Nebulite/Module/Domain/Initializer.hpp"
#include "Nebulite/Nebulite.hpp"
#include "Nebulite/Utility/IO/Capture.hpp"
#include "Nebulite/Utility/IO/FileManagement.hpp"

//------------------------------------------
namespace Nebulite::Core {

namespace {
double fontSize(auto const fontSizeKey, double defaultValue) {
    return Global::settings().get<double>(fontSizeKey).value_or(defaultValue) * Global::settings().get<double>(Module::Domain::GlobalSpace::Settings::Key::fontScale).value_or(1.0);
}
} // namespace

Renderer::Renderer(Data::JsonScope& documentReference, bool* flag_headless, Utility::IO::Capture& parentCapture) :
    Domain("Renderer", documentReference, parentCapture),
    headless(flag_headless),
    env(documentReference, parentCapture){
    //------------------------------------------
    // Initialize internal variables
    baseDirectory = Utility::IO::FileManagement::currentDir();

    //------------------------------------------
    // Inherit
    inherit(&env);

    //------------------------------------------
    // Start timers
    fps.controlTimer.start();
    fps.renderTimer.start();

    //------------------------------------------
    // Domain Modules
    Module::Domain::Initializer::initRenderer(this);
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
    if (!status.sdlInitialized) return;
    TTF_Quit();
    SDL_Quit();
}

void Renderer::setupDisplayValues() {
    // Load from settings
    // Default values should never be used, as settings should always exist
    // Still, just in case, we set them to 1000x1000 @ 1x scaling and 60 FPS
    auto const X = Global::settings().get<uint16_t>(Module::Domain::GlobalSpace::Settings::Key::resolutionX).value_or(1000);
    auto const Y = Global::settings().get<uint16_t>(Module::Domain::GlobalSpace::Settings::Key::resolutionX).value_or(1000);
    windowScale = Global::settings().get<uint8_t>(Module::Domain::GlobalSpace::Settings::Key::resolutionScaling).value_or(1);
    fps.target = Global::settings().get<uint16_t>(Module::Domain::GlobalSpace::Settings::Key::targetFPS).value_or(60);

    // Set in workspace
    domainScope.set<unsigned int>(Constants::KeyNames::Renderer::dispResXWindow, X*windowScale);
    domainScope.set<unsigned int>(Constants::KeyNames::Renderer::dispResYWindow, Y*windowScale);
    domainScope.set<int>(Constants::KeyNames::Renderer::dispResXLogical, X);
    domainScope.set<int>(Constants::KeyNames::Renderer::dispResYLogical, Y);

    // Start position at 0|0
    // TODO: Move to environment?
    domainScope.set<unsigned int>(Constants::KeyNames::Renderer::positionX, 0);
    domainScope.set<unsigned int>(Constants::KeyNames::Renderer::positionY, 0);
}

Constants::Event Renderer::preParse() {
    initSDL();
    return Constants::Event::Success;
}

void Renderer::initImgui() {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    // Pixel-friendly ImGui style for retro RPGs
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
    float const fullScale = SDL_GetDisplayContentScale(SDL_GetPrimaryDisplay()) * static_cast<float>(windowScale) * 0.6f; // adjust to taste
    style.FontScaleDpi = fullScale;

    // Color palette: dark backgrounds, warm accent for UI (tweak hex to taste)
    auto constexpr layer0  = ImVec4(0.05f, 0.07f, 0.10f, 1.00f); // deep navy
    auto constexpr layer1  = ImVec4(0.10f, 0.13f, 0.16f, 1.00f); // slightly lighter
    auto constexpr accent  = ImVec4(0.92f, 0.70f, 0.16f, 1.00f); // golden accent
    auto constexpr accent2 = ImVec4(0.48f, 0.86f, 1.00f, 1.00f); // cyan for highlights
    auto constexpr textCol = ImVec4(0.92f, 0.92f, 0.92f, 1.00f); // bright text
    auto constexpr border  = ImVec4(0.18f, 0.20f, 0.22f, 1.00f);
    auto constexpr button  = ImVec4(0.12f, 0.14f, 0.16f, 1.00f);

    auto& colors = style.Colors;
    colors[ImGuiCol_Text]                  = textCol;
    colors[ImGuiCol_WindowBg]              = layer0;
    colors[ImGuiCol_ChildBg]               = layer1;
    colors[ImGuiCol_PopupBg]               = layer1;
    colors[ImGuiCol_Border]                = border;
    colors[ImGuiCol_FrameBg]               = ImVec4(0.08f, 0.10f, 0.13f, 1.00f);
    colors[ImGuiCol_FrameBgHovered]        = ImVec4(0.16f, 0.18f, 0.20f, 1.00f);
    colors[ImGuiCol_FrameBgActive]         = ImVec4(0.22f, 0.24f, 0.26f, 1.00f);
    colors[ImGuiCol_Button]                = button;
    colors[ImGuiCol_ButtonHovered]         = accent2;
    colors[ImGuiCol_ButtonActive]          = accent;
    colors[ImGuiCol_Header]                = button;
    colors[ImGuiCol_HeaderHovered]         = accent2;
    colors[ImGuiCol_HeaderActive]          = accent;
    colors[ImGuiCol_Separator]             = ImVec4(0.14f, 0.16f, 0.18f, 1.00f);
    colors[ImGuiCol_ResizeGrip]            = button;
    colors[ImGuiCol_Tab]                   = button;
    colors[ImGuiCol_TabHovered]            = accent2;
    colors[ImGuiCol_TabActive]             = accent;
    colors[ImGuiCol_TitleBg]               = layer1;
    colors[ImGuiCol_TitleBgActive]         = layer1;

    // IO config
    ImGuiIO &io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard | ImGuiConfigFlags_NavEnableGamepad;

    // Load a pixel font if available; fallback to default
    // Use a font config that disables oversampling and enables pixel snapping
    auto const size = fontSize(Module::Domain::GlobalSpace::Settings::Key::fontSize1, 40.0);
    ImFontConfig font_cfg;
    font_cfg.OversampleH = 1;
    font_cfg.OversampleV = 1;
    font_cfg.PixelSnapH  = true;
    font_cfg.SizePixels = static_cast<float>(size) * fullScale;

    // Set different default font config
    ImFontConfig defaultFontCfg;
    defaultFontCfg.SizePixels = static_cast<float>(size) * fullScale;

    // Load font, fallback to default if not found or failed to load
    auto const fontPath = Global::settings().get<std::string>(Module::Domain::GlobalSpace::Settings::Key::fontMono).value_or("null");
    if (Utility::IO::FileManagement::fileExists(fontPath)) {
        if (ImFont* f = io.Fonts->AddFontFromFileTTF(fontPath.c_str(), 0, &font_cfg, io.Fonts->GetGlyphRangesDefault()); f) {
            io.FontDefault = f;
        }
        else {
            capture.warning.println("Failed to load font: ", fontPath, ". Falling back to default font.");
            io.Fonts->AddFontDefault(&defaultFontCfg);
        }
    } else {
        capture.warning.println("Font file: ", fontPath, " not found. Falling back to default font");
        io.Fonts->AddFontDefault(&defaultFontCfg);
    }

    // Initialize
    ImGui_ImplSDL3_InitForSDLRenderer(window, renderer);
    ImGui_ImplSDLRenderer3_Init(renderer);
}

void Renderer::initSDL() {
    if (status.sdlInitialized) return;

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
    int const w = domainScope.get<int>(Constants::KeyNames::Renderer::dispResXWindow).value_or(0);
    int const h = domainScope.get<int>(Constants::KeyNames::Renderer::dispResYWindow).value_or(0);

    //------------------------------------------
    // Window and renderer
    std::uint32_t const flags = *headless ? SDL_WINDOW_HIDDEN : 0
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
    Graphics::RmlInterface::instance().init(*this,w,h);

    //------------------------------------------
    // Cursor

    // TODO: Cursor is overwritten by RmlUi, we need to combine the logic and offer cursor overwrites for different scenarios.
    //       see availableCursors in Graphics::RmlSystemInterface

    // Try to load specified cursor, fall back to default
    static auto const cursorPath = Global::settings().get<std::string>(Module::Domain::GlobalSpace::Settings::Key::cursor).value_or("");
    if (!cursorPath.empty()) { // No file specified -> keep default cursor
        // Load pixel data
        if (SDL_Surface* cursorSurface = IMG_Load(cursorPath.c_str()); cursorSurface) {
            // Create cursor
            if (SDL_Cursor* cursor = SDL_CreateColorCursor(cursorSurface, 0, 0); cursor) {
                SDL_SetCursor(cursor);
            } else {
                capture.warning.println("Failed to create cursor: ", SDL_GetError());
            }
        }
        else {
            capture.warning.println("Failed to load cursor: ", cursorPath, ". Using default cursor.");
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
    auto const FontSizeGeneral = fontSize(Module::Domain::GlobalSpace::Settings::Key::fontSize3, 80.0);

    //------------------------------------------
    // Font location
    std::string const fontPath = Global::settings().get<std::string>(Module::Domain::GlobalSpace::Settings::Key::fontStandard).value_or("null");

    //------------------------------------------
    // Load general font
    font = TTF_OpenFont(fontPath.c_str(), static_cast<float>(FontSizeGeneral)); // Adjust size as needed
    if (font == nullptr) {
        // Handle font loading error
        capture.error.println("Failed to load font: ", fontPath);
        std::abort();
    }
}

//------------------------------------------
// Serialization / Deserialization

// TODO: serialize/deserialize domain itself, pass subDoc for env into renderer
//       However, this is not as important, as env might not stay as a child domain of renderer

std::string Renderer::serialize() {
    return env.serialize();
}

void Renderer::deserialize(std::string const& serialOrLink) noexcept {
    env.deserialize(
        serialOrLink,
        tilingInformation()
    );
}

//------------------------------------------
// Get objects

std::optional<size_t> Renderer::getIdFromIndex(std::size_t const index) const {
    return env.getIdFromIndex(index);
}

std::optional<size_t> Renderer::getIndexFromId(std::size_t const searchId) const {
    return env.getIndexFromId(searchId);
}

//------------------------------------------
// Viewport

void Renderer::setView(ViewSetting const view) noexcept {
    viewSetting = view;
}

std::vector<Data::TileCoordinate> Renderer::visibleTiles() const {
    auto getTileCount = [&]() -> std::pair<int, int> {
        auto const w = domainScope.get<int16_t>(Constants::KeyNames::Renderer::dispResXLogical).value_or(0);
        auto const h = domainScope.get<int16_t>(Constants::KeyNames::Renderer::dispResYLogical).value_or(0);

        switch (viewSetting) {
        case ViewSetting::high: return {
            w / tilingInformation().w + 1,
            h / tilingInformation().h + 1
        };
        case ViewSetting::low: return {
            w/4 / tilingInformation().w + 1,
            h/4 / tilingInformation().h + 1
        };
        case ViewSetting::lowest:
            return {1,1};
        default:
            std::unreachable();
        }
    };

    auto [wCount, hCount] = getTileCount();

    std::vector<Data::TileCoordinate> tiles;
    tiles.reserve(static_cast<size_t>(wCount)*static_cast<size_t>(hCount)*4u); // small fixed neighborhood
    for (auto const dX : std::views::iota(-wCount, wCount+1)) {
        for (auto const dY : std::views::iota(-hCount, hCount+1)) {
            tiles.emplace_back(
                static_cast<int16_t>(cameraTilePosition.x + dX),
                static_cast<int16_t>(cameraTilePosition.y + dY)
            );
        }
    }
    return tiles;
}

void Renderer::onViewport(Environment::Layer const layer, auto&& function) {
    for (auto const& [tile, coordinate] : env.viewport(visibleTiles(), layer)) {
        for (auto const& objects : tile->getBatchedObjects()) {
            for (auto* obj : objects) {
                function(obj);
            }
        }
    }
}

void Renderer::onViewportTiles(Environment::Layer const layer, auto&& function) {
    for (auto const& tileAndCoordinate : env.viewport(visibleTiles(), layer)) {
        function(tileAndCoordinate);
    }
}

Data::Tile const& Renderer::getTile(Environment::Layer const layer, Data::TileCoordinate const& pos) {
    return env.getContainerAt(pos, layer);
}

Data::TilingInformation Renderer::tilingInformation() {
    // If we ever decide to make the tiles depend on the resolution,
    // we must re-activate reinsertion of all objects on resolution change!
    return {.w=128, .h=128};
}

//------------------------------------------
// Pipeline

void Renderer::renderInit() const {
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255); // RGB values (black)
    SDL_RenderClear(renderer);
    ImGui_ImplSDLRenderer3_NewFrame();
    ImGui_ImplSDL3_NewFrame();
    ImGui::NewFrame();
}

void Renderer::renderFPS() const {
    ImGui::SetNextWindowPos(ImVec2(5.0f, 5.0f), ImGuiCond_Always);
    ImGui::SetNextWindowBgAlpha(0.35f);

    // Make the window tighter: small padding and item spacing
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(6.0f, 2.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(4.0f, 2.0f));

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

void Renderer::render() {
    //---------------------------------------
    // Pre-render processing

    // Event polling and processing
    pollEvents();
    for (auto const& event : events) {
        ImGui_ImplSDL3_ProcessEvent(&event);

        // Skip tab key if Imgui wants text input. Otherwise, RmlUi tries to focus some element.
        if (event.type == SDL_EVENT_KEY_DOWN && event.key.scancode == SDL_SCANCODE_TAB && ImGui::GetIO().WantTextInput) {
            continue;
        }

        Graphics::RmlInterface::instance().processRmlUiEvent(event);
    }

    // TODO: rml.isTextInputFocused() is somehow very buggy: cursor has to be below the text input field for it to register
    //       -> scaling issue or mouse delta accumulation issue?
    // External text focus management
    if (Graphics::RmlInterface::instance().isTextInputFocused() || ImGui::GetIO().WantTextInput) {
        SDL_StartTextInput(window);
    }
    else if (!Graphics::RmlInterface::instance().isTextInputFocused() && !ImGui::GetIO().WantTextInput) {
        SDL_StopTextInput(window);
    }

    //---------------------------------------
    // Frame rendering

    // Core
    renderInit();
    renderFrame();
    status.skippedUpdateLastFrame = status.skipUpdate;
    status.skipUpdate = false;
    updateModules();
    parseTaskQueues(true);

    // RML
    // Update variables
    float x = 0;
    float y = 0;
    SDL_GetMouseState(&x,&y);
    Graphics::RmlInterface::instance().update(
        static_cast<int>(x),
        static_cast<int>(y)
    );
    Graphics::RmlInterface::instance().render();

    // Finalize render
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
    Graphics::RmlInterface::instance().postRenderUpdate();
    postRenderCallback.clear();
}

Constants::Event Renderer::update() {
    if (!status.skipUpdate) { // Skip update if flagged
        // Update environment
        Global::instance().notifyEvent(env.update());
        env.updateObjects(
            visibleTiles(),
            tilingInformation(),
            Data::RendererProcessor::instance()
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
    static constexpr std::hash<std::string> hashString;
    static std::mt19937 randNum(hashString("RNG for FPS control"));

    double const target = 1000.0 / static_cast<double>(fps.target);
    double const remainder = target - static_cast<double>(static_cast<uint32_t>(target)); // between 0.0 and 1.0
    std::uint32_t const adjustedTarget = static_cast<uint32_t>(target) + (distribution(randNum) < remainder ? 1 : 0);
    return fps.controlTimer.projected_dt() >= adjustedTarget;
}

void Renderer::append(RenderObject* toAppend) {
    // Append to environment, based on layer
    env.append(
        toAppend,
        tilingInformation(),
        toAppend->getLayer()
    );
}

void Renderer::reinsertAllObjects() {
    env.reinsertAllObjects(tilingInformation());
}

//------------------------------------------
// Texture Management

bool Renderer::attachTextureAboveLayer(Environment::Layer const aboveThisLayer, std::string const& name, SDL_Texture* texture, std::optional<SDL_FRect> rect) {
    if (texture == nullptr) {
        return false; // Cannot attach a null texture
    }
    if (BetweenLayerTextures[aboveThisLayer].contains(name)) {
        return false; // Texture with this name already exists in the specified layer
    }
    BetweenLayerTextures[aboveThisLayer][name] = std::make_pair(texture, rect);
    return true;
}

bool Renderer::detachTextureAboveLayer(Environment::Layer const aboveThisLayer, std::string const& name) {
    if (BetweenLayerTextures[aboveThisLayer].contains(name)) {
        BetweenLayerTextures[aboveThisLayer].erase(name);
        return true;
    }
    return false;
}

void Renderer::detachAllTextures() {
    BetweenLayerTextures.clear();
}

//------------------------------------------
// Purge

void Renderer::purgeObjects() {
    env.purgeObjects();
}

void Renderer::purgeTextures() {
    // Release resources for TextureContainer
    for (auto const& texture : std::views::values(TextureContainer)) {
        SDL_DestroyTexture(texture);
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
// Setting

void Renderer::setTargetFPS(std::uint16_t const& targetFps) {
    fps.target = targetFps;
}

namespace {

template <typename T>
struct is_static_member_function : std::bool_constant<std::is_function_v<std::remove_pointer_t<T>>> {};

template <typename T>
inline constexpr bool is_static_member_function_v = is_static_member_function<T>::value;

} // namespace

// This does not change the settings file, only the current session
void Renderer::changeWindowSize(int const w, int const h, std::uint8_t  const scalar) {
    // Validate resolution and scalar
    if (w < 240 || w > 16384) {
        capture.error.println("Selected resolution is not supported:", w, "x", h, ". (w and h must be between 240 and 16384)");
        return;
    }
    if (h < 240 || h > 16384) {
        capture.error.println("Selected resolution is not supported:", w, "x", h, ". (w and h must be between 240 and 16384)");
        return;
    }
    if ( scalar < 1 || scalar > 8) {
        capture.error.println("Selected window scaling is not supported:", static_cast<int>(scalar), "x", " (must be between 1 and 8)");
        return;
    }

    // Find current center
    auto const currentCenterX = domainScope.get<int>(Constants::KeyNames::Renderer::positionX).value_or(0) + domainScope.get<int>(Constants::KeyNames::Renderer::dispResXLogical).value_or(0) / 2;
    auto const currentCenterY = domainScope.get<int>(Constants::KeyNames::Renderer::positionY).value_or(0) + domainScope.get<int>(Constants::KeyNames::Renderer::dispResYLogical).value_or(0) / 2;

    // Set new scalar
    windowScale = scalar;

    // Set the new resolution in the workspace
    domainScope.set<int>(Constants::KeyNames::Renderer::dispResXWindow, w * windowScale);
    domainScope.set<int>(Constants::KeyNames::Renderer::dispResYWindow, h * windowScale);
    domainScope.set<int>(Constants::KeyNames::Renderer::dispResXLogical, w);
    domainScope.set<int>(Constants::KeyNames::Renderer::dispResYLogical, h);
    domainScope.set<uint8_t>(Constants::KeyNames::Renderer::windowScale, windowScale);

    // Set new camera position (top left corner)
    auto const newCamPosX = currentCenterX - domainScope.get<int>(Constants::KeyNames::Renderer::dispResXLogical).value_or(0) / 2;
    auto const newCamPosY = currentCenterY - domainScope.get<int>(Constants::KeyNames::Renderer::dispResYLogical).value_or(0) / 2;
    domainScope.set<int>(Constants::KeyNames::Renderer::positionX, newCamPosX);
    domainScope.set<int>(Constants::KeyNames::Renderer::positionY, newCamPosY);

    // Set the physical window size
    SDL_SetWindowSize(window, w * windowScale, h * windowScale);

    // Rescale rml context
    Graphics::RmlInterface::instance().setDimensions(w * windowScale, h * windowScale);

    // We assume that the tiling information is based on renderer states such as resolution,
    // if it's not static. If that is the case, we must reinsert all objects to redistribute
    // on resolution change
    if constexpr (!is_static_member_function_v<decltype(&Renderer::tilingInformation)>) {
        // Unreachable code if it's static, so we use a NOLINTNEXTLINE to suppress the warning
        // NOLINTNEXTLINE
        reinsertAllObjects();
    }
}

void Renderer::setCam(int const X, int const Y, bool const isMiddle) const {
    int newPosX = X;
    int newPosY = Y;
    if (isMiddle) {
        newPosX -= domainScope.get<int>(Constants::KeyNames::Renderer::dispResXLogical).value_or(0) / 2;
        newPosY -= domainScope.get<int>(Constants::KeyNames::Renderer::dispResYLogical).value_or(0) / 2;
    }
    domainScope.set<int>(Constants::KeyNames::Renderer::positionX, newPosX);
    domainScope.set<int>(Constants::KeyNames::Renderer::positionY, newPosY);
}

void Renderer::moveCam(int const dX, int const dY) const {
    domainScope.set<int>(
        Constants::KeyNames::Renderer::positionX,
        domainScope.get<int>(Constants::KeyNames::Renderer::positionX).value_or(0) + dX
    );
    domainScope.set<int>(
        Constants::KeyNames::Renderer::positionY,
        domainScope.get<int>(Constants::KeyNames::Renderer::positionY).value_or(0) + dY
    );
}

SDL_FRect Renderer::scaleRectFromLogicalSize(SDL_FRect const& logicalRect) const {
    auto const scale = static_cast<float>(windowScale);
    return SDL_FRect{
        .x=logicalRect.x * scale,
        .y=logicalRect.y * scale,
        .w=logicalRect.w * scale,
        .h=logicalRect.h * scale
    };
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
    auto const w = domainScope.get<int16_t>(Constants::KeyNames::Renderer::dispResXLogical).value_or(0);
    auto const h = domainScope.get<int16_t>(Constants::KeyNames::Renderer::dispResYLogical).value_or(0);
    if (w == 0 || h == 0) {
        // Avoid division by zero
        capture.error.println("Display resolution is zero, cannot render frame.");
        std::abort();
    }

    // Get tile position of camera center
    cameraTilePosition = Data::RenderObjectContainer::getTilePos(
        {
            .x=dispPosX + w/2,
            .y=dispPosY + h/2
        },
        tilingInformation()
    );

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

    // Black background
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);

    //Render Objects
    //For all layers, starting at 0
    for (auto const& layer : Environment::getAllLayerTypes()) {
        // Render all objects in the viewport of this layer
        if (layer == Environment::Layer::background) {
            onViewportTiles(layer, [&](Environment::TileAndCoordinate const& tileAndCoordinate) {
                tileAndCoordinate.tile->render(
                    *this,
                    tileAndCoordinate.coordinate,
                    tilingInformation(),
                    capture,
                    dispPosX,
                    dispPosY,
                    windowScale
                );
            });
        }
        else {
            onViewport(layer, [&](RenderObject* obj) {
                obj->draw(
                    *this,
                    dispPosX,
                    dispPosY
                );
            });
        }

        // Render all textures that were attached from outside processes
        for (auto const& [texture, rect] : std::views::values(BetweenLayerTextures[layer])) {
            if (!texture) {
                continue; // Skip if texture is null
            }
            // We assume the rect was already scaled correctly when added
            if (!SDL_RenderTexture(renderer, texture, nullptr, rect.has_value() ? &rect.value() : nullptr)) {
                capture.error.println("Failed to render between-layer texture: ", SDL_GetError());
            }
        }

        // Process renderCallbacks for this layer
        for (auto const& callback : renderCallbacks[layer]) {
            callback();
        }
        renderCallbacks[layer].clear();
    }
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
    if (auto* const t = loadTextureToMemory(link); t != nullptr) TextureContainer[link] = t;
}

SDL_Texture* Renderer::loadTextureToMemory(std::string const& link) {
    std::string const path = Utility::IO::FileManagement::CombinePaths(baseDirectory, link);

    // Get file extension, based on last dot
    std::string extension;
    if (std::size_t const dotPos = path.find_last_of('.'); dotPos != std::string::npos) {
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

//------------------------------------------
// Callback

void Renderer::addRenderCallback(std::function<void()> const& function, Environment::Layer const aboveThisLayer) {
    renderCallbacks[aboveThisLayer].emplace_back(function);
}

void Renderer::addPostRenderCallback(std::function<void()> const& function) {
    postRenderCallback.emplace_back(function);
}

} // namespace Nebulite::Core
