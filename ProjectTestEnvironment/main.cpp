#include "TestEnv.h"

int main(int argc, char* argv[]) {
	// Redirect std::cerr to a file
    freopen("error_log.txt", "w", stderr);
    std::cerr << "Test log...\n";

	Platform::clearScreen();

	int opt = 5;
	std::string options = "FileManagement\nGeneral\nInventarObjekt\nJSONHandler\nKreatur\nMoveRuleSet\nRenderObject\nRenderer\nTalente\nTemplate\nQuit";
	while (opt != std::ranges::count(options,'\n')) {
		opt = DsaDebug::menueScreen(options, opt, "OPTIONS:\n----------------------\n", "\n----------------------");

		Platform::clearScreen();

		switch (opt) {
		case 0:
			TestEnv::_FileManagement::testMenue();
			break;
		case 1:
			TestEnv::_General::testMenue();
			break;
		case 2:
			TestEnv::_InventarObjekt::testMenue();
			break;
		case 3:
			TestEnv::_JSONHandler::testMenue();
			break;
		case 4:
			TestEnv::_Kreatur::testMenue();
			break;
		case 5:
			TestEnv::_MoveRuleSet::testMenue();
			break;
		case 6:
			TestEnv::_RenderObject::testMenue();
			break;
		case 7:
			TestEnv::_Renderer::testMenue();
			break;
		case 8:
			TestEnv::_Talente::testMenue();
			break;
		case 9:
			TestEnv::_Template::testMenue();
			break;
		}
	}

	return 0;
}
