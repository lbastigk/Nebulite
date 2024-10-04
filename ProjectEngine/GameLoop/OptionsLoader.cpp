#include "OptionsLoader.h"

// Constructor initializes the default options
Options::Options() {
    doc[namenKonvention.options.dispResX.c_str()] = "1080";
    doc[namenKonvention.options.dispResY.c_str()] = "1080";
    doc[namenKonvention.options.windowName.c_str()] = "coolgame";
}

// Method to set an option
void Options::SetOption(std::string key, std::string value) {
    JSONHandler::Set::Any<std::string>(doc,key,value);
}

// Method to get an option
std::string Options::GetOption(std::string key) {
    if (doc.HasMember(key.c_str())) {
        return JSONHandler::Get::Any<std::string>(doc,key);
    }
    return ""; // Default value when the key doesn't exist
}

// Method to save options to a JSON file
bool Options::Save() {
    FileManagement::WriteFile(filename_.c_str(),JSONHandler::serialize(doc));
    return true;
}

// Method to load options from a JSON file
bool Options::Load() {
    std::string file = FileManagement::LoadFile(FileManagement::CombinePaths(FileManagement::currentDir(),filename_));
    doc = JSONHandler::deserialize(file);
}

void Options::setFileName(std::string link) {
    filename_ = link;
}

// Method to convert the attr map to a JSON string
std::string Options::ToJsonString() {
    return JSONHandler::serialize(doc);
}
