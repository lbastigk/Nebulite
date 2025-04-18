#include "MoveRuleSet.h"

//-----------------------------------------------------------
//Constructor
MoveRuleSet::MoveRuleSet() {
	doc.SetObject();
}
MoveRuleSet::MoveRuleSet(rapidjson::Document& toCopy) {
	JSONHandler::copyDoc(doc, &toCopy);
}
MoveRuleSet::MoveRuleSet(const MoveRuleSet& other) {
	doc.CopyFrom(*(other.getDoc()), doc.GetAllocator());
}
MoveRuleSet& MoveRuleSet::operator=(const MoveRuleSet& other) {  // Assignment operator overload
	// Check for self-assignment
	if (this != &other) {
		JSONHandler::copyDoc(doc, other.getDoc());
	}
	return *this;
}

//-----------------------------------------------------------
//Destructor
MoveRuleSet::~MoveRuleSet() {
	if (doc.IsArray()) {
		doc.SetNull();
	}
};

//-----------------------------------------------------------
//Marshalling

std::string MoveRuleSet::serialize() {
	return JSONHandler::serialize(doc);
}

void MoveRuleSet::deserialize(std::string serialOrLink) {
	doc = JSONHandler::deserialize(serialOrLink);
}


rapidjson::Document* MoveRuleSet::getDoc() const {
	return const_cast<rapidjson::Document*>(&doc);
}




void MoveRuleSet::loadOperatorValues(double& a, double& b, std::string& operation, rapidjson::Value& arr, int& counter, rapidjson::Document& memberMemberDoc, std::string& memberKey, RenderObject& obj, rapidjson::Document* mainDoc) {
	
	// First is string operation, assuming it's a string
	operation = arr[counter][0].GetString();

	// Second is string num2, assuming it's a string
	std::string num2 = arr[counter][1].GetString();

	//----------------------------------------------
	//Load a,b
	//Check if num2 is a number
	double aStartVal;
	aStartVal = JSONHandler::Get::Any<double>(memberMemberDoc, namenKonvention.moveRuleSet.startValue, 0.0);
	if (num2.size() && (num2.at(0) < '0' || num2.at(0) > '9') && num2.at(0) != '-') {
		a = obj.valueGet<double>(memberKey);
		b = JSONHandler::Get::Any<double>(*mainDoc, num2, 0.0);
	}
	else {
		a = obj.valueGet<double>(memberKey, aStartVal);
		if (num2.size()) {
			b = stof(num2);
		}
	}
}


//-------------------------------------------------------------------------
// EXAMPLES

MoveRuleSet MoveRuleSet::Examples::setValue(std::string var, int value){
	MoveRuleSet mrs;

	//Make example moveset1
	std::vector<std::pair<std::string, std::string>> ruleSet;
	ruleSet.push_back(std::make_pair<std::string, std::string>(std::string(namenKonvention.calculator.set), std::to_string(value)));

	//Temp doc for var
	rapidjson::Document rule1;
	JSONHandler::Set::Any(rule1, namenKonvention.moveRuleSet.arrRuleSet, ruleSet);	//RuleSet
	JSONHandler::Set::Any(rule1, namenKonvention.moveRuleSet.counter, 0);				//RuleSet

	//Add to main doc
	//rule1 is rapidjson doc, how to convert to value?
	JSONHandler::Set::subDoc(*mrs.getDoc(), var, rule1);

	//Empty temp doc
	JSONHandler::empty(rule1);

	//Return
	return mrs;	
}

MoveRuleSet MoveRuleSet::Examples::goUp(std::string var) {
	MoveRuleSet mrs;

	//Make example moveset1
	std::vector<std::pair<std::string, std::string>> ruleSet;
	ruleSet.push_back(std::make_pair<std::string, std::string>(std::string(namenKonvention.calculator.add), std::string("1")));
	ruleSet.push_back(std::make_pair<std::string, std::string>(std::string(namenKonvention.moveRuleSet.loop), std::string("0")));

	//Temp doc for var
	rapidjson::Document rule1;
	JSONHandler::Set::Any(rule1, namenKonvention.moveRuleSet.arrRuleSet, ruleSet);	//RuleSet
	JSONHandler::Set::Any(rule1, namenKonvention.moveRuleSet.counter, 0);				//RuleSet

	//Add to main doc
	//rule1 is rapidjson doc, how to convert to value?
	JSONHandler::Set::subDoc(*mrs.getDoc(), var, rule1);

	//Empty temp doc
	JSONHandler::empty(rule1);

	//Return
	return mrs;
}

