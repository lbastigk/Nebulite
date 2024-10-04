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

//-----------------------------------------------------------
// Setting/Getting specific values

template <typename T> 
void MyTemplate::valueSet(std::string key, const T data) {
	JSONHandler::Set::Any<T>(doc, key, data);
}

template <typename T> 
T MyTemplate::valueGet(std::string key, const T& defaultValue) const {
	return JSONHandler::Get::Any<T>(doc, key, defaultValue);
}

rapidjson::Document* MyTemplate::getDoc() const {
	return const_cast<rapidjson::Document*>(&doc);
}



