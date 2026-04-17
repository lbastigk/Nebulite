//------------------------------------------
// Includes

// Standard library
#include <filesystem>
#include <iostream>
#include <fstream>

// Nebulite
#include "Nebulite.hpp"
#include "Utility/FileManagement.hpp"

//------------------------------------------
namespace Nebulite::Utility {

std::string FileManagement::CombinePaths(std::string_view const& baseDir, std::string_view const& innerDir) {
    std::filesystem::path const basePath(baseDir);
    std::filesystem::path const innerPath(innerDir);
    std::filesystem::path const fullPath = basePath / innerPath;
    return fullPath.string();
}

std::string FileManagement::LoadFile(std::string_view const& link) {
    std::filesystem::path const filepath(link);

    if (!exists(filepath)) {
        Global::capture().error.println("File '", filepath.string(), "' does not exist!");
        return "";
    }
    if (!is_regular_file(filepath)) {
        Global::capture().error.println("Path '", filepath.string(), "' is not a regular file!");
        return "";
    }

    std::ifstream file(filepath, std::ios::binary);
    if (!file.is_open()) {
        Global::capture().error.println("File '", filepath.string(), "' could not be opened for reading!");
        return "";
    }

    std::string content;
    try {
        auto const size = file_size(filepath);
        content.resize(size);
        file.read(content.data(), static_cast<std::streamsize>(content.size()));

        if (!file && !file.eof()) {
            Global::capture().error.println("Error reading file '", filepath.string(), "'!");
            content = "";
        }
    } catch (std::exception const& e) {
        Global::capture().error.println("Error reading file '", filepath.string(), "': ", e.what());
        content = "";
    }
    return content;
}

bool FileManagement::WriteFile(std::string_view const& filename, std::string_view const& text) {
    std::filesystem::path const filepath(filename); // Modern: handles encoding and platform separators
    std::ofstream file(filepath, std::ios::out);
    if (!file.is_open()) {
        return false;
    }
    file << text;
    return true;
}

char FileManagement::preferredSeparator() {
    return std::filesystem::path::preferred_separator;
}

std::string FileManagement::currentDir() {
    try {
        return std::filesystem::current_path().string();
    } catch (std::exception const& e) {
        Global::capture().error.println("Error getting current directory: ", e.what());
        return "";
    }
}

bool FileManagement::fileExists(std::string_view const& path) {
    return std::filesystem::exists(path);
}

std::vector<std::string> FileManagement::listFilesInDirectory(std::string_view const& dir){
    std::vector<std::string> files;
    try {
        for (const auto& entry : std::filesystem::directory_iterator(dir)) {
            if (entry.is_regular_file()) {
                    files.push_back(entry.path().filename().string());
            }
        }
    } catch (std::exception const& e) {
        Global::capture().error.println("Error listing files in directory '", dir, "': ", e.what());
    }
    return files;
}

} // namespace Nebulite::Utility
