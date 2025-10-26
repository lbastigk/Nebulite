#include "Utility/FileManagement.hpp"

//------------------------------------------
// Nebulite::Utility::FileManagement Functions

std::string Nebulite::Utility::FileManagement::CombinePaths(std::string const& baseDir, std::string const& innerDir){
    std::filesystem::path basePath(baseDir);
    std::filesystem::path innerPath(innerDir);
    std::filesystem::path fullPath = basePath / innerPath;
    return fullPath.string();
}

/**
 * @todo Switch back to std::filesystem
 */
std::string Nebulite::Utility::FileManagement::LoadFile(std::string const& link){  
    // Use C-style file I/O to avoid locale issues
    FILE* file = fopen(link.c_str(), "rb");
    if (!file){
        Nebulite::Utility::Capture::cerr() << "File '" << link << "' could not be opened!" << Nebulite::Utility::Capture::endl;
        return "";
    }
    
    // Get file size
    fseek(file, 0, SEEK_END);
    long fileSize = ftell(file);
    fseek(file, 0, SEEK_SET);
    
    if (fileSize <= 0){
        Nebulite::Utility::Capture::cerr() << "File '" << link << "' is empty or invalid!" << Nebulite::Utility::Capture::endl;
        fclose(file);
        return "";
    }
    
    // Read entire file into string
    std::string content(static_cast<size_t>(fileSize), '\0');
    size_t bytesRead = fread(&content[0], 1, static_cast<size_t>(fileSize), file);
    fclose(file);
    
    // Adjust string size to actual bytes read
    content.resize(bytesRead);
    return content;
}

void Nebulite::Utility::FileManagement::WriteFile(std::string const& filename, std::string const& text){
    std::filesystem::path filepath(filename);  // Modern: handles encoding and platform separators

    std::ofstream file(filepath, std::ios::out);
    if (!file.is_open()){
        Nebulite::Utility::Capture::cerr() << "File '" << filepath << "' could not be opened/created for writing!" << Nebulite::Utility::Capture::endl;
        return;
    }
    file << text;
}

char Nebulite::Utility::FileManagement::preferredSeparator(){
    return std::filesystem::path::preferred_separator;
}

std::string Nebulite::Utility::FileManagement::currentDir(){
    try {
        return std::filesystem::current_path().string();
    } catch (const std::exception& e){
        Nebulite::Utility::Capture::cerr() << "Error getting current directory: " << e.what() << Nebulite::Utility::Capture::endl;
        return "";
    }
}

bool Nebulite::Utility::FileManagement::fileExists(std::string const& path){
    return std::filesystem::exists(path);
}