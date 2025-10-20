#include "DomainModule/Renderer/RRDM_Console.hpp"
#include "Core/Renderer.hpp"
#include "Core/GlobalSpace.hpp"

namespace Nebulite::DomainModule::Renderer {

Nebulite::Constants::Error Console::update(){
    //------------------------------------------
    // Prerequisites

    // Initialize font if not done yet
    if(!initialized){
        init();
    }

    //------------------------------------------
    // Toggle
    events = domain->getEventHandles();

    // Toggling console mode
    bool consoleToggle = domain->getDoc()->get<int>(toggleKey.c_str(),0) == 1;
    if(consoleToggle){
        consoleMode = !consoleMode;
        if(consoleMode){
            SDL_StartTextInput();
            SDL_FlushEvents(SDL_FIRSTEVENT, SDL_LASTEVENT); // Flush all pending events
        }
        else{
            SDL_StopTextInput();
        }
    }

    //------------------------------------------
    // Input handling
    if (consoleMode) {
        processEvents();
    }

    //------------------------------------------
    // Processing
    processMode();

    //------------------------------------------
    // Return
    return Nebulite::Constants::ErrorTable::NONE();
}

void Console::renderConsole() {
    //------------------------------------------
    // Prerequisites

    // Derive consoleRect size from display size
    int x = 0;
    int y = globalDoc->get<int>(Nebulite::Constants::keyName.renderer.dispResY.c_str(),360) / 2;
    int w = globalDoc->get<int>(Nebulite::Constants::keyName.renderer.dispResX.c_str(),360);
    int h = globalDoc->get<int>(Nebulite::Constants::keyName.renderer.dispResY.c_str(),360) - y;

    //------------------------------------------
    // Texture Setup

    // Status
    bool textureStatus = false;
    textureStatus = textureStatus || !consoleTexture.texture_ptr;   // No texture yet
    textureStatus = textureStatus || consoleTexture.rect.w != w;    // Width changed
    textureStatus = textureStatus || consoleTexture.rect.h != h;    // Height changed

    // Update rectangle if needed
    if(textureStatus){
        if (consoleTexture.texture_ptr){
            SDL_DestroyTexture(consoleTexture.texture_ptr);
        }
        consoleTexture = {
            {x, y, w, h}, // rect
            SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, w, h)
        };
    }

    // Check if texture creation was successful
    if (!consoleTexture.texture_ptr) {
        std::cerr << "SDL_CreateTexture failed: " << SDL_GetError() << std::endl;
        return;
    }

    //------------------------------------------
    // Target: Texture
    SDL_SetRenderTarget(renderer, consoleTexture.texture_ptr);

    //------------------------------------------
    // Part 1: Background

    // Draw everything as before, but coordinates relative to (0,0)
    SDL_Rect localRect = {0, 0, consoleTexture.rect.w, consoleTexture.rect.h};
    SDL_SetRenderDrawColor(renderer, 0, 32, 128, 180);
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_RenderFillRect(renderer, &localRect);
    SDL_Color textColor = {255, 255, 255, 255};

    // Calculate text alignment if needed
    static uint16_t last_rect_h = 0;
    static uint16_t lineHeight = 0;
    if(last_rect_h != consoleTexture.rect.h || flag_recalculateTextAlignment){
        last_rect_h = consoleTexture.rect.h;
        lineHeight = calculateTextAlignment(consoleTexture.rect.h);
        flag_recalculateTextAlignment = false;
    }

    //------------------------------------------
    // Part 2: Input Line