MoveRuleSet MoveRuleSet::Examples::zickZack(std::string var) {
	MoveRuleSet mrs;

	//Make example moveset1
	std::vector<std::pair<std::string, std::string>> ruleSet;
	ruleSet.push_back(std::make_pair<std::string, std::string>(std::string(namenKonvention.calculator.add), std::string("20")));
	ruleSet.push_back(std::make_pair<std::string, std::string>(std::string(namenKonvention.moveRuleSet.wait), std::string("20")));
	ruleSet.push_back(std::make_pair<std::string, std::string>(std::string(namenKonvention.calculator.add), std::string("-20")));
	ruleSet.push_back(std::make_pair<std::string, std::string>(std::string(namenKonvention.moveRuleSet.wait), std::string("20")));
	ruleSet.push_back(std::make_pair<std::string, std::string>(std::string(namenKonvention.moveRuleSet.loop), std::string("")));

	//Temp doc for var
	rapidjson::Document rule1;
	JSONHandler::Set::Any(rule1, namenKonvention.moveRuleSet.arrRuleSet, ruleSet);	//RuleSet
	JSONHandler::Set::Any(rule1, namenKonvention.moveRuleSet.counter, 0);				//RuleSet

	//Add to main doc
	JSONHandler::Set::subDoc(*mrs.getDoc(), var, rule1);

	//Empty temp doc
	JSONHandler::empty(rule1);

	//Return
	return mrs;
}

MoveRuleSet MoveRuleSet::Examples::sin(std::string var, int radius, double phase, double addPerTick) {
	MoveRuleSet mrs;

	//---------------------------------------------------------
	//Make sine counter
	{
		std::vector<std::pair<std::string, std::string>> ruleSet;
		ruleSet.push_back(std::make_pair<std::string, std::string>(std::string(namenKonvention.calculator.add), std::to_string(addPerTick)));
		ruleSet.push_back(std::make_pair<std::string, std::string>(std::string(namenKonvention.moveRuleSet.loop), std::string("0")));

		//Temp doc for var
		rapidjson::Document rule1;
		JSONHandler::Set::Any(rule1, namenKonvention.moveRuleSet.arrRuleSet, ruleSet);	//RuleSet
		JSONHandler::Set::Any(rule1, namenKonvention.moveRuleSet.counter, 0);				//Counter
		JSONHandler::Set::Any(rule1, namenKonvention.moveRuleSet.startValue, phase);		//Start Value

		//Add to main doc
		JSONHandler::Set::subDoc(*mrs.getDoc(), var+"_cnt", rule1);

		//Empty temp doc
		JSONHandler::empty(rule1);
	}

	//---------------------------------------------------------
	//Sine value with no offset
	{
		std::vector<std::pair<std::string, std::string>> ruleSet;
		ruleSet.push_back(std::make_pair<std::string, std::string>(std::string(namenKonvention.calculator.setNoSkip), std::to_string(radius)));
		ruleSet.push_back(std::make_pair<std::string, std::string>(std::string(namenKonvention.calculator.a_sin_b), var + "_cnt"));
		ruleSet.push_back(std::make_pair<std::string, std::string>(std::string(namenKonvention.moveRuleSet.loop), std::string("0")));

		//Temp doc for var
		rapidjson::Document rule1;
		JSONHandler::Set::Any(rule1, namenKonvention.moveRuleSet.arrRuleSet, ruleSet);	//RuleSet
		JSONHandler::Set::Any(rule1, namenKonvention.moveRuleSet.counter, 0);				//RuleSet

		//Add to main doc
		JSONHandler::Set::subDoc(*mrs.getDoc(), var + "_diff", rule1);

		//Empty temp doc
		JSONHandler::empty(rule1);
	}

	//---------------------------------------------------------
	// Start Position
	{
		std::vector<std::pair<std::string, std::string>> ruleSet;
		ruleSet.push_back(std::make_pair<std::string, std::string>(std::string(namenKonvention.calculator.set), std::string(var)));

		//Temp doc for var
		rapidjson::Document rule1;
		JSONHandler::Set::Any(rule1, namenKonvention.moveRuleSet.arrRuleSet, ruleSet);	//RuleSet
		JSONHandler::Set::Any(rule1, namenKonvention.moveRuleSet.counter, 0);				//RuleSet

		//Add to main doc
		JSONHandler::Set::subDoc(*mrs.getDoc(), var + "_start", rule1);

		//Empty temp doc
		JSONHandler::empty(rule1);
	}

	//---------------------------------------------------------
	// Add Offset
	{
		std::vector<std::pair<std::string, std::string>> ruleSet;
		ruleSet.push_back(std::make_pair<std::string, std::string>(std::string(namenKonvention.calculator.setNoSkip), var + "_start"));
		ruleSet.push_back(std::make_pair<std::string, std::string>(std::string(namenKonvention.calculator.add), var + "_diff"));
		ruleSet.push_back(std::make_pair<std::string, std::string>(std::string(namenKonvention.moveRuleSet.loop), std::string("0")));

		//Temp doc for var
		rapidjson::Document rule1;
		JSONHandler::Set::Any(rule1, namenKonvention.moveRuleSet.arrRuleSet, ruleSet);	//RuleSet
		JSONHandler::Set::Any(rule1, namenKonvention.moveRuleSet.counter, 0);				//RuleSet

		//Add to main doc
		JSONHandler::Set::subDoc(*mrs.getDoc(), var, rule1);

		//Empty temp doc
		JSONHandler::empty(rule1);
	}
	return mrs;
}

