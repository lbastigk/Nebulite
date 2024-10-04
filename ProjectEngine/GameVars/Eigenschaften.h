#pragma once

#include "NamenKonventionen.h"
#include "JSONHandler.h"


class Eigenschaften {
public:
	//-----------------------------------------------------------
	//Constructor

	Eigenschaften();
	Eigenschaften(const Eigenschaften& other);
	Eigenschaften& operator=(const Eigenschaften& other);

	//-----------------------------------------------------------
	//Destructor
	~Eigenschaften();

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

//-----------------------------------------------------------
// Setting/Getting specific values

template <typename T> void Eigenschaften::valueSet(std::string key, const T data) {
	return JSONHandler::Set::Any<T>(doc, key, data);
}

template <typename T> T Eigenschaften::valueGet(std::string key, const T& defaultValue) {
	return JSONHandler::Get::Any<T>(doc, key, defaultValue);
}