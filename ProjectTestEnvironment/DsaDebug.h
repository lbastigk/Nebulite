#pragma once

#include "cstdio"
#include <iostream>
#include <sstream>
#include <string>


#include <ranges>
#include <algorithm>
#include <ctype.h>

#include "Platform.h"


class DsaDebug {
public:
    // Function to get used memory by the program itself on Windows, in kB
    static double getMemoryUsagekB();

    static void printWithPrefix(const std::string& str, const std::string& prefix);

    static int menueScreen(const std::string options, int startoption,std::string before = "", std::string after = "", bool showAll = true);

private:
    static void menueScreenTemplate();
};


