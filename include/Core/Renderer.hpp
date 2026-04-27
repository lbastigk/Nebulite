/**
 * @file Renderer.hpp
 * @brief This file contains the declaration of the Nebulite::Core::Renderer class, which is responsible for rendering
 *        the game objects and managing the rendering pipeline.
 */

#ifndef NEBULITE_CORE_RENDERER_HPP
#define NEBULITE_CORE_RENDERER_HPP

//------------------------------------------
// Predeclare

namespace Nebulite::Interaction {
class ContextScope;
} // namespace Nebulite::Interaction

//------------------------------------------
// Includes

// Standard library
#include <vector>

// External
#include <absl/container/flat_hash_map.h>
#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>

// Nebulite
#include "Core/Environment.hpp"
#include "Data/RendererProcessor.hpp"
#include "Graphics/RmlInterface.hpp"
#include "Interaction/Invoke.hpp"
#include "Interaction/Execution/Domain.hpp"
#include "Module/Base/RmlUiModule.hpp"
#include "Utility/TimeKeeper.hpp"

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
    // Pipeline

    /**
     * @brief Initializes SDL and related subsystems.
     */
    void initSDL();

    /**
     * @brief Initializes ImGui for the Renderer. Called within `initSDL()`.
     */
    void initImgui() const ;

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
    bool attachTextureAboveLayer(Environment::Layer const& aboveThisLayer, std::string const& name, SDL_Texture* texture, SDL_Rect* rect = nullptr) {
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
    bool detachTextureAboveLayer(Environment::Layer const& aboveThisLayer, std::string const& name) {
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
    void toggleFps(bool const& show = true) { status.showFps = show; }

    /**
     * @brief Sets the target FPS for the renderer.
     */
    void setTargetFPS(uint16_t const& targetFps);

    /**
     * @brief Sets the camera position.
     * @param X The new X position of the camera.
     * @param Y The new Y position of the camera.
     * @param isMiddle If true, the (x,y) coordinates relate to the middle of the screen.
     *                 If false, they relate to the top left corner.
     */
    void setCam(int const& X, int const& Y, bool const& isMiddle = false) const;

    /**
     * @brief Changes the window size.
     *        Total size is `w*scalar x h*scalar`
     * @param w The new pixel width of the window.
     * @param h The new pixel height of the window.
     * @param scalar The scaling factor to apply.
     */
    void changeWindowSize(int const& w, int const& h, uint8_t const& scalar);

    /**
     * @brief Moves the camera by a certain amount.
     * @param dX The amount to move the camera in the X direction.
     * @param dY The amount to move the camera in the Y direction.
     */
    void moveCam(int const& dX, int const& dY) const;

    [[nodiscard]] SDL_FRect scaleRectFromLogicalSize(SDL_FRect const& logicalRect) const {
        return SDL_FRect{
            logicalRect.x * static_cast<float>(windowScale),
            logicalRect.y * static_cast<float>(windowScale),
            logicalRect.w * static_cast<float>(windowScale),
            logicalRect.h * static_cast<float>(windowScale)
        };
    }

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
    [[nodiscard]] int getResX() const ;

    /**
     * @brief Gets the current resolution in the Y direction.
     * @return The current resolution in the Y direction.
     */
    [[nodiscard]] int getResY() const ;

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
    [[nodiscard]] int getPosX() const ;

    /**
     * @brief Gets the current position of the camera in the Y direction.
     *        The position is considered to be the top left corner of the screen.
     * @return The current position of the camera in the Y direction.
     */
    [[nodiscard]] int getPosY() const ;

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
    [[nodiscard]] SDL_Renderer* getSdlRenderer() const { return renderer; }

    /**
     * @brief Gets the SDL_Window instance.
     * @details Allows for access to the underlying SDL window for custom operations.
     * @return The SDL_Window instance.
     */
    [[nodiscard]] SDL_Window* getSdlWindow() const { return window; }

    /**
     * @brief Exposes the Rml::DataModelConstructor for external use.
     * @return A reference to the Rml::DataModelConstructor instance used by the Renderer.
     */
    [[nodiscard]] Rml::DataModelConstructor& getDataModelConstructor() { return rml.dataModelConstructor; }

    /**
     * @brief Gets the RenderObject ID from its index in the rendering pipeline.
     * @param index The index of the RenderObject in the rendering pipeline.
     * @return An optional containing the ID of the RenderObject if found, or std::nullopt if no object is associated with the given index.
     */
    [[nodiscard]] std::optional<size_t> getIdFromIndex(size_t const& index) const ;

    /**
     * @brief Gets the RenderObject index in the rendering pipeline from its ID.
     * @param domainId The domain ID of the RenderObject to search for.
     * @return An optional containing the index of the RenderObject in the rendering pipeline if found, or std::nullopt if no object is associated with the given ID.
     */
    [[nodiscard]] std::optional<size_t> getIndexFromId(size_t const& domainId) const ;

    /**
     * @brief Gets the RenderObject from its ID.
     * @param searchIndex The Index of the RenderObject to retrieve. In chronological order based on when they were appended to the Renderer.
     *        Does not change when objects are removed or purged.
     * @return A pointer to the RenderObject, or nullptr if not found.
     */
    std::optional<std::pair<RenderObject*, Data::JsonScope*>> getObjectFromIndex(size_t const& searchIndex) ;

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
    // Rml Context

    size_t rmlDocumentCount() const {
        return rml.countOpenedDocuments();
    }

    bool loadRmlDocument(std::string_view const& name, std::string_view const& path, Interaction::Context const& ctx, Interaction::ContextScope const& ctxScope) {
        return rml.loadDocument(name, path, ctx, ctxScope);
    }

    bool removeRmlDocument(size_t const& domainId, std::string_view const& name) {
        return rml.removeDocument(domainId, name);
    }

    bool removeRmlDocument(Rml::ElementDocument* doc) {
        return rml.removeDocument(doc);
    }

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

    static void destroyTexture(SDL_Texture* t) {
        SDL_DestroyTexture(t);
    }

    static bool isTextureValid(SDL_Texture const* t) noexcept {return t != nullptr; }

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
    * @brief Adds a callback function to be executed after the current render pass is complete.
    * @details Is executed once.
    * @param function The callback function to add.
    */
    void addPostRenderCallback(std::function<void()> const& function) noexcept {
        postRenderCallback.emplace_back(function);
    }

private:
    /**
     * @brief Called before parsing any commands.
     */
    [[nodiscard]] Constants::Event preParse() override;

    //static auto constexpr pixelFontPath  = "./Resources/Fonts/JetBrainsMono-Regular.ttf"; // TODO: Use a pixel font

    /**
     * @brief Holds threads for parallel processing of RenderObjects during the update phase.
     */
    Data::RendererProcessor rendererProcessor;

    //------------------------------------------
    // Rml Interface

    Graphics::RmlInterface rml;

    //------------------------------------------
    // Boolean Status Variables

    struct Status {
        bool showFps = true; // Set default to false later on
        bool skipUpdate = false;
        bool skippedUpdateLastFrame = false;
        bool sdlInitialized = false;
        bool quit = false; // Set to true when an SDL_QUIT event is received or outside wants to quit
        bool firstFrameRendered = false; // Used to manage first frame rendering and timing
    }status;

    // External Flags
    bool* headless = nullptr;

    //------------------------------------------
    // Append index to domain id

    absl::flat_hash_map<size_t, size_t> indexToIdMap;
    size_t indexCounter = 1; // Start at 1 to avoid confusion with default value of 0

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

    // Positions
    int16_t tilePositionX;
    int16_t tilePositionY;

    // Custom Subclasses
    Environment env;

    // Rendering
    uint8_t windowScale = 1;
    SDL_Window* window{};

    SDL_Renderer* renderer{};

    //------------------------------------------
    // Pipeline: Software / General

    void renderInit();

    void pollEvents();

    void renderFrame();

    void renderFPS() const;

    static void renderObjectToScreen(RenderObject* obj, int const& dispPosX, int const& dispPosY);

    //------------------------------------------
    // Event and routine Handling

    std::vector<SDL_Event> events;

    // Functions to execute after a full render pass
    std::vector<std::function<void()>> postRenderCallback;


    //------------------------------------------
    // For FPS Count and Control
    struct FpsControl {
        Utility::TimeKeeper controlTimer;
        Utility::TimeKeeper renderTimer;
        uint16_t target = 500; // Target framerate (e.g., 60 FPS)
        uint16_t realCounter = 0; // Counts fps in a 1-second-interval; reset every second
        uint16_t real = 0; // Actual fps this past second. Stores the last value of realCounter every second
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
            std::pair<SDL_Texture*, SDL_Rect*> // Pair of texture and its rectangle
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
#endif // NEBULITE_CORE_RENDERER_HPP
