#pragma once

#include "NamenKonventionen.cpp"
#include "JSONHandler.cpp"


class Basiswerte {
public:
	//-----------------------------------------------------------
	//Constructor

	Basiswerte() {
		JSONHandler::Set::Any(doc, namenKonvention.basiswerte.astralenergie, 0);
		JSONHandler::Set::Any(doc, namenKonvention.basiswerte.attacke, 0);
		JSONHandler::Set::Any(doc, namenKonvention.basiswerte.ausdauer, 0);
		JSONHandler::Set::Any(doc, namenKonvention.basiswerte.ausweichen, 0);
		JSONHandler::Set::Any(doc, namenKonvention.basiswerte.fernkampf, 0);
		JSONHandler::Set::Any(doc, namenKonvention.basiswerte.lebensenergie, 0);
		JSONHandler::Set::Any(doc, namenKonvention.basiswerte.magieresistenz, 0);
		JSONHandler::Set::Any(doc, namenKonvention.basiswerte.parade, 0);
		JSONHandler::Set::Any(doc, namenKonvention.basiswerte.trefferPunkteWaffe, 0);
		JSONHandler::Set::Any(doc, namenKonvention.basiswerte.trefferPunkteBoni, 0);
	}
	Basiswerte(const Basiswerte& other) {
		doc.CopyFrom(*(other.getDoc()), doc.GetAllocator());
	}
	Basiswerte& operator=(const Basiswerte& other) {  // Assignment operator overload
		if (this != &other) {
			JSONHandler::copyDoc(doc, other.getDoc());
		}
		return *this;
	}

	//-----------------------------------------------------------
	//Destructor
	~Basiswerte() {
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
		return const_cast<rapidjson::Document*>(&doc);
	}

private:
	rapidjson::Document doc;
};
