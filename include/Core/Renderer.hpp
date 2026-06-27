#ifndef CORE_RENDERER_HPP
#define CORE_RENDERER_HPP

//------------------------------------------
// Includes

// Standard library
#include <string>
#include <vector>

// External
#include <SDL3_ttf/SDL_ttf.h>
#include <absl/container/flat_hash_map.h>

// Nebulite
#include "Core/Environment.hpp"
#include "Data/RendererProcessor.hpp"
#include "Interaction/Execution/Domain.hpp"
#include "Utility/TimeKeeper.hpp"

//------------------------------------------
// Forward declarations

namespace Nebulite::Interaction {
class ContextScope;
} // namespace Nebulite::Interaction

//------------------------------------------
namespace Nebulite::Core {
/**
 * @class Nebulite::Core::Renderer
 * @brief Responsible for rendering game objects and managing the rendering pipeline.
 */
class Renderer final : public Interaction::Execution::Domain {
public:
    /**
     * @brief Initializes a Renderer with given dimensions and settings.
     * @param documentReference Reference to the JSON document
     * @param flag_headless Reference to the Boolean flag for headless mode.
     * @param parentCapture Reference to the parent capture for logging and error handling.
     *                      Either from the Domain that owns this one or from the global capture if this is a top-level domain.
     */
    Renderer(Data::JsonScope& documentReference, bool* flag_headless, Utility::IO::Capture& parentCapture);

    ~Renderer() override ;

    //------------------------------------------
    // Disallow copying and moving

    Renderer(Renderer const&) = delete;
    Renderer& operator=(Renderer const&) = delete;
    Renderer(Renderer&&) = delete;
    Renderer& operator=(Renderer&&) = delete;

    //------------------------------------------
    // Serialization / Deserialization

    /**
     * @brief Serializes the current state of the Renderer.
     * @return A JSON string representing the Renderer state.
     */
    std::string serialize();

    /**
     * @brief Deserializes the Renderer state from a JSON string or link.
     * @param serialOrLink The JSON string or link to deserialize.
     */
    void deserialize(std::string const& serialOrLink) noexcept ;

    //------------------------------------------
    // Get objects (exposing functions from environment)

    /**
    * @brief Gets the RenderObject ID from its index in the rendering pipeline.
    * @param index The index of the RenderObject in the rendering pipeline.
    * @return An optional containing the ID of the RenderObject if found, or std::nullopt if no object is associated with the given index.
    */
    std::optional<size_t> getIdFromIndex(size_t const& index) const ;

    /**
     * @brief Gets the RenderObject index in the rendering pipeline from its ID.
     * @param searchId The ID of the RenderObject to search for.
     * @return An optional containing the index of the RenderObject in the rendering pipeline if found, or std::nullopt if no object is associated with the given ID.
     */
    std::optional<size_t> getIndexFromId(size_t const& searchId) const ;

    //------------------------------------------
    // Pipeline

    /**
     * @brief Initializes SDL and related subsystems.
     */
    void initSDL();

    /**
     * @brief Initializes ImGui for the Renderer. Called within `initSDL()`.
     */
    void initImgui();

    /**
     * @brief Updates the renderer for the next frame.
     * @details Tasks performed:
     *          - clears the screen
     *          - renders frame
     *          - renders fps, if enabled
     *          - presents the frame
     *          - manages SDL events
     *          - manages state for next frame
     */
    void render();

    /**
     * @brief Updates the Renderer state.
     * @details Tasks performed:
     *          - updates timer
     *          - polls SDL events
     *          - polls mouse and keyboard state
     *          - sets global values
     *          - updates the invoke instance
     *          - updates the environment
     */
    [[nodiscard]] Constants::Event update() override;

    /**
     * @brief Checks if it's time to render the next frame based on the target FPS.
     */
    bool timeToRender();

    /**
     * @brief Appends a RenderObject to the Renderer to the rendering pipeline.
     * @details Tasks performed:
     *          - Sets id of the RenderObject.
     *          - Increases the id counter.
     *          - Appends the RenderObject to the environment.
     *          - Loads its texture
     *          - updates the rolling random number generator.
     * @param toAppend Pointer to the RenderObject to append.
     */
    void append(RenderObject* toAppend);

