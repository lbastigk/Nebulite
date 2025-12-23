//------------------------------------------
// Includes

// Standard library
#include <random>
#include <sys/types.h>

// External
#include <SDL.h>
#include <SDL_ttf.h>
#include <SDL_image.h>
#include <absl/container/flat_hash_map.h>

// Nebulite
#include "Nebulite.hpp"
#include "Core/Environment.hpp"
#include "Core/Renderer.hpp"
#include "DomainModule/Initializer.hpp"
#include "Interaction/Invoke.hpp"
#include "Utility/Capture.hpp"
#include "Utility/TimeKeeper.hpp"

//------------------------------------------
namespace Nebulite::Core {

Renderer::Renderer(Data::JSON* docRef, bool* flag_headless, unsigned int const& X, unsigned int const& Y)
    : Domain("Renderer", this, docRef),
      env(docRef),
      rngA(hashString("Seed for RNG A")),
      rngB(hashString("Seed for RNG B")) {

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

    //------------------------------------------
    // Set basic values inside global doc
    getDoc()->set<unsigned int>(Constants::KeyNames::Renderer::dispResX, X);
    getDoc()->set<unsigned int>(Constants::KeyNames::Renderer::dispResY, Y);

    getDoc()->set<unsigned int>(Constants::KeyNames::Renderer::positionX, 0);
    getDoc()->set<unsigned int>(Constants::KeyNames::Renderer::positionY, 0);

    //------------------------------------------
    // Start timers
    fps.controlTimer.start();
    fps.renderTimer.start();

    //------------------------------------------
    // Domain Modules
    DomainModule::Initializer::initRenderer(this);
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

    //Create SDL window
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        // SDL initialization failed
        Nebulite::cerr() << "SDL_Init Error: " << SDL_GetError() << Nebulite::endl;
    }
    // Define window via x|y|w|h
    int constexpr x = SDL_WINDOWPOS_CENTERED;
    int constexpr y = SDL_WINDOWPOS_CENTERED;
    int const w = getDoc()->get<int>(Constants::KeyNames::Renderer::dispResX, 0);
    int const h = getDoc()->get<int>(Constants::KeyNames::Renderer::dispResY, 0);

    uint32_t flags = *headless ? SDL_WINDOW_HIDDEN : SDL_WINDOW_SHOWN;
    //flags = flags | SDL_WINDOW_RESIZABLE; // Disabled for now, as it causes issues with the logical size rendering
    flags = flags | SDL_WINDOW_OPENGL;
    window = SDL_CreateWindow("Nebulite", x, y, w, h, flags);
    if (!window) {
        // Window creation failed
        Nebulite::cerr() << "SDL_CreateWindow Error: " << SDL_GetError() << Nebulite::endl;
        SDL_Quit();
    }

    //------------------------------------------
    // Renderer

    // Create a renderer
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) {
        Nebulite::cerr() << "Renderer creation failed: " << SDL_GetError() << Nebulite::endl;
    }

    // Set virtual rendering size
    SDL_RenderSetLogicalSize(
        renderer,
        w,
        h
        );

    //------------------------------------------
    // Fonts

    // Initialize SDL_ttf
    if (TTF_Init() < 0) {
        // Handle SDL_ttf initialization error
        SDL_Quit(); // Clean up SDL
    }
    loadFonts();

    //------------------------------------------
    // Audio

    // Init
    if (SDL_Init(SDL_INIT_AUDIO) < 0) {
        Nebulite::cerr() << "SDL_Init Error: " << SDL_GetError() << Nebulite::endl;
    } else {
        audio.desired.freq = 44100;
        audio.desired.format = AUDIO_S16SYS;
        audio.desired.channels = 1;
        audio.desired.samples = 1024;
        audio.desired.callback = nullptr;

        audio.device = SDL_OpenAudioDevice(nullptr, 0, &audio.desired, &audio.obtained, 0);
        if (audio.device == 0) {
            Nebulite::cerr() << "Failed to open audio device: " << SDL_GetError() << Nebulite::endl;
        } else {
            audioInitialized = true;
        }
    }

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
        Nebulite::cerr() << TTF_GetError() << " | " << fontPath << "\n";
    }
}

//------------------------------------------
// Pipeline

