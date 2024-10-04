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



class FileManagement{
public:
	static bool folderExists(const std::string& folderName);
    static std::string currentDir();

    static std::string CombinePaths(const std::string& baseDir, const std::string& innerDir);

    static std::string LoadFile(std::string link);

    static bool openFileWithDefaultProgram(const std::string& path);

    static void WriteFile(const char filename[], std::string text);

    static std::string preferred_separator();

    class FileTypeCollector {
    public:
        FileTypeCollector(std::string directory, std::string fileType, bool processSubDirectories = true);

        const std::vector<std::string>& getFileDirectories() const;

    private:
        std::vector<std::string> fileDirectories;

        void process(const std::string& directory, const std::string& fileType, bool processSubDirectories);
    };
};