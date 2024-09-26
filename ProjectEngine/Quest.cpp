#include <string>
#include <map>
#include <vector>

#include <Kreatur.cpp>
#include <interactionRequirement.cpp>

#pragma once




//Transition, hier Sachen wie Dialog, Talentproben etc.
class QuestNode {
	int nextStage;
	std::vector<interactionRequirement> requirementAnded;

	//class dialogueTree erstellen!
	// Verkn�pfung dialogueTree mit int NPC_ID
	// std::map<int/*NPC_ID*/, dialogueTree> availableDialogueAtNode
	// oder, falls mehr m�glichkeit da sein soll:
	// std::map<int, std::vector<dialogueTree>>
	// 
	// der Tree gibt dann transitionen zur�ck.
	// Trees sollen generell benutzt werden k�nnen, auch ohne Quests!
	// 
	// START ----------------------- return 1;
	//			|
	//			|------------------- return 2;
	//          |      L------------ return 3;
	//		    |
	//			L------------------- return 4;
	//
	// Der Return-Wert wird dann zum updaten der Quest verwendet
	// Sachen wie Items oder AP, welche man bekommt, lohnen sich wohl am meisten in QuestNode?
	// Schreiben in QuestNode, bei Transition dann �ber Klasse Quest dies an Gruppe �bergeben?

};

class QuestStage {
	std::string description;
	std::vector<QuestNode> nodes;
};

class Quest {
public:
	int ID;
	std::map<int, QuestStage> QuestStage;
};

