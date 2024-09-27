#include <vector>
#include <map>
#include <functional>

#include "Renderer.h"
#include "DsaDebug.h"
#include "Console.h"

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
    std::string placeItem();
    std::string deleteItem();
    std::string clearConsole();
    std::string serializeEnvironment();
    std::string countObjects();
    std::string help();
    std::string save();
private:
	class EditorConsole {
	public:
		EditorConsole();
		void print(std::string str);
        void clear();
        void mainScreen();
        void waitForInput();
        void helpScreen();
		void update();
		bool checkForInput();
		std::string getInput();
	private:
		Console Console;
		std::string input;
		std::string inputTemp;
	};
	class Display {	
	public:
		Display();
		void start();
		void update();
		void appendObject(RenderObject ro);
		int getSelectionX();
        int getSelectionY();
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

		Renderer Renderer;
	};

	Display Display;
	EditorConsole EditorConsole;
	//Environment Environment;
	std::string environmentLink;

	bool levelEditorStatus = true;

	//std::function map and functions for arguments
	std::map<std::string, std::function<std::string()>> userInputMap;
	
	bool getRenderObjectFromList(RenderObject& ro)
};

