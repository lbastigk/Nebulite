#include <iostream>
#include "rapidjson-master/include/rapidjson/document.h"
#include "rapidjson-master/include/rapidjson/writer.h"
#include "rapidjson-master/include/rapidjson/stringbuffer.h"
#include <string>
#include <sstream>
#include <map>
#include "Eigenschaften.cpp"
#include "Talente.cpp"
#include "Basiswerte.cpp"
#include "Inventar.cpp"

#pragma once

#include "NamenKonventionen.cpp"
#include "JSONHandler.cpp"


class Kreatur {
public:
    //-----------------------------------------------------------
    //Constructor

    Kreatur() {
        JSONHandler::Set::Any(doc, namenKonvention.kreatur.id, 0);
        JSONHandler::Set::Any(doc, namenKonvention.kreatur.abenteuerPunkte, 0);
        JSONHandler::Set::Any(doc, namenKonvention.kreatur.steigerungsPunkte, 0);
        JSONHandler::Set::Any(doc, namenKonvention.kreatur.stufe, 1);

        Basiswerte basiswerte;
        Eigenschaften eigenschaften;
        Talente talente;
        Inventar inventar;

        JSONHandler::Set::Any(doc, namenKonvention.basiswerte._self, basiswerte.getDoc());
        JSONHandler::Set::Any(doc, namenKonvention.eigenschaften._self, eigenschaften.getDoc());
        JSONHandler::Set::Any(doc, namenKonvention.talente._self, talente.getDoc());
        JSONHandler::Set::Any(doc, namenKonvention.inventar._self, inventar.getDoc());
    }
    Kreatur(const Kreatur& other) {
        doc.CopyFrom(*(other.getDoc()), doc.GetAllocator());
    }
    Kreatur& operator=(const Kreatur& other) {  // Assignment operator overload
        if (this != &other) {
            JSONHandler::copyDoc(doc, other.getDoc());
        }
        return *this;
    }

    //-----------------------------------------------------------
    //Destructor
    ~Kreatur() {
        if (doc.IsArray()) {
            doc.Empty();
        }
    };

    //-----------------------------------------------------------
    //Marshalling

    std::string serialize() {
        return JSONHandler::serialize(doc);
    }

    void deserialize(std::string serialOrLink) {
        doc = JSONHandler::deserialize(serialOrLink);
    }

    //-----------------------------------------------------------
    // Setting/Getting specific values
    template <typename T> void valueSet(std::string key, const T data) {
        JSONHandler::Set::Any(doc, key, data);
    }

    template <typename T> T valueGet(std::string key, const T& defaultValue = T()) {
        JSONHandler::Get::Any(doc, key, defaultValue);
    }

    rapidjson::Document* getDoc() const {
        return const_cast<rapidjson::Document*>(&doc);
    }

    //-----------------------------------------------------------
    // TODO...


private:
    rapidjson::Document doc;
};