    /**
     * @brief Reinserts all objects into the rendering pipeline.
     *        Does not change the id counter.
     */
    void reinsertAllObjects();

    /**
     * @brief Skips updating the next frame.
     * @details This can be useful to avoid rendering a frame when the application is not in focus,
     *          or when the rendering load is too high.
     */
    void skipUpdateNextFrame() { status.skipUpdate = true; }

    /**
     * @brief Checks if the next frame update is being skipped.
     * @details After a renderer tick, this is reset to false
     *          so new blocking processes have to call `skipUpdateNextFrame()` each frame.
     * @return True if the next frame update is being skipped, false otherwise.
     */
    [[nodiscard]] bool isSkippingUpdate() const noexcept { return status.skipUpdate; }

    /**
     * @brief Checks if the last frame update was skipped.
     *        Keeps the value from the last renderer tick.
     * @return True if the last frame update was skipped, false otherwise.
     */
    [[nodiscard]] bool hasSkippedUpdate() const { return status.skippedUpdateLastFrame; }

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
    bool attachTextureAboveLayer(Environment::Layer aboveThisLayer, std::string const& name, SDL_Texture* texture,  std::optional<SDL_FRect> rect = std::nullopt);

    /**
     * @brief Detaches a texture above a specific layer.
     * @param aboveThisLayer The layer above which to detach the texture.
     * @param name The name of the texture to remove.
     * @return True if the texture was successfully removed, false otherwise.
     */
    bool detachTextureAboveLayer(Environment::Layer aboveThisLayer, std::string const& name);

    /**
     * @brief Detaches all textures from all layers.
     */
    void detachAllTextures();

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
    void toggleFps(bool const show = true) { status.showFps = show; }

    /**
     * @brief Sets the target FPS for the renderer.
     */
    void setTargetFPS(std::uint16_t const& targetFps);

    /**
     * @brief Changes the window size.
     * @details Total size is `w*scalar x h*scalar`
     *          The center of the screen remains the same before and after resizing,
     *          so the camera position is adjusted accordingly.
     * @param w The new pixel width of the window.
     * @param h The new pixel height of the window.
     * @param scalar The scaling factor to apply.
     */
    void changeWindowSize(int w, int h, uint8_t scalar);

    /**
     * @brief Sets the camera position.
     * @param X The new X position of the camera.
     * @param Y The new Y position of the camera.
     * @param isMiddle If true, the (x,y) coordinates relate to the middle of the screen.
     *                 If false, they relate to the top left corner.
     */
    void setCam(int X, int Y, bool isMiddle = false) const;

    /**
     * @brief Moves the camera by a certain amount.
     * @param dX The amount to move the camera in the X direction.
     * @param dY The amount to move the camera in the Y direction.
     */
    void moveCam(int dX, int dY) const;

    /**
     * @brief Scales a rectangle from logical size to window size based on the current window scale factor.
     * @param logicalRect The rect from the logical coordinate system to scale.
     * @return The rect in the window coordinate system
     */
    [[nodiscard]] SDL_FRect scaleRectFromLogicalSize(SDL_FRect const& logicalRect) const ;

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
     * @brief Gets the current FPS.
     * @return The current FPS.
     */
    [[nodiscard]] std::uint16_t getFPS() const { return fps.real; }

    /**
     * @brief Gets the current tile position of the camera in the X direction.
     *        The position to check for tile position is considered to be the top left corner of the screen.
     * @return The current tile position of the camera in the X direction.
     */
    [[nodiscard]] std::int16_t getTilePositionX() const noexcept { return cameraTilePosition.x; }

    /**
     * @brief Gets the current tile position of the camera in the Y direction.
     *        The position to check for tile position is considered to be the top left corner of the screen.
     * @return The current tile position of the camera in the Y direction.
     */
    [[nodiscard]] std::int16_t getTilePositionY() const noexcept { return cameraTilePosition.y; }

