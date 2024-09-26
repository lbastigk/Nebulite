#include <vector>
#include <map>
#include <functional>

#include "Renderer.cpp"
#include "DsaDebug.cpp"
#include "Console.cpp"

#pragma once



class LevelEditor {
public:
	LevelEditor() {
		EditorConsole.clear();
		Display.start();
		EditorConsole.mainScreen();

		//Create userInputMap:
		userInputMap["place"] = [this]() {return this->placeItem(); };
		userInputMap["delete"] = [this]() {return this->deleteItem(); };
		userInputMap["clear"] = [this]() {return this->clearConsole(); };
		userInputMap["serialize"] = [this]() {return this->serializeEnvironment(); };
		userInputMap["count"] = [this]() {return this->countObjects(); };
		userInputMap["help"] = [this]() {return this->help(); };
		userInputMap["save"] = [this]() {return this->save(); };
	}
	void update() {
		Display.update();
		EditorConsole.update();

		if (EditorConsole.checkForInput()) {
			doInput();
		}
	}

	void doInput() {
		std::string in = EditorConsole.getInput();

		if (userInputMap.find(in) != userInputMap.end()) {
			std::string returnValue = userInputMap[in]();
			EditorConsole.print(returnValue);
		}
		else {
			EditorConsole.print("Command not found");
		}
	}

	void saveEnvironment() {
		FileManagement::WriteFile(environmentLink.c_str(), Display.serializeRendererEnvironment());
	}

	void loadEnvironment(std::string name) {
		std::string serialFile = FileManagement::LoadFile(name);
		Display.deserializeEnvironment(serialFile);
		environmentLink = name;
	}

	//Check if Level Editor is still active
	bool status() {
		return levelEditorStatus;
	}

	//Console input functions
	std::string placeItem() {
		//Create and append renderobject to level at selection position
		RenderObject ro;

		if (getRenderObjectFromList(ro)) {
			ro.valueSet(namenKonvention.renderObject.positionX, Display.getSelectionX());
			ro.valueSet(namenKonvention.renderObject.positionY, Display.getSelectionY());
			Display.appendObject(ro);

			return "Object placed! Is:\n" + ro.serialize();
		}
		else {
			return "Aborted, no object was placed.";
		}
	}
	std::string deleteItem() {
		Display.deleteObject();

		return "Object deleted!";
	}
	std::string clearConsole() {
		EditorConsole.clear();
		EditorConsole.mainScreen();

		return "";
	}
	std::string serializeEnvironment() {
		EditorConsole.print(Display.serializeRendererEnvironment());

		return "";
	}
	std::string countObjects(){
		EditorConsole.print(std::to_string(Display.objectCount()));

		return "";
	}
	std::string help() {
		EditorConsole.helpScreen();

		return "";
	}
	std::string save() {
		saveEnvironment();

		return "Saved";
	}

private:
	class EditorConsole {
	public:
		EditorConsole() {

		}

		void print(std::string str) {
			std::cout << str << "\n";
		}
		void clear() {
			system("cls");
		}
		void mainScreen() {
			print("DSA Engine Editor V0.1");
			print(".........................................");
		}
		void waitForInput() {
			while (!kbhit()) {

			}
			(void)_getch();
		}
		void helpScreen() {
			clear();
			print("place\t\t-\t\tPlace object at current position");
			print("delete\t\t-\t\tDelete object at current position");
			print("clear\t\t-\t\tClear console");
			print("Serialize\t-\t\tSerialize level");
			print("Count\t\t-\t\tCount total of renderobjects in level");
		}

		void update() {
			Console.refresh();
		}

		bool checkForInput() {
			return Console.hasInput();
		}

		std::string getInput() {
			return Console.getInput();
		}

	private:
		Console Console;

		std::string input;
		std::string inputTemp;
	};
	class Display {	
	public:
		Display() {
			SDL_ShowCursor(SDL_DISABLE); // This hides the cursor
		}

		void start() {
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
		
		void update() {
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

		void appendObject(RenderObject ro) {
			Renderer.append(ro);
		}

		int getSelectionX() {
			return Selection.valueGet<int>(namenKonvention.renderObject.positionX);
		}
		int getSelectionY() {
			return Selection.valueGet<int>(namenKonvention.renderObject.positionY);
		}

		void deleteObject() {
			int posX = Selection.valueGet<int>(namenKonvention.renderObject.positionX);
			int posY = Selection.valueGet<int>(namenKonvention.renderObject.positionY);

			
		}

		std::string serializeRenderer() {
			return Renderer.serialize();
		}
		std::string serializeRendererEnvironment() {
			return Renderer.serializeEnvironment();
		}

		void deserializeEnvironment(std::string serialFile) {
			Renderer.deserializeEnvironment(serialFile, Renderer.getResX(), Renderer.getResY(), Renderer.getThreadSize());
		}

		size_t objectCount() {
			return Renderer.getObjectCount();
		}

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
	
	bool getRenderObjectFromList(RenderObject& ro) {
		std::string dir = FileManagement::currentDir();
		std::string fullDir = FileManagement::CombinePaths(dir, std::string("Resources/Renderobjects"));
		FileManagement::FileTypeCollector ftc(fullDir,".txt",true);
		auto list = ftc.getFileDirectories();
		std::string listAsString;
		for (auto entry : list) {
			listAsString += entry;
			listAsString += "\n";
		}
		listAsString += "exit";


		int opt = DsaDebug::menueScreen(listAsString, 0, "Choose a Renderobject to place\n\n", "", false);

		if (opt == list.size()) {
			return false;
		}
		else {
			std::string link = list.at(opt);
			std::string file = FileManagement::LoadFile(link);
			ro.deserialize(file);
		}
	}
};