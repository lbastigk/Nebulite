#pragma once

#include "cstdio"
#include <iostream>
#include <sstream>
#include <string>


#include <ranges>
#include <algorithm>
#include <ctype.h>

#include "Platform.h"

extern int FUNCTION_DEPTH;

#define LOG_FUNC_I() 			std::cerr << "Entering:\t"; for(int it_depth=0;it_depth<FUNCTION_DEPTH;it_depth++){std::cerr << '\t';} std::cerr << typeid(*this).name() << "::" << __func__ << " D" << FUNCTION_DEPTH << std::endl;FUNCTION_DEPTH++;
#define LOG_FUNC_O() FUNCTION_DEPTH--; 	std::cerr << "Exiting: \t"; for(int it_depth=0;it_depth<FUNCTION_DEPTH;it_depth++){std::cerr << '\t';} std::cerr << typeid(*this).name() << "::" << __func__ << std::endl;
#define LOG_FDEPTH() std::cerr << "         \t"; for(int it_depth=0;it_depth<FUNCTION_DEPTH;it_depth++){std::cerr << '\t';}


class DsaDebug {
public:
    // Function to get used memory by the program itself on Windows, in kB
    static double getMemoryUsagekB();

    static void printWithPrefix(const std::string& str, const std::string& prefix);

    static int menueScreen(const std::string options, int startoption,std::string before = "", std::string after = "", bool showAll = true);

private:
    static void menueScreenTemplate();
};


