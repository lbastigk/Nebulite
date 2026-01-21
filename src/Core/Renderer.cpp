//------------------------------------------
// Includes

// Standard library
#include <random>

// External
#include <SDL3_image/SDL_image.h>
#include <absl/container/flat_hash_map.h>
#include <imgui_impl_sdl3.h>
#include "imgui_impl_sdlrenderer3.h"

// Nebulite
#include "Nebulite.hpp"
#include "Constants/KeyNames.hpp"
#include "Core/Environment.hpp"
#include "Core/Renderer.hpp"
#include "Core/RenderObject.hpp"
#include "DomainModule/GlobalSpace/Settings.hpp"
#include "DomainModule/Initializer.hpp"
#include "Interaction/Invoke.hpp"
#include "Utility/TimeKeeper.hpp"

//------------------------------------------
namespace Nebulite::Core {

Renderer::Renderer(JsonScope& documentReference, bool* flag_headless)
    : Domain("Renderer", *this, documentReference),
      env(documentReference){

    //------------------------------------------
    // Initialize internal variables

    // Window
    WindowScale = 1;
    headless = flag_headless;

    // Position
    tilePositionX = 0;
    tilePositionY = 0;

    // Base directory
    baseDirectory = Utility::FileManagement::currentDir();

    // Audio
    initWaveforms();

    //------------------------------------------
    // Start timers
    fps.controlTimer.start();
    fps.renderTimer.start();

    //------------------------------------------
    // Domain Modules
    DomainModule::Initializer::initRenderer(this);
}

void Renderer::initWaveforms() {
    // Waveform buffers: Sine wave buffer
    basicAudioWaveforms.sineBuffer = new std::vector<Sint16>(basicAudioWaveforms.samples);
    for (size_t i = 0; i < basicAudioWaveforms.samples; i++) {
        double const time = static_cast<double>(i) / basicAudioWaveforms.sampleRate;
        (*basicAudioWaveforms.sineBuffer)[i] = static_cast<int16_t>(32767 * 0.3 * sin(2.0 * M_PI * basicAudioWaveforms.frequency * time));
    }

    // Waveform buffers: Square wave buffer
    basicAudioWaveforms.squareBuffer = new std::vector<Sint16>(basicAudioWaveforms.samples);
    for (size_t i = 0; i < basicAudioWaveforms.samples; i++) {
        double const time = static_cast<double>(i) / basicAudioWaveforms.sampleRate;

        // Square wave: alternates between +1 and -1
        double const phase = 2.0 * M_PI * basicAudioWaveforms.frequency * time;
        double const squareValue = sin(phase) >= 0 ? 1.0 : -1.0;

        (*basicAudioWaveforms.squareBuffer)[i] = static_cast<int16_t>(32767 * 0.3 * squareValue);
    }

    // Waveform buffers: Triangle wave buffer
    basicAudioWaveforms.triangleBuffer = new std::vector<Sint16>(basicAudioWaveforms.samples);
    for (size_t i = 0; i < basicAudioWaveforms.samples; i++) {
        double const time = static_cast<double>(i) / basicAudioWaveforms.sampleRate;

        // Triangle wave: linear ramp up and down
        double const phase = fmod(basicAudioWaveforms.frequency * time, 1.0); // 0 to 1
        double triangleValue;

        if (phase < 0.5) {
            triangleValue = 4.0 * phase - 1.0; // -1 to +1 (rising)
        } else {
            triangleValue = 3.0 - 4.0 * phase; // +1 to -1 (falling)
        }

        (*basicAudioWaveforms.triangleBuffer)[i] = static_cast<int16_t>(32767 * 0.3 * triangleValue);
    }
}

// TODO: Move all settings to workspace!
void Renderer::setupDisplayValues() {
    // Load from settings
    // Default values should never be used, as settings should always exist
    // Still, just in case, we set them to 1000x1000 @ 1x scaling and 60 FPS
    auto const X = Global::settings().get<uint16_t>(DomainModule::GlobalSpace::Settings::Key::resolutionX, 1000);
    auto const Y = Global::settings().get<uint16_t>(DomainModule::GlobalSpace::Settings::Key::resolutionX, 1000);
    WindowScale = Global::settings().get<uint8_t>(DomainModule::GlobalSpace::Settings::Key::resolutionScaling, 1);
    fps.target = Global::settings().get<uint16_t>(DomainModule::GlobalSpace::Settings::Key::targetFPS, 60);

    // Set in workspace
    domainScope.set<unsigned int>(Constants::KeyNames::Renderer::dispResX, X);
    domainScope.set<unsigned int>(Constants::KeyNames::Renderer::dispResY, Y);

    // Start position at 0|0
    // TODO: Move to environment?
    domainScope.set<unsigned int>(Constants::KeyNames::Renderer::positionX, 0);
    domainScope.set<unsigned int>(Constants::KeyNames::Renderer::positionY, 0);
}

Constants::Error Renderer::preParse() {
    // Initialize SDL and related subsystems
    initSDL();
    return Constants::ErrorTable::NONE();
}

void Renderer::initSDL() {
    if (SDL_initialized)
        return;

    //------------------------------------------
    // Window
    setupDisplayValues();

    // Create SDL window
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        // SDL initialization failed
        Error::println("SDL_Init Error: ", SDL_GetError());
        std::abort();
    }
    // Define window via x|y|w|h
    int const w = domainScope.get<int>(Constants::KeyNames::Renderer::dispResX, 0);
    int const h = domainScope.get<int>(Constants::KeyNames::Renderer::dispResY, 0);

