#include "Eigenschaften.h"


//-----------------------------------------------------------
//Constructor

Eigenschaften::Eigenschaften() {
	JSONHandler::Set::Any(doc, namenKonvention.eigenschaften.charisma, 0);
	JSONHandler::Set::Any(doc, namenKonvention.eigenschaften.fingerfertigkeit, 0);
	JSONHandler::Set::Any(doc, namenKonvention.eigenschaften.gewandheit, 0);
	JSONHandler::Set::Any(doc, namenKonvention.eigenschaften.intuition, 0);
	JSONHandler::Set::Any(doc, namenKonvention.eigenschaften.klugheit, 0);
	JSONHandler::Set::Any(doc, namenKonvention.eigenschaften.koerperkraft, 0);
	JSONHandler::Set::Any(doc, namenKonvention.eigenschaften.konstitution, 0);
	JSONHandler::Set::Any(doc, namenKonvention.eigenschaften.mut, 0);
}
Eigenschaften::Eigenschaften(const Eigenschaften& other) {
	doc.CopyFrom(*(other.getDoc()), doc.GetAllocator());
}
Eigenschaften& Eigenschaften::operator=(const Eigenschaften& other) {  // Assignment operator overload
	if (this != &other) {
		JSONHandler::copyDoc(doc, other.getDoc());
	}
	return *this;
}

//-----------------------------------------------------------
//Destructor
Eigenschaften::~Eigenschaften() {
	if (doc.IsArray()) {
		doc.Empty();
	}
};

//-----------------------------------------------------------
//Marshalling

std::string Eigenschaften::serialize() {
	return JSONHandler::serialize(doc);
}

void Eigenschaften::deserialize(std::string serialOrLink) {
	doc = JSONHandler::deserialize(serialOrLink);
}





rapidjson::Document* Eigenschaften::getDoc() const {
	return const_cast<rapidjson::Document*>(&doc);
}