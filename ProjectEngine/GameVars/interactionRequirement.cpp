//
// RETIRED!
// 
// SEE Calculator.cpp and GameLoop.cpp
//

#include <string>
#include <vector>


#include <Kreatur.cpp>
#include <Savegame.cpp>

#pragma once

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
//				 meistens muss Inventar größergleich sein, 
//				 aber eventuell für einige nodes ist auch kleiner gleich interessant
//   * value: Anzahl
// - Typ: Quest
//   * Class: ID der Quest
//   * Subclass: done, not done, active
//   * value: Stage zu vergleichen, bei 0 ist Quest als ganzes zu nehmen
// - Typ: Basiswerte
//   *
//   //Umstellen Basiswerte auf std::map, um einfach string zu übergeben?
// - Typ: NPC
//   * Class: [TODO, was lohnt sich hier?]
//   * Subclass: [TODO, was lohnt sich hier?]
//   * value: ID des NPCs
//   // eventuell Sachen wie Tot, Lebendig
//   // Sachen wie z.B.
// - Typ: none
//   // Dieser Typ wird verwendet, um eine Queststage direkt zu überspringen. 
//   // Diese kann dann als QuestReq verwendet werden
//   // Also im prinzip zum erstellen von Queststages, die als "Anker" für andere Sachen dienen.
//
class interactionRequirement {
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

	interactionRequirement() {
		// Beispiel
		requiredFrom = mainCharacter;
		requirementType = namenKonvention.talente._self;
		requirementClass = namenKonvention.talente.koerper._self;
		requirementSubclass = namenKonvention.talente.koerper.sinnesschaerfe;
		value = 10;
	}

	//TODO
	// From and to JSON, once full data structure is clear!
};