/**
 * @file FileManagement.hpp
 * @brief This file contains the FileManagement class for managing file operations.
 */

#ifndef NEBULITE_UTILITY_FILEMANAGEMENT_HPP
#define NEBULITE_UTILITY_FILEMANAGEMENT_HPP

//------------------------------------------
// Includes

// Standard library
#include <string>

//------------------------------------------
namespace Nebulite::Utility {
/**
 * @class Nebulite::Utility::FileManagement
 * @brief Provides utilities for file management operations via a static class.
 */
class FileManagement{
public:
    /**
     * @brief Combines two paths into one.
     * 
     * This function takes a base directory and an inner directory and combines them
     * into a single path using the appropriate directory separator for the platform.
     * 
     * @param baseDir The base directory.
     * @param innerDir The inner directory to append to the base directory.
     * 
     * @return The combined path as a string.
     */
    static std::string CombinePaths(std::string const& baseDir, std::string const& innerDir);

    /**
     * @brief Loads the contents of a file into a string.
     * 
     * This function takes a file path and reads its contents into a string.
     * Outputs error messages to Nebulite::Utility::Capture::cerr() if the file cannot be opened or read.
     * 
     * @param link The path to the file to read.
     * @return The contents of the file as a string. If the file cannot be read,
     *         an empty string is returned.
     */
    static std::string LoadFile(std::string const& link);

    /**
     * @brief Writes a string to a file.
     * 
     * This function takes a filename and a string and writes the string to the file.
     * Outputs error messages to Nebulite::Utility::Capture::cerr() if the file cannot be opened/created for writing.
     * 
     * @param filename The name of the file to write to.
     * @param text The text to write to the file.
     */
    static void WriteFile(std::string const& filename, std::string const& text);

    /**
     * @brief Returns the preferred directory separator for the platform.
     * 
     * This function returns the preferred directory separator character for the platform.
     * 
     * @return The preferred directory separator character.
     */
    static char preferredSeparator();

    /**
     * @brief Returns the current working directory.
     * 
     * This function returns the current working directory as a string.
     * Outputs error messages to Nebulite::Utility::Capture::cerr() if the directory cannot be accessed.
     * 
     * @return The current working directory as a string.
     */
    static std::string currentDir();

    /**
     * @brief Checks if a file exists at the given path.
     * 
     * This function checks if a file exists at the specified path.
     * 
     * @param path The path to the file to check.
     * 
     * @return True if the file exists, false otherwise.
     */
    static bool fileExists(std::string const& path);
};
} // namespace Nebulite::Utility
#endif // NEBULITE_UTILITY_FILEMANAGEMENT_HPP