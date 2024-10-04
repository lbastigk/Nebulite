#include "JSONHandler.h"

//------------------------------------------------
// Get


void JSONHandler::Get::subDocOld(rapidjson::Document& doc, const std::string& key, rapidjson::Document& destination) {
    // Check if the main document is an object
    if (!doc.IsObject()) {
        // Handle error: The main document should be an object.
        return;
    }

    // Find the iterator for the key in the main document
    auto it = doc.FindMember(key.c_str());

    // Check if the key exists in the main document
    if (it == doc.MemberEnd()) {
        // Handle error: The key does not exist in the main document.
        return;
    }

    // Get the sub-document from the main document
    rapidjson::Value keyName(key.c_str(), doc.GetAllocator());

    //convert to string
    std::string s = JSONHandler::ConvertJSONValue(doc[keyName]);

    //convert to doc
    rapidjson::Document temp = JSONHandler::deserialize(s);

    //--------------------------------------------------------------------------
    // Reallocation 

    //Clear destination
    JSONHandler::empty(destination);

    //copy to destination from temp
    destination.Swap(temp);

    //clear temp
    JSONHandler::empty(temp);
}

void JSONHandler::Get::subDoc(rapidjson::Document& doc, const std::string& key, rapidjson::Document& destination) {
    // Check if the main document is an object
    if (!doc.IsObject()) {
        // Handle error: The main document should be an object.
        return;
    }

    // Find the iterator for the key in the main document
    auto it = doc.FindMember(key.c_str());

    // Check if the key exists in the main document
    if (it == doc.MemberEnd()) {
        // Handle error: The key does not exist in the main document.
        return;
    }

    //put into temp
    rapidjson::Document temp;
    temp.CopyFrom(doc[key.c_str()],temp.GetAllocator());

    //--------------------------------------------------------------------------
    // Reallocation 

    //Clear destination
    JSONHandler::empty(destination);

    //copy to destination from temp
    destination.Swap(temp);

    //clear temp
    //JSONHandler::empty(temp); //NOT NEEDED, memory gets freed
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
    // Ensure that the document is an object
    if (!doc.IsObject()) {
        doc.SetObject();
    }

    if (!subdoc.IsObject()) {
        subdoc.SetObject();
    }

    // Find the iterator for the key in the main document
    auto it = doc.FindMember(key.c_str());

    // If the key already exists, remove it
    if (it != doc.MemberEnd()) {
        //Without this, the AddMember part will add multiples of the same key!
        doc.RemoveMember(it);
    }
    
    //Set value
    rapidjson::Value keyName(key.c_str(), doc.GetAllocator());
    doc.AddMember(keyName, subdoc, doc.GetAllocator());

    //Get rid of memory leaks by doing a new copy: doc -> string -> doc
    rapidjson::Document temp;
    //temp = JSONHandler::deserialize(JSONHandler::serialize(doc)); //NOT NEEDED; COPYFROM WORKS!!!
    temp.CopyFrom(doc,temp.GetAllocator());

    // Clear subdoc to release its memory?
    //JSONHandler::empty(subdoc);

    //Clear main doc
    //JSONHandler::empty(doc);  //NOT NEEDED; SWAP WORKS FINE

    //copy to main doc from temp
    doc.Swap(temp);

    //clear temp
    //JSONHandler::empty(temp); //NOT NEEDED, memory gets freed
}

//------------------------------------------------
// General Functions
rapidjson::Document JSONHandler::deserialize(std::string serialOrLink) {
    rapidjson::Document doc;
    rapidjson::ParseResult res = doc.Parse(serialOrLink.c_str());
    if (res.IsError()) {
        std::string JSONString = FileManagement::LoadFile(serialOrLink.c_str());
        doc.Parse(JSONString.c_str());
    }
    return doc;
}

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



