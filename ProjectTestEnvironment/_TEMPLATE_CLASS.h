#pragma once


#include <iostream>
#include <sstream>

#include "JSONHandler.h"
#include "NamenKonventionen.h"

class MyTemplate {
public:
	//-----------------------------------------------------------
	//Constructor

	MyTemplate();
	MyTemplate(const MyTemplate& other);
	MyTemplate& operator=(const MyTemplate& other);

	//-----------------------------------------------------------
	//Destructor
	~MyTemplate();

	//-----------------------------------------------------------
	//Marshalling

	std::string serialize();

	void deserialize(std::string serialOrLink);

	//-----------------------------------------------------------
	// Setting/Getting specific values

	template <typename T> 
	void valueSet(std::string key, const T data);

	template <typename T> 
	T valueGet(std::string key, const T& defaultValue = T()) const;

	rapidjson::Document* getDoc() const;

	//-----------------------------------------------------------
	// TODO...


private:
	rapidjson::Document doc;
};


