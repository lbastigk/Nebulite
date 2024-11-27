#include "RenderObject.h"

#include <iostream>


//-----------------------------------------------------------
//Constructor

RenderObject::RenderObject() {
	JSONHandler::Set::Any(doc, namenKonvention.renderObject.isOverlay, false);
	JSONHandler::Set::Any(doc, namenKonvention.renderObject.positionX, 0);
	JSONHandler::Set::Any(doc, namenKonvention.renderObject.positionY, 0);
	JSONHandler::Set::Any(doc, namenKonvention.renderObject.pixelSizeX, 32);
	JSONHandler::Set::Any(doc, namenKonvention.renderObject.pixelSizeY, 32);
	JSONHandler::Set::Any(doc, namenKonvention.renderObject.imageLocation, "Resources/Sprites/TEST_BMP_SMALL.bmp");
	JSONHandler::Set::Any(doc, namenKonvention.renderObject.layer, 0);

	//for spritesheets
	JSONHandler::Set::Any(doc, namenKonvention.renderObject.isSpritesheet, false);
	JSONHandler::Set::Any(doc, namenKonvention.renderObject.spritesheetOffsetX, 0);
	JSONHandler::Set::Any(doc, namenKonvention.renderObject.spritesheetOffsetY, 0);
	JSONHandler::Set::Any(doc, namenKonvention.renderObject.spritesheetSizeX, 0);
	JSONHandler::Set::Any(doc, namenKonvention.renderObject.spritesheetSizeY, 0);

	//Move rules
	MoveRuleSet MoveRuleSet;
	JSONHandler::Set::subDoc(doc, namenKonvention.moveRuleSet._self, *MoveRuleSet.getDoc());

	//Build Rect on creation
	calculateDstRect();
	calculateSrcRect();
}
RenderObject::RenderObject(const RenderObject& other) {
	doc.CopyFrom(*(other.getDoc()), doc.GetAllocator());
	calculateDstRect();
	calculateSrcRect();
}
/*
RenderObject& RenderObject::operator=(const RenderObject& other) {  // Assignment operator overload
	if (this != &other) {
		dstRect = other.dstRect;
		JSONHandler::copyDoc(doc, other.getDoc());
	}
	return *this;
}
*/




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
	calculateDstRect();
	calculateSrcRect();
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

//-----------------------------------------------------------
void RenderObject::update() {
	//Temporary files
	rapidjson::Document tmpDoc;

	//Getting info from Renderobject subdoc
	JSONHandler::Get::subDoc(doc, namenKonvention.moveRuleSet._self, tmpDoc);

	//Pushing into temporary MoveruleSet object
	MoveRuleSet tmpMrs(tmpDoc);

	//Update
	tmpMrs.update(*this);
	//MoveRuleSet::update_static(*this, tmpDoc);

	//Pushing back into RenderObject doc
	JSONHandler::Set::subDoc(doc, namenKonvention.moveRuleSet._self, *tmpMrs.getDoc());

	//recalc rect
	calculateDstRect();
	calculateSrcRect();
}

void RenderObject::loadMoveSet(MoveRuleSet mrs) {
	//Temporary files
	rapidjson::Document tmpDoc;

	//Getting info from Renderobject subdoc
	JSONHandler::Get::subDoc(doc, namenKonvention.moveRuleSet._self, tmpDoc); //{"var":{moverules}}

	//Add moveset to doc
	for (auto it = mrs.getDoc()->MemberBegin(); it != mrs.getDoc()->MemberEnd(); ++it) {
		// Insert the member into the new document
		rapidjson::Document::AllocatorType& allocator = tmpDoc.GetAllocator();
		rapidjson::Value key(it->name, allocator);

		// add member
		tmpDoc.AddMember(key, it->value, allocator);
	}

	//Pushing back into RenderObject doc
	JSONHandler::Set::subDoc(doc, namenKonvention.moveRuleSet._self, tmpDoc);

	//Empty tmp
	JSONHandler::empty(tmpDoc);
}

void RenderObject::exampleMoveSet(std::string val) {
	//Template Moveset
	MoveRuleSet tmpMrs = MoveRuleSet::Examples::zickZack(namenKonvention.renderObject.positionX);

	//Pushing back into RenderObject doc
	JSONHandler::Set::subDoc(doc, namenKonvention.moveRuleSet._self, *tmpMrs.getDoc());
}

