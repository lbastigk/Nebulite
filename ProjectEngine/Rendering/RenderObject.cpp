#pragma once

#include <thread>

#include "NamenKonventionen.cpp"
#include "JSONHandler.cpp"
#include "SDL.h"
#include "MoveRuleSet.cpp"

class RenderObject {
public:
	//-----------------------------------------------------------
	//Constructor

	RenderObject() {
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
	// Copy Constructor
	RenderObject(const RenderObject& other) {
		dstRect = other.dstRect;
		doc.CopyFrom(*(other.getDoc()), doc.GetAllocator());
		//calculateRect();
	}
	RenderObject& operator=(const RenderObject& other) {  // Assignment operator overload
		if (this != &other) {
			dstRect = other.dstRect;
			JSONHandler::copyDoc(doc, other.getDoc());
		}
		return *this;
	}

	//-----------------------------------------------------------
	//Destructor
	~RenderObject() {
		if (doc.IsArray()) {
			doc.Empty();
		}
	};


	//-----------------------------------------------------------
	//Marshalling

	std::string serialize() {
		return JSONHandler::serialize(doc);
	}

	void deserialize(std::string serialOrLink) {
		doc = JSONHandler::deserialize(serialOrLink);
	}

	//-----------------------------------------------------------
	// Setting/Getting specific values
	template <typename T> void valueSet(std::string key, const T data) {
		JSONHandler::Set::Any(doc, key, data);
		calculateDstRect();
		calculateSrcRect();
	}

	template <typename T> T valueGet(std::string key, const T& defaultValue = T()){
		return JSONHandler::Get::Any<T>(doc, key, defaultValue);
	}

	rapidjson::Document* getDoc() const {
		return const_cast<rapidjson::Document*>(&doc);
	}

	SDL_Rect& getDstRect() {
		return dstRect;
	}
	void calculateDstRect() {
		dstRect = {
			(int)valueGet<double>(namenKonvention.renderObject.positionX),
			(int)valueGet<double>(namenKonvention.renderObject.positionY),
			(int)valueGet<double>(namenKonvention.renderObject.pixelSizeX), // Set the desired width
			(int)valueGet<double>(namenKonvention.renderObject.pixelSizeY), // Set the desired height
		};
	};
	SDL_Rect* getSrcRect() {
		if (valueGet<bool>(namenKonvention.renderObject.isSpritesheet)) {
			return &srcRect;
		}
		else {
			return nullptr;
		}
	}
	void calculateSrcRect() {
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
	void update() {
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

	void loadMoveSet(MoveRuleSet mrs) {
		//Temporary files
		rapidjson::Document tmpDoc;

		//Getting info from Renderobject subdoc
		JSONHandler::Get::subDoc(doc, namenKonvention.moveRuleSet._self, tmpDoc); //{"var":{moverules}}

		//Add moveset to doc
		for (auto it = mrs.getDoc()->MemberBegin(); it != mrs.getDoc()->MemberEnd(); ++it) {
			// Insert the member into the new document
			rapidjson::Document::AllocatorType& allocator = tmpDoc.GetAllocator();
			rapidjson::Value key(it->name, allocator);
			tmpDoc.AddMember(key, it->value, allocator);
		}

		//Pushing back into RenderObject doc
		JSONHandler::Set::subDoc(doc, namenKonvention.moveRuleSet._self, tmpDoc);

		//Empty tmp
		JSONHandler::empty(tmpDoc);
	}

	void exampleMoveSet(std::string val = namenKonvention.renderObject.positionX) {
		//Template Moveset
		MoveRuleSet tmpMrs = MoveRuleSet::Examples::zickZack(namenKonvention.renderObject.positionX);

		//Pushing back into RenderObject doc
		JSONHandler::Set::subDoc(doc, namenKonvention.moveRuleSet._self, *tmpMrs.getDoc());
	}

	//TODO
	bool hasMoveSet() {
		rapidjson::Document mrs;
		JSONHandler::Get::subDoc(doc, namenKonvention.moveRuleSet._self, mrs);
		return JSONHandler::Get::keyAmount(mrs) != 0;
	}

private:
	rapidjson::Document doc;

	SDL_Rect dstRect;
	SDL_Rect srcRect;
};

class RenderObjectContainer {
public:
	//-----------------------------------------------------------
	//Constructor

	RenderObjectContainer() {

	}


	//-----------------------------------------------------------
	//Marshalling
	std::string serialize() {
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


	void deserialize(const std::string& serialOrLink, int dispResX, int dispResY, int THREADSIZE) {

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

	void append(RenderObject toAppend, int dispResX, int dispResY, int THREADSIZE) {
		//new tile position
		unsigned int correspondingTileXpos;
		unsigned int correspondingTileYpos;

		// Get screen resolution
		//int dispResX = atoi(generalOptions.GetOption(namenKonvention.options.dispResX).c_str());
		//int dispResY = atoi(generalOptions.GetOption(namenKonvention.options.dispResY).c_str());

		// Calculate tile position based on screen resolution
		//+- in same container
		double valget;
		INT64 placeholder;

		valget = toAppend.valueGet<double>(namenKonvention.renderObject.positionX, 0.0);
		placeholder = (INT64)(valget / (double)dispResX);
		if (placeholder < 0) {
			correspondingTileXpos = (unsigned int)(-placeholder);
		}
		else {
			correspondingTileXpos = (unsigned int)(placeholder);
		}

		valget = toAppend.valueGet<double>(namenKonvention.renderObject.positionX, 0.0);
		placeholder = (INT64)(valget / (double)dispResY);
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

	void update_withThreads(int tileXpos, int tileYpos, int dispResX, int dispResY, int THREADSIZE) {
		//auto time1 = Time::gettime();

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
								// Your update logic for each object in the batch
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

		double valget;
		INT64 placeholder;

		unsigned int correspondingTileXpos;
		unsigned int correspondingTileYpos;

		for (int dX = (tileXpos == 0 ? 0 : -1); dX <= 1; dX++) {
			for (int dY = (tileYpos == 0 ? 0 : -1); dY <= 1; dY++) {
				if (isValidPosition(tileXpos + dX, tileYpos + dY)) {
					for (auto& batch : ObjectContainer[tileYpos + dY][tileXpos + dX]) {
						for (auto& obj : batch) {
							//-------------------------------------
							// Get new position in tile

							//X
							valget = obj.valueGet<double>(namenKonvention.renderObject.positionX, 0.0);
							placeholder = (INT64)(valget / (double)dispResX);
							if (placeholder < 0) {
								correspondingTileXpos = (unsigned int)(-placeholder);
							}
							else {
								correspondingTileXpos = (unsigned int)(placeholder);
							}

							//Y
							valget = obj.valueGet<double>(namenKonvention.renderObject.positionY, 0.0); // Use positionY here
							placeholder = (INT64)(valget / (double)dispResY);
							if (placeholder < 0) {
								correspondingTileYpos = (unsigned int)(-placeholder);
							}
							else {
								correspondingTileYpos = (unsigned int)(placeholder);
							}

							//-----------------------------------------
							// Check if it's in a new tile
							if (correspondingTileXpos != tileXpos + dX || correspondingTileYpos != tileYpos + dY) {
								// Pop render object out of container
								auto& tileContainer = ObjectContainer[correspondingTileYpos][correspondingTileXpos];
								for (auto& batch : tileContainer) {
									batch.erase(std::remove_if(
										batch.begin(),
										batch.end(),
										[&obj](const RenderObject& objInTile) {
											return &objInTile == &obj;
										}
									), batch.end());
								}

								// Call append function to re-insert
								append(obj,dispResX,dispResY,THREADSIZE);
							}
						}


					}
				}
			}
		}
	}

	void update(int tileXpos, int tileYpos, int dispResX, int dispResY, int THREADSIZE) {
		// Calculate tile position based on screen resolution
		//+- in same container
		double valget;
		INT64 placeholder;

		//new tile position
		unsigned int correspondingTileXpos;
		unsigned int correspondingTileYpos;

		//--------------------------------------------------------------------------------
		// Update only tiles that might be visible
		// since one tile is size of screen, a max of 9 tiles
		for (int dX = (tileXpos == 0 ? 0 : -1); dX <= 1; dX++) {
			for (int dY = (tileYpos == 0 ? 0 : -1); dY <= 1; dY++) {
				if (isValidPosition(tileXpos + dX, tileYpos + dY)) {
					for (auto& batch : ObjectContainer[tileYpos + dY][tileXpos + dX]) {
						for (auto& obj : batch) {
							obj.update();

							//-------------------------------------
							// Get new position in tile

							//X
							valget = obj.valueGet<double>(namenKonvention.renderObject.positionX, 0.0);
							placeholder = (INT64)(valget / (double)dispResX);
							if (placeholder < 0) {
								correspondingTileXpos = (unsigned int)(-placeholder);
							}
							else {
								correspondingTileXpos = (unsigned int)(placeholder);
							}

							//Y
							valget = obj.valueGet<double>(namenKonvention.renderObject.positionY, 0.0); // Use positionY here
							placeholder = (INT64)(valget / (double)dispResY);
							if (placeholder < 0) {
								correspondingTileYpos = (unsigned int)(-placeholder);
							}
							else {
								correspondingTileYpos = (unsigned int)(placeholder);
							}

							//-----------------------------------------
							// Check if it's in a new tile
							if (correspondingTileXpos != tileXpos + dX || correspondingTileYpos != tileYpos + dY) {
								// Pop render object out of container
								auto& tileContainer = ObjectContainer[correspondingTileYpos][correspondingTileXpos];
								for (auto& batch : tileContainer) {
									batch.erase(std::remove_if(
										batch.begin(),
										batch.end(),
										[&obj](const RenderObject& objInTile) {
											return &objInTile == &obj;
										}
									), batch.end());
								}

								// Call append function to re-insert (assuming it's a member function of the current class)
								append(obj,dispResX,dispResY,THREADSIZE);
							}
						}
					}
				}
			}
		}

	}

	bool isValidPosition(int x, int y) const {
		// Check if ObjectContainer is not empty
		if (!ObjectContainer.empty()) {
			// Check if y is within the valid range of rows
			if (y >= 0 && y < ObjectContainer.size()) {
				// Check if x is within the valid range of columns for the given row
				return x >= 0 && x < ObjectContainer[y].size();
			}
		}
		return false;
	}

	auto& getContainerAt(int x, int y) {
		return ObjectContainer[x][y];
	}

	void purgeObjects() {
		// Release resources for ObjectContainer
		for (auto& vec1 : ObjectContainer) {
			for (auto& vec2 : vec1) {
				for (auto& batch : vec2) {
					batch.clear(); // Assuming RenderObject doesn't need explicit cleanup
				}
			}
		}
	}

	size_t getObjectCount() {
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

private:
	//tileY		tileX		#			Batches		of Objects
	std::vector<std::vector<std::vector<std::vector<RenderObject>>>> ObjectContainer;

	class copyTemplates {

		void forEachObject(std::vector<std::vector<std::vector<std::vector<RenderObject>>>> ObjectContainer) {
			for (auto& vec1 : ObjectContainer) {
				for (auto& vec2 : vec1) {
					for (auto& batch : vec2) {
						batch.clear(); // Assuming RenderObject doesn't need explicit cleanup
					}
				}
			}
		}
	};
};