#include "FileManagement.h"

//--------------------------------------------------------
// FileManagement Functions

std::string FileManagement::CombinePaths(const std::string& baseDir, const std::string& innerDir) {
    std::filesystem::path basePath(baseDir);
    std::filesystem::path innerPath(innerDir);
    std::filesystem::path fullPath = basePath / innerPath;
    return fullPath.string();
}

std::string FileManagement::LoadFile(const std::string& link) {
    namespace fs = std::filesystem;

    fs::path filepath = fs::u8path(link);  // handles forward/backslashes and encodings
    std::ifstream data(filepath, std::ios::in);

    if (!data.is_open()) {
        std::cerr << "File '" << filepath << "' could not be opened!" << std::endl;
        return "";
    }

    std::stringstream toreturn;
    std::string line;
    while (std::getline(data, line)) {
        toreturn << line << '\n';
    }

    return toreturn.str();
}

void FileManagement::WriteFile(const std::string& filename, const std::string& text) {
    namespace fs = std::filesystem;

    fs::path filepath = fs::u8path(filename);  // Handles encoding and platform separators

    std::ofstream file(filepath, std::ios::out);  // Automatically opens file
    if (!file.is_open()) {
        std::cerr << "Failed to open file for writing: " << filepath << std::endl;
        return;
    }

    file << text;
}

char FileManagement::preferred_separator() {
    return std::filesystem::path::preferred_separator;
}

std::string FileManagement::currentDir() {
    try {
        return std::filesystem::current_path().string();
    } catch (const std::exception& e) {
        std::cerr << "Error getting current directory: " << e.what() << std::endl;
        return "";
    }
}