// For quick and dirty debugging, in case the rendering pipeline breaks somewhere
//# define debug_on_each_step 1
Constants::Error Renderer::update() {
    //------------------------------------------
    // Do all the steps of the rendering pipeline
    clear(); // 1.) Clear screen FIRST, so that functions like snapshot have access to the latest frame
    updateState(); // 2.) Update objects, states, etc.
    renderFrame(); // 3.) Render frame
    if (showFPS)
        renderFPS(); // 4.) Render fps count
    showFrame(); // 5.) Show Frame

    //------------------------------------------
    // SDL Polling at the end of the frame
    SDL_Event event{};
    events.clear();
    while (SDL_PollEvent(&event)) {
        // Store events for other processes, e.g. domain modules
        events.push_back(event);

        // Handle quit event
        if (event.type == SDL_QUIT) { quit = true; }
    }

    //------------------------------------------
    // Manage frame skipping
    skippedUpdateLastFrame = skipUpdate;
    skipUpdate = false;

    //------------------------------------------
    // Update modules
    updateModules();

    // Always return no critical error
    return Constants::ErrorTable::NONE();
}

bool Renderer::timeToRender() {
    // Goal: projected_dt() == target
    // Issue: target might be fractional, projected_dt() is integer milliseconds
    // Solution: use probabilistic rounding
    // Example: target = 16.67 ms
    // set target to 16, remainder = 0.67
    // so do 16 for 67% of the time, and 17 for 33% of the time
    static std::uniform_real_distribution<double> distribution(0.0, 1.0);
    static std::mt19937 randNum(hashString("RNG for FPS control"));
    double const target = 1000.0 / static_cast<double>(fps.target);
    double const remainder = target - static_cast<double>(static_cast<uint32_t>(target)); // between 0.0 and 1.0
    uint32_t const adjustedTarget = static_cast<uint32_t>(target) + (distribution(randNum) < remainder ? 1 : 0);
    return fps.controlTimer.projected_dt() >= adjustedTarget;
}

void Renderer::append(RenderObject* toAppend) {
    // Set ID
    toAppend->getDoc()->set<uint32_t>(Constants::KeyNames::RenderObject::id, renderObjectIdCounter);
    renderObjectIdCounter++;

    //Append to environment, based on layer
    env.append(
        toAppend,
        getDoc()->get<uint16_t>(Constants::KeyNames::Renderer::dispResX, 0),
        getDoc()->get<uint16_t>(Constants::KeyNames::Renderer::dispResY, 0),
        toAppend->getDoc()->get<uint8_t>(Constants::KeyNames::RenderObject::layer, 0)
        );

    //Load texture
    loadTexture(toAppend->getDoc()->get<std::string>(Constants::KeyNames::RenderObject::imageLocation));
}

void Renderer::reinsertAllObjects() {
    env.reinsertAllObjects(
        getDoc()->get<uint16_t>(Constants::KeyNames::Renderer::dispResX, 0),
        getDoc()->get<uint16_t>(Constants::KeyNames::Renderer::dispResY, 0)
        );
}

//------------------------------------------
// Special Functions

void Renderer::beep() const {
    // Beep sound effect
    if (audioInitialized) {
        auto const audioLength = static_cast<uint32_t>(basicAudioWaveforms.samples * sizeof(int16_t));
        SDL_QueueAudio(audio.device, basicAudioWaveforms.squareBuffer->data(), audioLength);
        SDL_PauseAudioDevice(audio.device, 0); // Start playing
    }
}

bool Renderer::snapshot(std::string link) const {
    if (!renderer) {
        Nebulite::cerr() << "Cannot take snapshot: renderer not initialized" << Nebulite::endl;
        return false;
    }

    // Get current window/render target size
    int width, height;
    if (window) {
        // Normal windowed mode
        SDL_GetWindowSize(window, &width, &height);
    } else {
        // Headless mode - get renderer output size
        SDL_GetRendererOutputSize(renderer, &width, &height);
    }

    // Create surface to capture pixels
    SDL_Surface* surface = SDL_CreateRGBSurface(0, width, height, 32,
                                                0x00ff0000, // Red mask
                                                0x0000ff00, // Green mask
                                                0x000000ff, // Blue mask
                                                0xff000000); // Alpha mask

    if (!surface) {
        Nebulite::cerr() << "Failed to create surface for snapshot: " << SDL_GetError() << Nebulite::endl;
        return false;
    }

    // Read pixels from renderer
    if (SDL_RenderReadPixels(renderer, nullptr, SDL_PIXELFORMAT_ARGB8888,
                             surface->pixels, surface->pitch) != 0) {
        Nebulite::cerr() << "Failed to read pixels for snapshot: " << SDL_GetError() << Nebulite::endl;
        SDL_FreeSurface(surface);
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
            Nebulite::cerr() << "Warning: Could not create directory " << directory << ": " << e.what() << Nebulite::endl;
            // Continue anyway - maybe directory already exists
        }
    }

    // Save surface as PNG
    int const result = IMG_SavePNG(surface, link.c_str());

    // Cleanup
    SDL_FreeSurface(surface);

    if (result != 0) {
        Nebulite::cerr() << "Failed to save snapshot: " << IMG_GetError() << Nebulite::endl;
        return false;
    }
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
        SDL_DestroyTexture(texture);
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

