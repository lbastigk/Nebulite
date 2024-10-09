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