MoveRuleSet MoveRuleSet::Examples::linearIncrease(std::string var, int amount, int diff, bool repeat, bool setback, int waitTime) {
	MoveRuleSet mrs;

	if (amount != 0) {
		int totalChange = amount;
		std::vector<std::pair<std::string, std::string>> ruleSet;
		diff = abs(diff) * (amount) / abs(amount);
		int rest = abs(amount) % abs(diff) * diff / abs(diff);
		amount = abs(amount) / abs(diff);

		for (int i = 0; i < amount; i++) {
			ruleSet.push_back(std::make_pair<std::string, std::string>(std::string(namenKonvention.calculator.add), std::to_string(diff)));
			for (int j = 0; j < waitTime; j++) {
				ruleSet.push_back(std::make_pair<std::string, std::string>(std::string(namenKonvention.calculator.add), std::to_string(0)));
			}
		}

		//Rest movement
		if (rest != 0) {
			ruleSet.push_back(std::make_pair<std::string, std::string>(std::string(namenKonvention.calculator.add), std::to_string(rest)));
			for (int j = 0; j < waitTime; j++) {
				ruleSet.push_back(std::make_pair<std::string, std::string>(std::string(namenKonvention.calculator.add), std::to_string(0)));
			}
		}

		//Setback and repeats
		if (setback){
			ruleSet.push_back(std::make_pair<std::string, std::string>(std::string(namenKonvention.calculator.add), std::to_string(-totalChange)));
		}
		if (repeat) {
			ruleSet.push_back(std::make_pair<std::string, std::string>(std::string(namenKonvention.moveRuleSet.loop), std::string("0")));
		}

		//Temp doc for var 1
		rapidjson::Document rule1;
		JSONHandler::Set::Any(rule1, namenKonvention.moveRuleSet.arrRuleSet, ruleSet);	//RuleSet
		JSONHandler::Set::Any(rule1, namenKonvention.moveRuleSet.counter, 0);				//RuleSet
		JSONHandler::Set::subDoc(*mrs.getDoc(), var, rule1);
		JSONHandler::empty(rule1);
	}

	//Return
	return mrs;
}

