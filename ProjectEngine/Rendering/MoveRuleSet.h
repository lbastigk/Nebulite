#pragma once

//#include <iostream>
#include <sstream>

#include "NamenKonventionen.h"
#include "JSONHandler.h"
#include "Calculator.h"
#include "RenderObject.h"

// Forward declaration for compiler
class RenderObject;
class MoveRuleSet {
public:
	//-----------------------------------------------------------
	//Constructor

	MoveRuleSet();
	MoveRuleSet(rapidjson::Document& toCopy);
	MoveRuleSet(const MoveRuleSet& other);
	MoveRuleSet& operator=(const MoveRuleSet& other);

	//-----------------------------------------------------------
	//Destructor
	~MoveRuleSet();

	//-----------------------------------------------------------
	//Marshalling

	std::string serialize();

	void deserialize(std::string serialOrLink);

	//-----------------------------------------------------------
	// Setting/Getting specific values

	template <typename T> void valueSet(std::string key, const T data);

	template <typename T> T valueGet(std::string key, const T& defaultValue = T()) const;

	rapidjson::Document* getDoc() const;


	//-----------------------------------------------------------
	// Going through ruleset, updating object attached to ruleset
	void update(auto& obj);

	class Examples{
	public:
		static MoveRuleSet goUp(std::string var);
		static MoveRuleSet zickZack(std::string var);
		static MoveRuleSet sin(std::string var, int radius, double phase, double addPerTick);
		static MoveRuleSet linearIncrease(std::string var, int amount, int diff = 1, bool repeat = false, bool setback = false, int waitTime = 0);
		static MoveRuleSet upAndDown(std::string var, int amount, int diff = 1, bool repeat = false, int waitTime = 0);

		class Move {
		public:
			static MoveRuleSet linear(int xAmount, int yAmount, int dX = 1, int dY = 1, bool repeat = false);
		};

		static MoveRuleSet count(std::string var, int start, int end, bool repeat, int waitTime);
	};
	

private:
	rapidjson::Document doc;

	void loadOperatorValues(double& a, double& b, std::string& operation, rapidjson::Value& arr, int& counter, rapidjson::Document& memberMemberDoc, std::string& memberKey, RenderObject& obj, rapidjson::Document* mainDoc);
};

//-----------------------------------------------------------
// Setting/Getting specific values

template <typename T> void MoveRuleSet::valueSet(std::string key, const T data) {
	JSONHandler::Set::Any<T>(doc, key, data);
}

template <typename T> T MoveRuleSet::valueGet(std::string key, const T& defaultValue) const {
	return JSONHandler::Get::Any<T>(doc, key, defaultValue);
}



//-----------------------------------------------------------
// Implementation of the update function
//
// Going through ruleset, updating object attached to ruleset
void MoveRuleSet::update(auto& obj) {

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


