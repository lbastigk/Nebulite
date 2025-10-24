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
    // Insert new lines from capture streams
    static size_t last_size = 0;
    size_t current_size = Nebulite::Utility::Capture::instance().getOutputLogPtr().size();
    if(current_size < last_size){
        // Log was cleared, reset
        last_size = 0;
    }
    for(size_t i = last_size; i < current_size; i++){
        // Split input line by newlines
        auto const& input = Nebulite::Utility::Capture::instance().getOutputLogPtr().at(i);
        auto const& lines = Nebulite::Utility::StringHandler::split(input.content, '\n');

        // Insert into text input
        Nebulite::Utility::TextInput::LineEntry::LineType type;
        switch (input.type){
        case Nebulite::Utility::Capture::OutputLine::COUT:
            type = Nebulite::Utility::TextInput::LineEntry::LineType::COUT;
            break;
        case Nebulite::Utility::Capture::OutputLine::CERR:
            type = Nebulite::Utility::TextInput::LineEntry::LineType::CERR;
            break;
        }
        for(auto const& line : lines){
            if(!line.empty()){
                textInput.insertLine(line, type);
            }
        }
    }
    last_size = current_size;

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

bool Console::ensureConsoleTexture() {
    //------------------------------------------
    // Prerequisites

    // Derive consoleRect size from display size
    double heightRatio = 0.75; // Console takes 75% of the screen height
    int y = globalDoc->get<int>(Nebulite::Constants::keyName.renderer.dispResY.c_str(),360) * (1.0 - heightRatio);
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
            {0, y, w, h}, // rect
            SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, w, h)
        };
    }

    // Validate texture
    return consoleTexture.texture_ptr != nullptr;
}

void Console::drawBackground(){
    //------------------------------------------
    // Draw everything as before, but coordinates relative to (0,0)
    SDL_Rect localRect = {0, 0, consoleTexture.rect.w, consoleTexture.rect.h};

    // If we have a background image, draw it instead
    if(backgroundImageTexture != nullptr){
        SDL_RenderCopy(renderer, backgroundImageTexture, NULL, &localRect);
    }
    else{
        SDL_SetRenderDrawColor(renderer, color.background.r, color.background.g, color.background.b, color.background.a);
        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
        SDL_RenderFillRect(renderer, &localRect);
    }
}

void Console::drawInput(uint16_t lineHeight) {
    // Add a darker background for the input line
    double posY = consoleTexture.rect.h - lineHeight - 1.5 * LINE_PADDING;
    SDL_Rect inputBackgroundRect = { 0, (int)posY, consoleTexture.rect.w, lineHeight + LINE_PADDING};
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 150);
    SDL_RenderFillRect(renderer, &inputBackgroundRect);

    // Render input text
    if (!textInput.getInputBuffer()->empty()) {
        std::string inputText = *textInput.getInputBuffer();

        // Create surface and texture
        SDL_Surface* textSurface = TTF_RenderText_Blended(consoleFont, inputText.c_str(), color.input);
        SDL_Texture* textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);

        // Define destination rectangle
        textInputRect.x = 10;
        textInputRect.y = consoleTexture.rect.h - LINE_PADDING - lineHeight;
        textInputRect.w = (double)textSurface->w / WindowScale;
        textInputRect.h = (double)textSurface->h / WindowScale;

        // Render the text
        SDL_RenderCopy(renderer, textTexture, NULL, &textInputRect);
        SDL_FreeSurface(textSurface);
        SDL_DestroyTexture(textTexture);

        // If we have a text: ABCDEF, we highlight all text to the right of the cursor
        // With a semi-transparent overlay
        uint16_t cursorOffsetFromEnd = textInput.getCursorOffset();
        if(cursorOffsetFromEnd > 0){
            std::string highlightText = textInput.getInputBuffer()->substr(textInput.getInputBuffer()->size() - cursorOffsetFromEnd, cursorOffsetFromEnd);
            SDL_Surface* highlightSurface = TTF_RenderText_Blended(consoleFont, highlightText.c_str(), color.highlight);
            SDL_Texture* highlightTexture = SDL_CreateTextureFromSurface(renderer, highlightSurface);
            textInputHighlightRect.x = textInputRect.x + textInputRect.w - highlightSurface->w;
            textInputHighlightRect.y = textInputRect.y;
            textInputHighlightRect.w = (double)highlightSurface->w;
            textInputHighlightRect.h = (double)highlightSurface->h;
            SDL_RenderCopy(renderer, highlightTexture, NULL, &textInputHighlightRect);
            SDL_FreeSurface(highlightSurface);
            SDL_DestroyTexture(highlightTexture);
        }
    }
}