    //------------------------------------------
    // Window and renderer

    uint32_t const flags = *headless ? SDL_WINDOW_HIDDEN : 0
        | SDL_WINDOW_HIGH_PIXEL_DENSITY;

    if (!SDL_CreateWindowAndRenderer("Nebulite", w, h, flags, &window, &renderer)) {
        Error::println("SDL_CreateWindowAndRenderer Error: ", SDL_GetError());
        std::abort();
    }

    // Set virtual rendering size
    SDL_SetRenderLogicalPresentation(
        renderer,
        w,
        h,
        SDL_LOGICAL_PRESENTATION_INTEGER_SCALE
    );
    SDL_SetWindowPosition(window, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);

    //------------------------------------------
    // IMGUI

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    ImGuiIO &io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad; // Enable Gamepad Controls

    // Setup scaling
    float const main_scale = SDL_GetDisplayContentScale(SDL_GetPrimaryDisplay());
    ImGuiStyle &style = ImGui::GetStyle();
    style.ScaleAllSizes(main_scale * WindowScale);
    style.FontScaleDpi = main_scale * WindowScale;
    style.FontSizeBase = 40.f;
    io.Fonts->AddFontDefault();

    ImGui_ImplSDL3_InitForSDLRenderer(window, renderer);
    ImGui_ImplSDLRenderer3_Init(renderer);

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
                Error::println("Failed to create cursor: ", SDL_GetError());
                // No quit, just use default cursor
            }
        }
    }

    //------------------------------------------
    // Fonts

    // Initialize SDL_ttf
    if (!TTF_Init()) {
        // Handle SDL_ttf initialization error
        Error::println("TTF_Init Error!");
        std::abort();
    }
    loadFonts();

    //------------------------------------------
    // Audio

    // Init
    if (!SDL_Init(SDL_INIT_AUDIO)) {
        Error::println("SDL_Init Error: ", SDL_GetError());
        std::abort();
    }
    audio.desired.freq = 44100;
    audio.desired.format = SDL_AUDIO_S16;
    audio.desired.channels = 1;
    //audio.desired.samples = 1024;
    //audio.desired.callback = nullptr;

    audio.device = SDL_OpenAudioDevice(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &audio.desired);
    if (audio.device == 0) {
        Error::println("Failed to open audio device: ", SDL_GetError());
        std::abort();
    }
    audioInitialized = true;

    //------------------------------------------
    // Check for remaining errors in SDL

    if (SDL_GetError()[0] != '\0') {
        Error::println("SDL Error during initialization: ", SDL_GetError());
        std::abort();
    }

    // All done
    SDL_initialized = true;
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
        Error::println("Failed to load font: ", fontPath);
        std::abort();
    }
}

//------------------------------------------
// Getting

int Renderer::getResX() const { return domainScope.get<int>(Constants::KeyNames::Renderer::dispResX, 0); }

int Renderer::getResY() const { return domainScope.get<int>(Constants::KeyNames::Renderer::dispResY, 0); }

