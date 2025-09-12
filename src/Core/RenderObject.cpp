#include "Core/RenderObject.hpp"
#include "DomainModule/RDM.hpp"
#include "Interaction/Deserializer.hpp"

//------------------------------------------
// Special member Functions

Nebulite::Core::RenderObject::RenderObject(Nebulite::Utility::JSON* global) 
: global(global), Nebulite::Interaction::Execution::Domain<Nebulite::Core::RenderObject>("RenderObject", this, &json) {

	//------------------------------------------
	// Document Values

	// General
	json.set(Nebulite::Constants::keyName.renderObject.id.c_str(),0);
	json.set(Nebulite::Constants::keyName.renderObject.positionX.c_str(), 0);
	json.set(Nebulite::Constants::keyName.renderObject.positionY.c_str(), 0);
	json.set(Nebulite::Constants::keyName.renderObject.imageLocation.c_str(), std::string("Resources/Sprites/TEST001P/001.bmp"));
	json.set(Nebulite::Constants::keyName.renderObject.layer.c_str(), 0);

	//for sprite
	json.set(Nebulite::Constants::keyName.renderObject.isSpritesheet.c_str(), false);
	json.set(Nebulite::Constants::keyName.renderObject.spritesheetOffsetX.c_str(), 0);
	json.set(Nebulite::Constants::keyName.renderObject.spritesheetOffsetY.c_str(), 0);
	json.set(Nebulite::Constants::keyName.renderObject.spritesheetSizeX.c_str(), 0);
	json.set(Nebulite::Constants::keyName.renderObject.spritesheetSizeY.c_str(), 0);
	json.set(Nebulite::Constants::keyName.renderObject.pixelSizeX.c_str(), 32);
	json.set(Nebulite::Constants::keyName.renderObject.pixelSizeY.c_str(), 32);

	// Invokes
	json.set_empty_array(Nebulite::Constants::keyName.renderObject.invokes.c_str());
	json.set_empty_array(Nebulite::Constants::keyName.renderObject.invokeSubscriptions.c_str());
	json.set((Nebulite::Constants::keyName.renderObject.invokeSubscriptions+"[0]").c_str(),"all");

	// Text
	json.set(Nebulite::Constants::keyName.renderObject.textStr.c_str(),std::string(""));
	json.set(Nebulite::Constants::keyName.renderObject.textFontsize.c_str(),0);
	json.set(Nebulite::Constants::keyName.renderObject.textDx.c_str(),0.0);
	json.set(Nebulite::Constants::keyName.renderObject.textDy.c_str(),0.0);
	json.set(Nebulite::Constants::keyName.renderObject.textColorR.c_str(),255);
	json.set(Nebulite::Constants::keyName.renderObject.textColorG.c_str(),255);
	json.set(Nebulite::Constants::keyName.renderObject.textColorB.c_str(),255);
	json.set(Nebulite::Constants::keyName.renderObject.textColorA.c_str(),255);

	//------------------------------------------
	// Internal Values

	//Build Rect on creation
	calculateDstRect();
	calculateSrcRect();

	textSurface = nullptr;
    textTexture = nullptr;

	//------------------------------------------
	// Flags
	flag.deleteFromScene = false;
	flag.calculateText = true;		// In order to calculate text texture on first update
	flag.reloadInvokes = true;		// In order to reload invokes on first update

	subscription_size = json.memberSize(Nebulite::Constants::keyName.renderObject.invokeSubscriptions.c_str());

	//------------------------------------------
    // Link inherited FuncTree json
    inherit(json.funcTree);

	//------------------------------------------
	// Initialize Domain Modules
	Nebulite::DomainModule::RDM_init(this);

	//------------------------------------------
	// Update cannot be called in constructor, 
	// as it relies on an invoke reference
}

Nebulite::Core::RenderObject::~RenderObject() {
    if (textSurface) {
        SDL_FreeSurface(textSurface);
        textSurface = nullptr;
    }

    if (textTexture) {
        SDL_DestroyTexture(textTexture);
        textTexture = nullptr;
    }

    // Clean up invoke entries - shared pointers will automatically handle cleanup
    entries_global.clear();
    entries_local.clear();
}

//------------------------------------------
// Marshalling

std::string Nebulite::Core::RenderObject::serialize() {
	return json.serialize();
}

