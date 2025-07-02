#include "RenderObject.h"


//-----------------------------------------------------------
//Constructor

RenderObject::RenderObject() {
	// General
	json.set(keyName.renderObject.id.c_str(),0);
	json.set(keyName.renderObject.positionX.c_str(), 0);
	json.set(keyName.renderObject.positionY.c_str(), 0);
	json.set(keyName.renderObject.imageLocation.c_str(), std::string("Resources/Sprites/TEST001P/001.bmp"));
	json.set(keyName.renderObject.layer.c_str(), 0);
	json.set(keyName.renderObject.deleteFlag.c_str(), false);

	//for sprite
	json.set(keyName.renderObject.isSpritesheet.c_str(), false);
	json.set(keyName.renderObject.spritesheetOffsetX.c_str(), 0);
	json.set(keyName.renderObject.spritesheetOffsetY.c_str(), 0);
	json.set(keyName.renderObject.spritesheetSizeX.c_str(), 0);
	json.set(keyName.renderObject.spritesheetSizeY.c_str(), 0);
	json.set(keyName.renderObject.pixelSizeX.c_str(), 32);
	json.set(keyName.renderObject.pixelSizeY.c_str(), 32);

	// Invokes
	json.set_empty_array(keyName.renderObject.invokes.c_str());
	json.set_empty_array(keyName.renderObject.invokeSubscriptions.c_str());
	json.set((keyName.renderObject.invokeSubscriptions+"[0]").c_str(),"all");
	json.set(keyName.renderObject.reloadInvokes.c_str(), true);

	// Text
	json.set(keyName.renderObject.textStr.c_str(),std::string(""));
	json.set(keyName.renderObject.textFontsize.c_str(),0);
	json.set(keyName.renderObject.textDx.c_str(),0.0);
	json.set(keyName.renderObject.textDy.c_str(),0.0);
	json.set(keyName.renderObject.flagCalculate.c_str(),true);
	json.set(keyName.renderObject.textColorR.c_str(),255);
	json.set(keyName.renderObject.textColorG.c_str(),255);
	json.set(keyName.renderObject.textColorB.c_str(),255);
	json.set(keyName.renderObject.textColorA.c_str(),255);

	//Build Rect on creation
	calculateDstRect();
	calculateSrcRect();

	textSurface = nullptr;
    textTexture = nullptr;
}


RenderObject::RenderObject(const RenderObject& other) {
	json.getDoc()->CopyFrom(*(other._getDoc()), json.getDoc()->GetAllocator());
	valueSet(keyName.renderObject.flagCalculate.c_str(),true);
	calculateDstRect();
	calculateSrcRect();
}




//-----------------------------------------------------------
//Marshalling

std::string RenderObject::serialize() {
	return json.serialize();
}

void RenderObject::deserialize(std::string serialOrLink) {
	json.deserialize(serialOrLink);

	// Prerequisites
	valueSet(keyName.renderObject.reloadInvokes.c_str(),true);
	calculateDstRect();
	calculateSrcRect();
}

