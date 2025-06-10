#include "RenderObject.h"


//-----------------------------------------------------------
//Constructor

RenderObject::RenderObject() {
	// General
	json.set(namenKonvention.renderObject.id.c_str(),0);
	json.set(namenKonvention.renderObject.isOverlay.c_str(), false);
	json.set(namenKonvention.renderObject.positionX.c_str(), 0);
	json.set(namenKonvention.renderObject.positionY.c_str(), 0);
	json.set(namenKonvention.renderObject.pixelSizeX.c_str(), 32);
	json.set(namenKonvention.renderObject.pixelSizeY.c_str(), 32);
	json.set(namenKonvention.renderObject.imageLocation.c_str(), std::string("Resources/Sprites/TEST001P/001.bmp"));
	json.set(namenKonvention.renderObject.layer.c_str(), 0);
	json.set(namenKonvention.renderObject.deleteFlag.c_str(), false);

	//for spritesheets
	json.set(namenKonvention.renderObject.isSpritesheet.c_str(), false);
	json.set(namenKonvention.renderObject.spritesheetOffsetX.c_str(), 0);
	json.set(namenKonvention.renderObject.spritesheetOffsetY.c_str(), 0);
	json.set(namenKonvention.renderObject.spritesheetSizeX.c_str(), 0);
	json.set(namenKonvention.renderObject.spritesheetSizeY.c_str(), 0);

	// Invokes
	json.set_empty_array("invokes");
	json.set(namenKonvention.renderObject.reloadInvokes.c_str(), true);

	// Text
	json.set(namenKonvention.renderObject.textStr.c_str(),std::string(""));
	json.set(namenKonvention.renderObject.textFontsize.c_str(),0);
	json.set(namenKonvention.renderObject.flagCalculate.c_str(),true);

	//Build Rect on creation
	calculateDstRect();
	calculateSrcRect();
}


RenderObject::RenderObject(const RenderObject& other) {
	json.getDoc()->CopyFrom(*(other._getDoc()), json.getDoc()->GetAllocator());
	valueSet(namenKonvention.renderObject.flagCalculate.c_str(),true);
	calculateDstRect();
	calculateSrcRect();
}

RenderObject& RenderObject::operator=(const RenderObject& other) {  // Assignment operator overload
	if (this != &other) {
		dstRect = other.dstRect;
		JSONHandler::copyDoc(*json.getDoc(), other._getDoc());
	}
	valueSet(namenKonvention.renderObject.flagCalculate.c_str(),true);
	return *this;
}



//-----------------------------------------------------------
//Marshalling

std::string RenderObject::serialize() {
	return json.serialize();
}

void RenderObject::deserialize(std::string serialOrLink) {
	json.deserialize(serialOrLink);

	// Prerequisites
	valueSet(namenKonvention.renderObject.reloadInvokes.c_str(),true);
	calculateDstRect();
	calculateSrcRect();
}

void RenderObject::calculateText(SDL_Renderer* renderer,TTF_Font* font,int renderer_X, int renderer_Y){
	float scalar = 1;
	float fontSize = valueGet<float>(namenKonvention.renderObject.textFontsize.c_str());
	std::string text = valueGet<std::string>(namenKonvention.renderObject.textStr.c_str());
	textRect.x = valueGet<float>(namenKonvention.renderObject.positionX.c_str()) + valueGet<float>(namenKonvention.renderObject.textDx.c_str()) - renderer_X;
	textRect.y = valueGet<float>(namenKonvention.renderObject.positionY.c_str()) + valueGet<float>(namenKonvention.renderObject.textDy.c_str()) - renderer_Y;
	textRect.w = scalar * fontSize * text.length(); // Width based on text length
	textRect.h = (int)((float)fontSize * 1.5 * scalar);
	if(valueGet<bool>(namenKonvention.renderObject.flagCalculate.c_str(),true)==true){
		SDL_Color textColor = { 255, 255, 255, 255 }; // White color
		textSurface = TTF_RenderText_Solid(font, text.c_str(), textColor);
		textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
		valueSet(namenKonvention.renderObject.flagCalculate.c_str(),false);
	}
	
}

