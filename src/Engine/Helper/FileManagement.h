#pragma once

#include <filesystem>
#include <string>
#include <cstring>
#include <vector>
//#include <direct.h> // For _getcwd

#include <iostream>
#include <sstream>
#include <fstream>
#include <string>
#include <stdio.h>
#include <stdlib.h>

//#include <Windows.h>
#include <unistd.h> // For getcwd

#include "Platform.h"
#include "StringHandler.h"
#include "JSON.h"

class FileManagement{
public:

    static std::string CombinePaths(const std::string& baseDir, const std::string& innerDir);

    static std::string LoadFile(const std::string& link);

    static void WriteFile(const std::string& filename, const std::string& text);

    static char preferred_separator();

    static std::string currentDir();
};

