#include "RenderObject.h"


//-----------------------------------------------------------
//Constructor

RenderObject::RenderObject() {
	JSONHandler::Set::Any(doc, namenKonvention.renderObject.id,0);

	JSONHandler::Set::Any(doc, namenKonvention.renderObject.isOverlay, false);
	JSONHandler::Set::Any(doc, namenKonvention.renderObject.positionX, 0);
	JSONHandler::Set::Any(doc, namenKonvention.renderObject.positionY, 0);
	JSONHandler::Set::Any(doc, namenKonvention.renderObject.pixelSizeX, 32);
	JSONHandler::Set::Any(doc, namenKonvention.renderObject.pixelSizeY, 32);
	JSONHandler::Set::Any(doc, namenKonvention.renderObject.imageLocation, "Resources/Sprites/TEST_BMP_SMALL.bmp");
	JSONHandler::Set::Any(doc, namenKonvention.renderObject.layer, 0);

	JSONHandler::Set::Any(doc, namenKonvention.renderObject.deleteFlag, false);

	//for spritesheets
	JSONHandler::Set::Any(doc, namenKonvention.renderObject.isSpritesheet, false);
	JSONHandler::Set::Any(doc, namenKonvention.renderObject.spritesheetOffsetX, 0);
	JSONHandler::Set::Any(doc, namenKonvention.renderObject.spritesheetOffsetY, 0);
	JSONHandler::Set::Any(doc, namenKonvention.renderObject.spritesheetSizeX, 0);
	JSONHandler::Set::Any(doc, namenKonvention.renderObject.spritesheetSizeY, 0);

	// Set doc["invokes"] as an empty array
	doc.AddMember("invokes", rapidjson::Value(rapidjson::kArrayType), doc.GetAllocator());

	//Build Rect on creation
	calculateDstRect();
	calculateSrcRect();

	// Insert Invokes
	JSONHandler::Set::Any(doc, namenKonvention.renderObject.reloadInvokes, true);

	// Create text
	// Create a surface with the text
	JSONHandler::Set::Any(doc,namenKonvention.renderObject.textStr,"");
	JSONHandler::Set::Any(doc,namenKonvention.renderObject.textFontsize,0);
	JSONHandler::Set::Any(doc,namenKonvention.renderObject.flagCalculate,true);
}


RenderObject::RenderObject(const RenderObject& other) {
	doc.CopyFrom(*(other.getDoc()), doc.GetAllocator());
	valueSet(namenKonvention.renderObject.flagCalculate,true);
	calculateDstRect();
	calculateSrcRect();
}

RenderObject& RenderObject::operator=(const RenderObject& other) {  // Assignment operator overload
	if (this != &other) {
		dstRect = other.dstRect;
		JSONHandler::copyDoc(doc, other.getDoc());
	}
	valueSet(namenKonvention.renderObject.flagCalculate,true);
	return *this;
}





//-----------------------------------------------------------
//Destructor
RenderObject::~RenderObject() {
	if (doc.IsArray()) {
		doc.Empty();
	}
};


//-----------------------------------------------------------
//Marshalling

std::string RenderObject::serialize() {
	return JSONHandler::serialize(doc);
}

void RenderObject::deserialize(std::string serialOrLink) {
	doc = JSONHandler::deserialize(serialOrLink);
	//std::cerr << "Doc was deserialized! From: \n" << serialOrLink << "\n to: \n" << JSONHandler::serialize(doc) << std::endl << std::endl;

	// Prerequisites
	valueSet(namenKonvention.renderObject.reloadInvokes,true);

	calculateDstRect();
	calculateSrcRect();
}

void RenderObject::calculateText(SDL_Renderer* renderer,TTF_Font* font,int renderer_X, int renderer_Y){
	float scalar = 1;
	float fontSize = valueGet<float>(namenKonvention.renderObject.textFontsize);
	std::string text = valueGet<std::string>(namenKonvention.renderObject.textStr);
	textRect.x = valueGet<float>(namenKonvention.renderObject.positionX) + valueGet<float>(namenKonvention.renderObject.textDx) - renderer_X;
	textRect.y = valueGet<float>(namenKonvention.renderObject.positionY) + valueGet<float>(namenKonvention.renderObject.textDy) - renderer_Y;
	textRect.w = scalar * fontSize * text.length(); // Width based on text length
	textRect.h = (int)((float)fontSize * 1.5 * scalar);
	if(valueGet<bool>(namenKonvention.renderObject.flagCalculate,true)==true){
		SDL_Color textColor = { 255, 255, 255, 255 }; // White color
		textSurface = TTF_RenderText_Solid(font, text.c_str(), textColor);
		textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
		valueSet(namenKonvention.renderObject.flagCalculate,false);
	}
	
}

SDL_Texture& RenderObject::getTextTexture(){
	return *textTexture;
}

SDL_Rect* RenderObject::getTextRect(){
	return &textRect;
}

rapidjson::Document* RenderObject::getDoc() const {
	return const_cast<rapidjson::Document*>(&doc);
}


