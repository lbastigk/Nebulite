#include "LevelEditor.h"

//---------------------------------------------
// LevelEditor Class

LevelEditor::LevelEditor() {
	Display.start();

	//Create userInputMap:
	int cursorX = Display.getSelectionX();
	int cursorY = Display.getSelectionY();
	optM.setTextBefore(
	std::string("Nebulite Level Editor V0.1\n")+
	std::string("Current cursor position: ") + 
	std::to_string(cursorX) + std::string("(") + std::to_string(cursorX%160) + std::string(")  |  ") +
	std::to_string(cursorY) + std::string("(") + std::to_string(cursorY%160) + std::string(")\n") +
	std::string("........................................."));
	optM.attachFunction(std::bind(&LevelEditor::placeItem,this),"place","...");
	optM.attachFunction(std::bind(&LevelEditor::deleteItem,this),"delete","...");
	optM.attachFunction(std::bind(&LevelEditor::serializeEnvironment,this),"serialize","...");
	optM.attachFunction(std::bind(&LevelEditor::countObjects,this),"count","...");
	optM.attachFunction(std::bind(&LevelEditor::save,this),"save","...");
	optM.changeType(OptionsMenu::typeConsole);
	optM.update(true);
}

void LevelEditor::update() {
	Display.update();
	levelEditorStatus = optM.update() != OptionsMenu::statusExit;

	int cursorX = Display.getSelectionX();
	int cursorY = Display.getSelectionY();

	if((cursorX != Display.getLastSelectionX()) || (cursorY != Display.getLastSelectionY())){
		optM.setTextBefore(
		std::string("Nebulite Level Editor V0.1\n")+
		std::string("Current cursor position: ") + 
		std::to_string(cursorX) + std::string("(") + std::to_string(cursorX%160) + std::string(")  |  ") +
		std::to_string(cursorY) + std::string("(") + std::to_string(cursorY%160) + std::string(")\n") +
		std::string("........................................."));

		optM.update(true);

		Display.setLastSelectionX(cursorX);
		Display.setLastSelectionY(cursorY);
	}

	
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
	// For resetting the screen
	std::cerr << "Before update\n";
	optM.update(true);
	std::cerr << "After update\n";
}

void LevelEditor::deleteItem() {
	Display.deleteObject();
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
	int i = 0;
	std::string dir = FileManagement::currentDir();
	std::string fullDir = FileManagement::CombinePaths(dir, std::string("Resources/Renderobjects"));
	FileManagement::FileTypeCollector ftc(fullDir,".json",true);
	auto list = ftc.getFileDirectories();

	OptionsMenu roEntries;
	roEntries.setTextBefore("Choose a Renderobject to place\n\n");
	for (auto entry : list) {
		roEntries.attachFunction([](){},entry);	//Lambda expresion to attach an empty function
	}

	// Choose from renderobjects
	roEntries.render();
	int opt = 0;
	while(opt == 0){
		opt = roEntries.update();
	}
	roEntries.update();

	if (opt == -1) {
		return false;
	}
	else {
		std::string link = list.at(opt-1);
		std::string file = FileManagement::LoadFile(link);
		ro.deserialize(file);
		return true;
	}
	return false;
}

//---------------------------------------------
// LevelEditor::Display Class
LevelEditor::Display::Display() {
	SDL_ShowCursor(SDL_DISABLE); // This hides the cursor
	mouseState = SDL_GetMouseState(&MousePosX, &MousePosY);
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
	lastMouseState = mouseState;
	mouseState = SDL_GetMouseState(&MousePosX, &MousePosY);
	
	//Append Cursor obj at Mouse position
	Cursor.valueSet(namenKonvention.renderObject.positionX, MousePosX + Renderer.getPosX());
	Cursor.valueSet(namenKonvention.renderObject.positionY, MousePosY + Renderer.getPosY());

	//Create Selection position
	uint32_t lms;
	RenderObject ro;
	switch (mouseState & (SDL_BUTTON_LMASK | SDL_BUTTON_RMASK | SDL_BUTTON_MMASK | SDL_BUTTON_X1MASK | SDL_BUTTON_X2MASK)) {
	case SDL_BUTTON_LMASK:
		// Left mouse button pressed
		// move selector
		Selection.valueSet(namenKonvention.renderObject.positionX, MousePosX - (MousePosX % 32));
		Selection.valueSet(namenKonvention.renderObject.positionY, MousePosY - (MousePosY % 32));
		break;
	case SDL_BUTTON_RMASK:
		// Right mouse button pressed
		// move cam
		SDL_SetRelativeMouseMode(SDL_TRUE);
		lms = (lastMouseState & (SDL_BUTTON_LMASK | SDL_BUTTON_RMASK | SDL_BUTTON_MMASK | SDL_BUTTON_X1MASK | SDL_BUTTON_X2MASK));
		if (lms != SDL_BUTTON_RMASK){
			lastMousePosX = MousePosX;
			lastMousePosY = MousePosY;
		}
		Renderer.moveCam(MousePosX - lastMousePosX,MousePosY - lastMousePosY);
		lastMousePosX = MousePosX;
		lastMousePosY = MousePosY;
		break;
	case SDL_BUTTON_MMASK:
		// Middle button pressed
		// Place last object
		lms = (lastMouseState & (SDL_BUTTON_LMASK | SDL_BUTTON_RMASK | SDL_BUTTON_MMASK | SDL_BUTTON_X1MASK | SDL_BUTTON_X2MASK));
		if (lms != SDL_BUTTON_MMASK){
			ro.deserialize(lastPlaced.serialize());
			ro.valueSet(namenKonvention.renderObject.positionX,getSelectionX());
			ro.valueSet(namenKonvention.renderObject.positionY,getSelectionY());
			appendObject(ro);
		}
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
		SDL_SetRelativeMouseMode(SDL_FALSE);
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
	lastPlaced.deserialize(ro.serialize());
}

int LevelEditor::Display::getSelectionX() {
	return Selection.valueGet<int>(namenKonvention.renderObject.positionX);
}

int LevelEditor::Display::getSelectionY() {
	return Selection.valueGet<int>(namenKonvention.renderObject.positionY);
}


int LevelEditor::Display::getLastSelectionX(){
	return lastCursorPosX;
};
int LevelEditor::Display::getLastSelectionY(){
	return lastCursorPosY;
};

void LevelEditor::Display::setLastSelectionX(int x){
	lastCursorPosX = x;
};
void LevelEditor::Display::setLastSelectionY(int y){
	lastCursorPosY = y;
};

void LevelEditor::Display::deleteObject() {
	int posX = Selection.valueGet<int>(namenKonvention.renderObject.positionX);
	int posY = Selection.valueGet<int>(namenKonvention.renderObject.positionY);
	Renderer.purgeObjectsAt(posX,posY);
}

std::string LevelEditor::Display::serializeRenderer() {
	return Renderer.serialize();
}

std::string LevelEditor::Display::serializeRendererEnvironment() {
	return Renderer.serializeEnvironment();
}

void LevelEditor::Display::deserializeEnvironment(std::string serialFile) {
	Renderer.deserializeEnvironment(serialFile);
}

size_t LevelEditor::Display::objectCount() {
	return Renderer.getObjectCount();
}