bool RenderObject::hasMoveSet() {
	rapidjson::Document mrs;
	JSONHandler::Get::subDoc(doc, namenKonvention.moveRuleSet._self, mrs);
	return JSONHandler::Get::keyAmount(mrs) != 0;
}

//--------------------------------------------------------------------------------------
// RenderObjectContainer

//-----------------------------------------------------------
//Constructor

RenderObjectContainer::RenderObjectContainer() {

}


//-----------------------------------------------------------
//Marshalling
std::string RenderObjectContainer::serialize() {
	// Initialize RapidJSON document
	rapidjson::Document doc;
	doc.SetObject();

	// Set up allocator
	rapidjson::Document::AllocatorType& allocator = doc.GetAllocator();

	// Set up array
	rapidjson::Value array(rapidjson::kArrayType);

	// Serialize all objects, add to array
	for (auto& vec1 : ObjectContainer) {
		for (auto& vec2 : vec1) {
			for (auto& batch : vec2) {
				for (auto& obj : batch) {
					std::string objStr = obj.serialize();

					// Parse the serialized string to a JSON object
					rapidjson::Document objDoc;
					objDoc.Parse(objStr.c_str());

					// Add to array as object
					if (objDoc.IsObject()) {
						// Create a new Value and copy objDoc into it
						rapidjson::Value objValue;
						objValue.CopyFrom(objDoc, allocator);

						// Push the copied value to the array
						array.PushBack(objValue, allocator);
					}
				}
			}
		}
	}

	// Add array to the document with a key
	doc.AddMember("objects", array, allocator);

	// Write to PrettyWriter
	rapidjson::StringBuffer buffer;
	rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(buffer);
	doc.Accept(writer);

	// Return as string
	return buffer.GetString(); // String should now be correct
}

void RenderObjectContainer::deserialize(const std::string& serialOrLink, int dispResX, int dispResY, int THREADSIZE) {
	// Deserialize and copy into "doc"
	rapidjson::Document doc = JSONHandler::deserialize(serialOrLink);

	// Check for parsing errors
	if (doc.HasParseError()) {
		std::cerr << "JSON parse error: " << doc.GetParseError() << " (at offset " << doc.GetErrorOffset() << ")" << std::endl;
		return;
	}

	// Check if the "objects" member exists and is an array
	if (doc.HasMember("objects") && doc["objects"].IsArray()) {
		const rapidjson::Value& arr = doc["objects"];

		// Loop through all objects in arr
		for (rapidjson::SizeType i = 0; i < arr.Size(); ++i) {
			const rapidjson::Value& item = arr[i];

			// Deserialize each item to RenderObject
			if (item.IsObject()) {
				// Convert the item to a string
				rapidjson::StringBuffer buffer;
				rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
				item.Accept(writer);

				std::string itemStr = buffer.GetString();

				// Deserialize the string into the RenderObject
				RenderObject ro;
				ro.deserialize(itemStr);

				// Append the RenderObject to your structure
				append(ro, dispResX, dispResY, THREADSIZE);
			}
			else {
				std::cerr << "Array item is not an object at index " << i << std::endl;
			}
		}
	}
	else {
		std::cerr << "'objects' not found or not an array in the document" << std::endl;
	}
}

//-----------------------------------------------------------
//-----------------------------------------------------------
// Pipeline