    /**
     * @brief Gets the SDL_Renderer instance.
     *        Allows for access to the underlying SDL renderer for custom rendering operations.
     * @return The SDL_Renderer instance.
     */
    [[nodiscard]] SDL_Renderer* getSdlRenderer() const { return renderer; }

    /**
     * @brief Gets the SDL_Window instance.
     * @details Allows for access to the underlying SDL window for custom operations.
     * @return The SDL_Window instance.
     */
    [[nodiscard]] SDL_Window* getSdlWindow() const { return window; }

    /**
     * @brief Gets the standard font used by the Renderer.
     *        Loads the font if it hasn't been loaded yet.
     * @return A pointer to the TTF_Font instance.
     */
    [[nodiscard]] TTF_Font* getStandardFont() const {return font;}

    /**
     * @brief Gets the current SDL event.
     * @return Vector of all SDL events from the current frame.
     */
    std::vector<SDL_Event>* getEventHandles() noexcept {return &events;}

    /**
     * @brief Gets the current window scale factor.
     */
    [[nodiscard]] unsigned int getWindowScale() const noexcept { return windowScale; }

    //------------------------------------------
    // Texture-Related

    /**
     * @brief Loads a texture from a file into memory without adding it to the TextureContainer.
     * @details Creates the necessary surface and texture object from a given file path,
     *          but does not store it in the TextureContainer. It is useful for temporary textures or
     *          textures that are managed externally.
     * @param link The file path to load the texture from.
     * @return A pointer to the loaded SDL_Texture, or nullptr if loading failed.
     */
    [[nodiscard]] SDL_Texture* loadTextureToMemory(std::string const& link);

    /**
     * @brief Retrieves a texture from the TextureContainer.
     * @details If the texture is not already loaded, it attempts to load it from the specified file path.
     * @param link The file path of the texture to retrieve.
     * @return A pointer to the SDL_Texture, or nullptr if loading failed.
     */
    [[nodiscard]] SDL_Texture* getTexture(std::string const& link);

    //------------------------------------------
    // Status

    /**
     * @brief Checks if the SDL Renderer is initialized
     * @return True if the SDL Renderer is initialized, false otherwise.
     */
    [[nodiscard]] bool isSdlInitialized() const noexcept { return status.sdlInitialized; }

    /**
     * @brief Checks if the Renderer is set to quit
     * @return True if the Renderer is set to quit, false otherwise.
     */
    [[nodiscard]] bool shouldQuit() const noexcept { return status.quit; }

    /**
     * @brief Sets the quit flag for the Renderer
     */
    void setQuit() noexcept { status.quit = true; }

    //------------------------------------------
    // Callback

    /**
     * @brief Adds a callback function to be executed during the render pass
     * @details Is executed once.
     * @param function The callback function to add
     * @param aboveThisLayer The layer above which to execute the callback. The callback will be executed after rendering the specified layer.
     */
    void addRenderCallback(std::function<void()> const& function, Environment::Layer aboveThisLayer = Environment::FinalLayer);

    /**
    * @brief Adds a callback function to be executed after the current render pass is complete.
    * @details Is executed once.
    * @param function The callback function to add.
    */
    void addPostRenderCallback(std::function<void()> const& function);

    //------------------------------------------
    // Viewport

    enum class ViewSetting : std::uint8_t {
        high,
        low,
        lowest
    } viewSetting = ViewSetting::high;

    /**
     * @brief Sets the renderer view setting, determining the amount of tiles being updated and rendered.
     * @param view The view setting to apply.
     */
    void setView(ViewSetting view) noexcept ;

    /**
     * @brief Gets all visible tiles of the current renderer view
     * @details Includes some margins, so not every tile is technically visible!
     * @return A vector of TileCoordinates representing the tiles that are currently visible in the renderer's viewport.
     */
    [[nodiscard]] std::vector<Data::TileCoordinate> visibleTiles() const ;

    /**
     * @brief Gets tiling information for the renderer.
     * @return A TilingInformation struct containing the width and height of the tiles used in the renderer.
     */
    static Data::TilingInformation tilingInformation();

