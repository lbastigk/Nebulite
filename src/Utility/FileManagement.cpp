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

std::string FileManagement::CombinePaths(std::string const& baseDir, std::string const& innerDir) {
    std::filesystem::path const basePath(baseDir);
    std::filesystem::path const innerPath(innerDir);
    std::filesystem::path const fullPath = basePath / innerPath;
    return fullPath.string();
}

/**
 * @todo Switch back to std::filesystem
 *       this old version was used to avoid locale issues on some platforms.
 *       Should work now, issue was probably some borked Kubuntu install.
 */
std::string FileManagement::LoadFile(std::string const& link) {
    // Use C-style file I/O to avoid locale issues
    FILE* file = fopen(link.c_str(), "rb");
    if (!file) {
        Nebulite::error::println("File '", link, "' could not be opened! Reason: ", strerror(errno));
        return "";
    }

    // Get file size
    fseek(file, 0, SEEK_END);
    long const fileSize = ftell(file);
    fseek(file, 0, SEEK_SET);

    if (fileSize <= 0) {
        Nebulite::error::println("File '", link, "' is empty or invalid!");
        fclose(file);
        return "";
    }

    // Read entire file into string
    std::string content(static_cast<size_t>(fileSize), '\0');
    size_t const bytesRead = fread(&content[0], 1, static_cast<size_t>(fileSize), file);
    fclose(file);

    // Adjust string size to actual bytes read
    content.resize(bytesRead);
    return content;
}

void FileManagement::WriteFile(std::string const& filename, std::string const& text) {
    std::filesystem::path const filepath(filename); // Modern: handles encoding and platform separators

    std::ofstream file(filepath, std::ios::out);
    if (!file.is_open()) {
        Nebulite::error::println("File '", filepath.string(), "' could not be opened/created for writing!");
        return;
    }
    file << text;
}

char FileManagement::preferredSeparator() {
    return std::filesystem::path::preferred_separator;
}

std::string FileManagement::currentDir() {
    try {
        return std::filesystem::current_path().string();
    } catch (std::exception const& e) {
        Nebulite::error::println("Error getting current directory: ", e.what());
        return "";
    }
}

bool FileManagement::fileExists(std::string const& path) {
    return std::filesystem::exists(path);
}

} // namespace Nebulite::Utility