/*
void RenderObjectContainer::append(RenderObject toAppend, int dispResX, int dispResY, int THREADSIZE) {
	//new tile position
	unsigned int correspondingTileXpos;
	unsigned int correspondingTileYpos;

	// Get screen resolution
	//int dispResX = atoi(generalOptions.GetOption(namenKonvention.options.dispResX).c_str());
	//int dispResY = atoi(generalOptions.GetOption(namenKonvention.options.dispResY).c_str());

	// Calculate tile position based on screen resolution
	//+- in same container
	double valget;
	int64_t placeholder;
	

	valget = toAppend.valueGet<double>(namenKonvention.renderObject.positionX, 0.0);
	placeholder = (int64_t)(valget / (double)dispResX);
	if (placeholder < 0) {
		correspondingTileXpos = (unsigned int)(-placeholder);
	}
	else {
		correspondingTileXpos = (unsigned int)(placeholder);
	}

	valget = toAppend.valueGet<double>(namenKonvention.renderObject.positionY, 0.0);
	placeholder = (int64_t)(valget / (double)dispResY);
	if (placeholder < 0) {
		correspondingTileYpos = (unsigned int)(-placeholder);
	}
	else {
		correspondingTileYpos = (unsigned int)(placeholder);
	}

	// Check if tileXpos and tileYpos are within valid range before appending
	if (!isValidPosition(correspondingTileXpos, correspondingTileYpos)) {
		// Handle the case where the position is out of bounds
		// Create vector at that spot to append to

		// Make sure the new row exists
		while (ObjectContainer.size() <= correspondingTileYpos) {
			ObjectContainer.emplace_back(std::vector<std::vector<std::vector<RenderObject>>>());
		}

		// Make sure the new column exists
		while (ObjectContainer[correspondingTileYpos].size() <= correspondingTileXpos) {
			ObjectContainer[correspondingTileYpos].emplace_back();
		}
	}

	// Add ObjectInstance to the new spot
	bool appended = false;
	for (auto& batch : ObjectContainer[correspondingTileYpos][correspondingTileXpos]) {
		// Only append to batch if size limit isn't reached
		if (batch.size() < THREADSIZE) {
			batch.push_back(toAppend);
			appended = true;
			break;  // Exit the loop after appending to the first available batch
		}
	}

	// If not appended, add a new batch
	if (!appended) {
		ObjectContainer[correspondingTileYpos][correspondingTileXpos].emplace_back();
		// push into the last batch:
		auto& lastBatch = ObjectContainer[correspondingTileYpos][correspondingTileXpos].back();
		lastBatch.push_back(toAppend);
	}

	// Load the associated image
	//loadTexture(toAppend);
}

*/

void RenderObjectContainer::append(RenderObject toAppend, int dispResX, int dispResY, int THREADSIZE) {
    // new tile position
    unsigned int correspondingTileXpos;
    unsigned int correspondingTileYpos;

    // Calculate tile position based on screen resolution
    double valget;
    int64_t placeholder;

    // Calculate correspondingTileXpos using positionX
    valget = toAppend.valueGet<double>(namenKonvention.renderObject.positionX, 0.0);
    placeholder = (int64_t)(valget / (double)dispResX);
    correspondingTileXpos = (placeholder < 0) ? (unsigned int)(-placeholder) : (unsigned int)(placeholder);

    // Calculate correspondingTileYpos using positionY
    valget = toAppend.valueGet<double>(namenKonvention.renderObject.positionY, 0.0);
    placeholder = (int64_t)(valget / (double)dispResY);
    correspondingTileYpos = (placeholder < 0) ? (unsigned int)(-placeholder) : (unsigned int)(placeholder);

    // Ensure the position is valid, grow the ObjectContainer if necessary
    if (!isValidPosition(correspondingTileXpos, correspondingTileYpos)) {
        // Ensure the row exists
        while (ObjectContainer.size() <= correspondingTileYpos) {
            ObjectContainer.emplace_back(std::vector<std::vector<std::vector<RenderObject>>>());
        }

        // Ensure the column exists
        while (ObjectContainer[correspondingTileYpos].size() <= correspondingTileXpos) {
            ObjectContainer[correspondingTileYpos].emplace_back(std::vector<std::vector<RenderObject>>());
        }
    }

    // Try to append to an existing batch, respecting the THREADSIZE limit
    bool appended = false;
    for (auto& batch : ObjectContainer[correspondingTileYpos][correspondingTileXpos]) {
        if (batch.size() < THREADSIZE) {
            batch.push_back(toAppend);
            appended = true;
            break;
        }
    }

    // If no batch was available or full, create a new batch and append
    if (!appended) {
        ObjectContainer[correspondingTileYpos][correspondingTileXpos].emplace_back();  // Add new batch
        auto& lastBatch = ObjectContainer[correspondingTileYpos][correspondingTileXpos].back();
        lastBatch.push_back(toAppend);
    }

    // Load the associated image (assuming loadTexture is defined elsewhere)
    // loadTexture(toAppend);
}


