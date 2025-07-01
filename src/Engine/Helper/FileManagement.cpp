#include "FileManagement.h"

//--------------------------------------------------------
// FileManagement Functions

bool FileManagement::folderExists(const std::string& folderName) {
    std::filesystem::path folderPath = std::filesystem::current_path() / folderName;

    // Check if the folder exists
    return std::filesystem::exists(folderPath) && std::filesystem::is_directory(folderPath);
}

std::string FileManagement::currentDir() {
    char currentDir[150];
    (void)getcwd(currentDir, sizeof(currentDir));
    std::string currentDirStr = currentDir;
    return currentDirStr;
}

std::string FileManagement::CombinePaths(const std::string& baseDir, const std::string& innerDir) {
    std::filesystem::path basePath(baseDir);
    std::filesystem::path innerPath(innerDir);
    std::filesystem::path fullPath = basePath / innerPath;
    return fullPath.string();
}

std::string FileManagement::LoadFile(std::string link) {
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
    else{
        std::cerr << "File '" << link << "' could not be opened!" << std::endl;
    }
    return toreturn.str();
}

bool FileManagement::openFileWithDefaultProgram(const std::string& path) {
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
        
        return Platform::openFile(StringHandler::wstringToString(fullPath));
    }
    else {
        // File does not exist or is not a regular file
        return false;
    }
}

void FileManagement::WriteFile(const char filename[], std::string text) {
    std::ofstream file_;
    file_.open(filename, std::ios_base::out);
    if (file_.is_open()) {
        file_ << text;
        file_.close();
    }
}

std::string FileManagement::preferred_separator() {
    return std::string(1, FILESYSTEM_SEPARATOR);  // Create a string with 1 occurrence of the char
}



//------------------------------------------------
// FileTypeCollector functions
FileManagement::FileTypeCollector::FileTypeCollector(std::string directory, std::string fileType, bool processSubDirectories) {
    process(directory, fileType, processSubDirectories);
}

std::vector<std::string> FileManagement::FileTypeCollector::getFileDirectories(bool useFullDir) {
    if (useFullDir) {
        return fileDirectories;
    } else {
        std::vector<std::string> shortenedDirectories;
        for (const auto& entry : fileDirectories) {
            std::string shortened = entry.substr(
                std::filesystem::current_path().string().size(),
                entry.size() - std::filesystem::current_path().string().size());
            shortenedDirectories.push_back(std::string(".") + shortened);
        }
        return shortenedDirectories;
    }
}

void FileManagement::FileTypeCollector::process(const std::string& directory, const std::string& fileType, bool processSubDirectories) {
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