void Nebulite::Core::RenderObject::deserialize(std::string serialOrLink) {

	// Check if argv1 provided is an object
	if(serialOrLink.starts_with('{')){
		json.deserialize(serialOrLink);
	}
	else{
		//------------------------------------------
		// Split the input into tokens
		std::vector<std::string> tokens = Nebulite::Utility::StringHandler::split(serialOrLink, '|');

		//------------------------------------------
        // Validity check
		if (tokens.empty()) {
			return; // or handle error properly
		}

		//------------------------------------------
		// Load the JSON file
		// First token is the path or serialized JSON
		json.deserialize(tokens[0]);

		//------------------------------------------
        // Now apply modifications
		tokens.erase(tokens.begin()); // Remove the first token (path or serialized JSON)
		for (const auto& token : tokens) {
			if (token.empty()) continue; // Skip empty tokens

			// Legacy: Handle key=value pairs
			if (token.find('=') != std::string::npos) {
				// Handle modifier (key=value)
				auto pos = token.find('=');
				std::string key = token.substr(0, pos);
				std::string value = token.substr(pos + 1);

				// Old implementation via direct set
				//json.set<std::string>(key.c_str(), value);

				// New implementation through functioncall
				parseStr("Nebulite::Core::RenderObject::deserialize set " + key + " " + value);
			}
			// Handle function call
			else {
				// Forward to FunctionTree for resolution
				parseStr("Nebulite::Core::RenderObject::deserialize " + token);
			}
		}
	}
	

	// Prerequisites
	flag.reloadInvokes = true;
	flag.calculateText = true;
	calculateDstRect();
	calculateSrcRect();

	subscription_size = json.memberSize(Nebulite::Constants::keyName.renderObject.invokeSubscriptions.c_str());
}

//------------------------------------------
// General functions

SDL_Texture* Nebulite::Core::RenderObject::getTextTexture(){
	return textTexture;
}

SDL_Rect* Nebulite::Core::RenderObject::getTextRect(){
	return &textRect;
}

SDL_Rect* Nebulite::Core::RenderObject::getDstRect() {
	return &dstRect;
}

void Nebulite::Core::RenderObject::calculateDstRect() {
	dstRect = {
		(int)floor(get<double>(Nebulite::Constants::keyName.renderObject.positionX.c_str())),
		(int)floor(get<double>(Nebulite::Constants::keyName.renderObject.positionY.c_str())),
		(int)floor(get<double>(Nebulite::Constants::keyName.renderObject.pixelSizeX.c_str())), // Set the desired width
		(int)floor(get<double>(Nebulite::Constants::keyName.renderObject.pixelSizeY.c_str())), // Set the desired height
	};
};

SDL_Rect* Nebulite::Core::RenderObject::getSrcRect() {
	if (get<bool>(Nebulite::Constants::keyName.renderObject.isSpritesheet.c_str())) {
		return &srcRect;
	}
	else {
		return nullptr;
	}
}

void Nebulite::Core::RenderObject::calculateSrcRect() {
	// Check if the object is a sprite
	if (get<bool>(Nebulite::Constants::keyName.renderObject.isSpritesheet.c_str())) {
		int offsetX = (int)get<double>(Nebulite::Constants::keyName.renderObject.spritesheetOffsetX.c_str(),0);
		int offsetY = (int)get<double>(Nebulite::Constants::keyName.renderObject.spritesheetOffsetY.c_str(),0);
		int spriteWidth = get<int>(Nebulite::Constants::keyName.renderObject.spritesheetSizeX.c_str(),0);
		int spriteHeight = get<int>(Nebulite::Constants::keyName.renderObject.spritesheetSizeY.c_str(),0);
		
		// Calculate the source rectangle for the sprite (which portion of the sprite sheet to render)
		srcRect = {
			offsetX,       // Start X from the sprite sheet offset
			offsetY,       // Start Y from the sprite sheet offset
			spriteWidth,   // The width of the sprite frame
			spriteHeight   // The height of the sprite frame
		};
	}
}

//------------------------------------------
// Outside communication with invoke for updating and estimation

