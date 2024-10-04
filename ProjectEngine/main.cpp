#include "Environment.h"
#include "FileManagement.h"
#include "Renderer.h"

int main(int argc, char* argv[]) {
	//Renderer Object
	Renderer Renderer;

	// Check for Ressources Folder
	if (!FileManagement::folderExists("Resources")) {
		std::cout << "Warning: Directory Resources not found!" << "\n\n";
	}

	// Get the current directory as a std::string
	std::string currentDir = FileManagement::currentDir();
	
	while (true) {
		//Render Current instances
		if (Renderer.timeToRender()) {
			Renderer.renderFrame();
			Renderer.renderFPS();
			Renderer.showFrame();
		}
		
		
	}
	//End of Program!
	Renderer.destroy();
	

	return 0;
}