void RenderObjectContainer::update_withThreads(int tileXpos, int tileYpos, int dispResX, int dispResY, int THREADSIZE) {
	//Thread vector
	std::vector<std::thread> threads;

	//--------------------------------------------------------------------------------
	// Update only tiles that might be visible
	// since one tile is size of screen, a max of 9 tiles
	for (int dX = (tileXpos == 0 ? 0 : -1); dX <= 1; dX++) {
		for (int dY = (tileYpos == 0 ? 0 : -1); dY <= 1; dY++) {
			if (isValidPosition(tileXpos + dX, tileYpos + dY)) {
				for (auto& batch : ObjectContainer[tileYpos + dY][tileXpos + dX]) {
					// Create thread for batch
					threads.emplace_back([this, &batch]() {
						// Perform batch update logic here
						// For example, update each object in the batch
						for (auto& obj : batch) {
							obj.update();
						}
						});
				}
			}
		}
	}

	//auto time2 = Time::gettime();

	//--------------------------------------------------------------------------------
	// Join all threads
	for (auto& thread : threads) {
		thread.join();
	}

	//auto time3 = Time::gettime();

	//--------------------------------------------------------------------------------
	//restructure the updated tiles
	update(tileXpos, tileYpos, dispResX, dispResY, THREADSIZE, true);
}

void RenderObjectContainer::update(int tileXpos, int tileYpos, int dispResX, int dispResY, int THREADSIZE, bool onlyRestructure) {
	// Calculate tile position based on screen resolution
	//+- in same container
	double valget;
	int64_t placeholder;

	//new tile position
	unsigned int correspondingTileXpos;
	unsigned int correspondingTileYpos;

	//--------------------------------------------------------------------------------
	// Update only tiles that might be visible
	// since one tile is size of screen, a max of 9 tiles

	std::vector<RenderObject> toReinsert;
	for (int dX = (tileXpos == 0 ? 0 : -1); dX <= 1; dX++) {
		for (int dY = (tileYpos == 0 ? 0 : -1); dY <= 1; dY++) {
			if (isValidPosition(tileXpos + dX, tileYpos + dY)) {
				for (auto& batch : ObjectContainer[tileYpos + dY][tileXpos + dX]) {
					std::vector<RenderObject> newBatch;
					for (auto& obj : batch) {
						if(!onlyRestructure){
							obj.update();
						}
						

						//-------------------------------------
						// Get new position in tile

						//X
						valget = obj.valueGet<double>(namenKonvention.renderObject.positionX, 0.0);
						placeholder = (int64_t)(valget / (double)dispResX);
						if (placeholder < 0) {
							correspondingTileXpos = (unsigned int)(-placeholder);
						}
						else {
							correspondingTileXpos = (unsigned int)(placeholder);
						}

						//Y
						valget = obj.valueGet<double>(namenKonvention.renderObject.positionY, 0.0); // Use positionY here
						placeholder = (int64_t)(valget / (double)dispResY);
						if (placeholder < 0) {
							correspondingTileYpos = (unsigned int)(-placeholder);
						}
						else {
							correspondingTileYpos = (unsigned int)(placeholder);
						}

						//-----------------------------------------
						// Check if it's in a new tile
						if (correspondingTileXpos != tileXpos + dX || correspondingTileYpos != tileYpos + dY) {
							toReinsert.push_back(obj);
						}
						else{
							newBatch.push_back(obj);
						}
					}
					batch = std::move(newBatch);
				}
			}
		}
	}
	for(const auto& obj : toReinsert){
		append(obj, dispResX, dispResY, THREADSIZE);
	}
}

bool RenderObjectContainer::isValidPosition(int x, int y) const {
    // Check if ObjectContainer is not empty
    if (!ObjectContainer.empty()) {
        // Check if y is within the valid range of rows
        if (y >= 0 && y < ObjectContainer.size()) {
            // Check if x is within the valid range of columns
            if (x >= 0 && x < ObjectContainer[y].size()) {
                // Check if there are any batches (inner vectors) in this position
                if (!ObjectContainer[y][x].empty()) {
                    // Return true if there is at least one non-empty batch in this position
                    return !ObjectContainer[y][x].empty();
                }
            }
        }
    }
    return false;
}

