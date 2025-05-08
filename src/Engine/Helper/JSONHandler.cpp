#include "JSONHandler.h"

//------------------------------------------------
// Get

void JSONHandler::Get::subDoc(rapidjson::Document& doc, const std::string& key, rapidjson::Document& destination) {
    // Check if the main document is an object
    if (!doc.IsObject()) {
        return; // Handle error
    }

    auto it = doc.FindMember(key.c_str());
    if (it == doc.MemberEnd()) {
        return; // Handle error
    }

    // Clear destination
    JSONHandler::empty(destination);

    // Copy value directly into destination
    destination.CopyFrom(it->value, destination.GetAllocator());
}

void JSONHandler::Get::listOfKeys(rapidjson::Document& doc, std::vector<std::string>& keys) {
    // Check if the document is an object
    if (!doc.IsObject()) {
        // Handle error, throw exception, or return as appropriate
        return;
    }

    // Iterate over the members of the object
    for (rapidjson::Value::ConstMemberIterator itr = doc.MemberBegin(); itr != doc.MemberEnd(); ++itr) {
        // Add the key to the vector
        keys.push_back(itr->name.GetString());
    }
}

int JSONHandler::Get::keyAmount(rapidjson::Document& doc) {
    int amt = 0;

    // Check if the document is an object
    if (!doc.IsObject()) {
        // Handle error, throw exception, or return as appropriate
        return amt;
    }

    // Iterate over the members of the object
    for (rapidjson::Value::ConstMemberIterator itr = doc.MemberBegin(); itr != doc.MemberEnd(); ++itr) {
        amt++;
    }
    return amt;
}

//------------------------------------------------
// Set

void JSONHandler::Set::subDoc(rapidjson::Document& doc, const std::string& key, rapidjson::Value& subdoc) {
    if (!doc.IsObject()) {
        doc.SetObject();
    }

    if (!subdoc.IsObject()) {
        subdoc.SetObject();
    }

    // Remove the existing key if it exists
    auto it = doc.FindMember(key.c_str());
    if (it != doc.MemberEnd()) {
        doc.RemoveMember(it);
    }

    // Create a deep copy of subdoc using doc's allocator
    rapidjson::Value subdocCopy;
    subdocCopy.CopyFrom(subdoc, doc.GetAllocator());

    // Key must be constructed with doc's allocator as well
    rapidjson::Value keyName(key.c_str(), doc.GetAllocator());

    // Add the deep-copied subdocument
    doc.AddMember(keyName, subdocCopy, doc.GetAllocator());
    

    //subdocCopy.SetNull();
    //subdocCopy.Clear();
}




//------------------------------------------------
// General Functions
/*
rapidjson::Document JSONHandler::deserialize(std::string serialOrLink) {
    rapidjson::Document doc;
    if(serialOrLink.starts_with("{")){
        rapidjson::ParseResult res = doc.Parse(serialOrLink.c_str());
    }
    else{
        std::string JSONString = FileManagement::LoadFile(serialOrLink.c_str());
        //std::cerr << "Loading file for:" << serialOrLink << std::endl;
        doc.Parse(JSONString.c_str());
    }
    return doc;
}
*/

rapidjson::Document JSONHandler::deserialize(std::string serialOrLink) {
    rapidjson::Document doc;

    if (serialOrLink.starts_with("{")) {
        rapidjson::ParseResult res = doc.Parse(serialOrLink.c_str());
    } else {
        // Split the input string by '|'
        std::vector<std::string> tokens;
        size_t start = 0;
        size_t end = 0;
        while ((end = serialOrLink.find('|', start)) != std::string::npos) {
            tokens.push_back(serialOrLink.substr(start, end - start));
            start = end + 1;
        }
        tokens.push_back(serialOrLink.substr(start)); // Last part

        if (tokens.empty()) {
            // Error: No file path given
            return doc; // or handle error properly
        }

        // Load the JSON file
        std::string JSONString = FileManagement::LoadFile(tokens[0].c_str());
        doc.Parse(JSONString.c_str());

        // Now apply modifications
        for (size_t i = 1; i < tokens.size(); ++i) {
            const std::string& assignment = tokens[i];
            size_t eqPos = assignment.find('=');
            if (eqPos != std::string::npos) {
                std::string key = assignment.substr(0, eqPos);
                std::string value = assignment.substr(eqPos + 1);

                JSONHandler::Set::Any<std::string>(doc, key, value);
            }
        }
    }
    return doc;
}


// Only used for loading/saving, not recommended during loop due to performance!
// Use Get/Set instead
std::string JSONHandler::serialize(const rapidjson::Document& doc) {
    rapidjson::StringBuffer buffer;
    rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(buffer);
    doc.Accept(writer);
    return buffer.GetString();
}

std::string JSONHandler::serializeVal(const rapidjson::Value& val) {
    rapidjson::StringBuffer buffer;
    rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(buffer);
    val.Accept(writer);
    return buffer.GetString();
}

std::string JSONHandler::ConvertJSONValue(rapidjson::Value& jsonValue) {
    rapidjson::StringBuffer buffer;
    rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
    jsonValue.Accept(writer);
    return buffer.GetString();
}

void JSONHandler::copyDoc(rapidjson::Document& destination, rapidjson::Document *toCopy) {
    destination.CopyFrom(*(toCopy), destination.GetAllocator());
}

void JSONHandler::empty(rapidjson::Document &doc) {
    doc.SetNull();
    doc.GetAllocator().Clear();
}



// Checks if string is a valid json doc

bool JSONHandler::isValid(std::string str) {
    rapidjson::Document document;

    // Parse the JSON string
    document.Parse(str.c_str());

    // Check if parsing was successful
    if (document.HasParseError()) {
        // Optionally, print the error message
        // const rapidjson::ParseErrorCode errorCode = document.GetParseError();
        // const char* errorMessage = rapidjson::GetParseError_En(errorCode);
        // std::cerr << "Parse error: " << errorMessage << std::endl;

        return false; 
    }
    return true;
}