//----------------------------------------------------------------------
// Helper function to convert data to JSON values (specializations may be required for custom types)
template <typename T>
void JSONHandler::ConvertToJSONValue(const T& data, rapidjson::Value& jsonValue, rapidjson::Document::AllocatorType& allocator) {
    //Single Variable Objects
    if constexpr (std::is_same_v<T, bool>) {
        jsonValue.SetBool(data);
    }
    else if constexpr (std::is_same_v<T, int>) {
        jsonValue.SetInt(data);
    }
    else if constexpr (std::is_same_v<T, float>) {
        jsonValue.SetFloat(data);
    }
    else if constexpr (std::is_same_v<T, double>) {
        jsonValue.SetDouble(data);
    }
    else if constexpr (std::is_same_v<T, float>) {
        jsonValue.SetDouble(static_cast<double>(data));
    }

    //Strings and similiar
    else if constexpr (std::is_same_v<T, std::string>) {    //C++ string
        jsonValue.SetString(data.c_str(), allocator);
    }
    else if constexpr (std::is_same_v<T, const char*>) {    //Const char string: direct passing into argument of a string
        jsonValue.SetString(data, allocator);
    }
    else if constexpr (std::is_same_v<T, char*>) {          //char string
        jsonValue.SetString(data, allocator);
    }


    //Multivariable Objects
    else if constexpr (is_pair<T>::value) {
        // Create a JSON array for the pair
        rapidjson::Value jsonArray(rapidjson::kArrayType);

        // Convert each item in the pair to a JSON value
        rapidjson::Value firstValue;
        ConvertToJSONValue(data.first, firstValue, allocator);
        jsonArray.PushBack(firstValue, allocator);

        rapidjson::Value secondValue;
        ConvertToJSONValue(data.second, secondValue, allocator);
        jsonArray.PushBack(secondValue, allocator);

        // Write into the final jsonValue
        jsonValue = jsonArray;
    }
    else if constexpr (is_map<T>::value) {
        rapidjson::Value jsonMap(rapidjson::kObjectType);

        for (const auto& entry : data) {
            rapidjson::Value key(entry.first.c_str(), allocator);
            rapidjson::Value value;
            ConvertToJSONValue(entry.second, value, allocator);
            jsonMap.AddMember(key, value, allocator);
        }
        jsonValue = jsonMap;
    }
    else if constexpr (is_vector<T>::value) {
        // Create a JSON array for the vector
        rapidjson::Value jsonArray(rapidjson::kArrayType);

        for (const auto& item : data) {
            // Convert each item in the vector to a JSON value
            rapidjson::Value jsonValue;
            ConvertToJSONValue(item, jsonValue, allocator);

            // Add the JSON value to the array
            jsonArray.PushBack(jsonValue, allocator);
        }
        jsonValue = jsonArray;
    }
    else if constexpr (is_rapidjson_document_ptr<T>::value) {
        jsonValue.CopyFrom(*data, allocator);
    }
    else if constexpr (is_rapidjson_value_ptr<T>::value) {
        return data;
    }
    

    //ELSE, rapidjson value directly
    else {
        
    }
}
template <typename T>
void JSONHandler::ConvertFromJSONValue(const rapidjson::Value& jsonValue, T& result) {
    // Implement the reverse conversion logic for each supported type
    if constexpr (std::is_same_v<T, bool>) {
        result = jsonValue.GetBool();
    }
    else if constexpr (std::is_same_v<T, int>) {
        result = jsonValue.GetInt();
    }
    else if constexpr (std::is_same_v<T, float>) {
        result = jsonValue.GetFloat();
    }
    else if constexpr (std::is_same_v<T, double>) {
        result = jsonValue.GetDouble();
    }
    else if constexpr (std::is_same_v<T, std::string>) {
        //allow more flexibility for return type string:

        if (jsonValue.IsBool()) {
            result = jsonValue.GetBool() ? "true" : "false";
        }
        else if (jsonValue.IsString()) {
            result = std::string(jsonValue.GetString());
        }
        else if (jsonValue.IsInt()) {
            result = std::to_string(jsonValue.GetInt());
        }
        else if (jsonValue.IsUint()) {
            result = std::to_string(jsonValue.GetUint());
        }
        else if (jsonValue.IsInt64()) {
            result = std::to_string(jsonValue.GetInt64());
        }
        else if (jsonValue.IsUint64()) {
            result = std::to_string(jsonValue.GetUint64());
        }
        else if (jsonValue.IsDouble()) {
            result = std::to_string(jsonValue.GetDouble());
        }
        else if (jsonValue.IsNull()) {
            result = "null";
        }
        else if (jsonValue.IsArray()) {
            result = "{Array}";
        }
        else if (jsonValue.IsObject()) {
            result = "{Object}";  // Just a placeholder since objects can't easily be converted to a single string
        }
        else {
            result = "unsupported type";
        }
        
    }
    else if constexpr (std::is_same_v<T, rapidjson::Document>) {
        result.CopyFrom(jsonValue, result.GetAllocator());
    }
    else if constexpr (is_rapidjson_document_ptr<T>::value) {
        result.CopyFrom(jsonValue, result->GetAllocator());
    }
    // Add more conversions for other types as needed
    // ...

    else {
        std::cout << "???\n";
        // Handle other data types here or provide specializations as needed
    }
}