int Renderer::getPosX() const { return domainScope.get<int>(Constants::KeyNames::Renderer::positionX, 0); }

int Renderer::getPosY() const { return domainScope.get<int>(Constants::KeyNames::Renderer::positionY, 0); }

//------------------------------------------
// Serialization / Deserialization

std::string Renderer::serialize() {
    return env.serialize();
}

void Renderer::deserialize(std::string const& serialOrLink) noexcept {
    env.deserialize(
        serialOrLink,
        domainScope.get<uint16_t>(Constants::KeyNames::Renderer::dispResX, 0),
        domainScope.get<uint16_t>(Constants::KeyNames::Renderer::dispResY, 0)
    );
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

    // Optional: constrain maximum size (min 0, max 150x50)
    ImGui::SetNextWindowSizeConstraints(ImVec2(0, 0), ImVec2(150.0f, 50.0f));

    ImGui::Begin(
        "FPS Overlay",
        nullptr,
        ImGuiWindowFlags_NoDecoration |
        ImGuiWindowFlags_AlwaysAutoResize |
        ImGuiWindowFlags_NoFocusOnAppearing |
        ImGuiWindowFlags_NoNav
    );

    ImGui::Text("FPS: %d", fps.real);
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
        if (event.type == SDL_EVENT_QUIT) { quit = true; }
    }
}

// For quick and dirty debugging, in case the rendering pipeline breaks somewhere
Constants::Error Renderer::update() {
    renderInit();
    renderFrame();
    pollEvents();
    skippedUpdateLastFrame = skipUpdate;
    skipUpdate = false;
    updateModules();

    // DEBUG: IMGUI test window
    ImGui::ShowDemoWindow();

    // Imgui Rendering
    if (showFPS) renderFPS();
    ImGui::Render();
    ImGui_ImplSDLRenderer3_RenderDrawData(ImGui::GetDrawData(), renderer);
    SDL_RenderPresent(renderer);

    // Process all events
    for (auto const& event : events) {
        // Modify the event if it's a mouse position event
        if (event.type == SDL_EVENT_MOUSE_MOTION) {
            // Scale the motion event according to window scale
            SDL_Event modifiedEvent = event;
            modifiedEvent.motion.x = static_cast<float>(event.motion.x) / static_cast<float>(WindowScale);
            modifiedEvent.motion.y = static_cast<float>(event.motion.y) / static_cast<float>(WindowScale);
            modifiedEvent.motion.xrel = static_cast<float>(event.motion.xrel) / static_cast<float>(WindowScale);
            modifiedEvent.motion.yrel = static_cast<float>(event.motion.yrel) / static_cast<float>(WindowScale);
            ImGui_ImplSDL3_ProcessEvent(&modifiedEvent);
        }
        else {
            ImGui_ImplSDL3_ProcessEvent(&event);
        }
    }

    //------------------------------------------
    // Check for SDL errors
    if (SDL_GetError()[0] != '\0') {
        Error::println("SDL Error during rendering: ", SDL_GetError());
        SDL_ClearError(); // Clear error after reporting
    }

    // Always return no critical error
    return Constants::ErrorTable::NONE();
}

void Renderer::updateState() {
    //------------------------------------------
    // Skip update if flagged
    if (!skipUpdate) {
        // Update environment
        auto const dispResX = domainScope.get<uint16_t>(Constants::KeyNames::Renderer::dispResX, 0);
        auto const dispResY = domainScope.get<uint16_t>(Constants::KeyNames::Renderer::dispResY, 0);
        env.updateObjects(tilePositionX, tilePositionY, dispResX, dispResY);
    }
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
    // Set ID
    toAppend->domainScope.set<uint32_t>(Constants::KeyNames::RenderObject::id, renderObjectIdCounter);
    renderObjectIdCounter++;

    //Append to environment, based on layer
    env.append(
        toAppend,
        domainScope.get<uint16_t>(Constants::KeyNames::Renderer::dispResX, 0),
        domainScope.get<uint16_t>(Constants::KeyNames::Renderer::dispResY, 0),
        toAppend->domainScope.get<uint8_t>(Constants::KeyNames::RenderObject::layer, 0)
        );

    //Load texture
    loadTexture(toAppend->domainScope.get<std::string>(Constants::KeyNames::RenderObject::imageLocation));
}

