#include "OptionsMenu.h"
#include <functional> // For std::bind
#include "Editor.h"
#include "DsaDebug.h"


int main(int argc, char* argv[]) {
    // Redirect std::cerr to a file
    freopen("error_log.txt", "w", stderr);

    // Editor
    Editor editor;

	// Create options menu
	OptionsMenu optM;
	optM.setOption(0);	//Start option
	// Attach functions using std::bind to bind the member functions to the object
	optM.attachFunction(std::bind(&Editor::LoadLevel, &editor), "Load Level");
	optM.attachFunction(std::bind(&Editor::CreateRenderobject, &editor), "Create Renderobject");
    optM.attachFunction(std::bind(&Editor::CreateMoveRuleSet, &editor), "Create Moveruleset");
	
	optM.render();		//First render for display
	
	//Loop until exit is chosen
	volatile int status = 0;
	while(status != OptionsMenu::statusExit){status = optM.update();}
    
	return 0;

}