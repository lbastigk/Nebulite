#include "Kreatur.h"


//-----------------------------------------------------------
//Constructor

Kreatur::Kreatur() {
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
Kreatur::Kreatur(const Kreatur& other) {
    doc.CopyFrom(*(other.getDoc()), doc.GetAllocator());
}
Kreatur& Kreatur::operator=(const Kreatur& other) {  // Assignment operator overload
    if (this != &other) {
        JSONHandler::copyDoc(doc, other.getDoc());
    }
    return *this;
}

//-----------------------------------------------------------
//Destructor
Kreatur::~Kreatur() {
    if (doc.IsArray()) {
        doc.Empty();
    }
};

//-----------------------------------------------------------
//Marshalling

std::string Kreatur::serialize() {
    return JSONHandler::serialize(doc);
}

void Kreatur::deserialize(std::string serialOrLink) {
    doc = JSONHandler::deserialize(serialOrLink);
}


rapidjson::Document* Kreatur::getDoc() const {
    return const_cast<rapidjson::Document*>(&doc);
}

