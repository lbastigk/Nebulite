#include "DomainModule/GlobalSpace/GDM_Input.hpp"
#include "Core/GlobalSpace.hpp"

void Nebulite::DomainModule::GlobalSpace::Input::update() {
    //------------------------------------------
	// 2-Step Update of Input state

	// 1.) Setting all delta values to 0, so they're only on delta for one frame
	if(reset_delta){
		// 1.) Mouse
		domain->global.set("input.mouse.delta.X",0);
		domain->global.set("input.mouse.delta.Y",0);
		domain->global.set("input.mouse.delta.left",0);
		domain->global.set("input.mouse.delta.right",0);

		// 2.) Keyboard
		for (int scancode = SDL_SCANCODE_UNKNOWN; scancode < SDL_NUM_SCANCODES; ++scancode) {
			const char* nameRaw = SDL_GetScancodeName(static_cast<SDL_Scancode>(scancode));
			if (nameRaw && nameRaw[0] != '\0') {
				std::string keyName = nameRaw;
				for (char& c : keyName) c = std::tolower(c);
				for (char& c : keyName) if (c == ' ') c = '_';

				// Don't add if there are special chars in Nebulite::Constants::keyName
				if(!Nebulite::Utility::StringHandler::containsAnyOf(keyName,Nebulite::Utility::JSON::reservedCharacters)){
					// Paths
					std::string deltaPath = "input.keyboard.delta." + keyName;

					domain->global.set<int>(deltaPath.c_str(), 0);
				}
			}
		}
		reset_delta = false;
	}

	// 2.) Polling mouse and keyboard state
	// Update every 10 ms
	// Too much polling time for current benchmarks, if we update each frame 
	// later on with fixed framerates of < 250 FPS perhaps not that big of a deal
	uint64_t projected_dt = RendererPollTime.projected_dt();
	
	if(projected_dt > 10){
		RendererPollTime.update();
		pollEvent();

		// Toggling console mode
		if(domain->RendererExists() && domain->global.get<int>("input.keyboard.delta.`",0) == 1){
            domain->getRenderer()->toggleConsoleMode();
			if(domain->getRenderer()->isConsoleMode()){
				SDL_StartTextInput();
				SDL_FlushEvents(SDL_FIRSTEVENT, SDL_LASTEVENT); // Flush all pending events
			}
			else{
				SDL_StopTextInput();
			}
		}

		// Reset delta values on next update
		reset_delta = true;
	}

    //------------------------------------------
	// Setting for forced global values
    for(const auto& pair : forced_global_values) {
        domain->global.set(pair.first.c_str(), pair.second);
    }
}

void Nebulite::DomainModule::GlobalSpace::Input::pollEvent() {
	//------------------------------------------
	// Mouse
    mouse.lastPosX = mouse.posX;
    mouse.lastPosY = mouse.posY;
    mouse.lastState = mouse.state;
    mouse.state = SDL_GetMouseState(&mouse.posX, &mouse.posY);

	// Cursor Position and state
	domain->global.set("input.mouse.current.X",mouse.posX);
	domain->global.set("input.mouse.current.Y",mouse.posY);
	domain->global.set("input.mouse.delta.X",mouse.posX-mouse.lastPosX);
	domain->global.set("input.mouse.delta.Y",mouse.posY-mouse.lastPosY);

	domain->global.set("input.mouse.current.left",!!(SDL_BUTTON(SDL_BUTTON_LEFT) & mouse.state));
	domain->global.set("input.mouse.current.right",!!(SDL_BUTTON(SDL_BUTTON_RIGHT) & mouse.state));
	domain->global.set("input.mouse.delta.left",
		!!(SDL_BUTTON(SDL_BUTTON_LEFT) & mouse.state) - 
		!!(SDL_BUTTON(SDL_BUTTON_LEFT) & mouse.lastState));
	domain->global.set("input.mouse.delta.right",
		!!(SDL_BUTTON(SDL_BUTTON_RIGHT) & mouse.state) - 
		!!(SDL_BUTTON(SDL_BUTTON_RIGHT) & mouse.lastState));

    //------------------------------------------
	// Keyboard

    // Get current keyboard state
    const Uint8* keyState = SDL_GetKeyboardState(NULL);
    for (int scancode = SDL_SCANCODE_UNKNOWN; scancode < SDL_NUM_SCANCODES; ++scancode) {
        const char* nameRaw = SDL_GetScancodeName(static_cast<SDL_Scancode>(scancode));
        if (nameRaw && nameRaw[0] != '\0') {
            std::string keyName = nameRaw;
            for (char& c : keyName) c = std::tolower(c);
            for (char& c : keyName) if (c == ' ') c = '_';

            // Don't add if there are special chars in Nebulite::Constants::keyName
			if(!Nebulite::Utility::StringHandler::containsAnyOf(keyName,Nebulite::Utility::JSON::reservedCharacters)){
				// Paths
				std::string currentPath = "input.keyboard.current." + keyName;
            	std::string deltaPath = "input.keyboard.delta." + keyName;

                // If key is currently pressed
				bool currentPressed = keyState[scancode] != 0;
				bool prevPressed    = prevKey[scancode];

				// Set current state (true/false as int)
				domain->global.set<int>(currentPath.c_str(), currentPressed);

				// Compute delta: 1 = pressed now but not before, -1 = released now but was pressed before, 0 = no change
				int delta = 0;
				if (currentPressed && !prevPressed) delta = 1;
				else if (!currentPressed && prevPressed) delta = -1;

				// Set delta
				domain->global.set<int>(deltaPath.c_str(), delta);

                // Store previous key state
                prevKey[scancode] = currentPressed;
			}
        }
    }

	//------------------------------------------
	// Set forced values, from internal vector
	for(const auto& pair : forced_global_values) {
		domain->global.set(pair.first.c_str(), pair.second);
	}
}

//------------------------------------------
// Available Functions

Nebulite::Constants::ERROR_TYPE Nebulite::DomainModule::GlobalSpace::Input::forceGlobal(int argc, char* argv[]) {
    if (argc < 3) {
        return Nebulite::Constants::ERROR_TYPE::TOO_FEW_ARGS;
    }
    if (argc > 3) {
        return Nebulite::Constants::ERROR_TYPE::TOO_MANY_ARGS;
    }

    std::string key = argv[1];
    std::string value = argv[2];
    forced_global_values[key] = value;
    return Nebulite::Constants::ERROR_TYPE::NONE;
}

Nebulite::Constants::ERROR_TYPE Nebulite::DomainModule::GlobalSpace::Input::clearForceGlobal(int argc, char* argv[]) {
    forced_global_values.clear();
    return Nebulite::Constants::ERROR_TYPE::NONE;
}