MoveRuleSet MoveRuleSet::Examples::upAndDown(std::string var, int amount, int diff, int repeat, int waitTime, int additional) {
	MoveRuleSet mrs;

	if (amount != 0) {
		int totalChange = amount;
		std::vector<std::pair<std::string, std::string>> ruleSet;
		diff = abs(diff) * (amount) / abs(amount);
		int rest = abs(amount) % abs(diff) * diff / abs(diff);
		amount = abs(amount) / abs(diff);

		for(int k = 0; k < repeat; k++){
			//#############################
			//UP
			for (int i = 0; i < amount; i++) {
				ruleSet.push_back(std::make_pair<std::string, std::string>(std::string(namenKonvention.calculator.add), std::to_string(diff)));

				//Add "wait" only if its not the last one
				if(i != amount-1){
					for (int j = 0; j < waitTime; j++) {
						ruleSet.push_back(std::make_pair<std::string, std::string>(std::string(namenKonvention.calculator.add), std::to_string(0)));
					}
				}
			}

			//Rest movement
			if (rest != 0) {
				for (int j = 0; j < waitTime; j++) {
					ruleSet.push_back(std::make_pair<std::string, std::string>(std::string(namenKonvention.calculator.add), std::to_string(0)));
				}
				ruleSet.push_back(std::make_pair<std::string, std::string>(std::string(namenKonvention.calculator.add), std::to_string(rest)));
			}

			//#############################
			//Wait between up and down
			for (int j = 0; j < waitTime; j++) {
				ruleSet.push_back(std::make_pair<std::string, std::string>(std::string(namenKonvention.calculator.add), std::to_string(0)));
			}

			//#############################
			//DOWN
			for (int i = 0; i < amount; i++) {
				ruleSet.push_back(std::make_pair<std::string, std::string>(std::string(namenKonvention.calculator.add), std::to_string(-diff)));
				//Add "wait" only if its not the last one
				if(i != amount-1){
					for (int j = 0; j < waitTime; j++) {
						ruleSet.push_back(std::make_pair<std::string, std::string>(std::string(namenKonvention.calculator.add), std::to_string(0)));
					}
				}
			}

			//Rest movement
			if (rest != 0) {
				for (int j = 0; j < waitTime; j++) {
					ruleSet.push_back(std::make_pair<std::string, std::string>(std::string(namenKonvention.calculator.add), std::to_string(0)));
				}
				ruleSet.push_back(std::make_pair<std::string, std::string>(std::string(namenKonvention.calculator.add), std::to_string(-rest)));
			}

			//#############################
			//Wait between up and down
			if(k != repeat-1){
				for (int j = 0; j < waitTime; j++) {
					ruleSet.push_back(std::make_pair<std::string, std::string>(std::string(namenKonvention.calculator.add), std::to_string(0)));
				}
			}
		}

		//Add additional
		if(additional){
			for (int j = 0; j < waitTime; j++) {
				ruleSet.push_back(std::make_pair<std::string, std::string>(std::string(namenKonvention.calculator.add), std::to_string(0)));
			}
			ruleSet.push_back(std::make_pair<std::string, std::string>(std::string(namenKonvention.calculator.add), std::to_string(additional)));
		}

		//Temp doc for var 1
		rapidjson::Document rule1;
		JSONHandler::Set::Any(rule1, namenKonvention.moveRuleSet.arrRuleSet, ruleSet);	//RuleSet
		JSONHandler::Set::Any(rule1, namenKonvention.moveRuleSet.counter, 0);				//RuleSet
		JSONHandler::Set::subDoc(*mrs.getDoc(), var, rule1);
		JSONHandler::empty(rule1);
	}

	//Return
	return mrs;
}

