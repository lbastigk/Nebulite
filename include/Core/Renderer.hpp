/**
 * @file Renderer.hpp
 *
 * This file contains the declaration of the Nebulite::Core::Renderer class, which is responsible for rendering
 * the game objects and managing the rendering pipeline.
 */

#ifndef NEBULITE_CORE_RENDERER_HPP
#define NEBULITE_CORE_RENDERER_HPP

//------------------------------------------
// Includes

// Standard library
#include <random>
#include <cstdint>

// External
#include <SDL.h>
#include <SDL_ttf.h>
#include <absl/container/flat_hash_map.h>

// Nebulite
#include "Core/Environment.hpp"
#include "Utility/TimeKeeper.hpp"
#include "Interaction/Invoke.hpp"

//------------------------------------------
namespace Nebulite::Core {

/**
 * @class Nebulite::Core::Renderer
 * @brief Responsible for rendering game objects and managing the rendering pipeline.
 */
NEBULITE_DOMAIN(Renderer) {
public:
    /**
     * @brief Initializes a Renderer with given dimensions and settings.
     * @param docRef Pointer to the JSON document
     * @param flag_headless Reference to the Boolean flag for headless mode.
     * @param X Width of the rendering area.
     * @param Y Height of the rendering area.
     */
    Renderer(Utility::JSON *docRef, bool *flag_headless, unsigned int const &X = 1080, unsigned int const &Y = 1080);

    //------------------------------------------
    // Disallow copying and moving

    Renderer(Renderer const &) = delete;
    Renderer &operator=(Renderer const &) = delete;
    Renderer(Renderer &&) = delete;
    Renderer &operator=(Renderer &&) = delete;

    //------------------------------------------
    // Serialization / Deserialization

    /**
     * @brief Serializes the current state of the Renderer.
     * @return A JSON string representing the Renderer state.
     */
    std::string serialize() {
        return env.serialize();
    }

    /**
     * @brief Deserializes the Renderer state from a JSON string or link.
     * @param serialOrLink The JSON string or link to deserialize.
     */
    void deserialize(std::string const &serialOrLink) noexcept {
        env.deserialize(
            serialOrLink,
            getDoc()->get<uint16_t>(Constants::keyName.renderer.dispResX, 0),
            getDoc()->get<uint16_t>(Constants::keyName.renderer.dispResY, 0)
            );
    }

    //------------------------------------------
    // Pipeline

    /**
     * @brief Initializes SDL and related subsystems.
     */
    void initSDL();

    /**
     * @brief Called before parsing any commands.
     */
    Constants::Error preParse() override;

    /**
     * @brief Updates the renderer for the next frame.
     *        - clears the screen
     *        - calls the state update function
     *        - renders frame
     *        - renders fps, if enabled
     *        - presents the frame
     *        - manages SDL events
     *        - manages state for next frame
     * @return True if update was done, false if skipped (e.g. console mode or other blocking processes are active).
     */
    bool tick();

    /**
     * @brief Checks if it's time to render the next frame based on the target FPS.
     */
    bool timeToRender();

    /**
     * @brief Appends a RenderObject to the Renderer to the rendering pipeline.
     *        - Sets id of the RenderObject.
     *        - Increases the id counter.
     *        - Appends the RenderObject to the environment.
     *        - Loads its texture
     *        - updates the rolling random number generator.
     * @param toAppend Pointer to the RenderObject to append.
     */
    void append(RenderObject *toAppend);

    /**
     * @brief Reinserts all objects into the rendering pipeline.
     *        Does not change the id counter.
     */
    void reinsertAllObjects();

    /**
     * @brief Skips updating the next frame.
     *        This can be useful to avoid rendering a frame when the application is not in focus,
     *        or when the rendering load is too high.
     */
    void skipUpdateNextFrame() { skipUpdate = true; }

    /**
     * @brief Checks if the next frame update is being skipped.
     *        After a renderer tick, this is reset to false
     *        so new blocking processes have to call `skipUpdateNextFrame()` each frame.
     * @return True if the next frame update is being skipped, false otherwise.
     */
    [[nodiscard]] bool isSkippingUpdate() const noexcept { return skipUpdate; }

    /**
     * @brief Checks if the last frame update was skipped.
     *        Keeps the value from the last renderer tick.
     * @return True if the last frame update was skipped, false otherwise.
     */
    [[nodiscard]] bool hasSkippedUpdate() const { return skippedUpdateLastFrame; }

    //------------------------------------------
    // Texture Management

    /**
     * @brief Attaches a texture above a specific layer.
     * @param aboveThisLayer The layer above which to attach the texture.
     * @param name The name of the texture.
     * @param texture The SDL_Texture to attach.
     * @param rect Optional SDL_Rect defining the area to render the texture.
     * @return True if the texture was successfully attached, false otherwise.
     */
    bool attachTextureAboveLayer(Environment::Layer const &aboveThisLayer, std::string const &name, SDL_Texture *texture, SDL_Rect *rect = nullptr) {
        if (texture == nullptr) {
            return false; // Cannot attach a null texture
        }
        if (BetweenLayerTextures[aboveThisLayer].contains(name)) {
            return false; // Texture with this name already exists in the specified layer
        }
        BetweenLayerTextures[aboveThisLayer][name] = std::make_pair(texture, rect);
        return true;
    }

    /**
     * @brief Detaches a texture above a specific layer.
     * @param aboveThisLayer The layer above which to detach the texture.
     * @param name The name of the texture to remove.
     * @return True if the texture was successfully removed, false otherwise.
     */
    bool detachTextureAboveLayer(Environment::Layer const &aboveThisLayer, std::string const &name) {
        if (BetweenLayerTextures[aboveThisLayer].contains(name)) {
            BetweenLayerTextures[aboveThisLayer].erase(name);
            return true;
        }
        return false;
    }

    /**
     * @brief Detaches all textures from all layers.
     */
    void detachAllTextures() {
        BetweenLayerTextures.clear();
    }

    //------------------------------------------
    // Special Functions

    /**
     * @brief Beeps the system speaker.
     */
    void beep() const;

    /**
     * @brief Takes a snapshot of the current Renderer state.
     * @param link The link to save the snapshot to.
     * @return True if the snapshot was successful, false otherwise.
     */
    [[nodiscard]] bool snapshot(std::string link) const;

    //------------------------------------------
    // Purge

    /**
     * @brief Purges all objects from the Renderer.
     */
    void purgeObjects();

    /**
     * @brief Purges all textures from the Renderer.
     */
    void purgeTextures();

    /**
     * @brief Destroys the Renderer and all associated resources.
     */
    void destroy();

    //------------------------------------------
    // Setting

    /**
     * @brief Toggles the display of the FPS counter.
     */
    void toggleFps(bool const &show = true) {
        showFPS = show;
    }

    /**
     * @brief Sets the target FPS for the renderer.
     */
    void setTargetFPS(uint16_t const& targetFps);

    /**
     * @brief Sets the camera position.
     * @param X The new X position of the camera.
     * @param Y The new Y position of the camera.
     * @param isMiddle If true, the (x,y) coordinates relate to the middle of the screen.
     * If false, they relate to the top left corner.
     */
    void setCam(int const &X, int const &Y, bool const &isMiddle = false) const;

    /**
     * @brief Changes the window size.
     *        Total size is `w*scalar x h*scalar`
     * @param w The new pixel width of the window.
     * @param h The new pixel height of the window.
     * @param scalar The scaling factor to apply.
     */
    void changeWindowSize(int const &w, int const &h, uint16_t const &scalar);

    /**
     * @brief Moves the camera by a certain amount.
     * @param dX The amount to move the camera in the X direction.
     * @param dY The amount to move the camera in the Y direction.
     */
    void moveCam(int const &dX, int const &dY) const;


    //------------------------------------------
    // Getting

    /**
     * @brief Gets the amount of textures currently loaded.
     * @return The number of textures.
     */
    [[nodiscard]] size_t getTextureAmount() const { return TextureContainer.size(); }

    /**
     * @brief Gets the amount of RenderObjects currently loaded.
     * @return The number of RenderObjects in the environment.
     */
    [[nodiscard]] size_t getObjectCount() const { return env.getObjectCount(); }

    /**
     * @brief Gets the current resolution in the X direction.
     * @return The current resolution in the X direction.
     */
    [[nodiscard]] int getResX() const { return getDoc()->get<int>(Constants::keyName.renderer.dispResX, 0); }

    /**
     * @brief Gets the current resolution in the Y direction.
     * @return The current resolution in the Y direction.
     */
    [[nodiscard]] int getResY() const { return getDoc()->get<int>(Constants::keyName.renderer.dispResY, 0); }

    /**
     * @brief Gets the current FPS.
     * @return The current FPS.
     */
    [[nodiscard]] uint16_t getFPS() const { return fps.real; }

    /**
     * @brief Gets the current position of the camera in the X direction.
     *        The position is considered to be the top left corner of the screen.
     * @return The current position of the camera in the X direction.
     */
    [[nodiscard]] int getPosX() const { return getDoc()->get<int>(Constants::keyName.renderer.positionX, 0); }

    /**
     * @brief Gets the current position of the camera in the Y direction.
     *        The position is considered to be the top left corner of the screen.
     * @return The current position of the camera in the Y direction.
     */
    [[nodiscard]] int getPosY() const { return getDoc()->get<int>(Constants::keyName.renderer.positionY, 0); }

    /**
     * @brief Gets the current tile position of the camera in the X direction.
     *        The position to check for tile position is considered to be the top left corner of the screen.
     * @return The current tile position of the camera in the X direction.
     */
    [[nodiscard]] int16_t getTilePositionX() const noexcept { return tilePositionX; }

    /**
     * @brief Gets the current tile position of the camera in the Y direction.
     *        The position to check for tile position is considered to be the top left corner of the screen.
     * @return The current tile position of the camera in the Y direction.
     */
    [[nodiscard]] int16_t getTilePositionY() const noexcept { return tilePositionY; }

    /**
     * @brief Gets the SDL_Renderer instance.
     *        Allows for access to the underlying SDL renderer for custom rendering operations.
     * @return The SDL_Renderer instance.
     */
    [[nodiscard]] SDL_Renderer *getSdlRenderer() const { return renderer; }

    /**
     * @brief Gets the RenderObject from its ID.
     * @param id The ID of the RenderObject to retrieve.
     * @return A pointer to the RenderObject, or nullptr if not found.
     */
    RenderObject *getObjectFromId(uint32_t const &id) {
        return env.getObjectFromId(id);
    }

    /**
     * @brief Gets the standard font used by the Renderer.
     *        Loads the font if it hasn't been loaded yet.
     * @return A pointer to the TTF_Font instance.
     */
    [[nodiscard]] TTF_Font *getStandardFont() const {
        // Should always be loaded at this point
        return font;
    }

    /**
     * @brief Gets the current SDL event.
     * @return Vector of all SDL events from the current frame.
     */
    std::vector<SDL_Event> *getEventHandles() noexcept {
        return &events;
    }

    /**
     * @brief Gets the current window scale factor.
     */
    [[nodiscard]] unsigned int getWindowScale() const noexcept { return WindowScale; }

    //------------------------------------------
    // Texture-Related

    /**
     * @brief Loads a texture from a file into memory without adding it to the TextureContainer.
     *        Creates the necessary surface and texture object from a given file path,
     *        but does not store it in the TextureContainer. It is useful for temporary textures or
     *        textures that are managed externally.
     * @param link The file path to load the texture from.
     * @return A pointer to the loaded SDL_Texture, or nullptr if loading failed.
     */
    [[nodiscard]] SDL_Texture *loadTextureToMemory(std::string const &link) const;

    //------------------------------------------
    // Status

    /**
     * @brief Checks if the Renderer is initialized
     */
    [[nodiscard]] bool isSdlInitialized() const noexcept { return SDL_initialized; }

    /**
     * @brief Checks if the Renderer is set to quit
     */
    [[nodiscard]] bool shouldQuit() const noexcept { return quit; }

    /**
     * @brief Sets the quit flag for the Renderer
     */
    void setQuit() noexcept { quit = true; }

private:
    //------------------------------------------
    // Boolean Status Variables
    bool audioInitialized = false;
    bool showFPS = true; // Set default to false later on
    bool skipUpdate = false;
    bool skippedUpdateLastFrame = false;
    bool SDL_initialized = false;
    bool quit = false; // Set to true when SDL_QUIT event is received or outside wants to quit

    // External Flags
    bool *headless = nullptr;

    //------------------------------------------
    // Audio
    struct Audio {
        SDL_AudioDeviceID device = 0;
        SDL_AudioSpec desired, obtained;
    } audio;

    struct BasicAudioWaveforms {
        double const frequency = 440.0; // 440 Hz beep
        double const duration = 200.0; // 200ms
        double const sampleRate = 44100.0;
        size_t const samples = static_cast<size_t>(sampleRate * duration / 1000.0); // Number of samples
        std::vector<int16_t> *sineBuffer = nullptr;
        std::vector<int16_t> *squareBuffer = nullptr;
        std::vector<int16_t> *triangleBuffer = nullptr;
    } basicAudioWaveforms;


    //------------------------------------------
    //General Variables
    std::string baseDirectory;

    /**
     * @brief Counter for assigning unique IDs to RenderObjects.
     *        Easier to debug if it starts at 1, as 0 might come up in overflows, and negative values may not be valid
     */
    uint32_t renderObjectIdCounter = 1;

    // Positions
    int16_t tilePositionX;
    int16_t tilePositionY;

    // Custom Subclasses
    Environment env;

    // Rendering
    uint16_t WindowScale = 1;
    SDL_Window *window{};
    SDL_Renderer *renderer{};

    //------------------------------------------
    // Event Handling
    SDL_Event event{};

    std::vector<SDL_Event> events;


    //------------------------------------------
    // RNG
    std::mt19937 rngA;
    std::mt19937 rngB;

    /**
     * @brief Hashes a string to produce a size_t value.
     *
     * Used for RNG seeding.
     */
    static std::size_t hashString(std::string const &str) { return std::hash<std::string>{}(str); }

    //------------------------------------------
    // Renderer::tick related Functions

    /**
     * @brief Clears the Renderer screen
     *        This function clears renderer to an all black screen.
     */
    void clear() const;

    /**
     * @brief Updates the Renderer state.
     *        - updates timer
     *        - polls SDL events
     *        - polls mouse and keyboard state
     *        - sets global values
     *        - updates the invoke instance
     *        - updates the environment
     */
    void updateState();

    /**
     * @brief Renders the current frame.
     *        Takes all RenderObjects potentially visible in the current frame and renders them.
     *        See the Environment and RenderObjectContainer class for more details on how objects
     *        are managed in the tile-based-container
     */
    void renderFrame();

    /**
     * @brief Renders the current FPS on screen
     */
    void renderFPS() const;

    /**
     * @brief Presents the rendered frame to the screen.
     */
    void showFrame() const;

    /**
     * @brief Renders a single object to the screen.
     * @param obj Pointer to the RenderObject to render.
     * @param dispPosX The X position on the screen to render the object.
     * @param dispPosY The Y position on the screen to render the object.
     * @return SDL_Error code from SDL_RenderCopy, 0 if successful.
     */
    int renderObjectToScreen(RenderObject *obj, int const &dispPosX, int const &dispPosY);

    //------------------------------------------
    //For FPS Count and Control
    struct FpsControl {
        Utility::TimeKeeper controlTimer;
        Utility::TimeKeeper renderTimer;

        uint16_t target = 500; // Target framerate (e.g., 60 FPS)
        uint16_t realCounter = 0; // Counts fps in a 1-second-interval; reset every second
        uint16_t real = 0;        // Actual fps this past second. Stores the last value of realCounter every second
    }fps;

    //------------------------------------------
    // Texture-Related

    /**
     * @brief Loads a texture into the TextureContainer
     *        Creates the necessary surface and texture object from a given file path.
     * @param link The file path to load the texture from.
     */
    void loadTexture(std::string const &link);

    /**
     * @brief Texture container for the Renderer
     *
     * This container holds all loaded textures from RenderObject sprites for the renderer, allowing for easy access and management.
     *
     * `TextureContainer[link] -> SDL_Texture*`
     */
    absl::flat_hash_map<std::string, SDL_Texture *> TextureContainer;

    /**
     * @brief Contains textures the renderer needs to render between layers
     *
     * `BetweenLayerTextures[layer][link] -> SDL_Texture*`
     */
    absl::flat_hash_map<
        Environment::Layer,
        absl::flat_hash_map<
            std::string,
            std::pair<SDL_Texture *, SDL_Rect *> // Pair of texture and its rectangle
        >
    > BetweenLayerTextures;

    //------------------------------------------
    // Font-Related

    // Define font properties
    SDL_Color textColor = {255, 255, 255, 255}; // White color

    // General font
    TTF_Font *font{};

    /**
     * @brief Loads fonts for the Renderer.
     */
    void loadFonts();
};
} // namespace Nebulite::Core
#endif // NEBULITE_CORE_RENDERER_HPP