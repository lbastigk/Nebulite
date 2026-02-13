//------------------------------------------
// Includes

// External
#include <SDL3_image/SDL_image.h>

// Nebulite
#include "Nebulite.hpp"
#include "Core/Renderer.hpp"
#include "DomainModule/Renderer/Console.hpp"
#include "DomainModule/Renderer/Input.hpp"

//------------------------------------------
namespace Nebulite::DomainModule::Renderer {

Constants::Error Console::update() {
    //------------------------------------------
    // Prerequisites

    // Initialize font if not done yet
    if (!initialized) {
        init();
    }

    //------------------------------------------
    // Execute autoType commands
    processAutotypeQueue();

    //------------------------------------------
    // Insert new lines from capture streams
    static size_t last_size = 0;
    size_t const current_size = Utility::Capture::instance().getOutputLog().size();
    if (current_size < last_size) {
        // Log was cleared, reset
        last_size = 0;
        flag_recalculateTextAlignment = true;
        textInput.getOutput().clear();
    }
    for (size_t i = last_size; i < current_size; i++) {
        // Split input line by newlines
        auto const& [lineContent, lineType] = Utility::Capture::instance().getOutputLog().at(i);
        auto const& lines = Utility::StringHandler::split(lineContent, '\n');

        // Insert into text input
        Utility::TextInput::LineEntry::LineType type;
        switch (lineType) {
        case Utility::OutputLine::Type::COUT:
            type = Utility::TextInput::LineEntry::LineType::COUT;
            break;
        case Utility::OutputLine::Type::CERR:
            type = Utility::TextInput::LineEntry::LineType::CERR;
            break;
        default:
            Error::println("Unknown OutputLine type encountered in ", std::string(__FUNCTION__), ". Please fix!");
            type = Utility::TextInput::LineEntry::LineType::CERR;
            break;
        }
        for (auto const& line : lines) {
            if (!line.empty()) {
                textInput.insertLine(line, type);
            }
        }
    }
    last_size = current_size;

    //------------------------------------------
    // Toggle
    events = domain.getEventHandles();

    // Toggling console mode
    static auto const toggleKey = Input::Key::keyboardDelta + "tab";
    if (moduleScope.get<int>(toggleKey, 0) == 1) {
        consoleMode = !consoleMode;
        if (consoleMode) {
            SDL_StartTextInput(domain.getSdlWindow());
            SDL_FlushEvents(SDL_EVENT_FIRST, SDL_EVENT_LAST); // Flush all pending events
        } else {
            SDL_StopTextInput(domain.getSdlWindow());
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
    return Constants::ErrorTable::NONE();
}

bool Console::ensureConsoleTexture() {
    //------------------------------------------
    // Prerequisites

    // Derive logical consoleRect size from display size
    double const consoleHeight = static_cast<double>(moduleScope.get<size_t>(Constants::KeyNames::Renderer::dispResY, 360)) * consoleLayout.heightRatio;
    static SDL_Rect currentConsolePosition;
    currentConsolePosition.x = 0;
    currentConsolePosition.y = static_cast<int>(moduleScope.get<double>(Constants::KeyNames::Renderer::dispResY, 360) - consoleHeight);
    currentConsolePosition.w = static_cast<int>(moduleScope.get<double>(Constants::KeyNames::Renderer::dispResX, 360));
    currentConsolePosition.h = static_cast<int>(moduleScope.get<double>(Constants::KeyNames::Renderer::dispResY, 360) - static_cast<double>(currentConsolePosition.y));

    //------------------------------------------
    // Texture Setup

    bool recreateTexture = !consoleTexture.texture_ptr; // No texture yet
    recreateTexture = recreateTexture || consoleTexture.rect.w != currentConsolePosition.w; // Width changed (logical)
    recreateTexture = recreateTexture || consoleTexture.rect.h != currentConsolePosition.h; // Height changed (logical)

    // Update rectangle and recreate texture if needed
    if (recreateTexture) {
        if (consoleTexture.texture_ptr) {
            SDL_DestroyTexture(consoleTexture.texture_ptr);
            consoleTexture.texture_ptr = nullptr;
        }

        // create texture wrapper rect (logical rect)
        consoleTexture.rect = currentConsolePosition;

        consoleTexture.texture_ptr = SDL_CreateTexture(
            renderer,
            SDL_PIXELFORMAT_RGBA8888,
            SDL_TEXTUREACCESS_TARGET,
            consoleTexture.rect.w,
            consoleTexture.rect.h
        );
        if (consoleTexture.texture_ptr) {
            SDL_SetTextureScaleMode(consoleTexture.texture_ptr, SDL_SCALEMODE_NEAREST);
        }
    }

    // Validate texture
    return consoleTexture.texture_ptr != nullptr;
}

void Console::drawBackground() const {
    //------------------------------------------
    // Draw everything as before, but coordinates relative to (0,0)
    // Use physical size when rendering into the physical-size render target
    const float phys_w = static_cast<float>(consoleTexture.rect.w);
    const float phys_h = static_cast<float>(consoleTexture.rect.h);
    SDL_FRect const localFRect = {
        0.0f,
        0.0f,
        phys_w,
        phys_h
    };

    // If we have a background image, draw it instead
    if (backgroundImageTexture != nullptr) {
        SDL_RenderTexture(renderer, backgroundImageTexture, nullptr, &localFRect);
    } else {
        SDL_SetRenderDrawColor(renderer, color.background.r, color.background.g, color.background.b, color.background.a);
        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
        SDL_RenderFillRect(renderer, &localFRect);
    }
}

void Console::drawInput(uint16_t const& lineHeight) {
    // Add a darker background for the input line (use physical coordinates)
    double const posY = static_cast<double>(consoleTexture.rect.h) - lineHeight - 1.5 * consoleLayout.paddingRatio * lineHeight;
    SDL_FRect const inputBackgroundFRect = {
        0.0f,
        static_cast<float>(posY),
        static_cast<float>(consoleTexture.rect.w),
        static_cast<float>(lineHeight + consoleLayout.paddingRatio * lineHeight)
    };
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 150);
    SDL_RenderFillRect(renderer, &inputBackgroundFRect);

    // Render input text
    if (!textInput.getInputBuffer()->empty()) {
        std::string const inputText = *textInput.getInputBuffer();

        // Create surface and texture (surface already at scaled font size)
        SDL_Surface* textSurface = TTF_RenderText_Blended(consoleFont, inputText.c_str(), 0, color.input);
        SDL_Texture* textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
        if (textTexture) {
            SDL_SetTextureScaleMode(textTexture, SDL_SCALEMODE_NEAREST);
        }

        // Define destination rectangle in physical pixels
        textInputRect.x = 10;
        textInputRect.y = static_cast<int>(consoleTexture.rect.h - consoleLayout.paddingRatio * lineHeight - lineHeight);
        textInputRect.w = textSurface->w;
        textInputRect.h = textSurface->h;

        SDL_FRect const textInputFRect = {
            static_cast<float>(textInputRect.x),
            static_cast<float>(textInputRect.y),
            static_cast<float>(textInputRect.w),
            static_cast<float>(textInputRect.h)
        };
        SDL_RenderTexture(renderer, textTexture, nullptr, &textInputFRect);
        SDL_DestroySurface(textSurface);
        SDL_DestroyTexture(textTexture);

        // Highlight to the right of the cursor (also in physical pixels)
        if (uint16_t const cursorOffsetFromEnd = textInput.getCursorOffset(); cursorOffsetFromEnd > 0) {
            std::string const highlightText = textInput.getInputBuffer()->substr(textInput.getInputBuffer()->size() - cursorOffsetFromEnd, cursorOffsetFromEnd);
            SDL_Surface* highlightSurface = TTF_RenderText_Blended(consoleFont, highlightText.c_str(), 0, color.highlight);
            SDL_Texture* highlightTexture = SDL_CreateTextureFromSurface(renderer, highlightSurface);
            if (highlightTexture) {
                SDL_SetTextureScaleMode(highlightTexture, SDL_SCALEMODE_NEAREST);
            }

            textInputHighlightRect.x = textInputRect.x + textInputRect.w - highlightSurface->w;
            textInputHighlightRect.y = textInputRect.y;
            textInputHighlightRect.w = highlightSurface->w;
            textInputHighlightRect.h = highlightSurface->h;
            SDL_FRect const TextInputHighlightFRect = {
                static_cast<float>(textInputHighlightRect.x),
                static_cast<float>(textInputHighlightRect.y),
                static_cast<float>(textInputHighlightRect.w),
                static_cast<float>(textInputHighlightRect.h)
            };
            SDL_RenderTexture(renderer, highlightTexture, nullptr, &TextInputHighlightFRect);
            SDL_DestroySurface(highlightSurface);
            SDL_DestroyTexture(highlightTexture);
        }
    }
}

void Console::drawOutput(uint16_t const& maxLineLength) {
    int32_t lineIndex = 0;
    std::string lineContentRest; // Rest of line after linebreak
    auto const outputSize = static_cast<int32_t>(textInput.getOutput().size());

    // Since we start from the bottom, we need to invert the scrolling offset
    // is increased in-loop due to linebreaks
    int32_t lineIndexOffset = -outputScrollingOffset;

    // Index-offset: If we have less history than lines,
    // We need to offset to align at the top
    int32_t y_start = line_y_positions[0];
    if (auto const availableLines = static_cast<int32_t>(line_y_positions.size()); outputSize < availableLines) {
        if (outputSize == 0) {
            y_start = line_y_positions[0];
        }
        else {
            y_start = line_y_positions[static_cast<size_t>(availableLines - outputSize)];
        }
    }

    // Render lines from bottom to top
    for (int32_t const& line_y_position : line_y_positions) {
        if (line_y_position > y_start)
            continue; // Skip lines under the start position
        if (lineIndex >= outputSize)
            break; // No more lines to show

        // Constrain outputScrollingOffset by investigating the current index
        int32_t const currentIndex = outputSize - 1 - lineIndex + lineIndexOffset;
        if (currentIndex < 0) {
            // Cour outputScrollingOffset is too high, we reached the top
            outputScrollingOffset--;
            break;
        }
        if (currentIndex >= outputSize) {
            // Current outputScrollingOffset is too low, we reached the bottom
            outputScrollingOffset++;
            break;
        }

        // Get line info
        auto lineInfo = textInput.getOutput().at(static_cast<size_t>(currentIndex));
        SDL_Color textColor = color.input;
        std::string content;

        if (!lineContentRest.empty()) {
            // We have a rest from the previous linebreak
            content = lineContentRest;
            lineContentRest.clear();
            lineIndexOffset++; // Increase offset since we are using an extra line
        } else {
            // New line
            switch (lineInfo.type) {
            case Utility::TextInput::LineEntry::LineType::CERR:
                textColor = color.cerrStream;
                content = lineInfo.content;
                break;
            case Utility::TextInput::LineEntry::LineType::COUT:
                textColor = color.coutStream;
                content = lineInfo.content;
                break;
            case Utility::TextInput::LineEntry::LineType::INPUT:
                textColor = color.input;
                content = "> " + lineInfo.content;
                break;
            default:
                std::unreachable();
            }
        }

        // If line is too long, split it
        if (content.length() > maxLineLength) {
            // Since we draw from bottom to top, we need to get the substring of the end first
            uint16_t thisLength = content.length() % maxLineLength;
            if (thisLength == 0)
                thisLength = maxLineLength;

            content = content.substr(content.length() - thisLength, thisLength);
            lineContentRest = lineInfo.content.substr(0, lineInfo.content.length() - thisLength);
        } else {
            lineContentRest.clear();
        }

        // Render line (surfaces are created at the scaled font size; use physical coordinates)
        SDL_Surface* textSurface = TTF_RenderText_Blended(consoleFont, content.c_str(), 0, textColor);
        SDL_Texture* textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
        if (textTexture) {
            SDL_SetTextureScaleMode(textTexture, SDL_SCALEMODE_NEAREST);
        }

        textOutputRect.x = 10;
        textOutputRect.y = static_cast<int>(line_y_position);
        textOutputRect.w = textSurface->w;
        textOutputRect.h = textSurface->h;
        SDL_FRect const textOutputFRect = {
            static_cast<float>(textOutputRect.x),
            static_cast<float>(textOutputRect.y),
            static_cast<float>(textOutputRect.w),
            static_cast<float>(textOutputRect.h)
        };
        SDL_RenderTexture(renderer, textTexture, nullptr, &textOutputFRect);
        SDL_DestroySurface(textSurface);
        SDL_DestroyTexture(textTexture);

        // Next line
        lineIndex++;
    }
}

void Console::renderConsole() {
    //------------------------------------------
    // Prerequisites

    // Ensure console texture is valid
    if (!ensureConsoleTexture()) {
        Error::println("SDL_CreateTexture failed: ", SDL_GetError());
        return;
    }

    // Calculate text alignment if needed
    static uint16_t maxLineLength = 10;
    static uint16_t lastTextureHeight = 0;
    static uint16_t lineHeight = 0;
    if (lastTextureHeight != consoleTexture.rect.h || flag_recalculateTextAlignment) {
        lastTextureHeight = static_cast<uint16_t>(consoleTexture.rect.h);
        lineHeight = calculateTextAlignment(static_cast<uint16_t>(consoleTexture.rect.h));
        flag_recalculateTextAlignment = false;

        // Experimentally derive max line length based on console width and font size
        // by creating increasing length strings until they exceed the width
        maxLineLength = 0;
        std::string testString;
        while (maxLineLength < 256) {
            // Use 'W' as it's typically the widest character, even though we use a monospaced font
            // This is a nice fallback in case we ever use a non-monospaced font
            testString += "W";
            SDL_Surface* testSurface = TTF_RenderText_Blended(consoleFont, testString.c_str(), 0, color.coutStream);
            if (static_cast<double>(testSurface->w) > consoleTexture.rect.w - 20) {
                // 20 for padding
                SDL_DestroySurface(testSurface);
                break;
            }
            SDL_DestroySurface(testSurface);
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

    // Present the console render-target to the main renderer:
    if (consoleTexture.texture_ptr) {
        // Source: physical pixels stored in the texture
        const float phys_w = static_cast<float>(consoleTexture.rect.w);
        const float phys_h = static_cast<float>(consoleTexture.rect.h);
        SDL_FRect const srcF = { 0.0f, 0.0f, phys_w, phys_h };

        // Destination: logical rectangle (keeps correct on-screen size)
        SDL_FRect const dstF = {
            static_cast<float>(consoleTexture.rect.x),
            static_cast<float>(consoleTexture.rect.y),
            static_cast<float>(consoleTexture.rect.w),
            static_cast<float>(consoleTexture.rect.h)
        };

        SDL_RenderTexture(renderer, consoleTexture.texture_ptr, &srcF, &dstF);
    }
}

void Console::init() {
    //--------------------------------------------------
    // References
    renderer = domain.getSdlRenderer();

    // Use a monospaced font for better alignment
    consoleFont = TTF_OpenFont(consoleFontPath.c_str(), consoleLayout.FONT_MAX_SIZE);
    if (!consoleFont) {
        Error::println("TTF_OpenFont failed for font: ", consoleFontPath);
        return;
    }

    //--------------------------------------------------
    // Console buffer

    // Initialize history with a welcome message
    textInput.insertLine("Welcome to Nebulite!");
    textInput.insertLine("Type 'help' for a list of commands.");
    textInput.insertLine("Console started at: " + Utility::Time::TimeIso8601(Utility::Time::ISO8601Format::YYYY_MM_DD_HH_MM_SS, true));

    //--------------------------------------------------
    // Start autoType timer
    autoType.waitTimer.start();

    //--------------------------------------------------
    // Console now fully functional
    initialized = true;
}

// TODO: While the calculation is correct, it could use proper usage of floating point math so every +- operation makes a difference
uint16_t Console::calculateTextAlignment(uint16_t const& rect_height) {
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
    //             = (N+3)*LINE_HEIGHT*PADDING_RATIO + (N+1)*LINE_HEIGHT
    //             = LINE_HEIGHT * ( (N+3)*PADDING_RATIO + (N+1) )
    // LINE_HEIGHT = (rect_height - (N+3)*LINE_PADDING) / (N+1)
    //             = rect_height / ( (N+3)*PADDING_RATIO + (N+1) )

    // LINE_HEIGHT = rect_height / ( (N+3)*PADDING_RATIO + (N+1) )
    // LINE_HEIGHT * ( (N+3)*PADDING_RATIO + (N+1) ) = rect_height
    // LINE_HEIGHT * (N+3)*PADDING_RATIO + LINE_HEIGHT * (N+1) = rect_height
    // LINE_HEIGHT * N * PADDING_RATIO + 3*LINE_HEIGHT*PADDING_RATIO + LINE_HEIGHT * N + LINE_HEIGHT = rect_height
    // N * (LINE_HEIGHT * PADDING_RATIO + LINE_HEIGHT) +  3*LINE_HEIGHT*PADDING_RATIO + LINE_HEIGHT = rect_height
    // N * (LINE_HEIGHT * (PADDING_RATIO + 1)) = rect_height - 3*LINE_HEIGHT*PADDING_RATIO - LINE_HEIGHT

    // N           = (rect_height - 3*LINE_PADDING) / (LINE_HEIGHT + LINE_PADDING)
    //             = (rect_height - 3*LINE_HEIGHT*PADDING_RATIO - LINE_HEIGHT) / (LINE_HEIGHT * (PADDING_RATIO + 1))

    // Constraints:
    // LINE_HEIGHT <= FONT_MAX_SIZE
    // MINIMUM_LINES <= N
    auto LINE_HEIGHT = consoleLayout.FONT_MAX_SIZE;
    auto const PADDING_RATIO = consoleLayout.paddingRatio;

    // See where we land for N, the amount of lines, with the maximum font size
    auto N = static_cast<uint16_t>(
        std::floor(
            (rect_height - 3*LINE_HEIGHT*PADDING_RATIO - LINE_HEIGHT) / (LINE_HEIGHT * (PADDING_RATIO + 1))
        )
    );

    // Reduce line height if we have less than minimum lines
    if (N < consoleLayout.MINIMUM_LINES) {
        N = consoleLayout.MINIMUM_LINES;
        LINE_HEIGHT = static_cast<uint16_t>(
            std::floor(
                static_cast<float>(rect_height) / static_cast<float>( (N+3)*PADDING_RATIO + (N+1) )
            )
        );
    }

    // Now, line height and N are final
    // Populate y positions
    line_y_positions.clear();
    double const startPos = rect_height - LINE_HEIGHT * PADDING_RATIO - 2 * LINE_HEIGHT;
    double const diff = LINE_HEIGHT + LINE_HEIGHT * PADDING_RATIO;
    for (int i = 1; i < N; i++) {
        // i=0 is reserved for input line
        line_y_positions.push_back( static_cast<int32_t>(startPos - i * diff));
    }

    // Set correct font size for SDL_ttf
    TTF_SetFontSize(consoleFont, LINE_HEIGHT);
    return LINE_HEIGHT;
}

void Console::processAutotypeQueue() {
    autoType.waitTimer.update();
    auto const dt_ms = autoType.waitTimer.get_dt_ms();
    if (autoType.waitTimeRemaining > 0) {
        if (dt_ms >= autoType.waitTimeRemaining) {
            autoType.waitTimeRemaining = 0;
        } else {
            autoType.waitTimeRemaining -= dt_ms;
        }
    } else {
        while (!autoType.activeQueue.empty()) {
            if (autoType.waitTimeRemaining > 0) {
                break; // Wait time set by a command, pause execution of further commands until next update
            }
            switch (auto const& [type, text] = autoType.activeQueue.front(); type) {
            case AutoType::Command::Type::TEXT:
                textInput.getInputBuffer()->append(text);
                break;
            case AutoType::Command::Type::ENTER:
                keyTriggerSubmit();
                break;
            case AutoType::Command::Type::CLOSE:
                consoleMode = false;
                SDL_StopTextInput(domain.getSdlWindow());
                break;
            case AutoType::Command::Type::WAIT:
                try {
                    autoType.waitTimeRemaining = std::stoul(text);
                } catch (std::exception const&) {
                    Error::println("Invalid wait time in autoType command: ", text);
                }
                break;
            case AutoType::Command::Type::HISTORY_UP:
                textInput.history_up();
                break;
            case AutoType::Command::Type::HISTORY_DOWN:
                textInput.history_down();
                break;
            default:
                std::unreachable();
            }
            autoType.activeQueue.pop();
        }
    }
}

} // namespace Nebulite::DomainModule::GlobalSpace::Console
