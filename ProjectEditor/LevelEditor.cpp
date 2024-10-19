#include "LevelEditor.h"

//---------------------------------------------
// LevelEditor Class

LevelEditor::LevelEditor() {
	Display.start();

	//Create userInputMap:
	optM.attachFunction(std::bind(&LevelEditor::placeItem,this),"place","...");
	optM.attachFunction(std::bind(&LevelEditor::deleteItem,this),"delete","...");
	optM.attachFunction(std::bind(&LevelEditor::clearConsole,this),"clear","...");
	optM.attachFunction(std::bind(&LevelEditor::serializeEnvironment,this),"serialize","...");
	optM.attachFunction(std::bind(&LevelEditor::countObjects,this),"count","...");
	optM.attachFunction(std::bind(&LevelEditor::save,this),"save","...");

	
	optM.changeType(OptionsMenu::typeConsole);
	clearConsole();
}

void LevelEditor::update() {
	Display.update();
	levelEditorStatus = optM.update() != OptionsMenu::statusExit;
}

void LevelEditor::saveEnvironment() {
	FileManagement::WriteFile(environmentLink.c_str(), Display.serializeRendererEnvironment());
}

void LevelEditor::loadEnvironment(std::string name) {
	std::string serialFile = FileManagement::LoadFile(name);
	Display.deserializeEnvironment(serialFile);
	environmentLink = name;
}

//Check if Level Editor is still active
bool LevelEditor::status() {
	return levelEditorStatus;
}

//Console input functions
void LevelEditor::placeItem() {
	//Create and append renderobject to level at selection position
	RenderObject ro;

	if (getRenderObjectFromList(ro)) {
		ro.valueSet(namenKonvention.renderObject.positionX, Display.getSelectionX());
		ro.valueSet(namenKonvention.renderObject.positionY, Display.getSelectionY());
		Display.appendObject(ro);
	}
}

void LevelEditor::deleteItem() {
	Display.deleteObject();
}

void LevelEditor::clearConsole() {
	Platform::clearScreen();
	std::cout << "DSA Engine Editor V0.1" << "\n";
	std::cout << "........................................." << "\n";
}

void LevelEditor::serializeEnvironment() {
	std::cout << Display.serializeRendererEnvironment();
}

void LevelEditor::countObjects(){
	std::cout << std::to_string(Display.objectCount());
}

void LevelEditor::save() {
	saveEnvironment();
}

// Choosing from a list of created renderobjects in the RenderObjects directory
bool LevelEditor::getRenderObjectFromList(RenderObject& ro) {
	std::string dir = FileManagement::currentDir();
	std::string fullDir = FileManagement::CombinePaths(dir, std::string("Resources/Renderobjects"));
	FileManagement::FileTypeCollector ftc(fullDir,".txt",true);
	auto list = ftc.getFileDirectories();
	std::string listAsString;

	OptionsMenu roEntries;
	roEntries.setTextBefore("Choose a Renderobject to place\n\n");
	for (auto entry : list) {
		roEntries.attachFunction([](){},entry);	//Lambda expresion to attach an empty function
	}

	int opt = 0;
	while(opt == 0){
		opt = optM.update();
	}
	if (opt == -1) {
		return false;
	}
	else {
		std::string link = list.at(opt-1);
		std::string file = FileManagement::LoadFile(link);
		ro.deserialize(file);
	}
}

//---------------------------------------------
// LevelEditor::Display Class
LevelEditor::Display::Display() {
	SDL_ShowCursor(SDL_DISABLE); // This hides the cursor
}

