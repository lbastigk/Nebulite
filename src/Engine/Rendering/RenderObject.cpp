#include "RenderObject.h"
#include "RenderObjectTree.h"

//-----------------------------------------------------------
// Special member Functions

Nebulite::RenderObject::RenderObject() : renderObjectTree(this) {

	//------------------------------------------------------------
	// Document Values

	// General
	json.set(Nebulite::keyName.renderObject.id.c_str(),0);
	json.set(Nebulite::keyName.renderObject.positionX.c_str(), 0);
	json.set(Nebulite::keyName.renderObject.positionY.c_str(), 0);
	json.set(Nebulite::keyName.renderObject.imageLocation.c_str(), std::string("Resources/Sprites/TEST001P/001.bmp"));
	json.set(Nebulite::keyName.renderObject.layer.c_str(), 0);

	//for sprite
	json.set(Nebulite::keyName.renderObject.isSpritesheet.c_str(), false);
	json.set(Nebulite::keyName.renderObject.spritesheetOffsetX.c_str(), 0);
	json.set(Nebulite::keyName.renderObject.spritesheetOffsetY.c_str(), 0);
	json.set(Nebulite::keyName.renderObject.spritesheetSizeX.c_str(), 0);
	json.set(Nebulite::keyName.renderObject.spritesheetSizeY.c_str(), 0);
	json.set(Nebulite::keyName.renderObject.pixelSizeX.c_str(), 32);
	json.set(Nebulite::keyName.renderObject.pixelSizeY.c_str(), 32);

	// Invokes
	json.set_empty_array(Nebulite::keyName.renderObject.invokes.c_str());
	json.set_empty_array(Nebulite::keyName.renderObject.invokeSubscriptions.c_str());
	json.set((Nebulite::keyName.renderObject.invokeSubscriptions+"[0]").c_str(),"all");
	json.set(Nebulite::keyName.renderObject.reloadInvokes.c_str(), true);

	// Text
	json.set(Nebulite::keyName.renderObject.textStr.c_str(),std::string(""));
	json.set(Nebulite::keyName.renderObject.textFontsize.c_str(),0);
	json.set(Nebulite::keyName.renderObject.textDx.c_str(),0.0);
	json.set(Nebulite::keyName.renderObject.textDy.c_str(),0.0);
	json.set(Nebulite::keyName.renderObject.textColorR.c_str(),255);
	json.set(Nebulite::keyName.renderObject.textColorG.c_str(),255);
	json.set(Nebulite::keyName.renderObject.textColorB.c_str(),255);
	json.set(Nebulite::keyName.renderObject.textColorA.c_str(),255);

	//------------------------------------------------------------
	// Internal Values

	//Build Rect on creation
	calculateDstRect();
	calculateSrcRect();

	textSurface = nullptr;
    textTexture = nullptr;

	//------------------------------------------------------------
	// Flags
	flag.deleteFromScene = false;
	flag.calculateText = true;		// In order to calculate text texture on first update
}

Nebulite::RenderObject::~RenderObject() {
    if (textSurface) {
        SDL_FreeSurface(textSurface);
        textSurface = nullptr;
    }

    if (textTexture) {
        SDL_DestroyTexture(textTexture);
        textTexture = nullptr;
    }
}




//-----------------------------------------------------------
// Marshalling

std::string Nebulite::RenderObject::serialize() {
	return json.serialize();
}


void Nebulite::RenderObject::deserialize(std::string serialOrLink) {

	// Check if argv1 provided is an object
	if(serialOrLink.starts_with('{')){
		json.deserialize(serialOrLink);
	}
	else{
		//----------------------------------------------------------
		// Split the input into tokens
		std::vector<std::string> tokens = StringHandler::split(serialOrLink, '|');

		//----------------------------------------------------------
        // Validity check
		if (tokens.empty()) {
			return; // or handle error properly
		}

		//----------------------------------------------------------
		// Load the JSON file
		// First token is the path or serialized JSON
		json.deserialize(tokens[0]);

		//----------------------------------------------------------
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
				parseStr("Nebulite::RenderObject::deserialize set " + key + " " + value);
			}
			// Handle function call
			else {
				// Forward to FunctionTree for resolution
				parseStr("Nebulite::RenderObject::deserialize " + token);
			}
		}
	}
	

	// Prerequisites
	valueSet(Nebulite::keyName.renderObject.reloadInvokes.c_str(),true);
	calculateDstRect();
	calculateSrcRect();
}

//-----------------------------------------------------------
// General functions

