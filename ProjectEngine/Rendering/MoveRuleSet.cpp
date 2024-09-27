#pragma once

#include <iostream>
#include <sstream>

#include "NamenKonventionen.cpp"
#include "JSONHandler.cpp"
#include "Calculator.cpp"

class MoveRuleSet {
public:
	//-----------------------------------------------------------
	//Constructor

	MoveRuleSet() {
		doc.SetObject();
	}
	MoveRuleSet(rapidjson::Document& toCopy) {
		JSONHandler::copyDoc(doc, &toCopy);
	}
	MoveRuleSet(const MoveRuleSet& other) {
		doc.CopyFrom(*(other.getDoc()), doc.GetAllocator());
	}
	MoveRuleSet& operator=(const MoveRuleSet& other) {  // Assignment operator overload
		// Check for self-assignment
		if (this != &other) {
			JSONHandler::copyDoc(doc, other.getDoc());
		}
		return *this;
	}

	//-----------------------------------------------------------
	//Destructor
	~MoveRuleSet() {
		if (doc.IsArray()) {
			doc.SetNull();
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
	// Going through ruleset, updating object attached to ruleset
	void update(auto& obj) {
		//-----------------------------------------------------------
		// About obj:
		// Ruleset can be attached to any object with the rapidjson template
		// 
		//Obj is a class object that contains the method:
		// rapidjson::Document* getDoc() const {
		// return const_cast<rapidjson::Document*>(&doc);
		// }

		//----------------------------------------------
		// Getting keys from naming struct
		const std::string& variableKey = namenKonvention.moveRuleSet.variable;		//variable of obj that needs to be changed
		const std::string& counterKey = namenKonvention.moveRuleSet.counter;		//current active rule
		const std::string& arrRuleSetKey = namenKonvention.moveRuleSet.arrRuleSet;	//Array of rules

		//----------------------------------------------
		// Getting pointer to main doc
		rapidjson::Document* mainDoc = obj.getDoc();
		rapidjson::Document& mainDocRef = *mainDoc;
		
		//----------------------------------------------
		// Extract all subdocs in doc
		// Structure:
		// MoveRuleSet		<doc>:
		//	var1			<doc>:
		//		counter		<int>
		//		tick		<int>
		//		ruleSet		<vector>
		//  var2			<doc>:
		//		counter		<int>
		//		tick		<int>
		//		ruleSet		<vector>
		//   ...
		//
		// So the following for-loop extracts from doc (MoveRuleSet) all subdocs var1, var2 etc.
		// to mainpulate their values

		//Needed subdocs
		rapidjson::Document tempMainDoc;	//rebuilding doc here in loop
		rapidjson::Document memberDoc;
		rapidjson::Document memberMemberDoc;

		//Other variables
		std::string memberKey;

		//Copy to temp, rebuild doc
		//Finished MRS are not copied back into doc
		tempMainDoc.CopyFrom(doc, tempMainDoc.GetAllocator());	
		JSONHandler::empty(doc);

		//Go through all MRS
		for (auto it = tempMainDoc.MemberBegin(); it != tempMainDoc.MemberEnd(); ++it) {
			//----------------------------------------------
			// From the doc iterator it, extract subdoc "memberDoc"
			memberKey = it->name.GetString();
			JSONHandler::Set::subDoc(memberDoc, memberKey, it->value);

			//Memberdoc now contains:
			//   var1 <doc>
			//     counter <int>
			//     ruleSet <vector>
			
			//----------------------------------------------
			//Needs to go one deeper!
			// as in, memberDoc[key] is the needed doc containing counter, tick and ruleset.
			// key is the current name of iterator

			//Get memberMemberDoc:
			JSONHandler::Get::subDoc(memberDoc, memberKey, memberMemberDoc);

			//Arr holds ruleset
			//counter holds what nth ruleset to use
			//tick holds progress in a wait operation, only called when needed
			rapidjson::Value& arr = memberMemberDoc[namenKonvention.moveRuleSet.arrRuleSet.c_str()];
			int counter = JSONHandler::Get::Any<int>(memberMemberDoc, namenKonvention.moveRuleSet.counter, 0);
			unsigned int currentTick = JSONHandler::Get::Any<int>(memberMemberDoc, namenKonvention.moveRuleSet.tick, 0);

			//----------------------------------------------
			// Used Variables
			double a, b;
			double result = 0;
			std::string operation;

			//----------------------------------------------
			// Depending on operation type...
			bool endreached = false;
			bool updateDone = false;	//certain operations should not trigger this, otherwise some movements might be slower
			while (!updateDone && !endreached) {
				//load all values each loop
				loadOperatorValues(a, b, operation, arr, counter, memberMemberDoc, memberKey, obj, mainDoc);

				//check if operation is repeat
				if (operation == namenKonvention.moveRuleSet.repeat) {
					
					if (currentTick == (int)b) {
						//ticks back to 0
						currentTick = 0;

						//Increase counter
						int size = arr.Size();
						if (counter == (size - 1)) {
							//Delete mrs, end reached!
							endreached = true;
						}
						counter = (counter + 1) % size;
					}
					else {
						currentTick++;							//Increase ticks
						counter = (counter - 1) % arr.Size();	//Increase counter
					}
				}

				//check for looping
				else if (operation == namenKonvention.moveRuleSet.loop) {
					//reset counter and ticks to 0
					if (b == 0) {
						counter = 0;
					}
					else {
						counter -= b;
					}
					currentTick = 0;
				}

				// TODO: Buggy, sets var back to 0!!!
				// check if operation is wait
				// This sets updateDone to true
				else if (operation == namenKonvention.moveRuleSet.wait) {
					unsigned int currentTick = (unsigned int)JSONHandler::Get::Any<int>(memberMemberDoc, namenKonvention.moveRuleSet.tick, 0);
					if (currentTick == (int)b) {
						//ticks back to 0
						currentTick = 0;

						//Increase counter
						int size = arr.Size();
						if (counter == (size - 1)) {
							//Delete mrs, end reached!
							endreached = true;
						}
						counter = (counter + 1) % size;
					}
					else {
						//Increase ticks
						currentTick++;
					}
					updateDone = true;
				}

				// Check for setnoskip
				else if (operation == namenKonvention.calculator.setNoSkip) {
					double result = a;

					// SetnoSkip means the set is not done as a standalone
					// but in the same "tick" the next operation is done too
					// Do "set" first:

					//Calculate
					result = Calculator::performOperation(operation, a, b);

					//Increase counter
					int size = arr.Size();
					if (counter == (size - 1)) {
						//Delete mrs, end reached!
						endreached = true;
					}
					counter = (counter + 1) % size;

					//set value to doc
					JSONHandler::Set::Any<double>(mainDocRef, memberKey, result);	//Cast to double for now
				}

				// Do actual calc operation
				// This sets updateDone to true
				else if (Calculator::operationExists(operation)) {
					// Do operation
					result = a;

					//Calculate
					if (Calculator::operationExists(operation)) {
						result = Calculator::performOperation(operation, result, b);
					}

					//Increase counter
					int size = arr.Size();
					if (counter == (size - 1)) {
						//Delete mrs, end reached!
						endreached = true;
					}
					counter = (counter + 1) % size;
					updateDone = true;
				}
			}
			
			//-------------------------------------------------------------------------------------------------------
			// One update Loop done
			// send result tick and counter info to doc
			// And the moveset, if not finished
			// Save result
			JSONHandler::Set::Any<double>(mainDocRef, memberKey, result);	//Cast to double for now
			if (!endreached) {
				JSONHandler::Set::Any<int>(memberMemberDoc, namenKonvention.moveRuleSet.counter, counter);
				JSONHandler::Set::Any<int>(memberMemberDoc, namenKonvention.moveRuleSet.tick, currentTick);
				JSONHandler::Set::subDoc(doc, memberKey, memberMemberDoc);
			}
			

			//----------------------------------------------
			//Deleting temp docs etc.
			JSONHandler::empty(memberDoc);
			JSONHandler::empty(memberMemberDoc);
		}
		JSONHandler::empty(tempMainDoc);

		
	}

	

	class Examples{
	public:
		static MoveRuleSet goUp(std::string var) {
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

		static MoveRuleSet zickZack(std::string var) {
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

		static MoveRuleSet sin(std::string var, int radius, double phase, double addPerTick) {
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

		static MoveRuleSet linearIncrease(std::string var, int amount, int diff = 1, bool repeat = false, bool setback = false, int waitTime = 0) {
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

		static MoveRuleSet upAndDown(std::string var, int amount, int diff = 1, bool repeat = false, int waitTime = 0) {
			MoveRuleSet mrs;

			if (amount != 0) {
				int totalChange = amount;
				std::vector<std::pair<std::string, std::string>> ruleSet;
				diff = abs(diff) * (amount) / abs(amount);
				int rest = abs(amount) % abs(diff) * diff / abs(diff);
				amount = abs(amount) / abs(diff);

				//#############################
				//UP
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

				//#############################
				//DOWN
				for (int i = 0; i < amount; i++) {
					ruleSet.push_back(std::make_pair<std::string, std::string>(std::string(namenKonvention.calculator.add), std::to_string(-diff)));
					for (int j = 0; j < waitTime; j++) {
						ruleSet.push_back(std::make_pair<std::string, std::string>(std::string(namenKonvention.calculator.add), std::to_string(0)));
					}
				}

				//Rest movement
				if (rest != 0) {
					ruleSet.push_back(std::make_pair<std::string, std::string>(std::string(namenKonvention.calculator.add), std::to_string(-rest)));
					for (int j = 0; j < waitTime; j++) {
						ruleSet.push_back(std::make_pair<std::string, std::string>(std::string(namenKonvention.calculator.add), std::to_string(0)));
					}
				}
				

				//Handle repeats


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


		class Move {
		public:
			static MoveRuleSet linear(int xAmount, int yAmount, int dX = 1, int dY = 1, bool repeat = false) {
				MoveRuleSet mrs;

				//X Movement
				if (xAmount != 0) {
					std::vector<std::pair<std::string, std::string>> ruleSetX;
					dX = abs(dX) * (xAmount) / abs(xAmount);
					int rest = abs(xAmount) % abs(dX) * dX/abs(dX);
					xAmount = abs(xAmount) / abs(dX);
					ruleSetX.push_back(std::make_pair<std::string, std::string>(std::string(namenKonvention.calculator.add), std::to_string(dX)));
					ruleSetX.push_back(std::make_pair<std::string, std::string>(std::string(namenKonvention.moveRuleSet.repeat), std::to_string(xAmount - 1)));
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
					ruleSetY.push_back(std::make_pair<std::string, std::string>(std::string(namenKonvention.calculator.add), std::to_string(dY)));
					ruleSetY.push_back(std::make_pair<std::string, std::string>(std::string(namenKonvention.moveRuleSet.repeat), std::to_string(yAmount - 1)));
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

			
		};

		static MoveRuleSet count(std::string var, int start, int end, bool repeat, int waitTime) {
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
	};
	

private:
	rapidjson::Document doc;

	void loadOperatorValues(double& a, double& b, std::string& operation, rapidjson::Value& arr, int& counter, rapidjson::Document& memberMemberDoc, std::string& memberKey, auto& obj, rapidjson::Document* mainDoc) {
		
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
};