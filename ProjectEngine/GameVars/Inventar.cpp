#include "Inventar.h"

//-----------------------------------------------------------------
// InventarObjekt

//-----------------------------------------------------------
//Constructor

InventarObjekt::InventarObjekt() {

	//-----------------------------------------------------------
	//Allgemein

	//ID
	JSONHandler::Set::Any(doc, namenKonvention.inventarObjekt.id, 0);

	//Typ
	std::pair <std::string, std::string> typPair = std::make_pair(namenKonvention.inventarObjekt.typ.verbrauchsGegenstand._self, namenKonvention.inventarObjekt.typ.text.notiz);
	JSONHandler::Set::Any(doc, namenKonvention.inventarObjekt.typ._self, typPair);

	//Effektdauer in Sekunden
	JSONHandler::Set::Any(doc, namenKonvention.inventarObjekt.effektDauer, 0);

	//-----------------------------------------------------------
	//Effekte und Benötigt zum doc ergänzen

	//Subdoc für Effekte
	Basiswerte Basiswerte1;
	Eigenschaften Eigenschaften1;
	Talente Talente1;
	rapidjson::Document docEffekt;
	JSONHandler::Set::subDoc(docEffekt, namenKonvention.inventarObjekt.effekt.Basiswerte, *Basiswerte1.getDoc());
	JSONHandler::Set::subDoc(docEffekt, namenKonvention.inventarObjekt.effekt.Eigenschaften, *Eigenschaften1.getDoc());
	JSONHandler::Set::subDoc(docEffekt, namenKonvention.inventarObjekt.effekt.Talente, *Talente1.getDoc());

	//Subdocs für Benötigt
	Basiswerte Basiswerte2;
	Eigenschaften Eigenschaften2;
	Talente Talente2;
	rapidjson::Document docBenötigt;
	JSONHandler::Set::subDoc(docBenötigt, namenKonvention.inventarObjekt.effekt.Basiswerte, *Basiswerte2.getDoc());
	JSONHandler::Set::subDoc(docBenötigt, namenKonvention.inventarObjekt.effekt.Eigenschaften, *Eigenschaften2.getDoc());
	JSONHandler::Set::subDoc(docBenötigt, namenKonvention.inventarObjekt.effekt.Talente, *Talente2.getDoc());
	
	//Zu hauptdoc hinzufügen
	JSONHandler::Set::subDoc(doc, namenKonvention.inventarObjekt.effekt._self, docEffekt);
	JSONHandler::Set::subDoc(doc, namenKonvention.inventarObjekt.benötigt._self, docBenötigt);
}
InventarObjekt::InventarObjekt(const InventarObjekt& other) {
	doc.CopyFrom(*(other.getDoc()), doc.GetAllocator());
}
InventarObjekt& InventarObjekt::operator=(const InventarObjekt& other) {  // Assignment operator overload
	if (this != &other) {
		JSONHandler::copyDoc(doc, other.getDoc());
	}
	return *this;
}

//-----------------------------------------------------------
//Destructor
InventarObjekt::~InventarObjekt() {
	if (doc.IsArray()) {
		doc.Empty();
	}
};

//-----------------------------------------------------------
//Marshalling

std::string InventarObjekt::serialize() {
	return JSONHandler::serialize(doc);
}

void InventarObjekt::deserialize(std::string serialOrLink) {
	doc = JSONHandler::deserialize(serialOrLink);
}


rapidjson::Document* InventarObjekt::getDoc() const {
	return const_cast<rapidjson::Document*>(&doc);
}




//--------------------------------------------------------------------------
// Inventar

//-----------------------------------------------------------
//Constructor

Inventar::Inventar() {
	
}
Inventar::Inventar(const Inventar& other) {
	doc.CopyFrom(*(other.getDoc()), doc.GetAllocator());
}
Inventar& Inventar::operator=(const Inventar& other) {  // Assignment operator overload
	// Check for self-assignment
	if (this != &other) {
		// Deep copy the rapidjson::Document
		doc.CopyFrom(*(other.getDoc()), doc.GetAllocator());
		// Alternatively, you can use the assignment operator: doc = other.doc;
	}
	return *this;
}

//-----------------------------------------------------------
//Destructor
Inventar::~Inventar() {
	if (doc.IsArray()) {
		doc.Empty();
	}
};

//-----------------------------------------------------------
//Marshalling

std::string Inventar::serialize() {
	return JSONHandler::serialize(doc);
}

void Inventar::deserialize(std::string serialOrLink) {
	doc = JSONHandler::deserialize(serialOrLink);
}



rapidjson::Document* Inventar::getDoc() const {
	//return &doc;
	return const_cast<rapidjson::Document*>(&doc);
}