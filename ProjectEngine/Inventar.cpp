#pragma once

#include <Eigenschaften.cpp>
#include <Talente.cpp>
#include <Basiswerte.cpp>

#include "NamenKonventionen.cpp"
#include "JSONHandler.cpp"


class InventarObjekt {
public:
	//-----------------------------------------------------------
	//Constructor

	InventarObjekt() {
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

		//Objekte
		Basiswerte Basiswerte;
		Eigenschaften Eigenschaften;
		Talente Talente;

		//Subdocs
		rapidjson::Document docEffekt;
		rapidjson::Document docBenötigt;

		//Subdocs füllen
		JSONHandler::Set::Any(docEffekt, namenKonvention.inventarObjekt.effekt.Basiswerte, Basiswerte.getDoc());
		JSONHandler::Set::Any(docEffekt, namenKonvention.inventarObjekt.effekt.Eigenschaften, Eigenschaften.getDoc());
		JSONHandler::Set::Any(docEffekt, namenKonvention.inventarObjekt.effekt.Talente, Talente.getDoc());
		JSONHandler::Set::Any(docBenötigt, namenKonvention.inventarObjekt.effekt.Basiswerte, Basiswerte.getDoc());
		JSONHandler::Set::Any(docBenötigt, namenKonvention.inventarObjekt.effekt.Eigenschaften, Eigenschaften.getDoc());
		JSONHandler::Set::Any(docBenötigt, namenKonvention.inventarObjekt.effekt.Talente, Talente.getDoc());
		
		//Zu hauptdoc hinzufügen
		JSONHandler::Set::Any(doc, namenKonvention.inventarObjekt.effekt._self, &docEffekt);
		JSONHandler::Set::Any(doc, namenKonvention.inventarObjekt.benötigt._self, &docBenötigt);

		docEffekt.Empty();
		docBenötigt.Empty();
	}
	InventarObjekt(const InventarObjekt& other) {
		doc.CopyFrom(*(other.getDoc()), doc.GetAllocator());
	}
	InventarObjekt& operator=(const InventarObjekt& other) {  // Assignment operator overload
		if (this != &other) {
			JSONHandler::copyDoc(doc, other.getDoc());
		}
		return *this;
	}

	//-----------------------------------------------------------
	//Destructor
	~InventarObjekt() {
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

	rapidjson::Document* getDoc() const {
		return const_cast<rapidjson::Document*>(&doc);
	}

	template <typename T> T valueGet(std::string key, const T& defaultValue = T()) {
		JSONHandler::Get::Any(doc, key, defaultValue);
	}

	//-----------------------------------------------------------
	// TODO...
	
	
private:
	rapidjson::Document doc;
};


class Inventar {
public:
	//-----------------------------------------------------------
	//Constructor

	Inventar() {
		
	}
	Inventar(const Inventar& other) {
		doc.CopyFrom(*(other.getDoc()), doc.GetAllocator());
	}
	Inventar& operator=(const Inventar& other) {  // Assignment operator overload
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
	~Inventar() {
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
		return JSONHandler::Set::Any<T>(doc, key, data);
	}

	template <typename T> T valueGet(std::string key, const T& defaultValue = T()) {
		return JSONHandler::Get::Any<T>(doc, key, defaultValue);
	}

	rapidjson::Document* getDoc() const {
		//return &doc;
		return const_cast<rapidjson::Document*>(&doc);
	}

	//-----------------------------------------------------------
	// TODO...


private:
	rapidjson::Document doc;
};