void Console::drawOutput(uint16_t maxLineLength) {
    uint16_t lineIndex = 0;
    std::string lineContentRest;                        // Rest of line after linebreak
    uint16_t outputSize = textInput.getOutput()->size();

    // Since we start from the bottom, we need to invert the scrolling offset
    // is increased in-loop due to linebreaks
    int32_t lineIndexOffset = - outputScrollingOffset;   

    // Index-offset: If we have less history than lines,
    // We need to offset to align at the top
    int16_t y_start = line_y_positions[0];
    if(outputSize < line_y_positions.size()){
        y_start = line_y_positions[line_y_positions.size() - outputSize];
    }

    // Render lines from bottom to top
    for(uint16_t line_y_position : line_y_positions){
        if(line_y_position > y_start) continue;  // Skip lines under the start position
        if(lineIndex >= outputSize)  break;     // No more lines to show

        // Constrain outputScrollingOffset by investigating the current index
        int32_t currentIndex = outputSize - 1 - lineIndex + lineIndexOffset;
        if(currentIndex < 0){
            // Cour outputScrollingOffset is too high, we reached the top
            outputScrollingOffset --;
            break;
        }
        if(currentIndex >= outputSize){
            // Current outputScrollingOffset is too low, we reached the bottom
            outputScrollingOffset ++;
            break;
        }

        // Get line info
        auto lineInfo = textInput.getOutput()->at(currentIndex);
        SDL_Color textColor;
        std::string content;

        if(!lineContentRest.empty()){
            // We have a rest from the previous linebreak
            content = lineContentRest;
            lineContentRest.clear();
            lineIndexOffset++; // Increase offset since we are using an extra line
        }
        else{
            // New line
            switch (lineInfo.type){
            case Nebulite::Utility::TextInput::LineEntry::LineType::CERR:
                textColor = color.cerrStream;
                content = lineInfo.content;
                break;
            case Nebulite::Utility::TextInput::LineEntry::LineType::COUT:
                textColor = color.coutStream;
                content = lineInfo.content;
                break;
            case Nebulite::Utility::TextInput::LineEntry::LineType::INPUT:
                textColor = color.input;
                content = "> " + lineInfo.content;
                break;
            }
        }

        // If line is too long, split it
        if(content.length() > maxLineLength){
            // Since we draw from bottom to top, we need to get the substring of the end first
            uint16_t thisLength = content.length() % maxLineLength;
            if(thisLength == 0) thisLength = maxLineLength;

            content = content.substr(content.length() - thisLength, thisLength);
            lineContentRest = lineInfo.content.substr(0, lineInfo.content.length() - thisLength);
        }
        else{
            lineContentRest.clear();
        }

        // Render line
        SDL_Surface* textSurface = TTF_RenderText_Blended(consoleFont, content.c_str(), textColor);
        SDL_Texture* textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
        textOutputRect.x = 10;
        textOutputRect.y = line_y_position;
        textOutputRect.w = (double)textSurface->w / WindowScale;
        textOutputRect.h = (double)textSurface->h / WindowScale;

        SDL_RenderCopy(renderer, textTexture, NULL, &textOutputRect);
        SDL_FreeSurface(textSurface);
        SDL_DestroyTexture(textTexture);

        // Next line
        lineIndex++;
    }
}

void Console::renderConsole() {
    //------------------------------------------
    // Prerequisites

    // Ensure console texture is valid
    if(!ensureConsoleTexture()){
        Nebulite::Utility::Capture::cerr() << "SDL_CreateTexture failed: " << SDL_GetError() << Nebulite::Utility::Capture::endl;
        return;
    }

    // Calculate text alignment if needed
    static uint16_t maxLineLength = 10;
    static uint16_t lastTextureHeight = 0;
    static uint16_t lineHeight = 0;
    if(lastTextureHeight != consoleTexture.rect.h || flag_recalculateTextAlignment){
        lastTextureHeight = consoleTexture.rect.h;
        lineHeight = calculateTextAlignment(consoleTexture.rect.h);
        flag_recalculateTextAlignment = false;

        // Experimentally derive max line length based on console width and font size
        // by creating increasing length strings until they exceed the width
        maxLineLength = 0;
        std::string testString;
        while(maxLineLength < 256){
            // Use 'W' as it's typically the widest character, even though we use a monospaced font
            // This is a nice fallback in case we ever use a non-monospaced font
            testString += "W";
            SDL_Surface* testSurface = TTF_RenderText_Blended(consoleFont, testString.c_str(), color.coutStream);
            if((double)testSurface->w / WindowScale > consoleTexture.rect.w - 20){ // 20 for padding
                SDL_FreeSurface(testSurface);
                break;
            }
            SDL_FreeSurface(testSurface);
            maxLineLength++;
        }
    }

    //------------------------------------------
    // Rendering
    SDL_SetRenderTarget(renderer, consoleTexture.texture_ptr);
    drawBackground();
    drawInput(lineHeight);
    drawOutput(maxLineLength);
    SDL_SetRenderTarget(renderer, nullptr);
}

