#include "RenderObject.h"

//-----------------------------------------------------------
// Special member Functions

Nebulite::RenderObject::RenderObject() {
	// General
	json.set(Nebulite::keyName.renderObject.id.c_str(),0);
	json.set(Nebulite::keyName.renderObject.positionX.c_str(), 0);
	json.set(Nebulite::keyName.renderObject.positionY.c_str(), 0);
	json.set(Nebulite::keyName.renderObject.imageLocation.c_str(), std::string("Resources/Sprites/TEST001P/001.bmp"));
	json.set(Nebulite::keyName.renderObject.layer.c_str(), 0);
	json.set(Nebulite::keyName.renderObject.deleteFlag.c_str(), false);

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
	json.set(Nebulite::keyName.renderObject.flagCalculate.c_str(),true);
	json.set(Nebulite::keyName.renderObject.textColorR.c_str(),255);
	json.set(Nebulite::keyName.renderObject.textColorG.c_str(),255);
	json.set(Nebulite::keyName.renderObject.textColorB.c_str(),255);
	json.set(Nebulite::keyName.renderObject.textColorA.c_str(),255);

	//Build Rect on creation
	calculateDstRect();
	calculateSrcRect();

	textSurface = nullptr;
    textTexture = nullptr;
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

// TODO:
//
void Nebulite::RenderObject::deserialize(std::string serialOrLink) {
	// TODO:
	// Extend RenderObject deserialization to support function call modifiers 
	// and a cascading (pseudo-inheritance) resolution model.
	//
	// The serialized input may contain a base path followed by a list of 
	// modifiers or function calls, e.g.:
	//     ./path/to/resource.json|posX=100|align_text_with_size 200 center|set_empty_array tags
	//
	// Modifiers (key=value) are handled by the JSON system. Function calls
	// (non-assignment tokens) are attempted on RenderObject's FunctionTree.
	//
	// If a function call is not found in RenderObject, it should be forwarded 
	// to the JSON system's FunctionTree for resolution (pseudo-inheritance model).
	//
	// This allows RenderObject-level behavior extensions while preserving deep 
	// JSON configurability without needing explicit prefixes or routing hints.
	// 
	// Future extensibility: this cascading system may be generalized to 
	// include additional component handlers (e.g. physics, editor, etc.).

	json.deserialize(serialOrLink);

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
	if(valueGet<bool>(Nebulite::keyName.renderObject.flagCalculate.c_str(),true)){
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
			json.get(Nebulite::keyName.renderObject.textColorR.c_str(),255),
			json.get(Nebulite::keyName.renderObject.textColorG.c_str(),255),
			json.get(Nebulite::keyName.renderObject.textColorB.c_str(),255),
			json.get(Nebulite::keyName.renderObject.textColorA.c_str(),255)
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
		valueSet(Nebulite::keyName.renderObject.flagCalculate.c_str(),false);
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
		triple.valueContainsResolveKeyword = triple.value.find(InvokeResolveKeyword) != std::string::npos;	// only needs to be resolved if it contains keyword

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

void Nebulite::RenderObject::reloadInvokes() {
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
			entry.selfPtr = this;
			entry.topic =			invoke.get<std::string>("topic","all");
			entry.isGlobal = 		invoke.get<bool>("isGlobal",true);
			entry.invokes_self = 	parseInvokeTriples(invoke,"self_invokes");
			entry.invokes_other = 	parseInvokeTriples(invoke,"other_invokes");
			entry.invokes_global = 	parseInvokeTriples(invoke,"global_invokes");

			// Allow array splitting for logical arg
			if(invoke.memberCheck("logicalArg") == Nebulite::JSON::KeyType::array){
				entry.logicalArg = InvokeResolveKeywordWithOpenParanthesis;
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
    json.set(Nebulite::keyName.renderObject.reloadInvokes.c_str(), false);
}

//-----------------------------------------------------------
// TODO: Change this to be a two-part:
//
// - first check against all other in list WITHOUT updating
//   (allows for threading)
// - store pointer pairs as std::vector<std::pair<RenderObject& RenderObject&>>
// - after object pre-update, call actual update via invoke class that changes all objects
// - additionally, the effects on self/other can be stored as a map where the key is the pointer this_shared
void Nebulite::RenderObject::update(Nebulite::Invoke* globalInvoke) {
	//------------------------------------
	// Check all invokes
	if (globalInvoke) {
		//------------------------------
		// 1.) Reload invokes if needed
		if (valueGet<int>(Nebulite::keyName.renderObject.reloadInvokes.c_str(),true)){
			reloadInvokes();
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
		//     This just generates pairs that need to be updated
		for(int i = 0; i < json.memberSize(Nebulite::keyName.renderObject.invokeSubscriptions.c_str());i++){
			std::string key = Nebulite::keyName.renderObject.invokeSubscriptions + "[" + std::to_string(i) + "]";
			std::string subscription = json.get<std::string>(key.c_str(),"");
			globalInvoke->listen(this,subscription);
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


uint64_t Nebulite::RenderObject::estimateCompuationalCost(){
	if(valueGet<int>(Nebulite::keyName.renderObject.reloadInvokes.c_str(),true)){
		reloadInvokes();
	}
	return cmds_internal.size();
}

