#pragma once

#include "Helper/NamenKonventionen.h"
#include "JSONHandler.h"


class Basiswerte {
public:
	Basiswerte();
	Basiswerte(const Basiswerte& other);
	Basiswerte& operator=(const Basiswerte& other);

	//-----------------------------------------------------------
	//Destructor
	~Basiswerte();

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