void Renderer::changeWindowSize(int const& w, int const& h, uint16_t const& scalar) {
    WindowScale = scalar;
    if (w < 240 || w > 16384) {
        Nebulite::cerr() << "Selected resolution is not supported:" << w << "x" << h << "x" << Nebulite::endl;
        return;
    }
    if (h < 240 || h > 16384) {
        Nebulite::cerr() << "Selected resolution is not supported:" << w << "x" << h << "x" << Nebulite::endl;
        return;
    }

    getDoc()->set<int>(Constants::KeyNames::Renderer::dispResX, w);
    getDoc()->set<int>(Constants::KeyNames::Renderer::dispResY, h);

    // Update the window size
    SDL_SetWindowSize(
        window,
        getDoc()->get<int>(Constants::KeyNames::Renderer::dispResX, 360) * WindowScale,
        getDoc()->get<int>(Constants::KeyNames::Renderer::dispResY, 360) * WindowScale
        );
    SDL_RenderSetLogicalSize(
        renderer,
        getDoc()->get<int>(Constants::KeyNames::Renderer::dispResX, 360),
        getDoc()->get<int>(Constants::KeyNames::Renderer::dispResY, 360)
        );

    // Turn off console mode
    // Not needed anymore, console should dynamically adapt to new window size
    // consoleMode = false;

    // Reinsert objects, due to new tile size
    reinsertAllObjects();
}

void Renderer::moveCam(int const& dX, int const& dY) const {
    getDoc()->set<int>(
        Constants::KeyNames::Renderer::positionX,
        getDoc()->get<int>(Constants::KeyNames::Renderer::positionX, 0) + dX
        );
    getDoc()->set<int>(
        Constants::KeyNames::Renderer::positionY,
        getDoc()->get<int>(Constants::KeyNames::Renderer::positionY, 0) + dY
        );
}

void Renderer::setCam(int const& X, int const& Y, bool const& isMiddle) const {
    if (isMiddle) {
        int const newPosX = X - getDoc()->get<int>(Constants::KeyNames::Renderer::dispResX, 0) / 2;
        int const newPosY = Y - getDoc()->get<int>(Constants::KeyNames::Renderer::dispResY, 0) / 2;
        getDoc()->set<int>(Constants::KeyNames::Renderer::positionX, newPosX);
        getDoc()->set<int>(Constants::KeyNames::Renderer::positionY, newPosY);
    } else {
        getDoc()->set<int>(Constants::KeyNames::Renderer::positionX, X);
        getDoc()->set<int>(Constants::KeyNames::Renderer::positionY, Y);
    }
}

//------------------------------------------
// Setting

void Renderer::setTargetFPS(uint16_t const& targetFps) {
    fps.target = targetFps;
}

//------------------------------------------
// Renderer::tick Functions

void Renderer::clear() const {
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255); // RGB values (black)
    SDL_RenderClear(renderer);
}

void Renderer::updateState() {
    //------------------------------------------
    // Skip update if flagged
    if (skipUpdate) {
        return;
    }

    // Update invoke pairs, getting broadcast-listen-pairs from last env update
    Nebulite::global().getInvoke().update();

    // Update environment
    auto const dispResX = getDoc()->get<uint16_t>(Constants::KeyNames::Renderer::dispResX, 0);
    auto const dispResY = getDoc()->get<uint16_t>(Constants::KeyNames::Renderer::dispResY, 0);
    env.updateObjects(tilePositionX, tilePositionY, dispResX, dispResY);
}

void Renderer::renderFrame() {
    //------------------------------------------
    // Store for faster access

    // Get camera position
    auto const dispPosX = getDoc()->get<int16_t>(Constants::KeyNames::Renderer::positionX, 0);
    auto const dispPosY = getDoc()->get<int16_t>(Constants::KeyNames::Renderer::positionY, 0);

    // Depending on position, set tiles to render
    tilePositionX = static_cast<int16_t>(dispPosX / getDoc()->get<int16_t>(Constants::KeyNames::Renderer::dispResX, 0));
    tilePositionY = static_cast<int16_t>(dispPosY / getDoc()->get<int16_t>(Constants::KeyNames::Renderer::dispResY, 0));

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
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255); // Black background
    int error = 0;

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
                    error = renderObjectToScreen(obj, dispPosX, dispPosY);
                    if (error != 0) {
                        Nebulite::cerr() << "Error rendering object ID " << obj->getDoc()->get<uint32_t>(Constants::KeyNames::RenderObject::id, 0) << ": " << error << Nebulite::endl;
                    }
                }
            }
        }

        // Render all textures that were attached from outside processes
        for (auto const& [texture, rect] : std::views::values(BetweenLayerTextures[layer])) {
            if (!texture) {
                continue; // Skip if texture is null
            }
            SDL_RenderCopy(renderer, texture, nullptr, rect);
        }
    }
}

