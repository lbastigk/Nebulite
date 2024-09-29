#pragma once

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
	//Effekte und Ben�tigt zum doc erg�nzen

	//Objekte
	Basiswerte Basiswerte;
	Eigenschaften Eigenschaften;
	Talente Talente;

	//Subdocs
	rapidjson::Document docEffekt;
	rapidjson::Document docBen�tigt;

	//Subdocs f�llen
	JSONHandler::Set::Any(docEffekt, namenKonvention.inventarObjekt.effekt.Basiswerte, Basiswerte.getDoc());
	JSONHandler::Set::Any(docEffekt, namenKonvention.inventarObjekt.effekt.Eigenschaften, Eigenschaften.getDoc());
	JSONHandler::Set::Any(docEffekt, namenKonvention.inventarObjekt.effekt.Talente, Talente.getDoc());
	JSONHandler::Set::Any(docBen�tigt, namenKonvention.inventarObjekt.effekt.Basiswerte, Basiswerte.getDoc());
	JSONHandler::Set::Any(docBen�tigt, namenKonvention.inventarObjekt.effekt.Eigenschaften, Eigenschaften.getDoc());
	JSONHandler::Set::Any(docBen�tigt, namenKonvention.inventarObjekt.effekt.Talente, Talente.getDoc());
	
	//Zu hauptdoc hinzuf�gen
	JSONHandler::Set::Any(doc, namenKonvention.inventarObjekt.effekt._self, &docEffekt);
	JSONHandler::Set::Any(doc, namenKonvention.inventarObjekt.ben�tigt._self, &docBen�tigt);

	docEffekt.Empty();
	docBen�tigt.Empty();
}
InventarObjekt::InventarObjekt(const InventarObjekt& other) {
	doc.CopyFrom(*(other.getDoc()), doc.GetAllocator());
}
InventarObjekt::InventarObjekt& operator=(const InventarObjekt& other) {  // Assignment operator overload
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

//-----------------------------------------------------------
// Setting/Getting specific values
template <typename T> void InventarObjekt::valueSet(std::string key, const T data) {
	JSONHandler::Set::Any(doc, key, data);
}

rapidjson::Document* InventarObjekt::getDoc() const {
	return const_cast<rapidjson::Document*>(&doc);
}

template <typename T> T InventarObjekt::valueGet(std::string key, const T& defaultValue = T()) {
	JSONHandler::Get::Any(doc, key, defaultValue);
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
Inventar::Inventar& operator=(const Inventar& other) {  // Assignment operator overload
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

//-----------------------------------------------------------
// Setting/Getting specific values

template <typename T> void Inventar::valueSet(std::string key, const T data) {
	return JSONHandler::Set::Any<T>(doc, key, data);
}

template <typename T> T Inventar::valueGet(std::string key, const T& defaultValue = T()) {
	return JSONHandler::Get::Any<T>(doc, key, defaultValue);
}

rapidjson::Document* Inventar::getDoc() const {
	//return &doc;
	return const_cast<rapidjson::Document*>(&doc);
}