SDL_Rect& RenderObject::getDstRect() {
	return dstRect;
}
void RenderObject::calculateDstRect() {
	dstRect = {
		(int)valueGet<double>(namenKonvention.renderObject.positionX),
		(int)valueGet<double>(namenKonvention.renderObject.positionY),
		(int)valueGet<double>(namenKonvention.renderObject.pixelSizeX), // Set the desired width
		(int)valueGet<double>(namenKonvention.renderObject.pixelSizeY), // Set the desired height
	};
};
SDL_Rect* RenderObject::getSrcRect() {
	if (valueGet<bool>(namenKonvention.renderObject.isSpritesheet)) {
		return &srcRect;
	}
	else {
		return nullptr;
	}
}
void RenderObject::calculateSrcRect() {
	// Check if the object is a sprite
	if (valueGet<bool>(namenKonvention.renderObject.isSpritesheet)) {
		int offsetX = (int)valueGet<double>(namenKonvention.renderObject.spritesheetOffsetX,0);
		int offsetY = (int)valueGet<double>(namenKonvention.renderObject.spritesheetOffsetY,0);
		int spriteWidth = valueGet<int>(namenKonvention.renderObject.spritesheetSizeX,0);
		int spriteHeight = valueGet<int>(namenKonvention.renderObject.spritesheetSizeY,0);
		
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
std::vector<InvokeTriple> parseInvokeTriples(const rapidjson::Value& arr) {
    std::vector<InvokeTriple> res;
    if (arr.IsArray()) {
        for (auto& kv : arr.GetArray()) {
            if (kv.IsObject() &&
                kv.HasMember("changeType") &&
                kv.HasMember("key") &&
                kv.HasMember("value")) {
                InvokeTriple triple;
                triple.changeType = kv["changeType"].GetString();
                triple.key = kv["key"].GetString();
                triple.value = kv["value"].GetString();
                res.push_back(std::move(triple));
            }
        }
    }
    return res;
}

void RenderObject::reloadInvokes(std::shared_ptr<RenderObject> this_shared) {
    cmds_general.clear();
    cmds_internal.clear();

    auto& doc = *this_shared.get()->getDoc(); // convenience reference
    if (doc.HasMember("invokes") && doc["invokes"].IsArray() && doc["invokes"].Size()) {
        rapidjson::Value& invokes = doc["invokes"];
        for (rapidjson::SizeType i = 0; i < invokes.Size(); ++i) {
			//--------------------------
			// Get invoke
            rapidjson::Document serializedInvoke;
            if (invokes[i].IsString()) {
				serializedInvoke = JSONHandler::deserialize(invokes[i].GetString());
            } else if (invokes[i].IsObject()) {
                serializedInvoke.CopyFrom(invokes[i], serializedInvoke.GetAllocator());
            } else {
                continue;
            }

			if (serializedInvoke.IsObject()){
				//--------------------------
				// Build entry
				InvokeEntry entry;
				entry.selfPtr = this_shared;
				entry.logicalArg = 	JSONHandler::Get::Any<std::string>(serializedInvoke, "logicalArg", "");
				entry.isGlobal = 	JSONHandler::Get::Any<bool>(serializedInvoke, "isGlobal", true);

				entry.invokes_self = 	parseInvokeTriples(serializedInvoke["self_invokes"]);
				entry.invokes_other = 	parseInvokeTriples(serializedInvoke["other_invokes"]);
				entry.invokes_global = 	parseInvokeTriples(serializedInvoke["global_invokes"]);

				entry.functioncalls.clear();
				if (serializedInvoke.HasMember("functioncalls") && serializedInvoke["functioncalls"].IsArray()) {
					for (auto& fn : serializedInvoke["functioncalls"].GetArray()) {
						if (fn.IsString()) {
							entry.functioncalls.push_back(fn.GetString());
						}
					}
				}

				// DEBUG: check entry:
				/*
				std::cout << "[DEBUG] Invoke Entry loaded:" << std::endl;
				std::cout << "Global:             " << entry.isGlobal << " | " << entry.logicalArg << std::endl;
				std::cout << "Size self:          " << entry.invokes_self.size() << std::endl;
				std::cout << "Size other:         " << entry.invokes_other.size() << std::endl;
				std::cout << "Size global:        " << entry.invokes_global.size() << std::endl;
				std::cout << "Size functioncalls: " << entry.functioncalls.size() << std::endl;
				//*/
				
				// Append
				auto ptr = std::make_shared<InvokeEntry>(std::move(entry));

				if(entry.isGlobal){
					cmds_general.push_back(ptr);
				}
				else{
					cmds_internal.push_back(ptr);
				}
			}
        }
    }

    JSONHandler::Set::Any(doc, namenKonvention.renderObject.reloadInvokes, false);
}

//-----------------------------------------------------------
// TODO: Change this to be a two-part:
//
// - first check against all other in list WITHOUT updating
//   (allows for threading)
// - store pointer pairs as std::vector<std::pair<RenderObject& RenderObject&>>
// - after object pre-update, call actual update via invoke class that changes all objects
void RenderObject::update(Invoke* globalInvoke, std::shared_ptr<RenderObject> this_shared) {
	//------------------------------------
	// Check all invokes
	if (globalInvoke) {
		// Reload invokes if needed
		if (valueGet<int>(namenKonvention.renderObject.reloadInvokes,true)){
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



