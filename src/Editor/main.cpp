#include "OptionsMenu.h"
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
	optM.changeType(OptionsMenu::typeScrollingMenu);	// Type scrolling for now, as console type needs some rework
														// Scrolling works like this:
														// MENU TEXT:
														// -> Option1
														//    Option2
														//    ...
														//
														// Console should work like this:
														// foo arg1 arg2 arg3
														//
														// In the future, scolling menu should still be supported, 
														// although only functions without args are reasonable with this approach
	optM.setTextBefore("Nebulite Editor GUI pre-alpha\n");

	// Functions
	optM.attachFunction([&editor]() { editor.LoadLevel(); }, "Load Level");
	optM.attachFunction([&editor]() { editor.CreateRenderobject(); }, "Create Renderobject");
	optM.attachFunction([&editor]() { editor.CreateMoveRuleSet(); }, "Create Moveruleset");

	
	optM.render();		//First render for display
	
	//Loop until exit is chosen
	volatile int status = 0;
	while(status != OptionsMenu::statusExit){status = optM.update();}
    
	return 0;

}