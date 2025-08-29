/**
 * @file Nebulite::Utility::FileManagement.h
 * 
 * @brief Provides utilities for file management operations.
 */

#pragma once

//-----------------------------------------------------------
// Includes

// General
#include <filesystem>
#include <string>
#include <cstring>
#include <iostream>
#include <sstream>
#include <fstream>
#include <string>

//-----------------------------------------------------------
namespace Nebulite{
namespace Utility {
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
    static std::string CombinePaths(const std::string& baseDir, const std::string& innerDir);

    /**
     * @brief Loads the contents of a file into a string.
     * 
     * This function takes a file path and reads its contents into a string.
     * Outputs error messages to std::cerr if the file cannot be opened or read.
     * 
     * @param link The path to the file to read.
     * 
     * @return The contents of the file as a string. If the file cannot be read,
     *         an empty string is returned.
     */
    static std::string LoadFile(const std::string& link);

    /**
     * @brief Writes a string to a file.
     * 
     * This function takes a filename and a string and writes the string to the file.
     * Outputs error messages to std::cerr if the file cannot be opened/created for writing.
     * 
     * @param filename The name of the file to write to.
     * @param text The text to write to the file.
     */
    static void WriteFile(const std::string& filename, const std::string& text);

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
     * Outputs error messages to std::cerr if the directory cannot be accessed.
     * 
     * @return The current working directory as a string.
     */
    static std::string currentDir();
};
} // namespace Utility
} // namespace Nebulite
