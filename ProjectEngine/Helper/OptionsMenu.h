#pragma once

#include "Platform.h"

#include <iostream>
#include <vector>
#include <tuple>
#include <functional>
#include <string>
#include <sstream>
#include <Console.h>

class OptionsMenu {
public:
    using FunctionPtr = std::function<void()>;  // Define the function pointer type

    OptionsMenu() : currentOption(0) {}

    void setTextBefore(std::string text);
    void setTextAfter(std::string text);

    // Update, get input and manipulate
    // return is 0 if no option was chosen
    // return is positive if option n was chosen and the corresponding function was executed
    // return is -1 if exit was chosen
    int update(bool render = true);

    // Renders all available options, where the current option has an arrow next to it
    void render();

    std::string createText();

    // Attach a function to the menu
    void attachFunction(FunctionPtr func, const std::string& text, const std::string& description = "");

    // Clear all function entries
    void clearEntries();

    // Set current option
    void setOption(int opt);

    // Change type of Menu
    void changeType(int type);



    // Constants
    const static int statusExit = -1;
    const static int typeScrollingMenu = 0;
    const static int typeKonsole = 1;

private:
    Console console;

    int menuType = 0;

    std::string textBefore = "";
    std::string textAfter = "";

    std::vector<std::tuple<FunctionPtr, std::string, std::string>> functions;  // Vector of function-name-description tuple
    bool showExitEntry = true;  // Whether to show the exit entry
    int currentOption = 0;           // Current option index
};