void Nebulite::Core::RenderObject::update(Nebulite::Interaction::Invoke* globalInvoke) {
	//------------------------------------------
	// Update Domain
	for(auto& module : modules){
		module->update();
	}
	getDoc()->update();

	//------------------------------------------
	// Check all invokes
	if (globalInvoke) {
		//------------------------------------------
		// 1.) Reload invokes if needed
		if (flag.reloadInvokes) {
			Nebulite::Interaction::Deserializer::parse(entries_global, entries_local, this, globalInvoke->getDocumentCache(), globalInvoke->getGlobalPointer());
			flag.reloadInvokes = false;
		}

		//------------------------------------------
		// 2.) Directly solve local invokes (loop)
		for (auto entry : entries_local){
			if(globalInvoke->isTrueLocal(entry)){
				globalInvoke->updateLocal(entry);
			}
		}

		//------------------------------------------
		// 3.) Checks this object against all conventional invokes
		//	   Manipulation happens at the Invoke::update routine later on
		//     This just generates pairs that need to be updated
		for(int i = 0; i < subscription_size;i++){
			std::string key = Nebulite::Constants::keyName.renderObject.invokeSubscriptions + "[" + std::to_string(i) + "]";
			std::string subscription = json.get<std::string>(key.c_str(),"");
			globalInvoke->listen(this,subscription);
		}
        

		//------------------------------------------
		// 4.) Append general invokes from object itself back for global check
		//     This makes sure that no invokes from inactive objects stay in the list
		for (auto entry : entries_global){
			// add pointer to invoke command to global
			globalInvoke->broadcast(entry);
		}
    }else{
		std::cerr << "Invoke is nullptr!" << std::endl;
	}

	//------------------------------------------
	//recalc rect
	calculateDstRect();
	calculateSrcRect();
}

uint64_t Nebulite::Core::RenderObject::estimateComputationalCost(Nebulite::Interaction::Invoke* globalInvoke) {

	//------------------------------------------
	// Reload invokes if needed
	if (flag.reloadInvokes){
		Nebulite::Interaction::Deserializer::parse(entries_global, entries_local, this, globalInvoke->getDocumentCache(), globalInvoke->getGlobalPointer());
		flag.reloadInvokes = false;
	}

	//------------------------------------------
	// Count number of $ and { in logical Arguments
	uint64_t cost = 0;

	// Global entries aren't relevant for this type of cost estimation, as they are evaluated elsewhere

	// Global entries
	/*
	for (auto entry : entries_global) {
		std::string expr = entry->logicalArg.getFullExpression();
		cost += std::count(expr.begin(), expr.end(), '$');
		cost += std::count(expr.begin(), expr.end(), '{');

		// Count number of $ in exprs
		for (auto& expr : entry->exprs) {
			cost += std::count(expr.value.begin(), expr.value.end(), '$');
			cost += std::count(expr.value.begin(), expr.value.end(), '{');
		}
	}
	*/


	// Local entries
	for (auto entry : entries_local) {
		std::string expr = entry->logicalArg.getFullExpression();
		cost += std::count(expr.begin(), expr.end(), '$');

		// Count number of $ in exprs
		for (auto& expr : entry->exprs) {
			cost += std::count(expr.value.begin(), expr.value.end(), '$');
			cost += std::count(expr.value.begin(), expr.value.end(), '{');
		}
	}

	return cost;
}

//------------------------------------------
// Outside communication with Renderer for text calculation

void Nebulite::Core::RenderObject::calculateText(SDL_Renderer* renderer,TTF_Font* font,int renderer_X, int renderer_Y){
	
	// RECT position to renderer
	textRect.x = 	get<double>(Nebulite::Constants::keyName.renderObject.positionX.c_str()) + 
					get<double>(Nebulite::Constants::keyName.renderObject.textDx.c_str()) - renderer_X;
	textRect.y = 	get<double>(Nebulite::Constants::keyName.renderObject.positionY.c_str()) + 
					get<double>(Nebulite::Constants::keyName.renderObject.textDy.c_str()) - renderer_Y;
	
	// Recreate texture if recalculate was triggered by user. This is needed for:
	// - new text
	// - new color
	// - new text size
	if(flag.calculateText){
		// Free previous texture
        if (textTexture != nullptr) {
            SDL_DestroyTexture(textTexture);
            textTexture = nullptr;
        }
		
		// Settings influenced by a new text
		double scalar = 1.0;
		double fontSize = get<double>(Nebulite::Constants::keyName.renderObject.textFontsize.c_str());
		std::string text = get<std::string>(Nebulite::Constants::keyName.renderObject.textStr.c_str());
		textRect.w = scalar * fontSize * text.length();
		textRect.h = static_cast<int>(fontSize * 1.5f * scalar);

		// Create text
		SDL_Color textColor = { 
			(Uint8)json.get<int>(Nebulite::Constants::keyName.renderObject.textColorR.c_str(),255),
			(Uint8)json.get<int>(Nebulite::Constants::keyName.renderObject.textColorG.c_str(),255),
			(Uint8)json.get<int>(Nebulite::Constants::keyName.renderObject.textColorB.c_str(),255),
			(Uint8)json.get<int>(Nebulite::Constants::keyName.renderObject.textColorA.c_str(),255)
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
