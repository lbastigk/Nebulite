#pragma once

#include "NamenKonventionen.cpp"
#include "JSONHandler.cpp"


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