void Nebulite::RenderObject::calculateText(SDL_Renderer* renderer,TTF_Font* font,int renderer_X, int renderer_Y){
	
	// RECT position to renderer
	textRect.x = 	valueGet<float>(Nebulite::keyName.renderObject.positionX.c_str()) + 
					valueGet<float>(Nebulite::keyName.renderObject.textDx.c_str()) - renderer_X;
	textRect.y = 	valueGet<float>(Nebulite::keyName.renderObject.positionY.c_str()) + 
					valueGet<float>(Nebulite::keyName.renderObject.textDy.c_str()) - renderer_Y;
	
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
		float scalar = 1;
		float fontSize = valueGet<float>(Nebulite::keyName.renderObject.textFontsize.c_str());
		std::string text = valueGet<std::string>(Nebulite::keyName.renderObject.textStr.c_str());
		textRect.w = scalar * fontSize * text.length();
		textRect.h = static_cast<int>(fontSize * 1.5f * scalar);

		// Create text
		SDL_Color textColor = { 
			(Uint8)json.get<int>(Nebulite::keyName.renderObject.textColorR.c_str(),255),
			(Uint8)json.get<int>(Nebulite::keyName.renderObject.textColorG.c_str(),255),
			(Uint8)json.get<int>(Nebulite::keyName.renderObject.textColorB.c_str(),255),
			(Uint8)json.get<int>(Nebulite::keyName.renderObject.textColorA.c_str(),255)
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

SDL_Texture* Nebulite::RenderObject::getTextTexture(){
	return textTexture;
}

SDL_Rect* Nebulite::RenderObject::getTextRect(){
	return &textRect;
}


SDL_Rect* Nebulite::RenderObject::getDstRect() {
	return &dstRect;
}
void Nebulite::RenderObject::calculateDstRect() {
	dstRect = {
		(int)floor(valueGet<double>(Nebulite::keyName.renderObject.positionX.c_str())),
		(int)floor(valueGet<double>(Nebulite::keyName.renderObject.positionY.c_str())),
		(int)floor(valueGet<double>(Nebulite::keyName.renderObject.pixelSizeX.c_str())), // Set the desired width
		(int)floor(valueGet<double>(Nebulite::keyName.renderObject.pixelSizeY.c_str())), // Set the desired height
	};
};
SDL_Rect* Nebulite::RenderObject::getSrcRect() {
	if (valueGet<bool>(Nebulite::keyName.renderObject.isSpritesheet.c_str())) {
		return &srcRect;
	}
	else {
		return nullptr;
	}
}
void Nebulite::RenderObject::calculateSrcRect() {
	// Check if the object is a sprite
	if (valueGet<bool>(Nebulite::keyName.renderObject.isSpritesheet.c_str())) {
		int offsetX = (int)valueGet<double>(Nebulite::keyName.renderObject.spritesheetOffsetX.c_str(),0);
		int offsetY = (int)valueGet<double>(Nebulite::keyName.renderObject.spritesheetOffsetY.c_str(),0);
		int spriteWidth = valueGet<int>(Nebulite::keyName.renderObject.spritesheetSizeX.c_str(),0);
		int spriteHeight = valueGet<int>(Nebulite::keyName.renderObject.spritesheetSizeY.c_str(),0);
		
		// Calculate the source rectangle for the sprite (which portion of the sprite sheet to render)
		srcRect = {
			offsetX,       // Start X from the sprite sheet offset
			offsetY,       // Start Y from the sprite sheet offset
			spriteWidth,   // The width of the sprite frame
			spriteHeight   // The height of the sprite frame
		};
	}
}

void Nebulite::RenderObject::update(Nebulite::Invoke* globalInvoke) {
	//------------------------------------
	// Check all invokes
	if (globalInvoke) {
		//------------------------------
		// 1.) Reload invokes if needed
		if (valueGet<int>(Nebulite::keyName.renderObject.reloadInvokes.c_str(),true)){
			Invoke::parseFromJSON(json, entries_global, entries_local, this);
			valueSet<bool>(Nebulite::keyName.renderObject.reloadInvokes.c_str(),false);
		}

		//------------------------------
		// 2.) Directly solve local invokes (loop)
		for (const auto& entry : entries_local){
			if(globalInvoke->isTrueLocal(entry)){
				globalInvoke->updateLocal(entry);
			}
		}

		//------------------------------
		// 3.) Checks this object against all conventional invokes
		//	   Manipulation happens at the Invoke::update routine later on
		//     This just generates pairs that need to be updated
		for(int i = 0; i < json.memberSize(Nebulite::keyName.renderObject.invokeSubscriptions.c_str());i++){
			std::string key = Nebulite::keyName.renderObject.invokeSubscriptions + "[" + std::to_string(i) + "]";
			std::string subscription = json.get<std::string>(key.c_str(),"");
			globalInvoke->listen(this,subscription);
		}
        

		//------------------------------
		// 4.) Append general invokes from object itself back for global check
		//     This makes sure that no invokes from inactive objects stay in the list
		for (const auto& entry : entries_global){
			// add pointer to invoke command to global
			globalInvoke->broadcast(entry);
		}
    }else{
		std::cerr << "Invoke is nullptr!" << std::endl;
	}

	//------------------------------------
	//recalc rect
	calculateDstRect();
	calculateSrcRect();
}



uint64_t Nebulite::RenderObject::estimateComputationalCost(){

	//------------------------------------------
	// Reload invokes if needed
	if (valueGet<int>(Nebulite::keyName.renderObject.reloadInvokes.c_str(),true)){
		Invoke::parseFromJSON(json, entries_global, entries_local, this);
		valueSet<bool>(Nebulite::keyName.renderObject.reloadInvokes.c_str(),false);
	}
	

	//------------------------------------------
	// Count number of $ in logical Arguments
	uint64_t cost = 0;

	// Global entries
	for (const auto& entry : entries_global) {
		cost += std::count(entry->logicalArg.begin(), entry->logicalArg.end(), '$');

		// Count number of $ in exprs
		for (const auto& expr : entry->exprs) {
			cost += std::count(expr.value.begin(), expr.value.end(), '$');
		}
	}

	// Local entries
	for (const auto& entry : entries_local) {
		cost += std::count(entry->logicalArg.begin(), entry->logicalArg.end(), '$');

		// Count number of $ in exprs
		for (const auto& expr : entry->exprs) {
			cost += std::count(expr.value.begin(), expr.value.end(), '$');
		}
	}


	return cost;
}


Nebulite::ERROR_TYPE Nebulite::RenderObject::parseStr(const std::string& str){
	// Priority 1: RenderObjectTree
	if(renderObjectTree.hasFunction(str)) {
		return renderObjectTree.parseStr(str);
	}

	// Priority 2: JSONTree
	return json.parseStr(str);
}
