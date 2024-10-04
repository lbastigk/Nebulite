#pragma once

//
// RETIRED!
// 
// SEE Calculator.cpp and GameLoop.cpp
//

#include <string>
#include <vector>


#include "Kreatur.h"
#include "Savegame.h"

//Requirement Ideeen:
// - Typ:Talente
//   * Class: Talentklasse
//   * Subclass: Talent
//   * value: Anzahl
// - Typ:Eigenschaften
//   * Class: Eigenschaft
//   * Subclass: [ignore]
//   * value: Anzahl
// - Typ:Inventargegenstand
//   * Class: ID des objektes [String]
//   * Subclass: vergleichsoperand, 
//				 meistens muss Inventar gr��ergleich sein, 
//				 aber eventuell f�r einige nodes ist auch kleiner gleich interessant
//   * value: Anzahl
// - Typ: Quest
//   * Class: ID der Quest
//   * Subclass: done, not done, active
//   * value: Stage zu vergleichen, bei 0 ist Quest als ganzes zu nehmen
// - Typ: Basiswerte
//   *
//   //Umstellen Basiswerte auf std::map, um einfach string zu �bergeben?
// - Typ: NPC
//   * Class: [TODO, was lohnt sich hier?]
//   * Subclass: [TODO, was lohnt sich hier?]
//   * value: ID des NPCs
//   // eventuell Sachen wie Tot, Lebendig
//   // Sachen wie z.B.
// - Typ: none
//   // Dieser Typ wird verwendet, um eine Queststage direkt zu �berspringen. 
//   // Diese kann dann als QuestReq verwendet werden
//   // Also im prinzip zum erstellen von Queststages, die als "Anker" f�r andere Sachen dienen.
//
class InteractionRequirement {
	enum from {
		mainCharacter,
		groupAnyone,
		activeMember
	};
	int requiredFrom;

	std::string requirementType;
	std::string requirementClass;
	std::string requirementSubclass;
	int value;

	InteractionRequirement();
};