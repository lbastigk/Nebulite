#include "DomainModule/GlobalSpace/GDM_Console.hpp"
#include "Core/GlobalSpace.hpp"

namespace Nebulite::DomainModule::GlobalSpace {

void Console::update(){
    //------------------------------------------
    // Prerequisites

    // Requites renderer
    if(!domain->RendererExists()){
        return; // No renderer, no console
    }

    // Initialize font if not done yet
    if(!initialized){
        init();
    }

    //------------------------------------------
    // Toggle
    consoleMode = domain->getRenderer()->isConsoleMode();
    events = domain->getRenderer()->getEventHandles();

    // Toggling console mode
    bool consoleToggle = domain->getDoc()->get<int>(toggleKey.c_str(),0) == 1;
    if(consoleToggle){
        domain->getRenderer()->toggleConsoleMode();
        if(domain->getRenderer()->isConsoleMode()){
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
        for (const auto& event : *events) {
            switch (event.type) {
                case SDL_TEXTINPUT:
                    *consoleInputBuffer += event.text.text;
                    break;

                case SDL_KEYDOWN:
                    switch (event.key.keysym.sym) {
                        //------------------------------------------
                        // Text input manipulation

                        // Remove last character on backspace
                        case SDLK_BACKSPACE:
                            TextInput::backspace(this);
                            break;

                        // Submit command on Enter
                        case SDLK_RETURN:
                        case SDLK_KP_ENTER:
                            TextInput::submit(this);
                            break;	

                        // Cursor movement
                        case SDLK_LEFT:
                            /**
                             * @todo Move cursor left
                             */
                            break;
                        case SDLK_RIGHT:
                            /**
                             * @todo Move cursor right
                             */
                            break;

                        //------------------------------------------
                        // UP/DOWN to cycle through past commands
                        case SDLK_UP:
                            TextInput::history_up(this);
                            break;
                        case SDLK_DOWN:
                            TextInput::history_down(this);
                            break;
                    }
                    break;
            }
        }
    }

    //------------------------------------------
    // Rendering
    if (consoleMode) {
        // Render texture and attach
        renderConsole();

        // Check if texture is valid
        if(!consoleTexture.texture_ptr){
            std::cerr << "Could not attach Console: Console texture is null!" << std::endl;
            return;
        }

        // Attach texture above UI layer
        (void)domain->getRenderer()->attachTextureAboveLayer(
            Nebulite::Core::Environment::Layer::UI,
            "console_overlay", 
            consoleTexture.texture_ptr,
            &consoleTexture.rect
        );
    }
    else{
        // Clear texture and detach
        (void)domain->getRenderer()->detachTextureAboveLayer(
            Nebulite::Core::Environment::Layer::UI,
            "console_overlay"
        );
        if(consoleTexture.texture_ptr){
            SDL_DestroyTexture(consoleTexture.texture_ptr);
            consoleTexture.texture_ptr = nullptr;
        }
    }
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
    int lineHeight = (double)TTF_FontHeight(consoleFont);

    //------------------------------------------
    // Part 2: Input Line
    if (!consoleInputBuffer->empty()) {
        SDL_Surface* textSurface = TTF_RenderText_Blended(consoleFont, consoleInputBuffer->c_str(), textColor);
        SDL_Texture* textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);

        SDL_Rect textRect;
        textRect.x = 10;
        textRect.y = consoleTexture.rect.h - lineHeight - 10;
        textRect.w = (double)textSurface->w;
        textRect.h = (double)textSurface->h;

        SDL_RenderCopy(renderer, textTexture, NULL, &textRect);
        SDL_FreeSurface(textSurface);
        SDL_DestroyTexture(textTexture);
    }

    //------------------------------------------
    // Part 3: Output Lines
    int maxLines = floor(consoleTexture.rect.h - 20 - lineHeight) / lineHeight;
    int startLine = std::max(0, (int)consoleOutput.size() - maxLines);
    for (int i = 0; i < maxLines && (startLine + i) < consoleOutput.size(); ++i) {
        const std::string& line = consoleOutput[startLine + i];
        SDL_Surface* textSurface = TTF_RenderText_Blended(consoleFont, line.c_str(), textColor);
        SDL_Texture* textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);

        SDL_Rect lineRect;
        lineRect.x = 10;
        lineRect.y = 10 + i * lineHeight;
        lineRect.w = (double)textSurface->w;
        lineRect.h = (double)textSurface->h;

        SDL_RenderCopy(renderer, textTexture, NULL, &lineRect);
        SDL_FreeSurface(textSurface);
        SDL_DestroyTexture(textTexture);
    }

    //------------------------------------------
    // Target: Back to window
    SDL_SetRenderTarget(renderer, nullptr);
}

void Console::init(){
    //--------------------------------------------------
    // References

    consoleFont = domain->getRenderer()->getStandardFont();
    renderer = domain->getSDLRenderer();
    invoke = domain->invoke.get();
    globalDoc = invoke->getGlobalPointer();

    //--------------------------------------------------
    // Console buffer

    // Initialize history with a welcome message
    // In reality, this is just done because the program segfaults 
    // if we try to access a history with less than 2 elements
    commandIndexZeroBuffer = "echo Welcome to Nebulite!";
    TextInput::submit(this,false);  // Add to history but do not execute
    commandIndexZeroBuffer = "echo Type 'help' for a list of commands.";
    TextInput::submit(this,false);  // Add to history but do not execute

    //--------------------------------------------------
    // Console now fully functional
    initialized = true;
}

//--------------------------------------------------
// TextInput methods

void Console::TextInput::submit(Console *console, bool execute){
    if (!console->consoleInputBuffer->empty()) {
        // History and output
        console->commandHistory.emplace_back(*console->consoleInputBuffer);
        console->consoleOutput.emplace_back("> " + *console->consoleInputBuffer);

        // Add to queue
        if(execute){
            console->invoke->getQueue()->emplace_back(*console->consoleInputBuffer);
            if(console->selectedCommandIndex != 0){
                // If we were browsing history, reset to latest input
                console->selectedCommandIndex = 0;
                console->consoleInputBuffer = &console->commandIndexZeroBuffer;
            }
        }

        // Like in typical consoles, we clear the output
        console->commandIndexZeroBuffer.clear();
    }
}

void Console::TextInput::backspace(Console *console){
    if (!console->consoleInputBuffer->empty()) {
        console->consoleInputBuffer->pop_back();
    }
}

void Console::TextInput::history_up(Console *console){
    console->selectedCommandIndex++;
    
    // Get command from history
    if(console->selectedCommandIndex > console->commandHistory.size()){
        console->selectedCommandIndex = console->commandHistory.size();
    }
    if(console->selectedCommandIndex > 0){
        console->consoleInputBuffer = &console->commandHistory[console->commandHistory.size() - console->selectedCommandIndex];
    }
}

void Console::TextInput::history_down(Console *console){
    if(console->selectedCommandIndex > 0) console->selectedCommandIndex--;
    
    // Get command from buffer or history
    if(console->selectedCommandIndex == 0){
        console->consoleInputBuffer = &console->commandIndexZeroBuffer;
    }
    else{
        console->consoleInputBuffer = &console->commandHistory[console->commandHistory.size() - console->selectedCommandIndex];
    }
}

}   // namespace Nebulite::DomainModule::GlobalSpace::Console