    // Add a darker background for the input line
    double posY = consoleTexture.rect.h - lineHeight - 1.5 * LINE_PADDING;
    SDL_Rect inputBackgroundRect = { 0, (int)posY, consoleTexture.rect.w, lineHeight + LINE_PADDING};
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 150);
    SDL_RenderFillRect(renderer, &inputBackgroundRect);

    // Render input text
    if (!textInput.getInputBuffer()->empty()) {
        std::string inputText = *textInput.getInputBuffer();

        // Create surface and texture
        SDL_Surface* textSurface = TTF_RenderText_Blended(consoleFont, inputText.c_str(), textColor);
        SDL_Texture* textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);

        // Define destination rectangle
        SDL_Rect textRect;
        textRect.x = 10;
        textRect.y = consoleTexture.rect.h - LINE_PADDING - lineHeight;
        textRect.w = (double)textSurface->w / WindowScale;
        textRect.h = (double)textSurface->h / WindowScale;

        // Render the text
        SDL_RenderCopy(renderer, textTexture, NULL, &textRect);
        SDL_FreeSurface(textSurface);
        SDL_DestroyTexture(textTexture);

        // If we have a text: ABCDEF, we highlight all text to the right of the cursor
        // With a semi-transparent overlay
        uint16_t cursorOffsetFromEnd = textInput.getCursorOffset();
        if(cursorOffsetFromEnd > 0){
            std::string highlightText = textInput.getInputBuffer()->substr(textInput.getInputBuffer()->size() - cursorOffsetFromEnd, cursorOffsetFromEnd);
            SDL_Surface* highlightSurface = TTF_RenderText_Blended(consoleFont, highlightText.c_str(), {255,0,0,255});
            SDL_Texture* highlightTexture = SDL_CreateTextureFromSurface(renderer, highlightSurface);
            SDL_Rect highlightRect;
            highlightRect.x = textRect.x + textRect.w - highlightSurface->w;
            highlightRect.y = textRect.y;
            highlightRect.w = (double)highlightSurface->w;
            highlightRect.h = (double)highlightSurface->h;
            SDL_RenderCopy(renderer, highlightTexture, NULL, &highlightRect);
            SDL_FreeSurface(highlightSurface);
            SDL_DestroyTexture(highlightTexture);
        }
    }

    //------------------------------------------
    // Part 3: Output Lines
    int line_index = 0;
    uint16_t outputSize = textInput.getOutput()->size();

    // Index-offset: If we have less history than lines,
    // We need to offset to align at the top
    int16_t y_start = line_y_pos[0];
    if(outputSize < line_y_pos.size()){
        y_start = line_y_pos[line_y_pos.size() - outputSize];
    }

    // Render lines from bottom to top
    for(uint16_t y : line_y_pos){
        if(y > y_start) continue;               // Skip lines under the start position
        if(line_index >= outputSize)  break;    // No more lines to show         

        // Get line
        std::string line = textInput.getOutput()->at(outputSize - 1 - line_index);
        line_index++;

        // Render line
        SDL_Surface* textSurface = TTF_RenderText_Blended(consoleFont, line.c_str(), textColor);
        SDL_Texture* textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);

        SDL_Rect textRect;
        textRect.x = 10;
        textRect.y = y;
        textRect.w = (double)textSurface->w / WindowScale;
        textRect.h = (double)textSurface->h / WindowScale;

        SDL_RenderCopy(renderer, textTexture, NULL, &textRect);
        SDL_FreeSurface(textSurface);
        SDL_DestroyTexture(textTexture);
    }

    // [DEBUG] Draw a line at every y position
    /*
    for(uint16_t y : line_y_pos){
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_RenderDrawLine(renderer, 0, y - LINE_PADDING/2, consoleTexture.rect.w, y - LINE_PADDING/2);
    }
    //*/

    //------------------------------------------
    // Target: Back to window
    SDL_SetRenderTarget(renderer, nullptr);
}

void Console::init(){
    //--------------------------------------------------
    // References
    consoleFont = domain->getStandardFont();
    renderer = domain->getSdlRenderer();
    globalDoc = domain->getDoc();

    //--------------------------------------------------
    // Console buffer

    // Initialize history with a welcome message
    textInput.insertLine("Welcome to Nebulite!");
    textInput.insertLine("Type 'help' for a list of commands.");

    //--------------------------------------------------
    // Console now fully functional
    initialized = true;
}

uint8_t Console::calculateTextAlignment(uint16_t rect_height){
    // Populating the rect:
    /*
    <PADDING>
    <LINE 1>
    <PADDING>
    <LINE 2>
    <PADDING>
    ...
    <PADDING>
    <LINE N>
    <PADDING>
    <PADDING>
    <INPUT LINE>
    <PADDING>
    */

    // Formula:
    // rect_height = (N+3)*LINE_PADDING + (N+1)*LINE_HEIGHT
    // LINE_HEIGHT = (rect_height - (N+3)*LINE_PADDING) / (N+1)
    // N           = (rect_height - 3*LINE_PADDING) / (LINE_HEIGHT + LINE_PADDING)

    // Constraints:
    // LINE_HEIGHT <= FONT_MAX_SIZE
    // MINIMUM_LINES <= N
    uint16_t LINE_HEIGHT = FONT_MAX_SIZE;

    // See where we land for N with the maximum font size
    uint16_t N = floor((double)(rect_height - 3*LINE_PADDING) / (double)(LINE_HEIGHT + LINE_PADDING));

    // Reduce line height if we have less than minimum lines
    if(N < MINIMUM_LINES){
        N = MINIMUM_LINES;
        LINE_HEIGHT = (rect_height - (N+3)*LINE_PADDING) / (N+1);
    }

    // Now, line height and N are final
    // Populate y positions
    line_y_pos.clear();
    for(int i = 1; i < N; i++){ // i=0 is reserved for input line
        line_y_pos.push_back( rect_height - LINE_PADDING - 2*LINE_HEIGHT - i*(LINE_HEIGHT + LINE_PADDING) );
    }

    // Set correct font size for SDL_ttf
    WindowScale = global->getRenderer()->getWindowScale();
    TTF_SetFontSize(consoleFont, LINE_HEIGHT * WindowScale);

    return LINE_HEIGHT;
}