std::vector<std::vector<RenderObject>>& RenderObjectContainer::getContainerAt(int x, int y) {
	return ObjectContainer[y][x];
}

void RenderObjectContainer::purgeObjects() {
	// Release resources for ObjectContainer
	for (auto& vec1 : ObjectContainer) {
		for (auto& vec2 : vec1) {
			for (auto& batch : vec2) {
				batch.clear(); // Assuming RenderObject doesn't need explicit cleanup
			}
		}
	}
}

void RenderObjectContainer::purgeObjectsAt(int x, int y, int dispResX, int dispResY){
	// new tile position
    unsigned int correspondingTileXpos;
    unsigned int correspondingTileYpos;

    // Calculate tile position based on screen resolution
    int64_t placeholder;
    placeholder = (int64_t)(x / (double)dispResX);
    correspondingTileXpos = (placeholder < 0) ? (unsigned int)(-placeholder) : (unsigned int)(placeholder);
	placeholder = (int64_t)(y / (double)dispResX);
    correspondingTileYpos = (placeholder < 0) ? (unsigned int)(-placeholder) : (unsigned int)(placeholder);

	if (isValidPosition(correspondingTileXpos, correspondingTileYpos)) {
		auto& batches = ObjectContainer[correspondingTileYpos][correspondingTileXpos];
		std::vector<std::vector<RenderObject>> newBatches;

		for (auto& batch : batches) {
			std::vector<RenderObject> newBatch;

			for (auto& object : batch) {
				if (!(object.valueGet<int>(namenKonvention.renderObject.positionX) == x &&
					object.valueGet<int>(namenKonvention.renderObject.positionY) == y)) {
					// Retain objects that don't match the condition
					newBatch.push_back(object);
				}
			}

			// Only add non-empty batches to the newBatches vector
			if (!newBatch.empty()) {
				newBatches.push_back(std::move(newBatch));
			}
		}

		// Replace the old batches with the new ones
		ObjectContainer[correspondingTileYpos][correspondingTileXpos] = std::move(newBatches);
	
	}	

}

size_t RenderObjectContainer::getObjectCount() {
	// Calculate the total item count
	size_t totalCount = 0;

	for (auto& vec1 : ObjectContainer) {
		for (auto& vec2 : vec1) {
			for (auto& batch : vec2) {
				totalCount += batch.size();
			}
		}
	}
	return totalCount;
}

SDL_Texture* RenderObjectContainer::getTexture(int screenSizeX, int screenSizeY, SDL_Renderer *renderer, int tileXpos, int tileYpos, int Xpos, int Ypos, auto& TextureContainer){
	// Create a texture to use as a render target
	// Each tile grid is the size of screen, this way the entire screen is always tiled
	SDL_Texture* sceneTexture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, 3*screenSizeX, 3*screenSizeY);

	// Set the render target to the sceneTexture
	SDL_SetRenderTarget(renderer, sceneTexture);

	// Clear the render target (optional, depending on if you want transparency)
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
	SDL_RenderClear(renderer);

	SDL_Rect rect;
	int error;

	//Between dx +-1
	for (int dX = (tileXpos == 0 ? 0 : -1); dX <= 1; dX++) {
		// And dy +-1
		for (int dY = (tileYpos == 0 ? 0 : -1); dY <= 1; dY++) {
			// If valid
			if (isValidPosition(tileXpos + dX, tileYpos + dY)) {
				// For all batches inside
				for (auto& batch : (getContainerAt(tileXpos + dX, tileYpos + dY))) {
					// For all objects inside each batch
					for (auto& obj : batch) {
						//Texture loading is handled in append
						std::string innerdir = obj.valueGet<std::string>(namenKonvention.renderObject.imageLocation);
						obj.calculateSrcRect();

						rect = obj.getDstRect();
						rect.x -= Xpos;	//subtract camera posX
						rect.y -= Ypos;	//subtract camera posY

						// Render the texture to the window
						error = SDL_RenderCopy(renderer, TextureContainer[innerdir], obj.getSrcRect(), &rect);
						if (error != 0){
							std::cerr << "SDL Error while rendering Frame: " << error << std::endl;
						}
					}
				}
			}
		}
	}
	// Reset standard target
	SDL_SetRenderTarget(renderer, nullptr);

	return sceneTexture;
}