void Console::init(){
    //--------------------------------------------------
    // References
    renderer = domain->getSdlRenderer();
    globalDoc = domain->getDoc();

    // Use a monospaced font for better alignment
    consoleFont = TTF_OpenFont("Resources/Fonts/JetBrainsMono-Medium.ttf", FONT_MAX_SIZE * global->getRenderer()->getWindowScale());
    if(!consoleFont){
        Nebulite::Utility::Capture::cerr() << "TTF_OpenFont failed: " << TTF_GetError() << Nebulite::Utility::Capture::endl;
        return;
    }

    //--------------------------------------------------
    // Console buffer

    // Initialize history with a welcome message
    textInput.insertLine("Welcome to Nebulite!");
    textInput.insertLine("Type 'help' for a list of commands.");
    textInput.insertLine("Console started at: " + Nebulite::Utility::Time::TimeIso8601(Nebulite::Utility::Time::ISO8601FORMATTER::YYYY_MM_DD_HH_MM_SS, true));

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
    line_y_positions.clear();
    for(int i = 1; i < N; i++){ // i=0 is reserved for input line
        line_y_positions.push_back( rect_height - LINE_PADDING - 2*LINE_HEIGHT - i*(LINE_HEIGHT + LINE_PADDING) );
    }

    // Set correct font size for SDL_ttf
    WindowScale = global->getRenderer()->getWindowScale();
    TTF_SetFontSize(consoleFont, LINE_HEIGHT * WindowScale);

    return LINE_HEIGHT;
}

//--------------------------------------------------
// Event processing

void Console::keyTriggerSubmit(){
    std::string command;
    command = textInput.submit();
    if(!command.empty()){
        // Parse command on global level for full access to all functions
        global->parseStr(std::string(__FUNCTION__) + " " + command);
    }
    outputScrollingOffset = 0; // Reset scrolling to bottom on new input
}

void Console::keyTriggerScrollUp(){
    if(outputScrollingOffset < UINT16_MAX - 1){
        outputScrollingOffset += 1;
    }
}

void Console::keyTriggerScrollDown(){
    if(outputScrollingOffset > 0){
        outputScrollingOffset -= 1;
    }
}

void Console::keyTriggerZoomIn(const SDL_KeyboardEvent& key){
    // Make sure that ctrl is held
    if(!(key.keysym.mod & KMOD_CTRL)) return;
    domain->parseStr(__FUNCTION__ + std::string(" ") + Nebulite::DomainModule::Renderer::Console::consoleZoom_name + " in");
}

void Console::keyTriggerZoomOut(const SDL_KeyboardEvent& key){
    // Make sure that ctrl is held
    if(!(key.keysym.mod & KMOD_CTRL)) return;
    domain->parseStr(__FUNCTION__ + std::string(" ") + Nebulite::DomainModule::Renderer::Console::consoleZoom_name + " out");
}

