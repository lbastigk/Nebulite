#include <vector>
#include <map>

#pragma once

//Wird:
// - Im Savegame gespeichert
// - Aufgerufen von Quest, um bedingungen zu checken.
// - Bei nicht vorhandensein ist 0 Anzunehmen! 
class QuestProgress {
private:
	std::map<int/*QuestID*/, std::vector<int>/*QuestStages*/> QuestStagesDone;
};

class Statistics {
private:
	std::map<int/*ID*/, int/*Amount*/> AmountKilled;	//useful for interactionRequirement; 10 wolfes killed or certain NPC with id killed
};