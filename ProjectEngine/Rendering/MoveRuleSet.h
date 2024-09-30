#pragma once

#include <iostream>
#include <sstream>

#include "NamenKonventionen.h"
#include "JSONHandler.h"
#include "Calculator.h"

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

	void loadOperatorValues(double& a, double& b, std::string& operation, rapidjson::Value& arr, int& counter, rapidjson::Document& memberMemberDoc, std::string& memberKey, auto& obj, rapidjson::Document* mainDoc);
};