SDL_Texture& RenderObject::getTextTexture(){
	return *textTexture;
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
		(int)valueGet<double>(namenKonvention.renderObject.positionX.c_str()),
		(int)valueGet<double>(namenKonvention.renderObject.positionY.c_str()),
		(int)valueGet<double>(namenKonvention.renderObject.pixelSizeX.c_str()), // Set the desired width
		(int)valueGet<double>(namenKonvention.renderObject.pixelSizeY.c_str()), // Set the desired height
	};
};
SDL_Rect* RenderObject::getSrcRect() {
	if (valueGet<bool>(namenKonvention.renderObject.isSpritesheet.c_str())) {
		return &srcRect;
	}
	else {
		return nullptr;
	}
}
void RenderObject::calculateSrcRect() {
	// Check if the object is a sprite
	if (valueGet<bool>(namenKonvention.renderObject.isSpritesheet.c_str())) {
		int offsetX = (int)valueGet<double>(namenKonvention.renderObject.spritesheetOffsetX.c_str(),0);
		int offsetY = (int)valueGet<double>(namenKonvention.renderObject.spritesheetOffsetY.c_str(),0);
		int spriteWidth = valueGet<int>(namenKonvention.renderObject.spritesheetSizeX.c_str(),0);
		int spriteHeight = valueGet<int>(namenKonvention.renderObject.spritesheetSizeY.c_str(),0);
		
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
std::vector<Invoke::InvokeTriple> parseInvokeTriples(Nebulite::JSON& doc, std::string key) {
	std::string arr;
    std::vector<Invoke::InvokeTriple> res;
    int size = doc.memberSize(key);
	for(int i = 0; i < size; i++){
		arr = key + "[" + std::to_string(i) + "]";
		Invoke::InvokeTriple triple;
        triple.changeType = doc.get<std::string>((arr+"changeType").c_str(),"");
        triple.key = 		doc.get<std::string>((arr+"key").c_str(),"");
        triple.value = 		doc.get<std::string>((arr+"value").c_str(),"");

		// DEBUG ENTRY:
		/*
		std::cout << "Triple Entry: " << key << std::endl;
		std::cout << "changeType: " << triple.changeType << std::endl;
		std::cout << "value:      " << triple.value << std::endl;
		//*/


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
			Invoke::InvokeEntry entry;
			entry.selfPtr = this_shared;
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

			// DEBUG ENTRY:
			/*
			std::cout << "Entry: " << key << std::endl;
			std::cout << "Logic: " << entry.logicalArg << std::endl;
			std::cout << "isGlobal: " << entry.isGlobal << std::endl;
			std::cout << "Size self :  " << entry.invokes_self.size() << std::endl;
			std::cout << "Size other:  " << entry.invokes_other.size() << std::endl;
			std::cout << "Size global: " << entry.invokes_global.size() << std::endl;
			//*/

			
			
			// Append
			auto ptr = std::make_shared<Invoke::InvokeEntry>(std::move(entry));

			if(entry.isGlobal){
				cmds_general.push_back(ptr);
			}
			else{
				cmds_internal.push_back(ptr);
			}

		}
	}
    json.set(namenKonvention.renderObject.reloadInvokes.c_str(), false);
}

//-----------------------------------------------------------
// TODO: Change this to be a two-part:
//
// - first check against all other in list WITHOUT updating
//   (allows for threading)
// - store pointer pairs as std::vector<std::pair<RenderObject& RenderObject&>>
// - after object pre-update, call actual update via invoke class that changes all objects
// - additionally, the effects on self/other can be stored as a map where the key is the pointer this_shared
void RenderObject::update(Invoke* globalInvoke, std::shared_ptr<RenderObject> this_shared) {
	//------------------------------------
	// Check all invokes
	if (globalInvoke) {
		// Reload invokes if needed
		if (valueGet<int>(namenKonvention.renderObject.reloadInvokes.c_str(),true)){
			reloadInvokes(this_shared);
		}

		// solve local invokes (loop)
		for (const auto& cmd : cmds_internal){
			if(globalInvoke->isTrue(cmd,this_shared))globalInvoke->updateLocal(cmd);
		}

		// Checks this object against all conventional invokes for manipulation
        globalInvoke->checkAgainstList(this_shared);

		// Next step: append general invokes from object itself back for global check:
		for (const auto& cmd : cmds_general){
			// add pointer to invoke command to global
			globalInvoke->append(cmd);
		}
    }else{
		std::cerr << "Invoke is nullptr!" << std::endl;
	}

	//------------------------------------
	//recalc rect
	calculateDstRect();
	calculateSrcRect();
}



