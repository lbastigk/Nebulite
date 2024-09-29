#include "InteractionRequirement.h"

#pragma once

InteractionRequirement::InteractionRequirement() {
		// Beispiel
		requiredFrom = mainCharacter;
		requirementType = namenKonvention.talente._self;
		requirementClass = namenKonvention.talente.koerper._self;
		requirementSubclass = namenKonvention.talente.koerper.sinnesschaerfe;
		value = 10;
	}