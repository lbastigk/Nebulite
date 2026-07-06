/**
 * @file FileManagement.hpp
 * @brief This file contains the FileManagement class for managing file operations.
 */

#ifndef UTILITY_IO_FILEMANAGEMENT_HPP
#define UTILITY_IO_FILEMANAGEMENT_HPP

//------------------------------------------
// Includes

// Standard library
#include <string>
#include <string_view>
#include <vector>

//------------------------------------------
namespace Nebulite::Utility::IO {
/**
 * @class Nebulite::Utility::IO::FileManagement
 * @brief Provides utilities for file management operations via a static class.
 */
class FileManagement{
public:
    /**
     * @brief Combines two paths into one using the appropriate directory separator for the platform.
     * @param baseDir The base directory.
     * @param innerDir The inner directory to append to the base directory.
     * @return The combined path as a string.
     */
    static std::string CombinePaths(std::string_view baseDir, std::string_view innerDir);

    /**
     * @brief Loads the contents of a file into a string.
     * @details Outputs error messages to the Global error capture if the file cannot be opened or read.
     * @param link The path to the file to read.
     * @return The contents of the file as a string. If the file cannot be read,
     *         an empty string is returned.
     */
    static std::string LoadFile(std::string_view link);

    /**
     * @brief Writes a string to a file.
     * @details Takes a filename and a string and writes the string to the file.
     *          Outputs error messages to the Global error capture if the file cannot be opened/created for writing.
     * @param filename The name of the file to write to.
     * @param text The text to write to the file.
     * @return True on success, false on failure
     */
    [[nodiscard]] static bool WriteFile(std::string_view filename, std::string_view text);

    /**
     * @brief Returns the preferred directory separator for the platform.
     * @return The preferred directory separator character.
     */
    static char preferredSeparator() noexcept ;

    /**
     * @brief Returns the current working directory.
     * @details Outputs error messages to the Global error capture if the directory cannot be accessed.
     * @return The current working directory as a string.
     */
    static std::string currentDir();

    /**
     * @brief Checks if a given path is a file.
     * @param path The path to the file to check.
     * @return True if the file exists, false otherwise.
     */
    static bool fileExists(std::string_view path);

    /**
     * @brief Checks if a given path is a directory
     * @param path The path to check
     * @return True if the directory exists, false otherwise
     */
    static bool isDirectory(std::string_view path);

    /**
     * @brief List files in a directory
     * @details Outputs error messages to the Global error capture if the directory cannot be accessed.
     * @param dir The directory to search
     * @return A vector of file names in the directory. If the directory cannot be accessed, an empty vector is returned.
     */
    static std::vector<std::string> listFilesInDirectory(std::string_view dir);

    /**
     * @brief Lists files and directories in a directory
     * @details Outputs error messages to the Global error capture if the directory cannot be accessed.
     * @param dir The directory to search
     * @return A vector of file and directory names in the directory. If the directory cannot be accessed, an empty vector is returned.
     */
     static std::vector<std::string> listContentInDirectory(std::string_view dir);
};
} // namespace Nebulite::Utility::IO
#endif // UTILITY_IO_FILEMANAGEMENT_HPP