void RenderObject::calculateText(SDL_Renderer* renderer,TTF_Font* font,int renderer_X, int renderer_Y){
	
	// RECT position to renderer
	textRect.x = 	valueGet<float>(keyName.renderObject.positionX.c_str()) + 
					valueGet<float>(keyName.renderObject.textDx.c_str()) - renderer_X;
	textRect.y = 	valueGet<float>(keyName.renderObject.positionY.c_str()) + 
					valueGet<float>(keyName.renderObject.textDy.c_str()) - renderer_Y;
	
	// Recreate texture if recalculate was triggered by user. This is needed for:
	// - new text
	// - new color
	// - new text size
	if(valueGet<bool>(keyName.renderObject.flagCalculate.c_str(),true)){
		// Free previous texture
        if (textTexture) {
            SDL_DestroyTexture(textTexture);
            textTexture = nullptr;
        }
		
		// Settings influenced by a new text
		float scalar = 1;
		float fontSize = valueGet<float>(keyName.renderObject.textFontsize.c_str());
		std::string text = valueGet<std::string>(keyName.renderObject.textStr.c_str());
		textRect.w = scalar * fontSize * text.length();
		textRect.h = static_cast<int>(fontSize * 1.5f * scalar);

		// Create text
		SDL_Color textColor = { 
			json.get(keyName.renderObject.textColorR.c_str(),255),
			json.get(keyName.renderObject.textColorG.c_str(),255),
			json.get(keyName.renderObject.textColorB.c_str(),255),
			json.get(keyName.renderObject.textColorA.c_str(),255)
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
		valueSet(keyName.renderObject.flagCalculate.c_str(),false);
	}
}

SDL_Texture* RenderObject::getTextTexture(){
	return textTexture;
}

SDL_Rect* RenderObject::getTextRect(){
	return &textRect;
}

rapidjson::Document* RenderObject::_getDoc() const {
	return json.getDoc();
	//return const_cast<rapidjson::Document*>(&doc);
}


SDL_Rect& RenderObject::getDstRect() {
	return dstRect;
}
void RenderObject::calculateDstRect() {
	dstRect = {
		(int)floor(valueGet<double>(keyName.renderObject.positionX.c_str())),
		(int)floor(valueGet<double>(keyName.renderObject.positionY.c_str())),
		(int)floor(valueGet<double>(keyName.renderObject.pixelSizeX.c_str())), // Set the desired width
		(int)floor(valueGet<double>(keyName.renderObject.pixelSizeY.c_str())), // Set the desired height
	};
};
SDL_Rect* RenderObject::getSrcRect() {
	if (valueGet<bool>(keyName.renderObject.isSpritesheet.c_str())) {
		return &srcRect;
	}
	else {
		return nullptr;
	}
}
void RenderObject::calculateSrcRect() {
	// Check if the object is a sprite
	if (valueGet<bool>(keyName.renderObject.isSpritesheet.c_str())) {
		int offsetX = (int)valueGet<double>(keyName.renderObject.spritesheetOffsetX.c_str(),0);
		int offsetY = (int)valueGet<double>(keyName.renderObject.spritesheetOffsetY.c_str(),0);
		int spriteWidth = valueGet<int>(keyName.renderObject.spritesheetSizeX.c_str(),0);
		int spriteHeight = valueGet<int>(keyName.renderObject.spritesheetSizeY.c_str(),0);
		
		// Calculate the source rectangle for the sprite (which portion of the sprite sheet to render)
		srcRect = {
			offsetX,       // Start X from the sprite sheet offset
			offsetY,       // Start Y from the sprite sheet offset
			spriteWidth,   // The width of the sprite frame
			spriteHeight   // The height of the sprite frame
		};
	}
}

// Helper for parsing invoke triples
std::vector<Nebulite::Invoke::InvokeTriple> parseInvokeTriples(Nebulite::JSON& doc, std::string key) {
	std::string arr;
    std::vector<Nebulite::Invoke::InvokeTriple> res;
    int size = doc.memberSize(key);
	for(int i = 0; i < size; i++){
		arr = key + "[" + std::to_string(i) + "]";
		Nebulite::Invoke::InvokeTriple triple;
        triple.key = 		doc.get<std::string>((arr+"key").c_str(),"");
        triple.value = 		doc.get<std::string>((arr+"value").c_str(),"");

		std::string changeType = doc.get<std::string>((arr+"changeType").c_str(),"");
		if(changeType == "add"){
			triple.changeType = Nebulite::Invoke::InvokeTriple::ChangeType::add;
		}
		else if(changeType == "multiply"){
			triple.changeType = Nebulite::Invoke::InvokeTriple::ChangeType::multiply;
		}
		else if(changeType == "concat"){
			triple.changeType = Nebulite::Invoke::InvokeTriple::ChangeType::concat;
		}
		else{
			// assume set
			triple.changeType = Nebulite::Invoke::InvokeTriple::ChangeType::set;
		}

        res.push_back(std::move(triple));
	}
    return res;
}

void RenderObject::reloadInvokes(std::shared_ptr<RenderObject> this_shared) {
    cmds_general.clear();
    cmds_internal.clear();

	Nebulite::JSON::KeyType type = json.memberCheck("invokes");

	if(type == Nebulite::JSON::KeyType::array){
		int size = json.memberSize("invokes");
		for(int i = 0; i < size; i++){
			std::string key = "invokes[" + std::to_string(i) + "]";

			Nebulite::JSON invoke;
			if(json.memberCheck(key.c_str()) == Nebulite::JSON::KeyType::value){
				std::string link = "";
				link = json.get(key.c_str(),link);
				invoke.deserialize(link);
			}
			else if (json.memberCheck(key.c_str()) == Nebulite::JSON::KeyType::document){
				invoke = json.get_subdoc(key.c_str());
			}
			else{
				std::cerr << "Unsupported invoke. Skipping." << std::endl;
				continue;
			}


			//--------------------------
			// Build entry
			Nebulite::Invoke::InvokeEntry entry;
			entry.selfPtr = this_shared;
			entry.topic =			invoke.get<std::string>("topic","all");
			entry.isGlobal = 		invoke.get<bool>("isGlobal",true);
			entry.invokes_self = 	parseInvokeTriples(invoke,"self_invokes");
			entry.invokes_other = 	parseInvokeTriples(invoke,"other_invokes");
			entry.invokes_global = 	parseInvokeTriples(invoke,"global_invokes");

			// Allow array splitting for logical arg
			if(invoke.memberCheck("logicalArg") == Nebulite::JSON::KeyType::array){
				entry.logicalArg = "$(";
				int size = invoke.memberSize("logicalArg");
				std::string key;
				for (int j = 0; j < size; j++){
					key = "logicalArg[" + std::to_string(j) + "]";
					entry.logicalArg += "(";
					entry.logicalArg += invoke.get<std::string>(key.c_str(),"");
					entry.logicalArg += ")";
					if(j != size-1){
						entry.logicalArg += "*";
					}
				}
				entry.logicalArg += ")";
			}
			else if(invoke.memberCheck("logicalArg") == Nebulite::JSON::KeyType::value){
				entry.logicalArg = invoke.get<std::string>("logicalArg","1");
			}
			else{
				entry.logicalArg = "1";
			}
			

			entry.functioncalls.clear();

			std::string arr;
			int size = invoke.memberSize("functioncalls");
			for(int i = 0; i < size; i++){
				arr = "functioncalls[" + std::to_string(i) + "]";
				entry.functioncalls.push_back(invoke.get<std::string>(arr.c_str(),""));
			}

			// Append
			auto ptr = std::make_shared<Nebulite::Invoke::InvokeEntry>(std::move(entry));

			if(entry.isGlobal){
				cmds_general.push_back(ptr);
			}
			else{
				cmds_internal.push_back(ptr);
			}

		}
	}
    json.set(keyName.renderObject.reloadInvokes.c_str(), false);
}

//-----------------------------------------------------------
// TODO: Change this to be a two-part:
//
// - first check against all other in list WITHOUT updating
//   (allows for threading)
// - store pointer pairs as std::vector<std::pair<RenderObject& RenderObject&>>
// - after object pre-update, call actual update via invoke class that changes all objects
// - additionally, the effects on self/other can be stored as a map where the key is the pointer this_shared
void RenderObject::update(Nebulite::Invoke* globalInvoke, std::shared_ptr<RenderObject> this_shared) {
	//------------------------------------
	// Check all invokes
	if (globalInvoke) {
		//------------------------------
		// 1.) Reload invokes if needed
		if (valueGet<int>(keyName.renderObject.reloadInvokes.c_str(),true)){
			reloadInvokes(this_shared);
		}

		//------------------------------
		// 2.) Directly solve local invokes (loop)
		for (const auto& cmd : cmds_internal){
			if(globalInvoke->isTrueLocal(cmd)){
				globalInvoke->updateLocal(cmd);
			}
		}

		//------------------------------
		// 3.) Checks this object against all conventional invokes
		//	   Manipulation happens at the Invoke::update routine later on
		//     This just generates true pairs that need to be updated
		for(int i = 0; i < json.memberSize(keyName.renderObject.invokeSubscriptions.c_str());i++){
			std::string key = keyName.renderObject.invokeSubscriptions + "[" + std::to_string(i) + "]";
			std::string subscription = json.get<std::string>(key.c_str(),"");
			globalInvoke->listen(this_shared,subscription);
		}
        

		//------------------------------
		// 4.) Append general invokes from object itself back for global check
		//     This makes sure that no invokes from inactive objects stay in the list
		for (const auto& cmd : cmds_general){
			// add pointer to invoke command to global
			globalInvoke->broadcast(cmd);
		}
    }else{
		std::cerr << "Invoke is nullptr!" << std::endl;
	}

	//------------------------------------
	//recalc rect
	calculateDstRect();
	calculateSrcRect();
}



