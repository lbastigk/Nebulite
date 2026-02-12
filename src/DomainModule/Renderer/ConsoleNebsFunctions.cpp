//------------------------------------------
// Includes

// External
#include <SDL3_image/SDL_image.h>

// Nebulite
#include "DomainModule/Renderer/Console.hpp"

//------------------------------------------
namespace Nebulite::DomainModule::Renderer {

Constants::Error Console::consoleOpen() {
    consoleMode = true;
    return Constants::ErrorTable::NONE();
}

Constants::Error Console::consoleClose() {
    consoleMode = false;
    return Constants::ErrorTable::NONE();
}

Constants::Error Console::consoleZoom(int const argc, char** argv) {
    //------------------------------------------
    // Prerequisites

    // Validate arguments
    if (argc > 2) {
        return Constants::ErrorTable::FUNCTIONAL::TOO_MANY_ARGS();
    }

    //------------------------------------------
    // Determine zoom direction

    if (argc == 2) {

        if (std::string const direction = argv[1]; direction == "in" || direction == "+") {
            if (consoleLayout.FONT_MAX_SIZE <= 48) {
                consoleLayout.FONT_MAX_SIZE++;
                flag_recalculateTextAlignment = true;
            }
        } else if (direction == "out" || direction == "-") {
            if (consoleLayout.FONT_MAX_SIZE >= 8) {
                consoleLayout.FONT_MAX_SIZE--;
                flag_recalculateTextAlignment = true;
            }
        } else {
            return Constants::ErrorTable::FUNCTIONAL::UNKNOWN_ARG();
        }
    }

    //------------------------------------------
    // Return
    return Constants::ErrorTable::NONE();
}

Constants::Error Console::consoleSetBackground(int const argc, char** argv) {
    //------------------------------------------
    // Prerequisites

    // Validate arguments
    if (argc < 2) {
        return Constants::ErrorTable::FUNCTIONAL::TOO_FEW_ARGS();
    }
    if (argc > 2) {
        return Constants::ErrorTable::FUNCTIONAL::TOO_MANY_ARGS();
    }

    //------------------------------------------
    // Delete previous background if any
    if (backgroundImageTexture != nullptr) {
        SDL_DestroyTexture(backgroundImageTexture);
        backgroundImageTexture = nullptr;
    }

    //------------------------------------------
    // Load image

    std::string const imagePath = argv[1];
    SDL_Surface* imageSurface = SDL_LoadBMP(imagePath.c_str());
    if (!imageSurface) {
        // Try to load as PNG/JPG using SDL_image
        imageSurface = IMG_Load(imagePath.c_str());
        if (!imageSurface) {
            return Constants::ErrorTable::FILE::CRITICAL_INVALID_FILE();
        }
    }

    // Create texture from surface
    SDL_Texture* backgroundTexture = SDL_CreateTextureFromSurface(renderer, imageSurface);
    SDL_DestroySurface(imageSurface);
    if (!backgroundTexture) {
        return Constants::ErrorTable::TEXTURE::CRITICAL_TEXTURE_INVALID();
    }
    SDL_SetTextureScaleMode(backgroundTexture, SDL_SCALEMODE_NEAREST);

    // Set as console background
    backgroundImageTexture = backgroundTexture;

    //------------------------------------------
    // Return
    return Constants::ErrorTable::NONE();
}

//------------------------------------------
// Autotype functions

Constants::Error Console::consoleAutotypeText(std::span<std::string const> const& args) {
    if (args.size() < 2) {
        return Constants::ErrorTable::FUNCTIONAL::TOO_FEW_ARGS();
    }
    autotypeQueue.push(AutotypeCommand{AutotypeCommand::Type::TEXT, Utility::StringHandler::recombineArgs(args.subspan(1))});
    return Constants::ErrorTable::NONE();
}

Constants::Error Console::consoleAutotypeEnter() {
    autotypeQueue.push({AutotypeCommand::Type::ENTER, ""});
    return Constants::ErrorTable::NONE();
}

Constants::Error Console::consoleAutotypeExecute() {
    // add queue to active queue
    while (!autotypeQueue.empty()) {
        autotypeActiveQueue.push(autotypeQueue.front());
        autotypeQueue.pop();
    }
    return Constants::ErrorTable::NONE();
}

Constants::Error Console::consoleAutotypeWait(std::span<std::string const> const& args) {
    if (args.size() > 2) {
        return Constants::ErrorTable::FUNCTIONAL::TOO_MANY_ARGS();
    }
    if (args.size() < 2) {
        return Constants::ErrorTable::FUNCTIONAL::TOO_FEW_ARGS();
    }
    autotypeQueue.push({AutotypeCommand::Type::WAIT, args[1]});
    return Constants::ErrorTable::NONE();
}

Constants::Error Console::consoleAutotypeClose() {
    autotypeQueue.push({AutotypeCommand::Type::CLOSE, ""});
    return Constants::ErrorTable::NONE();
}

Constants::Error Console::consoleAutotypeHistoryUp() {
    autotypeQueue.push({AutotypeCommand::Type::HISTORY_UP, ""});
    return Constants::ErrorTable::NONE();
}

Constants::Error Console::consoleAutotypeHistoryDown() {
    autotypeQueue.push({AutotypeCommand::Type::HISTORY_DOWN, ""});
    return Constants::ErrorTable::NONE();
}

} // namespace Nebulite::DomainModule::Renderer
