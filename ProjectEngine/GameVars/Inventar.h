#pragma once

#include <Eigenschaften.h>
#include <Talente.h>
#include <Basiswerte.h>

#include "NamenKonventionen.h"
#include "JSONHandler.h"


class InventarObjekt {
public:
	//-----------------------------------------------------------
	//Constructor

	InventarObjekt();
	InventarObjekt(const InventarObjekt& other);
	InventarObjekt& operator=(const InventarObjekt& other);

	//-----------------------------------------------------------
	//Destructor
	~InventarObjekt();

	//-----------------------------------------------------------
	//Marshalling

	std::string serialize();

	void deserialize(std::string serialOrLink);

	//-----------------------------------------------------------
	// Setting/Getting specific values
	template <typename T> void valueSet(std::string key, const T data);

	rapidjson::Document* getDoc() const;

	template <typename T> T valueGet(std::string key, const T& defaultValue = T());
	
	
private:
	rapidjson::Document doc;
};


class Inventar {
public:
	//-----------------------------------------------------------
	//Constructor

	Inventar() {
		
	}
	Inventar(const Inventar& other);
	Inventar& operator=(const Inventar& other);

	//-----------------------------------------------------------
	//Destructor
	~Inventar();

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