    /**
     * @brief Executes a function on each RenderObject in the visible tiles of a specific layer.
     * @param layer The layer for which to execute the function on visible RenderObjects.
     * @param function The function to execute
     */
    void onViewport(Environment::Layer layer, auto&& function);

    /**
     * @brief Executes a function on each visible tile of a specific layer
     * @param layer The layer for which to execute the function on visible tiles
     * @param function The function to execute
     */
    void onViewportTiles(Environment::Layer layer, auto&& function);

    /**
     * @brief Gets the tile at a specific coordinate and layer.
     * @param layer The layer
     * @param pos The tile coordinate
     * @return A reference to the tile at the specified coordinate and layer.
     */
    Data::Tile const& getTile(Environment::Layer layer, Data::TileCoordinate const& pos);

private:
    /**
     * @brief Called before parsing any commands.
     */
    [[nodiscard]] Constants::Event preParse() override;

    /**
     * @brief Holds threads for parallel processing of RenderObjects during the update phase.
     */
    Data::RendererProcessor rendererProcessor;

    //------------------------------------------
    // Boolean Status Variables

    struct Status {
        bool showFps = true; // Set default to false later on
        bool skipUpdate = false;
        bool skippedUpdateLastFrame = false;
        bool sdlInitialized = false;
        bool quit = false; // Set to true when an SDL_QUIT event is received or outside wants to quit
    }status;

    // External Flags
    bool* headless = nullptr;

    //------------------------------------------
    // Display

    /**
     * @brief Sets up display values in the workspace.
     */
    void setupDisplayValues() ;

    //------------------------------------------
    // General Variables

    /**
     * @brief Base directory for resource loading.
     * @todo Route all resource loading through Global::instance() for consistent path management.
     */
    std::string baseDirectory;

    /**
     * @brief Represents the tile at the middle of the screen
     */
    Data::TileCoordinate cameraTilePosition;

    // Custom Subclasses
    Environment env;

    // Rendering
    uint8_t windowScale = 1;
    SDL_Window* window{};

    SDL_Renderer* renderer{};

    //------------------------------------------
    // Pipeline: Software / General

    void renderInit() const ;

    void pollEvents();

    void renderFrame();

    void renderFPS() const;

    //------------------------------------------
    // Event and routine Handling

    std::vector<SDL_Event> events;

    // Functions to execute during rendering
    absl::flat_hash_map<Environment::Layer, std::vector<std::function<void()>>> renderCallbacks;

    // Functions to execute after a full render pass
    std::vector<std::function<void()>> postRenderCallback;

    //------------------------------------------
    // For FPS Count and Control
    struct FpsControl {
        Utility::TimeKeeper controlTimer;
        Utility::TimeKeeper renderTimer;
        std::uint16_t target = 500; // Target framerate (e.g., 60 FPS)
        std::uint16_t realCounter = 0; // Counts fps in a 1-second-interval; reset every second
        std::uint16_t real = 0; // Actual fps this past second. Stores the last value of realCounter every second
    } fps;

    //------------------------------------------
    // Texture-Related

    /**
     * @brief Loads a texture into the TextureContainer
     *        Creates the necessary surface and texture object from a given file path.
     * @param link The file path to load the texture from.
     */
    void loadTexture(std::string const& link);

    /**
     * @brief Texture container for the Renderer
     * @details Holds all loaded textures from RenderObject sprites for the renderer, allowing for easy access and management.
     *          `TextureContainer[link] -> Texture*`
     */
    absl::flat_hash_map<std::string, SDL_Texture*> TextureContainer;

    /**
     * @brief Contains textures the renderer needs to render between layers
     * @details `BetweenLayerTextures[layer][link] -> SDL_Texture*`
     */
    absl::flat_hash_map<
        Environment::Layer,
        absl::flat_hash_map<
            std::string,
            std::pair<SDL_Texture*, std::optional<SDL_FRect>> // Pair of texture and its rectangle
        >
    > BetweenLayerTextures;

    //------------------------------------------
    // Font-Related

    // General font
    TTF_Font* font{};

    /**
     * @brief Loads fonts for the Renderer.
     */
    void loadFonts();
};
} // namespace Nebulite::Core
#endif // CORE_RENDERER_HPP