void Renderer::reinsertAllObjects() {
    env.reinsertAllObjects(
    domainScope.get<uint16_t>(Constants::KeyNames::Renderer::dispResX, 0),
    domainScope.get<uint16_t>(Constants::KeyNames::Renderer::dispResY, 0)
    );
}

//------------------------------------------
// Special Functions

void Renderer::beep() const {
    if (!audioInitialized)
        return;

    // SDL3: use an SDL_AudioStream to enqueue PCM data (lazy-initialized)
    static SDL_AudioStream* s_beepStream = nullptr;
    int const audioLength = static_cast<int>(basicAudioWaveforms.samples * sizeof(int16_t));

    if (!s_beepStream) {
        s_beepStream = SDL_CreateAudioStream(&audio.desired, &audio.desired);
        if (!s_beepStream) {
            Error::println("Failed to create audio stream: ", SDL_GetError());
            return;
        }
    }

    if (SDL_PutAudioStreamData(s_beepStream, basicAudioWaveforms.squareBuffer->data(), audioLength) != 0) {
        Error::println("Failed to push audio to stream: ", SDL_GetError());
        return;
    }

    // Ensure the device is running; SDL3's pause API uses a single-argument form
    SDL_PauseAudioDevice(audio.device);
}

bool Renderer::snapshot(std::string link) const {
    // Get current window/render target size
    int width, height;
    if (window) {
        // Normal windowed mode
        SDL_GetWindowSize(window, &width, &height);
    } else {
        // Headless mode - get renderer output size
        SDL_GetCurrentRenderOutputSize(renderer, &width, &height);
    }

    // Create surface to capture pixels
    SDL_Rect const fullScreenRect = {0, 0, width, height};
    auto const surface = SDL_RenderReadPixels(renderer, &fullScreenRect);
    if (!surface) {
        Error::println("Failed to read pixels for snapshot: ", SDL_GetError());
        SDL_DestroySurface(surface);
        return false;
    }

    // Create directory if it doesn't exist
    std::string directory = link.substr(0, link.find_last_of("/\\"));

    // Edge case: check if link contains no directory:
    if (link.find_last_of("/\\") == std::string::npos) {
        directory = "./Resources/Snapshots";
        link = directory + "/" + link;
    }

    if (!directory.empty()) {
        // Create directory using C++17 filesystem
        try {
            std::filesystem::create_directories(directory);
        } catch (std::exception const& e) {
            Error::println("Warning: Could not create directory ", directory, ": ", e.what());
            // Continue anyway - maybe directory already exists
        }
    }

    // Save surface as PNG
    if (int const result = IMG_SavePNG(surface, link.c_str()); result != 0 && SDL_GetError()[0] != '\0') {
        Error::println("Failed to save snapshot!");
        return false;
    }

    // Cleanup
    SDL_DestroySurface(surface);
    return true;
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
    if (!SDL_initialized)
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
        Error::println("Selected resolution is not supported:", w, "x", h);
        return;
    }
    if (h < 240 || h > 16384) {
        Error::println("Selected resolution is not supported:", w, "x", h);
        return;
    }
    if ( scalar < 1 || scalar > 8) {
        Error::println("Selected window scaling is not supported:", static_cast<int>(scalar), "x");
        return;
    }

    // Store previous scale
    uint8_t const prevScale = WindowScale == 0 ? 1 : WindowScale;
    WindowScale = scalar;

    // If scalar changed, apply relative style scaling once
    if (scalar != prevScale) {
        float const relativeStyleScale = static_cast<float>(WindowScale) / static_cast<float>(prevScale);
        ImGui::GetStyle().ScaleAllSizes(1.0f / relativeStyleScale);
        ImGui::GetStyle().FontScaleDpi /= relativeStyleScale;
    }

    // Set the new resolution in the workspace
    domainScope.set<int>(Constants::KeyNames::Renderer::dispResX, w);
    domainScope.set<int>(Constants::KeyNames::Renderer::dispResY, h);

    // Set the physical window size
    SDL_SetWindowSize(window, w * WindowScale, h * WindowScale);

    // Use integer logical presentation so scaling is done in integer steps (crisp pixels).
    SDL_SetRenderLogicalPresentation(renderer, w, h, SDL_LOGICAL_PRESENTATION_INTEGER_SCALE);

    // Reinsert objects due to new tile / logical size
    reinsertAllObjects();
}

