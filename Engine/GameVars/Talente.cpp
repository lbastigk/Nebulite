#include "Talente.h"


//-----------------------------------------------------------
//Constructor

Talente::Talente() {
    int i = 1;

    //Temp doc for hierarchy
    rapidjson::Document temp;

    //koerper
    JSONHandler::Set::Any(temp, namenKonvention.talente.koerper.schleichen, 0);
    JSONHandler::Set::Any(temp, namenKonvention.talente.koerper.selbstbeherrschung, 0);
    JSONHandler::Set::Any(temp, namenKonvention.talente.koerper.sinnesschaerfe, 0);
    JSONHandler::Set::Any(temp, namenKonvention.talente.koerper.taschendiebstahl, 0);
    JSONHandler::Set::Any(temp, namenKonvention.talente.koerper.zwergennase, 0);
    JSONHandler::Set::subDoc(doc, namenKonvention.talente.koerper._self, temp);    //add to main doc
    JSONHandler::empty(temp);

    //natur
    JSONHandler::Set::Any(temp, namenKonvention.talente.natur.fallenstellen, 0);
    JSONHandler::Set::Any(temp, namenKonvention.talente.natur.pflanzenkunde, 0);
    JSONHandler::Set::Any(temp, namenKonvention.talente.natur.tierkunde, 0);
    JSONHandler::Set::Any(temp, namenKonvention.talente.natur.wildnisleben, 0);
    JSONHandler::Set::subDoc(doc, namenKonvention.talente.natur._self, temp);    //add to main doc
    JSONHandler::empty(temp);

    //wissen
    JSONHandler::Set::Any(temp, namenKonvention.talente.wissen.gassenwissen, 0);
    JSONHandler::Set::Any(temp, namenKonvention.talente.wissen.heilkundeGift, 0);
    JSONHandler::Set::Any(temp, namenKonvention.talente.wissen.heilkundeWunden, 0);
    JSONHandler::Set::Any(temp, namenKonvention.talente.wissen.magiekunde, 0);
    JSONHandler::Set::subDoc(doc, namenKonvention.talente.wissen._self, temp);    //add to main doc
    JSONHandler::empty(temp);

    //gesellschaft
    JSONHandler::Set::Any(temp, namenKonvention.talente.gesellschaft.betoeren, 0);
    JSONHandler::Set::Any(temp, namenKonvention.talente.gesellschaft.etikette, 0);
    JSONHandler::Set::Any(temp, namenKonvention.talente.gesellschaft.feilschen, 0);
    JSONHandler::Set::Any(temp, namenKonvention.talente.gesellschaft.menschenkenntnis, 0);
    JSONHandler::Set::Any(temp, namenKonvention.talente.gesellschaft.ueberreden, 0);
    JSONHandler::Set::subDoc(doc, namenKonvention.talente.gesellschaft._self, temp);    //add to main doc
    JSONHandler::empty(temp);

    //handwerk
    JSONHandler::Set::Any(temp, namenKonvention.talente.handwerk.alchimie, 0);
    JSONHandler::Set::Any(temp, namenKonvention.talente.handwerk.bogenbau, 0);
    JSONHandler::Set::Any(temp, namenKonvention.talente.handwerk.fallenEntschaerfen, 0);
    JSONHandler::Set::Any(temp, namenKonvention.talente.handwerk.schloesserKnacken, 0);
    JSONHandler::Set::Any(temp, namenKonvention.talente.handwerk.schmieden, 0);
    JSONHandler::Set::subDoc(doc, namenKonvention.talente.handwerk._self, temp);    //add to main doc
    JSONHandler::empty(temp);

    //nahkampf
    JSONHandler::Set::Any(temp, namenKonvention.talente.nahkampf.dolche, 0);
    JSONHandler::Set::Any(temp, namenKonvention.talente.nahkampf.fechtwaffen, 0);
    JSONHandler::Set::Any(temp, namenKonvention.talente.nahkampf.hiebwaffen, 0);
    JSONHandler::Set::Any(temp, namenKonvention.talente.nahkampf.raufen, 0);
    JSONHandler::Set::Any(temp, namenKonvention.talente.nahkampf.saebel, 0);
    JSONHandler::Set::Any(temp, namenKonvention.talente.nahkampf.schwerter, 0);
    JSONHandler::Set::Any(temp, namenKonvention.talente.nahkampf.speere, 0);
    JSONHandler::Set::Any(temp, namenKonvention.talente.nahkampf.staebe, 0);
    JSONHandler::Set::Any(temp, namenKonvention.talente.nahkampf.zweihandHiebwaffen, 0);
    JSONHandler::Set::Any(temp, namenKonvention.talente.nahkampf.zweihandSchwerter, 0);
    JSONHandler::Set::subDoc(doc, namenKonvention.talente.nahkampf._self, temp);    //add to main doc
    JSONHandler::empty(temp);

    //fernkampf
    JSONHandler::Set::Any(temp, namenKonvention.talente.fernkampf.armbrust, 0);
    JSONHandler::Set::Any(temp, namenKonvention.talente.fernkampf.bogen, 0);
    JSONHandler::Set::Any(temp, namenKonvention.talente.fernkampf.wurfwaffen, 0);
    JSONHandler::Set::subDoc(doc, namenKonvention.talente.fernkampf._self, temp);    //add to main doc
    JSONHandler::empty(temp);
}
Talente::Talente(const Talente& other) {
    doc.CopyFrom(*(other.getDoc()), doc.GetAllocator());
}
Talente& Talente::operator=(const Talente& other) {  // Assignment operator overload
    if (this != &other) {
        JSONHandler::copyDoc(doc, other.getDoc());
    }
    return *this;
}

//-----------------------------------------------------------
//Destructor
Talente::~Talente() {
    if (doc.IsArray()) {
        doc.Empty();
    }
};

//-----------------------------------------------------------
//Marshalling

std::string Talente::serialize() {
    return JSONHandler::serialize(doc);
}

void Talente::deserialize(std::string serialOrLink) {
    doc = JSONHandler::deserialize(serialOrLink);
}


rapidjson::Document* Talente::getDoc() const {
    return const_cast<rapidjson::Document*>(&doc);
}






