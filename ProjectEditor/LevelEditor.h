#include <vector>
#include <map>
#include <functional>

#include "Renderer.h"
#include "DsaDebug.h"
#include "Console.h"
#include "OptionsMenu.h"

#pragma once

class LevelEditor {
public:
	LevelEditor();
    void update();
    void doInput();
    void saveEnvironment();
    void loadEnvironment(std::string name);

	//Check if Level Editor is still active
	bool status();

	//Console input functions
    void placeItem();
    void deleteItem();
    void serializeEnvironment();
    void countObjects();
    void help();
    void save();

	
private:
	class Display {	
	public:
		Display();
		void start();
		void update();
		void appendObject(RenderObject ro);
		int getSelectionX();
        int getSelectionY();

		int getLastSelectionX();
		int getLastSelectionY();

		void setLastSelectionX(int x);
		void setLastSelectionY(int y);

		void deleteObject();
		std::string serializeRenderer();
        std::string serializeRendererEnvironment();
		void deserializeEnvironment(std::string serialFile);
		size_t objectCount();
	private:
		RenderObject Cursor;
		RenderObject Selection;
		int MousePosX = 0;
		int MousePosY = 0;

		int lastMousePosX = 0;
		int lastMousePosY = 0;

		int lastRenderPosX = 0;
		int lastRenderPosY = 0;

		int lastCursorPosX = 0;
		int lastCursorPosY = 0;

		uint32_t lastMouseState = 0;
		uint32_t mouseState = 0;

		RenderObject lastPlaced;

		Renderer Renderer;
	};

	OptionsMenu optM;

	Display Display;
	// EditorConsole EditorConsole;

	//Environment Environment;
	std::string environmentLink;

	bool levelEditorStatus = true;

	//std::function map and functions for arguments
	// std::map<std::string, std::function<std::string()>> userInputMap;
	
	bool getRenderObjectFromList(RenderObject& ro);
	
};