//--------------------------------------------------
// Event processing

void Console::processEvents(){
    static std::string command;
    for (const auto& event : *events) {
        switch (event.type) {
            case SDL_TEXTINPUT:
                textInput.append(event.text.text);
                break;

            case SDL_KEYDOWN:
                switch (event.key.keysym.sym) {
                    //------------------------------------------
                    // Text input manipulation

                    // Remove last character on backspace
                    case SDLK_BACKSPACE:
                        textInput.backspace();
                        break;

                    // Submit command on Enter
                    case SDLK_RETURN:
                    case SDLK_KP_ENTER:
                        command = textInput.submit();
                        if(!command.empty()){
                            // Parse command on global level for full access to all functions
                            global->parseStr(std::string(__FUNCTION__) + " " + command);
                        }
                        break;	

                    // Cursor movement
                    case SDLK_LEFT:
                        textInput.moveCursorLeft();
                        break;
                    case SDLK_RIGHT:
                        textInput.moveCursorRight();
                        break;

                    //------------------------------------------
                    // UP/DOWN to cycle through past commands
                    case SDLK_UP:
                        textInput.history_up();
                        break;
                    case SDLK_DOWN:
                        textInput.history_down();
                        break;
                }
                break;
        }
    }
}

void Console::processMode(){
    if (consoleMode) {
        // Render texture and attach
        renderConsole();

        // Check if texture is valid
        if(!consoleTexture.texture_ptr){
            std::cerr << "Could not attach Console: Console texture is null!" << std::endl;
            return;
        }

        // Attach texture above UI layer
        (void)domain->attachTextureAboveLayer(
            Nebulite::Core::Environment::Layer::UI,
            "console_overlay", 
            consoleTexture.texture_ptr,
            &consoleTexture.rect
        );

        // Skip updating the renderer for this frame, as we are in console mode
        domain->skipUpdateNextFrame(); 
    }
    else{
        // Clear texture and detach
        (void)domain->detachTextureAboveLayer(
            Nebulite::Core::Environment::Layer::UI,
            "console_overlay"
        );
        if(consoleTexture.texture_ptr){
            SDL_DestroyTexture(consoleTexture.texture_ptr);
            consoleTexture.texture_ptr = nullptr;
        }
    }
}

//------------------------------------------
// Category strings

const std::string Console::console_name = "console";
const std::string Console::console_desc = R"(Console commands and settings.
Contains commands to manipulate the in-application console.
)";

//------------------------------------------
// Available Functions

Nebulite::Constants::Error Console::consoleZoom(int argc,  char* argv[]){
    //------------------------------------------
    // Prerequisites

    // Validate arguments
    if(argc > 2){
        return Nebulite::Constants::ErrorTable::FUNCTIONAL::TOO_MANY_ARGS();
    }

    //------------------------------------------
    // Determine zoom direction

    if(argc == 2){
        std::string direction = argv[1];
        if(direction == "in" || direction == "+"){
            if(FONT_MAX_SIZE <= 48){
                FONT_MAX_SIZE++;
                flag_recalculateTextAlignment = true;
            }
        }
        else if(direction == "out" || direction == "-"){
            if(FONT_MAX_SIZE >= 8){
                FONT_MAX_SIZE--;
                flag_recalculateTextAlignment = true;
            }
        }
        else{
            return Nebulite::Constants::ErrorTable::FUNCTIONAL::UNKNOWN_ARG();
        }
    }

    //------------------------------------------
    // Return
    return Nebulite::Constants::ErrorTable::NONE();
}
const std::string Console::consoleZoom_name = "console zoom";
const std::string Console::consoleZoom_desc = R"(Reduces or increases the console font size.

Usage: zoom [in/out/+/-]
- in / + : Zooms in (increases font size)
- out / - : Zooms out (decreases font size)
)";

}   // namespace Nebulite::DomainModule::GlobalSpace::Console
