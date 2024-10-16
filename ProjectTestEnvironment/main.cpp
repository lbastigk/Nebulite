#include "TestEnv.h"

int main(int argc, char* argv[]) {
	// Redirect std::cerr to a file
    freopen("error_log.txt", "w", stderr);

	// Create options menu
	OptionsMenu optM;
	optM.setOption(6);	//Start option
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
	optM.render();		//First render for display
	
	//Loop until exit is chosen
	volatile int status = 0;
	while(status != OptionsMenu::statusExit){status = optM.update();}

	return 0;
}
