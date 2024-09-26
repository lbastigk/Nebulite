#pragma once

#include <iostream>
#include <sstream>

#include "NamenKonventionen.cpp"
#include "JSONHandler.cpp"


class MyTemplate {
public:
	//-----------------------------------------------------------
	//Constructor

	MyTemplate() {
		JSONHandler::Set::Any(doc, namenKonvention.MyTemplate.bsp1, 0);
	}
	MyTemplate(const MyTemplate& other) {
		doc.CopyFrom(*(other.getDoc()), doc.GetAllocator());
	}
	MyTemplate& operator=(const MyTemplate& other) {  // Assignment operator overload
		// Check for self-assignment
		if (this != &other) {
			JSONHandler::copyDoc(doc, other.getDoc());
		}
		return *this;
	}

	//-----------------------------------------------------------
	//Destructor
	~MyTemplate() {
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
		JSONHandler::Set::Any<T>(doc, key, data);
	}

	template <typename T> T valueGet(std::string key, const T& defaultValue = T()) const {
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