//------------------------------------------
// Includes

// Nebulite
#include "Nebulite.hpp"
#include "Core/RenderObject.hpp"
#include "Data/JSON.hpp"
#include "DomainModule/Initializer.hpp"
#include "DomainModule/JSON/SimpleData.hpp"
#include "Interaction/Ruleset.hpp"
#include "Interaction/RulesetCompiler.hpp"

//------------------------------------------
namespace Nebulite::Core {

//------------------------------------------
// Special member Functions

// Helper function to initialize RenderObject in constructor

RenderObject::RenderObject() : Domain("RenderObject", this, &document), baseTexture(&document) {
    //------------------------------------------
    // Document Values

    // General

    // Initialize to 0, Renderer itself sets proper id, which starts at 1
    document.set(Constants::keyName.renderObject.id, 0);
    document.set(Constants::keyName.renderObject.positionX, 0);
    document.set(Constants::keyName.renderObject.positionY, 0);
    document.set(Constants::keyName.renderObject.imageLocation, std::string("Resources/Sprites/TEST001P/001.bmp"));
    document.set(Constants::keyName.renderObject.layer, 0);

    //for sprite
    document.set(Constants::keyName.renderObject.isSpritesheet, false);
    document.set(Constants::keyName.renderObject.spritesheetOffsetX, 0);
    document.set(Constants::keyName.renderObject.spritesheetOffsetY, 0);
    document.set(Constants::keyName.renderObject.spritesheetSizeX, 0);
    document.set(Constants::keyName.renderObject.spritesheetSizeY, 0);
    document.set(Constants::keyName.renderObject.pixelSizeX, 32);
    document.set(Constants::keyName.renderObject.pixelSizeY, 32);

    // Invokes
    document.setEmptyArray(Constants::keyName.renderObject.invokes.c_str());
    document.setEmptyArray(Constants::keyName.renderObject.invokeSubscriptions.c_str());
    document.set(Constants::keyName.renderObject.invokeSubscriptions + "[0]", std::string("all"));

    // Text
    document.set(Constants::keyName.renderObject.textStr, std::string(""));
    document.set(Constants::keyName.renderObject.textFontsize, 0);
    document.set(Constants::keyName.renderObject.textDx, 0.0);
    document.set(Constants::keyName.renderObject.textDy, 0.0);
    document.set(Constants::keyName.renderObject.textColorR, 255);
    document.set(Constants::keyName.renderObject.textColorG, 255);
    document.set(Constants::keyName.renderObject.textColorB, 255);
    document.set(Constants::keyName.renderObject.textColorA, 255);

    //------------------------------------------
    // Internal Values
    textSurface = nullptr;
    textTexture = nullptr;

    //------------------------------------------
    // Flags
    flag.deleteFromScene = false;
    flag.calculateText = true; // In order to calculate text texture on first update
    flag.reloadInvokes = true; // In order to reload invokes on first update
    subscription_size = document.memberSize(Constants::keyName.renderObject.invokeSubscriptions);

    //------------------------------------------
    // Initialize Linkages, References and DomainModules and object itself
    init();
}

void RenderObject::init() {
    //------------------------------------------
    // Link inherited Domains
    inherit(&document);
    inherit(&baseTexture);

    //------------------------------------------
    // Link frequently used values
    linkFrequentRefs();

    //------------------------------------------
    // Initialize Domain Modules
    DomainModule::Initializer::initRenderObject(this);

    //------------------------------------------
    // Update once to initialize
    update();
    calculateSrcRect();
    calculateDstRect();
}

RenderObject::~RenderObject() {
    if (textSurface) {
        SDL_FreeSurface(textSurface);
        textSurface = nullptr;
    }

    if (textTexture) {
        SDL_DestroyTexture(textTexture);
        textTexture = nullptr;
    }

    // Clean up invoke entries - shared pointers will automatically handle cleanup
    rulesetsGlobal.clear();
    rulesetsLocal.clear();
}

//------------------------------------------
// Marshalling

std::string RenderObject::serialize() {
    return document.serialize();
}

void RenderObject::deserialize(std::string const& serialOrLink) {
    // Check if argv1 provided is an object
    if (Data::JSON::isJsonOrJsonc(serialOrLink)) {
        document.deserialize(serialOrLink);
    } else {
        //------------------------------------------
        // Split the input into tokens
        std::vector<std::string> tokens = Utility::StringHandler::split(serialOrLink, '|');

        //------------------------------------------
        // Validity check
        if (tokens.empty()) {
            return; // or handle error properly
        }

        //------------------------------------------
        // Load the JSON file
        // First token is the path or serialized JSON
        document.deserialize(tokens[0]);

        //------------------------------------------
        // Now apply modifications
        tokens.erase(tokens.begin()); // Remove the first token (path or serialized JSON)
        for (auto const& token : tokens) {
            if (token.empty())
                continue; // Skip empty tokens

            // Legacy: Handle key=value pairs
            if (token.find('=') != std::string::npos) {
                // Handle modifier (key=value)
                auto const pos = token.find('=');
                std::string key = token.substr(0, pos);
                std::string value = token.substr(pos + 1);
                std::string call = __FUNCTION__;
                call.append(" " + DomainModule::JSON::SimpleData::set_name);
                call.append(" " + key);
                call.append(" " + value);
                if (auto const err = parseStr(call); err != Constants::ErrorTable::NONE()) {
                    Nebulite::cerr() << "Warning: Failed to parse modifier '" << token << "' during RenderObject deserialization." << Nebulite::endl;
                    Nebulite::cerr() << "Error Code: " << err.getDescription() << Nebulite::endl;
                }
            }
            // Handle function call
            else {
                // Forward to FunctionTree for resolution
                if (auto const err = parseStr(__FUNCTION__ + std::string(" ") + token); err != Constants::ErrorTable::NONE()) {
                    Nebulite::cerr() << "Warning: Failed to parse function call '" << token << "' during RenderObject deserialization." << Nebulite::endl;
                    Nebulite::cerr() << "Error Code: " << err.getDescription() << Nebulite::endl;
                }
            }
        }
    }

    // Re-Establish frequent references
    linkFrequentRefs();

    // Prerequisites
    flag.reloadInvokes = true;
    flag.calculateText = true;

    // Update subscription size
    subscription_size = document.memberSize(Constants::keyName.renderObject.invokeSubscriptions);
}

//------------------------------------------
// General functions

SDL_Texture* RenderObject::getTextTexture() const {
    return textTexture;
}

SDL_Rect* RenderObject::getTextRect() {
    return &textRect;
}

SDL_Rect* RenderObject::getDstRect() {
    return &dstRect;
}

void RenderObject::calculateDstRect() {
    dstRect = {
        static_cast<int>(floor(*refs.posX)),
        static_cast<int>(floor(*refs.posY)),
        static_cast<int>(floor(*refs.pixelSizeX)), // Set the desired width
        static_cast<int>(floor(*refs.pixelSizeY)), // Set the desired height
    };
}

SDL_Rect* RenderObject::getSrcRect() {
    if (*refs.isSpritesheet > DBL_EPSILON) {
        // isSpritesheet is true
        return &srcRect;
    }
    return nullptr;
}

void RenderObject::calculateSrcRect() {
    // Check if the object is a sprite
    if (*refs.isSpritesheet > DBL_EPSILON) {
        // isSpritesheet is true
        // Calculate the source rectangle for the sprite (which portion of the sprite sheet to render)
        srcRect = {
            static_cast<int>(*refs.spritesheetOffsetX), // Start X from the sprite sheet offset
            static_cast<int>(*refs.spritesheetOffsetY), // Start Y from the sprite sheet offset
            static_cast<int>(*refs.spritesheetSizeX), // The width of the sprite frame
            static_cast<int>(*refs.spritesheetSizeY) // The height of the sprite frame
        };
    }
}

void RenderObject::linkFrequentRefs() {
    // Identity
    refs.id = document.getStableDoublePointer(Constants::keyName.renderObject.id);

    // Position and Size
    refs.posX = document.getStableDoublePointer(Constants::keyName.renderObject.positionX);
    refs.posY = document.getStableDoublePointer(Constants::keyName.renderObject.positionY);
    refs.pixelSizeX = document.getStableDoublePointer(Constants::keyName.renderObject.pixelSizeX);
    refs.pixelSizeY = document.getStableDoublePointer(Constants::keyName.renderObject.pixelSizeY);

    // Spritesheet
    refs.isSpritesheet = document.getStableDoublePointer(Constants::keyName.renderObject.isSpritesheet);
    refs.spritesheetOffsetX = document.getStableDoublePointer(Constants::keyName.renderObject.spritesheetOffsetX);
    refs.spritesheetOffsetY = document.getStableDoublePointer(Constants::keyName.renderObject.spritesheetOffsetY);
    refs.spritesheetSizeX = document.getStableDoublePointer(Constants::keyName.renderObject.spritesheetSizeX);
    refs.spritesheetSizeY = document.getStableDoublePointer(Constants::keyName.renderObject.spritesheetSizeY);

    // Text
    refs.fontSize = document.getStableDoublePointer(Constants::keyName.renderObject.textFontsize);
    refs.textDx = document.getStableDoublePointer(Constants::keyName.renderObject.textDx);
    refs.textDy = document.getStableDoublePointer(Constants::keyName.renderObject.textDy);
    refs.textColorR = document.getStableDoublePointer(Constants::keyName.renderObject.textColorR);
    refs.textColorG = document.getStableDoublePointer(Constants::keyName.renderObject.textColorG);
    refs.textColorB = document.getStableDoublePointer(Constants::keyName.renderObject.textColorB);
    refs.textColorA = document.getStableDoublePointer(Constants::keyName.renderObject.textColorA);
}

//------------------------------------------
// Outside communication with invoke for updating and estimation

Constants::Error RenderObject::update() {
    //------------------------------------------
    // Update modules and all inner domains
    updateModules();
    document.update();
    baseTexture.update();

    //------------------------------------------
    // Check all invokes
    if (Nebulite::global().getInvoke() != nullptr) {
        //------------------------------------------
        // 1.) Reload invokes if needed
        if (flag.reloadInvokes) {
            Interaction::RulesetCompiler::parse(rulesetsGlobal, rulesetsLocal, this);
            flag.reloadInvokes = false;
        }

        //------------------------------------------
        // 2.) Directly solve local invokes (loop)
        for (auto const& entry : rulesetsLocal) {
            if (Interaction::Invoke::checkRulesetLogicalCondition(entry->logicalArg, entry->selfPtr)) {
                Nebulite::global().getInvoke()->applyRuleset(entry);
            }
        }

        //------------------------------------------
        // 3.) Checks this object against all conventional invokes
        //	   Manipulation happens at the Invoke::update routine later on
        //     This just generates pairs that need to be updated
        for (size_t idx = 0; idx < subscription_size; idx++) {
            std::string key = Constants::keyName.renderObject.invokeSubscriptions + "[" + std::to_string(idx) + "]";
            auto const subscription = document.get<std::string>(key, "");
            Nebulite::global().getInvoke()->listen(this, subscription, static_cast<uint32_t>(*refs.id));
        }

        //------------------------------------------
        // 4.) Append general invokes from object itself back for global check
        //     This makes sure that no invokes from inactive objects stay in the list
        for (auto const& entry : rulesetsGlobal) {
            // add pointer to invoke command to global
            Nebulite::global().getInvoke()->broadcast(entry);
        }
    } else {
        return Constants::ErrorTable::RENDERER::CRITICAL_INVOKE_NULLPTR();
    }
    // No evaluation of domainModules for now, just return NONE
    return Constants::ErrorTable::NONE();
}

uint64_t RenderObject::estimateComputationalCost(bool const& onlyInternal) {
    //------------------------------------------
    // Reload invokes if needed
    if (flag.reloadInvokes) {
        Interaction::RulesetCompiler::parse(rulesetsGlobal, rulesetsLocal, this);
        flag.reloadInvokes = false;
    }

    //------------------------------------------
    // Count number of $ and { in logical Arguments
    uint64_t cost = 0;

    // Local entries
    cost = std::accumulate(
        rulesetsLocal.begin(), rulesetsLocal.end(), cost,
        [](uint64_t const acc, std::shared_ptr<Interaction::Ruleset> const& entry) {
            return acc + entry->estimatedCost;
        }
        );

    // Global entries
    if (!onlyInternal) {
        cost = std::accumulate(
            rulesetsGlobal.begin(), rulesetsGlobal.end(), cost,
            [](uint64_t const acc, std::shared_ptr<Interaction::Ruleset> const& entry) {
                return acc + entry->estimatedCost;
            }
            );
    }

    return cost;
}

//------------------------------------------
// Outside communication with Renderer for text calculation

void RenderObject::calculateText(SDL_Renderer* renderer, TTF_Font* font, int const& renderPositionX, int const& renderPositionY) {
    // Set font size if changed
    textRect.x = static_cast<int>(*refs.posX + *refs.textDx - static_cast<double>(renderPositionX));
    textRect.y = static_cast<int>(*refs.posY + *refs.textDy - static_cast<double>(renderPositionY));

    // Recreate texture if recalculate was triggered by user. This is needed for:
    // - new text
    // - new color
    // - new text size
    if (flag.calculateText) {
        // Free previous texture
        if (textTexture != nullptr) {
            SDL_DestroyTexture(textTexture);
            textTexture = nullptr;
        }

        // Settings influenced by a new text
        double constexpr scalar = 1.0; // Perhaps needed later on for scaling
        auto const text = document.get<std::string>(Constants::keyName.renderObject.textStr.c_str());
        textRect.w = static_cast<int>(*refs.fontSize * static_cast<double>(text.length()) * scalar);
        textRect.h = static_cast<int>(*refs.fontSize * 1.5 * scalar);

        // Create text
        SDL_Color const textColor = {
            static_cast<Uint8>(*refs.textColorR),
            static_cast<Uint8>(*refs.textColorG),
            static_cast<Uint8>(*refs.textColorB),
            static_cast<Uint8>(*refs.textColorA)
        };

        // Create texture
        if (!text.empty() && font && renderer) {
            textSurface = TTF_RenderText_Solid(font, text.c_str(), textColor);
            if (textSurface) {
                textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
                SDL_FreeSurface(textSurface); // Free surface after creating texture
                textSurface = nullptr;
            }
        }

        // Set flag back to false
        flag.calculateText = false;
    }
}

} // namespace Nebulite::Core