#include "TestEnv.h"

// Args for SDL
int main(int argc, char* argv[]) {
	enum testOption{
		FileManagement,
		Gemeral,
		InventarObjekt,
		JSONHandler,
		Kreatur,
		MoveRuleSet,
		RenderObject,
		Renderer,
		Talente,
		Template
	};

	// Redirect std::cerr to a file
	// This is only recommended for testenvironment for further debugging purposes
    freopen("error_log.txt", "w", stderr);

	// Create options menu
	OptionsMenu optM;
	optM.changeType(OptionsMenu::typeScrollingMenu);	// User chooses option with arrow keys in a list, pressing enter to confirm
	optM.attachFunction(TestEnv::_FileManagement::testMenue,	"FileManagement");
	optM.attachFunction(TestEnv::_General::testMenue,			"General");
	optM.attachFunction(TestEnv::_InventarObjekt::testMenue,	"InventarObjekt");
	optM.attachFunction(TestEnv::_JSONHandler::testMenue,		"JSONHandler");
	optM.attachFunction(TestEnv::_Kreatur::testMenue,			"Kreatur");
	optM.attachFunction(TestEnv::_MoveRuleSet::testMenue,		"MoveRuleSet");
	optM.attachFunction(TestEnv::_RenderObject::testMenue,		"RenderObject");
	optM.attachFunction(TestEnv::_Renderer::testMenue,			"Renderer");
	optM.attachFunction(TestEnv::_Talente::testMenue,			"Talente");
	optM.attachFunction(TestEnv::_Template::testMenue,			"Template");
	optM.setOption(testOption::RenderObject);	//Start option, chosen arbitrarely depending on the current most used option to avoid wasting time
	optM.render();		//First render for display
	
	//Loop until exit is chosen
	volatile int status = 0;
	while(status != OptionsMenu::statusExit){status = optM.update();}

	return 0;
}