MoveRuleSet MoveRuleSet::Examples::Move::linear(int xAmount, int yAmount, int dX, int dY, bool repeat) {
	MoveRuleSet mrs;

	//X Movement
	if (xAmount != 0) {
		std::vector<std::pair<std::string, std::string>> ruleSetX;
		dX = abs(dX) * (xAmount) / abs(xAmount);
		int rest = abs(xAmount) % abs(dX) * dX/abs(dX);
		xAmount = abs(xAmount) / abs(dX);

		for(int j = 0; j < xAmount; j++){
			ruleSetX.push_back(std::make_pair<std::string, std::string>(std::string(namenKonvention.calculator.add), std::to_string(dX)));
		}
		if (rest != 0) {
			ruleSetX.push_back(std::make_pair<std::string, std::string>(std::string(namenKonvention.calculator.add), std::to_string(rest)));
		}
		if (repeat) {
			ruleSetX.push_back(std::make_pair<std::string, std::string>(std::string(namenKonvention.moveRuleSet.loop), std::string("0")));
		}

		//Temp doc for var 1
		rapidjson::Document rule1;
		JSONHandler::Set::Any(rule1, namenKonvention.moveRuleSet.arrRuleSet, ruleSetX);	//RuleSet
		JSONHandler::Set::Any(rule1, namenKonvention.moveRuleSet.counter, 0);				//RuleSet
		JSONHandler::Set::subDoc(*mrs.getDoc(), namenKonvention.renderObject.positionX, rule1);

		JSONHandler::empty(rule1);
	}

	//Y Movement
	if (yAmount != 0) {
		std::vector<std::pair<std::string, std::string>> ruleSetY;
		dY = abs(dY) * (yAmount) / abs(yAmount);
		int rest = abs(yAmount) % abs(dY) * dY / abs(dY);;
		yAmount = abs(yAmount) / abs(dY);

		for(int j = 0; j < yAmount; j++){
			ruleSetY.push_back(std::make_pair<std::string, std::string>(std::string(namenKonvention.calculator.add), std::to_string(dY)));
		}
		if (rest != 0) {
			ruleSetY.push_back(std::make_pair<std::string, std::string>(std::string(namenKonvention.calculator.add), std::to_string(rest)));
		}
		if (repeat) {
			ruleSetY.push_back(std::make_pair<std::string, std::string>(std::string(namenKonvention.moveRuleSet.loop), std::string("0")));
		}

		//Temp doc for var 2
		rapidjson::Document rule2;
		JSONHandler::Set::Any(rule2, namenKonvention.moveRuleSet.arrRuleSet, ruleSetY);	//RuleSet
		JSONHandler::Set::Any(rule2, namenKonvention.moveRuleSet.counter, 0);				//RuleSet
		JSONHandler::Set::subDoc(*mrs.getDoc(), namenKonvention.renderObject.positionY, rule2);

		JSONHandler::empty(rule2);
	}

	//Return
	return mrs;
}

MoveRuleSet MoveRuleSet::Examples::count(std::string var, int start, int end, bool repeat, int waitTime) {
	MoveRuleSet mrs;

	//Make example moveset1
	std::vector<std::pair<std::string, std::string>> ruleSet;
	for (int i = start; i < end; i++) {
		ruleSet.push_back(std::make_pair<std::string, std::string>(std::string(namenKonvention.calculator.set), std::to_string(i)));
		if (waitTime) {
			ruleSet.push_back(std::make_pair<std::string, std::string>(std::string(namenKonvention.moveRuleSet.wait), std::to_string(waitTime)));
		}
	}
	if (repeat) {
		ruleSet.push_back(std::make_pair<std::string, std::string>(std::string(namenKonvention.moveRuleSet.loop), std::string("0")));
	}

	//Temp doc for var
	rapidjson::Document rule1;
	JSONHandler::Set::Any(rule1, namenKonvention.moveRuleSet.arrRuleSet, ruleSet);	//RuleSet
	JSONHandler::Set::Any(rule1, namenKonvention.moveRuleSet.counter, 0);				//RuleSet

	//Add to main doc
	//rule1 is rapidjson doc, how to convert to value?
	JSONHandler::Set::subDoc(*mrs.getDoc(), var, rule1);

	//Empty temp doc
	JSONHandler::empty(rule1);

	//Return
	return mrs;
}