void LevelEditor::Display::start() {
	Renderer.setFPS(60);

	Cursor.valueSet(namenKonvention.renderObject.pixelSizeX, 24);
	Cursor.valueSet(namenKonvention.renderObject.pixelSizeY, 24);
	Cursor.valueSet(namenKonvention.renderObject.imageLocation, "Resources/Cursor/Drakensang.png");
	Cursor.valueSet(namenKonvention.renderObject.layer, (int)Environment::RenderObjectLayers::menue);

	Selection.valueSet(namenKonvention.renderObject.pixelSizeX, 32);
	Selection.valueSet(namenKonvention.renderObject.pixelSizeX, 32);
	Selection.valueSet(namenKonvention.renderObject.imageLocation, "Resources/Editor/Selection/Selection_00000.png");
	Selection.valueSet(namenKonvention.renderObject.positionX, -100);
	Selection.valueSet(namenKonvention.renderObject.positionY, -100);
	Selection.valueSet(namenKonvention.renderObject.layer, (int)Environment::RenderObjectLayers::menue);
}

void LevelEditor::Display::update() {
	//Mouse state
	uint32_t mouseState = SDL_GetMouseState(&MousePosX, &MousePosY);
	
	//Append Cursor obj at Mouse position
	Cursor.valueSet(namenKonvention.renderObject.positionX, MousePosX);
	Cursor.valueSet(namenKonvention.renderObject.positionY, MousePosY);

	//Create Selection position
	switch (mouseState & (SDL_BUTTON_LMASK | SDL_BUTTON_RMASK | SDL_BUTTON_MMASK | SDL_BUTTON_X1MASK | SDL_BUTTON_X2MASK)) {
	case SDL_BUTTON_LMASK:
		// Left mouse button pressed
		// Handle left mouse button press
		Selection.valueSet(namenKonvention.renderObject.positionX, MousePosX - (MousePosX % 32));
		Selection.valueSet(namenKonvention.renderObject.positionY, MousePosY - (MousePosY % 32));
		break;
	case SDL_BUTTON_RMASK:
		// Right mouse button pressed
		// Handle right mouse button press
		Selection.valueSet(namenKonvention.renderObject.positionX, -100);
		Selection.valueSet(namenKonvention.renderObject.positionY, -100);
		break;
	case SDL_BUTTON_MMASK:
		// Middle mouse button pressed
		// Handle middle mouse button press
		break;
	case SDL_BUTTON_X1MASK:
		// X1 mouse button pressed (usually back or extra mouse button)
		// Handle X1 mouse button press
		break;
	case SDL_BUTTON_X2MASK:
		// X2 mouse button pressed (usually forward or extra mouse button)
		// Handle X2 mouse button press
		break;
	default:
		// No mouse buttons pressed
		break;
	}

	// Append additional objects
	Renderer.append(Cursor);
	Renderer.append(Selection);

	//Render
	Renderer.handleEvent();
	if (Renderer.timeToRender()) {
		//Render Current instances
		Renderer.renderFrame();

		//Render FPS
		Renderer.renderFPS();

		// Present the renderer
		Renderer.showFrame();

		//Update all visible
		Renderer.update();		
	}

	// Delete menue objects
	Renderer.purgeLayer(Environment::RenderObjectLayers::menue);
}

void LevelEditor::Display::appendObject(RenderObject ro) {
	Renderer.append(ro);
}

int LevelEditor::Display::getSelectionX() {
	return Selection.valueGet<int>(namenKonvention.renderObject.positionX);
}

int LevelEditor::Display::getSelectionY() {
	return Selection.valueGet<int>(namenKonvention.renderObject.positionY);
}

void LevelEditor::Display::deleteObject() {
	int posX = Selection.valueGet<int>(namenKonvention.renderObject.positionX);
	int posY = Selection.valueGet<int>(namenKonvention.renderObject.positionY);

	
}

std::string LevelEditor::Display::serializeRenderer() {
	return Renderer.serialize();
}

std::string LevelEditor::Display::serializeRendererEnvironment() {
	return Renderer.serializeEnvironment();
}

void LevelEditor::Display::deserializeEnvironment(std::string serialFile) {
	Renderer.deserializeEnvironment(serialFile, Renderer.getResX(), Renderer.getResY(), Renderer.getThreadSize());
}

size_t LevelEditor::Display::objectCount() {
	return Renderer.getObjectCount();
}
