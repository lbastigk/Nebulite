#include <iostream>
#include <string>
#include <vector>

#include <functional>

#include "OptionsMenu.h"

int main() {
    OptionsMenu optm;
    optm.changeType(OptionsMenu::typeConsole);
    optm.attachFunction([](){},"1","Option 0");
    optm.attachFunction([](){},"1","Option 1");
    optm.attachFunction([](){},"2","Option 2");
    optm.attachFunction([](){},"3","Option 3");
    optm.attachFunction([](){},"4","Option 4");
    optm.attachFunction([](){},"5","Option 5");
    optm.attachFunction([](){},"6","Option 6");
    optm.attachFunction([](){},"7","Option 7");
    optm.attachFunction([](){},"8","Option 8");
    optm.attachFunction([](){},"9","Option 9");
    while(optm.update() != -1)
}