int Renderer::renderObjectToScreen(RenderObject* obj, int const& dispPosX, int const& dispPosY) {
    //------------------------------------------
    // Texture Loading

    // Check for texture
    auto const innerDirectory = obj->getDoc()->get<std::string>(Constants::KeyNames::RenderObject::imageLocation);

    // Load texture if not yet loaded
    if (TextureContainer.find(innerDirectory) == TextureContainer.end()) {
        loadTexture(innerDirectory);
    }

    // Link texture if not yet linked
    if (obj->isTextureValid() == false) {
        obj->linkExternalTexture(TextureContainer[innerDirectory]);
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
    if (!obj->getSDLTexture()) {
        Nebulite::cerr()
            << "Error: RenderObject ID "
            << obj->getDoc()->get<uint32_t>(Constants::KeyNames::RenderObject::id, 0)
            << " texture with path '"
            << innerDirectory
            << "' not found"
            << Nebulite::endl;
        return -1;
    }

    //------------------------------------------
    // Rendering

    // Render the texture
    int const error_sprite = SDL_RenderCopy(renderer, obj->getSDLTexture(), obj->getSrcRect(), obj->getDstRect());

    // Render the text
    int error_text = 0;
    if (obj->getDoc()->get<double>(Constants::KeyNames::RenderObject::textFontsize) > 0) {
        obj->calculateText(
            renderer,
            font,
            dispPosX,
            dispPosY
            );
        if (obj->getTextTexture() && obj->getTextRect()) {
            error_text = SDL_RenderCopy(renderer, obj->getTextTexture(), nullptr, obj->getTextRect());
        }
    }

    //------------------------------------------
    // Return
    if (error_sprite != 0) {
        return error_sprite;
    }
    return error_text;
}

void Renderer::renderFPS() const {
    // Size of the font
    double constexpr fontSize = 16;

    // Create a string with the FPS value
    std::string const fpsText = "FPS: " + std::to_string(fps.real);

    // Define the destination rectangle for rendering the text
    SDL_Rect const textRect = {
        10,
        10,
        static_cast<int>(fontSize * static_cast<int>(fpsText.length())),
        static_cast<int>(fontSize * 1.5)
    }; // Adjust position as needed

    // Clear the area where the FPS text will be rendered
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255); // Set background color (black)
    SDL_RenderFillRect(renderer, &textRect);

    // Create a surface with the text
    SDL_Surface* textSurface = TTF_RenderText_Solid(font, fpsText.c_str(), textColor);

    // Create a texture from the text surface
    SDL_Texture* textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);

    // Render the text texture
    SDL_RenderCopy(renderer, textTexture, nullptr, &textRect);

    // Free the text surface and texture
    SDL_FreeSurface(textSurface);
    SDL_DestroyTexture(textTexture);
}

void Renderer::showFrame() const {
    SDL_RenderPresent(renderer);
}

//------------------------------------------
// Texture-Related

void Renderer::loadTexture(std::string const& link) {
    if (SDL_Texture* texture = loadTextureToMemory(link)) {
        TextureContainer[link] = texture;
    }
}

/**
 * @todo Texture not created with SDL_TEXTUREACCESS_TARGET, so cannot be used with SDL_SetRenderTarget
 */
SDL_Texture* Renderer::loadTextureToMemory(std::string const& link) const {
    std::string const path = Utility::FileManagement::CombinePaths(baseDirectory, link);

    // Get file extension, based on last dot
    std::string extension;
    if (size_t const dotPos = path.find_last_of('.'); dotPos != std::string::npos) {
        extension = path.substr(dotPos + 1);
    } else {
        Nebulite::cerr() << "Failed to load image '" << path << "': No file extension found." << Nebulite::endl;
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
        Nebulite::cerr() << "Failed to load image '" << path << "': " << SDL_GetError() << Nebulite::endl;
        return nullptr;
    }

    // Create texture from surface
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface); // Free the surface after creating texture

    // Check for texture issues
    if (!texture) {
        Nebulite::cerr() << "Failed to create texture from surface: " << SDL_GetError() << Nebulite::endl;
        return nullptr;
    }

    // Store texture in container
    return texture;
}

} // namespace Nebulite::Core