void Console::processKeyDownEvent(const SDL_KeyboardEvent& key){
    switch (key.keysym.sym) {
        //------------------------------------------
        // Text input manipulation

        // Remove last character on backspace
        case SDLK_BACKSPACE:
            textInput.backspace();
            break;

        // Submit command on Enter
        case SDLK_RETURN:
        case SDLK_KP_ENTER:
            keyTriggerSubmit();
            break;	

        // Cursor movement
        case SDLK_LEFT:
            textInput.moveCursorLeft();
            break;
        case SDLK_RIGHT:
            textInput.moveCursorRight();
            break;

        /**
         * @todo: Implement copy/paste functionality with CTRL + C/V
         * perhaps integrate a clipboard manager into Renderer class?
         * Then, it could be used for both the console and other TextInput instances.
         *
         * For this to work properly, we should consider text handling to be only registered by one instance at a time.
         * Perhaps we can move every keytrigger event to Renderer, and have it forward events to the active TextInput instance.
         * 
         * Then we may have functions to move focus to objects:
         * 
         * for Domain RenderObject:
         * textfocus self
         * 
         * which itself sends a "textfocus id" to the "bus" that triggers on the scope of the Renderer by using:
         * 
         * textfocus id <renderer_object_id>
         * 
         * Then, we could have more textfocus additions like:
         * textfocus gui <gui_element_id>
         * textfocus console force on/off
         * etc.
         *
         * Then, we could have Renderer::setActiveTextInput(shared<TextInput>) which overwrites the current active text input, if its not forced.
         * And on Renderer::update(), we forward all text input and key events to that active TextInput instance.
         * Meaning this entire function would be moved to TextInput class, and Console would just set itself as active when in console mode.
         *
         * If a textfocus is active, we may wish to disable normal key processing from Input DomainModule.
         * 
         * For this to work, we need to modify TextInput into two modes:
         * - On submit, store text in vector (for console)
         * - On submit, lose focus           (for single-line text inputs)
         * 
         * This requires a big rework of TextInput class, we may wish to extract the core functionality of single-line text input with cursor to a separate class.
         * - TextInput for core functionality
         * - Use Capture class directly for console readout
         * Since the current TextInput class has a lot logic that is the same as Capture's output log.
         * Only thing left is to add a Capture type for commands entered.
         */

        //------------------------------------------
        // UP/DOWN to cycle through past commands
        case SDLK_UP:
            textInput.history_up();
            break;
        case SDLK_DOWN:
            textInput.history_down();
            break;

        //------------------------------------------
        // Scroll through output with PAGE UP/DOWN
        case SDLK_PAGEUP:
            keyTriggerScrollUp();
            break;
        case SDLK_PAGEDOWN:
            keyTriggerScrollDown();
            break;

        //------------------------------------------
        // Zoom in/out with +/- keys
        case SDLK_PLUS:
        case SDLK_KP_PLUS:
            keyTriggerZoomIn(key);
            break;

        case SDLK_MINUS:
        case SDLK_KP_MINUS:
            keyTriggerZoomOut(key);
            break;
    }
}

void Console::processEvents(){
    for (auto const& event : *events) {
        switch (event.type) {
            case SDL_TEXTINPUT:
                // Do not append if ctrl is held (to allow copy/paste and other shortcuts)
                if (event.key.keysym.mod & KMOD_CTRL) {
                    break;
                }
                textInput.append(event.text.text);
                break;

            case SDL_KEYDOWN:
                processKeyDownEvent(event.key);
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
            Nebulite::Utility::Capture::cerr() << "Could not attach Console: Console texture is null!" << Nebulite::Utility::Capture::endl;
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

std::string const Console::console_name = "console";
std::string const Console::console_desc = R"(Console commands and settings.
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
std::string const Console::consoleZoom_name = "console zoom";
std::string const Console::consoleZoom_desc = R"(Reduces or increases the console font size.

Usage: zoom [in/out/+/-]
- in / + : Zooms in (increases font size)
- out / - : Zooms out (decreases font size)
)";

Nebulite::Constants::Error Console::consoleSetBackground(int argc,  char* argv[]){
    //------------------------------------------
    // Prerequisites

    // Validate arguments
    if(argc < 2){
        return Nebulite::Constants::ErrorTable::FUNCTIONAL::TOO_FEW_ARGS();
    }
    if(argc > 2){
        return Nebulite::Constants::ErrorTable::FUNCTIONAL::TOO_MANY_ARGS();
    }

    //------------------------------------------
    // Delete previous background if any
    if(backgroundImageTexture != nullptr){
        SDL_DestroyTexture(backgroundImageTexture);
        backgroundImageTexture = nullptr;
    }

    //------------------------------------------
    // Load image

    std::string imagePath = argv[1];
    SDL_Surface* imageSurface = SDL_LoadBMP(imagePath.c_str());
    if(!imageSurface){
        // Try to load as PNG/JPG using SDL_image
        imageSurface = IMG_Load(imagePath.c_str());
        if(!imageSurface){
            return Nebulite::Constants::ErrorTable::FILE::CRITICAL_INVALID_FILE();
        }
    }

    // Create texture from surface
    SDL_Texture* backgroundTexture = SDL_CreateTextureFromSurface(renderer, imageSurface);
    SDL_FreeSurface(imageSurface);
    if(!backgroundTexture){
        return Nebulite::Constants::ErrorTable::TEXTURE::CRITICAL_TEXTURE_INVALID();
    }

    // Set as console background
    backgroundImageTexture = backgroundTexture;

    //------------------------------------------
    // Return
    return Nebulite::Constants::ErrorTable::NONE();
}
std::string const Console::consoleSetBackground_name = "console set-background";
std::string const Console::consoleSetBackground_desc = R"(Sets a background image for the console.

Usage: set-background <image_path>
)";

}   // namespace Nebulite::DomainModule::GlobalSpace::Console
