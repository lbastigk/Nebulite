#include "_TEMPLATE_CLASS.h"


//-----------------------------------------------------------
//Constructor

MyTemplate::MyTemplate() {
	JSONHandler::Set::Any<int>(doc, namenKonvention.MyTemplate.bsp1, 0);
}

MyTemplate::MyTemplate(const MyTemplate& other) {
	doc.CopyFrom(*(other.getDoc()), doc.GetAllocator());
}
MyTemplate& MyTemplate::operator=(const MyTemplate& other) {  // Assignment operator overload
	// Check for self-assignment
	if (this != &other) {
		JSONHandler::copyDoc(doc, other.getDoc());
	}
	return *this;
}

//-----------------------------------------------------------
//Destructor
MyTemplate::~MyTemplate() {
	if (doc.IsArray()) {
		doc.Empty();
	}
};

//-----------------------------------------------------------
//Marshalling

std::string MyTemplate::serialize() {
	return JSONHandler::serialize(doc);
}

void MyTemplate::deserialize(std::string serialOrLink) {
	doc = JSONHandler::deserialize(serialOrLink);
}



rapidjson::Document* MyTemplate::getDoc() const {
	return const_cast<rapidjson::Document*>(&doc);
}



