#include <iostream>
#include <fstream>
#include <map>
#include "document.h"
#include "writer.h"
#include "stringbuffer.h"

#include "Helper/NamenKonventionen.h"

class Options {
public:
    // Constructor initializes the default options
    Options();

    // Method to set an option
    void SetOption(const std::string& key, const std::string& value);

    // Method to get an option
    std::string GetOption(const std::string& key);

    // Method to save options to a JSON file
    bool Save();

    // Method to load options from a JSON file
    bool Load();

    void setFileName(std::string link);

    // Method to convert the attr map to a JSON string
    std::string ToJsonString();

private:
    // A map to store available options and their values
    std::map<std::string, std::string> attr;
    std::string filename_;
};
