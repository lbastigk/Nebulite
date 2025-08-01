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
    // Use C-style file I/O to avoid locale issues
    FILE* file = fopen(link.c_str(), "rb");
    if (!file) {
        std::cerr << "File '" << link << "' could not be opened!" << std::endl;
        return "";
    }
    
    // Get file size
    fseek(file, 0, SEEK_END);
    long fileSize = ftell(file);
    fseek(file, 0, SEEK_SET);
    
    if (fileSize <= 0) {
        std::cerr << "File '" << link << "' is empty or invalid!" << std::endl;
        fclose(file);
        return "";
    }
    
    // Read entire file into string
    std::string content(fileSize, '\0');
    size_t bytesRead = fread(&content[0], 1, fileSize, file);
    fclose(file);
    
    // Adjust string size to actual bytes read
    content.resize(bytesRead);
    return content;
}

void FileManagement::WriteFile(const std::string& filename, const std::string& text) {
    namespace fs = std::filesystem;
    fs::path filepath(filename);  // Modern: handles encoding and platform separators

    std::ofstream file(filepath, std::ios::out);
    if (!file.is_open()) {
        std::cerr << "File '" << filepath << "' could not be opened for writing!" << std::endl;
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