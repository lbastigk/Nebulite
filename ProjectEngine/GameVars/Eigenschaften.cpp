#pragma once

#include "NamenKonventionen.cpp"
#include "JSONHandler.cpp"


class Eigenschaften {
public:
	//-----------------------------------------------------------
	//Constructor

	Eigenschaften() {
		JSONHandler::Set::Any(doc, namenKonvention.eigenschaften.charisma, 0);
		JSONHandler::Set::Any(doc, namenKonvention.eigenschaften.fingerfertigkeit, 0);
		JSONHandler::Set::Any(doc, namenKonvention.eigenschaften.gewandheit, 0);
		JSONHandler::Set::Any(doc, namenKonvention.eigenschaften.intuition, 0);
		JSONHandler::Set::Any(doc, namenKonvention.eigenschaften.klugheit, 0);
		JSONHandler::Set::Any(doc, namenKonvention.eigenschaften.koerperkraft, 0);
		JSONHandler::Set::Any(doc, namenKonvention.eigenschaften.konstitution, 0);
		JSONHandler::Set::Any(doc, namenKonvention.eigenschaften.mut, 0);
	}
	Eigenschaften(const Eigenschaften& other) {
		doc.CopyFrom(*(other.getDoc()), doc.GetAllocator());
	}
	Eigenschaften& operator=(const Eigenschaften& other) {  // Assignment operator overload
		if (this != &other) {
			JSONHandler::copyDoc(doc, other.getDoc());
		}
		return *this;
	}

	//-----------------------------------------------------------
	//Destructor
	~Eigenschaften() {
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


	//-----------------------------------------------------------
	// TODO...


private:
	rapidjson::Document doc;
};
