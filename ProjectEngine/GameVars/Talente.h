#pragma once

#include "NamenKonventionen.h"
#include "JSONHandler.h"


class Talente {
public:
    //-----------------------------------------------------------
    //Constructor

    Talente();
    Talente(const Talente& other);
    Talente& operator=(const Talente& other);

    //-----------------------------------------------------------
    //Destructor
    ~Talente();

    //-----------------------------------------------------------
    //Marshalling

    std::string serialize();

    void deserialize(std::string serialOrLink);

    

    //-----------------------------------------------------------
    // Setting/Getting specific values

    template <typename T> void valueSet(std::string key, const T data);

    template <typename T> T valueGet(std::string key, const T& defaultValue = T());

    rapidjson::Document* getDoc() const;


private:
    rapidjson::Document doc;
};

//-----------------------------------------------------------
// Setting/Getting specific values

template <typename T> void Talente::valueSet(std::string key, const T data) {
    return JSONHandler::Set::Any<T>(doc, key, data);
}

template <typename T> T Talente::valueGet(std::string key, const T& defaultValue) {
    return JSONHandler::Get::Any<T>(doc, key, defaultValue);
}