void Renderer::moveCam(int const& dX, int const& dY) const {
    domainScope.set<int>(
        Constants::KeyNames::Renderer::positionX,
        domainScope.get<int>(Constants::KeyNames::Renderer::positionX, 0) + dX
        );
    domainScope.set<int>(
        Constants::KeyNames::Renderer::positionY,
        domainScope.get<int>(Constants::KeyNames::Renderer::positionY, 0) + dY
        );
}

void Renderer::setCam(int const& X, int const& Y, bool const& isMiddle) const {
    int newPosX = X;
    int newPosY = Y;
    if (isMiddle) {
        newPosX -= domainScope.get<int>(Constants::KeyNames::Renderer::dispResX, 0) / 2;
        newPosY -= domainScope.get<int>(Constants::KeyNames::Renderer::dispResY, 0) / 2;
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
    auto const dispPosX = domainScope.get<int16_t>(Constants::KeyNames::Renderer::positionX, 0);
    auto const dispPosY = domainScope.get<int16_t>(Constants::KeyNames::Renderer::positionY, 0);

    // Depending on position, set tiles to render
    tilePositionX = static_cast<int16_t>(dispPosX / domainScope.get<int16_t>(Constants::KeyNames::Renderer::dispResX, 0));
    tilePositionY = static_cast<int16_t>(dispPosY / domainScope.get<int16_t>(Constants::KeyNames::Renderer::dispResY, 0));

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
    for (auto layer : *env.getAllLayers()) {
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
            SDL_FRect const rectF = {
                static_cast<float>(rect->x),
                static_cast<float>(rect->y),
                static_cast<float>(rect->w),
                static_cast<float>(rect->h)
            };
            if (!SDL_RenderTexture(renderer, texture, nullptr, &rectF)) {
                Error::println("Failed to render between-layer texture: ", SDL_GetError());
            }
        }
    }
}

void Renderer::renderObjectToScreen(RenderObject* obj, int const& dispPosX, int const& dispPosY) {
    //------------------------------------------
    // Texture Loading

    // Check for texture
    /**
    * @todo Find some way to remove the get-call.
    *       Since the actual image does not change often and does not modify any state, we could use a runner function
    *       that asynchronously reloads the texture path if needed:
    *       std::atomic<std::string>& RenderObject::getImageLocation() {
    *           // Return a reference to an atomic string that stores the path
    *       }
    *       std::atomic<std::string>& RenderObject::checkImageLocation(){
    *           // Ran asynchronously every X seconds by a runner, called from the Renderer owning the Runner
    *           // 1.) Get actual location form document scope
    *           // 2.) if different from stored path, set stored path to new path
    *       }
    *       We could also update the texture container in the runner, but then we would have to lock the container during rendering.
    *       Since we have to map string to texture anyway, we can just do it here.
     */
    auto const innerDirectory = obj->domainScope.get<std::string>(Constants::KeyNames::RenderObject::imageLocation);

    // Load texture if not yet loaded
    if (TextureContainer.find(innerDirectory) == TextureContainer.end()) {
        loadTexture(innerDirectory);
    }

    // Link texture if not yet linked
    if (!obj->isTextureValid()) {
        if (auto const t = TextureContainer[innerDirectory]; isTextureValid(t)) {
            obj->linkExternalTexture(t);
        }
    }

    //------------------------------------------
    // Source and Destination Rectangles

    // Calculate source rect
    obj->calculateSrcRect();

    // Calculate position rect
    obj->calculateDstRect();
    obj->getDstRect()->x -= dispPosX; // Subtract X camera position
    obj->getDstRect()->y -= dispPosY; // Subtract Y camera position

    //------------------------------------------
    // Error Checking
    if (!obj->isTextureValid()) {
        Error::println("Error: RenderObject ID ",
            obj->domainScope.get<uint32_t>(Constants::KeyNames::RenderObject::id, 0),
            " texture with path '",
            innerDirectory,
            "' not found");
    }

    //------------------------------------------
    // Rendering

    // Render the texture
    SDL_Rect const* src = obj->getSrcRect();
    SDL_FRect srcF = {};
    SDL_FRect const* srcFP = nullptr;
    if (src) {
        srcF = {static_cast<float>(src->x), static_cast<float>(src->y), static_cast<float>(src->w), static_cast<float>(src->h)};
        srcFP = &srcF;
    }
    SDL_Rect const* dst = obj->getDstRect();
    SDL_FRect dstF = {};
    SDL_FRect const* dstFP = nullptr;
    if (dst) {
        dstF = {static_cast<float>(dst->x), static_cast<float>(dst->y), static_cast<float>(dst->w), static_cast<float>(dst->h)};
        dstFP = &dstF;
    }

    // Render to screen
    if (auto const t = obj->getSDLTexture(); t != nullptr) {
        if (SDL_RenderTexture(renderer, t, srcFP, dstFP) != 0 && SDL_GetError()[0] != '\0') {
            auto const id = obj->domainScope.get<uint32_t>(Constants::KeyNames::RenderObject::id, 0);
            Error::println("Error rendering RenderObject ID ", id, ": ", SDL_GetError());
        }
    }

    // Render the text
    if (obj->isTextRenderingEnabled()) {
        obj->calculateText(
            renderer,
            font,
            dispPosX,
            dispPosY
            );
        if (obj->getTextTexture() && obj->getTextRect()) {
            auto const textRectF = SDL_FRect{
                static_cast<float>(obj->getTextRect()->x),
                static_cast<float>(obj->getTextRect()->y),
                static_cast<float>(obj->getTextRect()->w),
                static_cast<float>(obj->getTextRect()->h)
            };

            if (SDL_RenderTexture(renderer, obj->getTextTexture(), nullptr, &textRectF) != 0 && SDL_GetError()[0] != '\0') {
                auto const id = obj->domainScope.get<uint32_t>(Constants::KeyNames::RenderObject::id, 0);
                Error::println("Error rendering text for RenderObject ID ", id, ": ", SDL_GetError());
            }
        }
    }
}

