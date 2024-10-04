#pragma once

#include <iostream>
#include <string>
#include <sstream>
#include <map>

#include "Eigenschaften.h"
#include "Talente.h"
#include "Basiswerte.h"
#include "Inventar.h"

#include "NamenKonventionen.h"
#include "JSONHandler.h"



class Kreatur {
public:
    //-----------------------------------------------------------
    //Constructor

    Kreatur();
    Kreatur(const Kreatur& other);
    Kreatur& operator=(const Kreatur& other);

    //-----------------------------------------------------------
    //Destructor
    ~Kreatur();

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
template <typename T> void Kreatur::valueSet(std::string key, const T data) {
    JSONHandler::Set::Any(doc, key, data);
}

template <typename T> T Kreatur::valueGet(std::string key, const T& defaultValue) {
    JSONHandler::Get::Any(doc, key, defaultValue);
}



