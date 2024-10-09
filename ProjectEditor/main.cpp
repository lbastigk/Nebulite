#include "Editor.h"
#include "DsaDebug.h"


int main(int argc, char* argv[]) {
    Editor Editor;

    //Options menue
    Platform::clearScreen();
    int opt = 0;
    std::string options = "Load Level\nCreate Renderobject\nCreate Moveruleset\nQuit";
    while (opt != std::ranges::count(options, '\n')) {
        opt = DsaDebug::menueScreen(options, opt, "OPTIONS:\n----------------------\n", "\n----------------------");
        Platform::clearScreen();

        switch (opt) {
        case 0:
            Editor.LoadLevel();
            break;
        case 1:
            Editor.CreateRenderobject();
            break;
        case 2:
            Editor.CreateMoveRuleSet();
            break;
        }
        
    }
}