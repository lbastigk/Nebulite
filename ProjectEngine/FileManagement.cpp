#include <filesystem>
#include <string>
#include <cstring>
#include <vector>
#include <direct.h> // For _getcwd

#include <iostream>
#include <sstream>
#include <fstream>
#include <string>
#include <stdio.h>
#include <stdlib.h>

#include <Windows.h>

#pragma once

class FileManagement{
public:
	static bool folderExists(const std::string& folderName) {
        std::filesystem::path folderPath = std::filesystem::current_path() / folderName;

        // Check if the folder exists
        return std::filesystem::exists(folderPath) && std::filesystem::is_directory(folderPath);
    }
    static std::string currentDir() {
        char currentDir[150];
        (void)_getcwd(currentDir, sizeof(currentDir));
        std::string currentDirStr = currentDir;
        return currentDirStr;
    }

    static std::string CombinePaths(const std::string& baseDir, const std::string& innerDir) {
        std::filesystem::path basePath(baseDir);
        std::filesystem::path innerPath(innerDir);
        std::filesystem::path fullPath = basePath / innerPath;
        return fullPath.string();
    }

    static std::string LoadFile(std::string link) {
        std::ifstream data;                           //creates stream for data
        data.open(link);                     //open text file dump
        std::string line;
        std::stringstream toreturn;
        int i = 1;
        int linecount = 0;

        //checks if file really is open
        if (data.is_open()) {
            //loop, reading line by line:
            //Get first line
            while (getline(data, line)) {
                linecount++;
                toreturn << line << '\n';
            }
            data.close();
        }
        return toreturn.str();
    }

    static bool openFileWithDefaultProgram(const std::string& path) {
        std::wstring fullPath;

        if (std::filesystem::path(path).is_absolute()) {
            // If the path is absolute, use it directly
            fullPath = std::wstring(path.begin(), path.end());
        }
        else {
            // If the path is relative, combine it with the current directory
            std::string tmp = CombinePaths(currentDir(), path);
            fullPath = std::wstring(tmp.begin(),tmp.end());
        }

        if (std::filesystem::exists(fullPath) && std::filesystem::is_regular_file(fullPath)) {
            // Use ShellExecute to open the file with the default program
            HINSTANCE result = ShellExecute(nullptr, L"open", fullPath.c_str(), nullptr, nullptr, SW_SHOWNORMAL);

            // Check the result
            if ((intptr_t)result > 32) {
                // Success
                return true;
            }
            else {
                // Failed to open the file
                return false;
            }
        }
        else {
            // File does not exist or is not a regular file
            return false;
        }
    }

    static void WriteFile(const char filename[], std::string text) {
        std::ofstream file_;
        file_.open(filename, std::ios_base::out);
        if (file_.is_open()) {
            file_ << text;
            file_.close();
        }
    }

    static std::string preferred_separator() {
        // Get the preferred separator as a wide character
        wchar_t separator = std::filesystem::path::preferred_separator;

        // Convert the wide character to an std::string
        std::string separatorStr(1, static_cast<char>(separator));

        return separatorStr;
    }

    class FileTypeCollector {
    public:
        FileTypeCollector(const std::string& directory, const std::string& fileType, bool processSubDirectories = true) {
            process(directory, fileType, processSubDirectories);
        }

        const std::vector<std::string>& getFileDirectories() const {
            return fileDirectories;
        }

    private:
        std::vector<std::string> fileDirectories;

        void process(const std::string& directory, const std::string& fileType, bool processSubDirectories) {
            std::string fullDirectory;

            // Check if directory is a full directory path
            if (!std::filesystem::is_directory(directory)) {
                // If not, combine with the directory of the program
                fullDirectory = std::filesystem::current_path().string() + preferred_separator() + directory;   //This part works, preferred_separator is member of FileManagement
            }
            else {
                fullDirectory = directory;
            }

            // Now, push every file directory of type fileType into the vector fileDirectories
            if (std::filesystem::is_directory(fullDirectory)) {
                for (const auto& entry : std::filesystem::directory_iterator(fullDirectory)) {
                    if (entry.is_regular_file() && entry.path().extension() == fileType) {
                        fileDirectories.push_back(entry.path().string());
                    }
                    else if (entry.is_directory() && processSubDirectories) {
                        process(entry.path().string(), fileType, true);
                    }
                }
            }
        }
    };
};