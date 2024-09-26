#include <iostream>
#include <fstream>
#include <map>
#include "rapidjson-master/include/rapidjson/document.h"
#include "rapidjson-master/include/rapidjson/writer.h"
#include "rapidjson-master/include/rapidjson/stringbuffer.h"

#include "Namenkonventionen.cpp"

class Options {
public:
    // Constructor initializes the default options
    Options() {
        attr[namenKonvention.options.dispResX] = "1080";
        attr[namenKonvention.options.dispResY] = "1080";
        attr[namenKonvention.options.windowName] = "coolgame";
    }

    // Method to set an option
    void SetOption(const std::string& key, const std::string& value) {
        attr[key] = value;
    }

    // Method to get an option
    std::string GetOption(const std::string& key) {
        if (attr.find(key) != attr.end()) {
            return attr[key];
        }
        return ""; // Default value when the key doesn't exist
    }

    // Method to save options to a JSON file
    bool Save() {
        rapidjson::Document doc;
        doc.SetObject();
        rapidjson::Document::AllocatorType& allocator = doc.GetAllocator();

        for (const auto& option : attr) {
            rapidjson::Value key(option.first.c_str(), allocator);
            rapidjson::Value value(option.second.c_str(), allocator);
            doc.AddMember(key, value, allocator);
        }

        rapidjson::StringBuffer buffer;
        rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
        doc.Accept(writer);

        std::ofstream file(filename_);
        if (!file.is_open()) {
            std::cerr << "Error: Could not open file for writing." << std::endl;
            return false;
        }

        file << buffer.GetString();
        file.close();

        return true;
    }

    // Method to load options from a JSON file
    bool Load() {
        std::ifstream file(filename_);
        if (!file.is_open()) {
            std::cerr << "Warning: Could not open file for reading. Using default options." << std::endl;
            return false;
        }

        std::string jsonStr((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
        file.close();

        rapidjson::Document doc;
        if (doc.Parse(jsonStr.c_str()).HasParseError() || !doc.IsObject()) {
            std::cerr << "Error: Failed to parse options JSON. Using default options." << std::endl;
            return false;
        }

        for (auto it = doc.MemberBegin(); it != doc.MemberEnd(); ++it) {
            if (it->value.IsString()) {
                attr[it->name.GetString()] = it->value.GetString();
            }
        }
        return true;
    }

    void setFileName(std::string link) {
        filename_ = link;
    }

    // Method to convert the attr map to a JSON string
    std::string ToJsonString() {
        rapidjson::Document doc;
        doc.SetObject();
        rapidjson::Document::AllocatorType& allocator = doc.GetAllocator();

        for (const auto& option : attr) {
            rapidjson::Value key(option.first.c_str(), allocator);
            rapidjson::Value value(option.second.c_str(), allocator);
            doc.AddMember(key, value, allocator);
        }

        rapidjson::StringBuffer buffer;
        rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
        doc.Accept(writer);

        return buffer.GetString();
    }

private:
    // A map to store available options and their values
    std::map<std::string, std::string> attr;
    std::string filename_;
};
