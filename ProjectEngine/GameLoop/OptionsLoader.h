#pragma once

#include <string.h>

#include "document.h"
#include "writer.h"
#include "stringbuffer.h"

#include "FileManagement.h"
#include "NamenKonventionen.h"
#include "JSONHandler.h"

class Options {
public:
    // Constructor initializes the default options
    Options();

    // Method to set an option
    void SetOption(std::string key, std::string value);

    // Method to get an option
    std::string GetOption(std::string key);

    // Method to save options to a JSON file
    bool Save();

    // Method to load options from a JSON file
    bool Load();

    void setFileName(std::string link);

    // Method to convert the attr map to a JSON string
    std::string ToJsonString();

private:
    // A map to store available options and their values
    rapidjson::Document doc;
    std::string filename_;
};
