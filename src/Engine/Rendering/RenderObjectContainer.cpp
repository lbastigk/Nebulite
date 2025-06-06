//--------------------------------------------------------------------------------------
// RenderObjectContainer
#include "RenderObjectContainer.h"

//-----------------------------------------------------------
//Constructor

RenderObjectContainer::RenderObjectContainer() {

}


//-----------------------------------------------------------
//Marshalling
std::string RenderObjectContainer::serialize() {
	//---------------------------------------
	// Setup

	// Initialize RapidJSON document
	rapidjson::Document doc;
	doc.SetObject();

	// Set up allocator
	rapidjson::Document::AllocatorType& allocator = doc.GetAllocator();

	// Set up array
	rapidjson::Value array(rapidjson::kArrayType);

	//---------------------------------------
	// Get all objects in container+
	//*
	// Get string
	std::vector<rapidjson::Document*> objects;
	std::vector<std::string> serializedObjects;
	for (auto& vec1 : ObjectContainer) {
		for (auto& vec2 : vec1) {
			for (auto& batch : vec2) {
				for (auto& obj : batch) {
					serializedObjects.push_back(obj.get()->serialize());	
				}
			}
		}
	}
	// Re-Serialize and insert
	for (auto& string : serializedObjects) {
		rapidjson::Document parsed;
		parsed.Parse(string.c_str());
		if (parsed.HasParseError() || !parsed.IsObject()) {
			std::cerr << "[ERROR] Invalid JSON input!" << std::endl;
			continue;
		}
		rapidjson::Value copiedValue(parsed, allocator);  // Deep copy using correct allocator
		array.PushBack(copiedValue, allocator);
	}
	//*/

	// Add array to the document with a key
	doc.AddMember("objects", array, allocator);

	//---------------------------------------
	// Return as string
	return JSONHandler::serialize(doc);
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
				auto ptr = std::make_shared<RenderObject>(std::move(ro));

				// Append the RenderObject to your structure
				append(ptr, dispResX, dispResY, THREADSIZE);
			}
			else {
				RenderObject ro;
				ro.deserialize(item.GetString());
				auto ptr = std::make_shared<RenderObject>(std::move(ro));
				append(ptr, dispResX, dispResY, THREADSIZE);
				//std::cerr << "Array item is not an object at index " << i << std::endl;
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

void RenderObjectContainer::append(std::shared_ptr<RenderObject> toAppend, int dispResX, int dispResY, int THREADSIZE) {
    // new tile position
    unsigned int correspondingTileXpos;
    unsigned int correspondingTileYpos;

    // Calculate tile position based on screen resolution
    double valget;
    int64_t placeholder;

    // Calculate correspondingTileXpos using positionX
    valget = toAppend.get()->valueGet<double>(namenKonvention.renderObject.positionX.c_str(), 0.0);
    placeholder = (int64_t)(valget / (double)dispResX);
    correspondingTileXpos = (placeholder < 0) ? (unsigned int)(-placeholder) : (unsigned int)(placeholder);

    // Calculate correspondingTileYpos using positionY
    valget = toAppend.get()->valueGet<double>(namenKonvention.renderObject.positionY.c_str(), 0.0);
    placeholder = (int64_t)(valget / (double)dispResY);
    correspondingTileYpos = (placeholder < 0) ? (unsigned int)(-placeholder) : (unsigned int)(placeholder);

    // Ensure the position is valid, grow the ObjectContainer if necessary
    if (!isValidPosition(correspondingTileXpos, correspondingTileYpos)) {
        // Ensure the row exists
        while (ObjectContainer.size() <= correspondingTileYpos) {
            ObjectContainer.emplace_back(std::vector<std::vector<std::vector<std::shared_ptr<RenderObject>>>>());
        }

        // Ensure the column exists
        while (ObjectContainer[correspondingTileYpos].size() <= correspondingTileXpos) {
            ObjectContainer[correspondingTileYpos].emplace_back(std::vector<std::vector<std::shared_ptr<RenderObject>>>());
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
}


void RenderObjectContainer::update_withThreads(int tileXpos, int tileYpos, int dispResX, int dispResY, int THREADSIZE,Invoke* globalInvoke) {
	std::cerr << "Update with Threads called! This function is currently not supported!" << std::endl;
	
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
					threads.emplace_back([this, &batch, globalInvoke]() {
						// Perform batch update logic here
						// For example, update each object in the batch
						for (auto& obj : batch) {
							obj->update(globalInvoke,obj);
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
	update(tileXpos, tileYpos, dispResX, dispResY, THREADSIZE,globalInvoke, true);
}

//*
void RenderObjectContainer::update(int tileXpos, int tileYpos, int dispResX, int dispResY, int THREADSIZE, Invoke* globalInvoke, bool onlyRestructure) {
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
	// [P] - Tile with Player
	// [ ] - loaded Tiles
	//
	// [ ][ ][ ]
	// [ ][P][ ]
	// [ ][ ][ ]

	std::vector<std::shared_ptr<RenderObject>> toReinsert;
	for (int dX = (tileXpos == 0 ? 0 : -1); dX <= 1; dX++) {
		for (int dY = (tileYpos == 0 ? 0 : -1); dY <= 1; dY++) {
			if (isValidPosition(tileXpos + dX, tileYpos + dY)) {
				for (auto& batch : ObjectContainer[tileYpos + dY][tileXpos + dX]) {
					std::vector<std::shared_ptr<RenderObject>> newBatch;
					for (auto& obj : batch) {
						if(!onlyRestructure){
							// Updates local invokes, sends global ones up to Invoke Object
							obj->update(globalInvoke,obj);	// leaks around 2.3 KiB per call
						}
						
						//-----------------------------------------
						// Check delete flag
						if (!obj->valueGet(namenKonvention.renderObject.deleteFlag.c_str(),false)){
							
							//-------------------------------------
							// Get new position in tile
							//X
							valget = obj->valueGet<double>(namenKonvention.renderObject.positionX.c_str(), 0.0);
							placeholder = abs((int64_t)(valget / (double)dispResX));
							correspondingTileXpos = (unsigned int)(placeholder);

							//Y
							valget = obj->valueGet<double>(namenKonvention.renderObject.positionY.c_str(), 0.0);
							placeholder = abs((int64_t)(valget / (double)dispResY));
							correspondingTileYpos = (unsigned int)(placeholder);

							//-----------------------------------------
							// Check if it's in a new tile
							if (correspondingTileXpos != tileXpos + dX || correspondingTileYpos != tileYpos + dY) {
								toReinsert.push_back(obj);
							}
							else{
								newBatch.push_back(obj);
							}
						}
						else{
							//dont reinsert: gets deleted
						}
					}
					// Give new batch of objects still in same tile
					//batch = std::move(newBatch);
					batch.swap(newBatch);

					newBatch.clear();
					newBatch.shrink_to_fit();
				}
			}
		}
	}

	// Add objects back to renderer that are now in a different tile position
	for(const auto& obj : toReinsert){
		append(obj, dispResX, dispResY, THREADSIZE);
	}
	toReinsert.clear();
	toReinsert.shrink_to_fit();
}
//*/

void RenderObjectContainer::reinsertAllObjects(int dispResX, int dispResY, int THREADSIZE) {
	std::vector<std::shared_ptr<RenderObject>> toReinsert;

	for (auto& Y : ObjectContainer) {
		for (auto& X : Y) {
			for (auto& batch : X) {
				for (auto& obj : batch) {
					toReinsert.push_back(obj); // Keep shared_ptr reference
				}
				batch.clear(); // Clear batch after move
			}
			X.clear(); // Clear tile row
		}
		Y.clear(); // Clear tile column
	}
	ObjectContainer.clear(); // Fully reset

	// Reinsert with consistent pointer ownership
	for (const auto& obj : toReinsert) {
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

std::vector<std::vector<std::shared_ptr<RenderObject>>>& RenderObjectContainer::getContainerAt(int x, int y) {
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
		std::vector<std::vector<std::shared_ptr<RenderObject>>> newBatches;

		for (auto& batch : batches) {
			std::vector<std::shared_ptr<RenderObject>> newBatch;

			for (auto& object : batch) {
				if (!(object->valueGet<int>(namenKonvention.renderObject.positionX.c_str()) == x &&
					object->valueGet<int>(namenKonvention.renderObject.positionY.c_str()) == y)) {
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

size_t RenderObjectContainer::getObjectCountAtTile(int x, int y) {
	// Calculate the total item count
	size_t totalCount = 0;

	if(isValidPosition(x,y)){
		for (auto& batch : ObjectContainer[y][x]) {
				totalCount += batch.size();
			}
		return totalCount;
	}
	else{
		return 0;
	}
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
						std::string innerdir = obj->valueGet<std::string>(namenKonvention.renderObject.imageLocation.c_str());
						obj->calculateSrcRect();

						rect = obj->getDstRect();
						rect.x -= Xpos;	//subtract camera posX
						rect.y -= Ypos;	//subtract camera posY

						// Render the texture to the window
						error = SDL_RenderCopy(renderer, TextureContainer[innerdir], obj->getSrcRect(), &rect);
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