/*
void Renderer::renderFPS() const {
    // Size of the font
    double constexpr fontSize = 16;

    // Create a string with the FPS value
    std::string const fpsText = "FPS: " + std::to_string(fps.real);

    // Define the destination rectangle for rendering the text
    SDL_FRect const textRect = {
        static_cast<float>(10.0) / static_cast<float>(WindowScale),
        static_cast<float>(10.0) / static_cast<float>(WindowScale),
        static_cast<float>(fontSize * static_cast<double>(fpsText.length()) / static_cast<double>(WindowScale)) ,
        static_cast<float>(fontSize * 1.5 / static_cast<double>(WindowScale))
    }; // Adjust position as needed

    // Clear the area where the FPS text will be rendered
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255); // Set background color (black)
    SDL_RenderFillRect(renderer, &textRect);

    // Create a surface with the text
    SDL_Surface* textSurface = TTF_RenderText_Solid(font, fpsText.c_str(), 0, textColor);

    // Create a texture from the text surface
    SDL_Texture* textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);

    // Render the text texture
    SDL_RenderTexture(renderer, textTexture, nullptr, &textRect);

    // Free the text surface and texture
    SDL_DestroySurface(textSurface);
    SDL_DestroyTexture(textTexture);
}
*/

//------------------------------------------
// Texture-Related

void Renderer::loadTexture(std::string const& link) {
    if (auto const t = loadTextureToMemory(link); t != nullptr) TextureContainer[link] = t;
}


SDL_Texture* Renderer::loadTextureToMemory(std::string const& link) const {
    std::string const path = Utility::FileManagement::CombinePaths(baseDirectory, link);

    // Get file extension, based on last dot
    std::string extension;
    if (size_t const dotPos = path.find_last_of('.'); dotPos != std::string::npos) {
        extension = path.substr(dotPos + 1);
    } else {
        Error::println("Failed to load image '", path, "': No file extension found.");
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
        Error::println("Failed to load image '", path, "': ", SDL_GetError());
        return nullptr;
    }

    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_DestroySurface(surface); // Free the surface after creating texture

    // Check for texture issues
    if (!texture) {
        Error::println("Failed to create texture from image '", path, "': ", SDL_GetError());
        return nullptr;
    }

    SDL_SetTextureScaleMode(texture, SDL_SCALEMODE_NEAREST);
    return texture;
}

} // namespace Nebulite::Core
