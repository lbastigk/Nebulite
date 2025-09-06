#include "DomainModule/GlobalSpace/GDM_Input.hpp"
#include "Core/GlobalSpace.hpp"

void Nebulite::DomainModule::GlobalSpace::Input::update() {
    //------------------------------------------
	// 2-Step Update of Input state

	// 1.) Setting all delta values to 0, so they're only on delta for one frame
	if(reset_delta_on_next_update){
		reset_delta_values();
		reset_delta_on_next_update = false;
	}

	// 2.) Polling mouse and keyboard state
	// Update every 10 ms
	// Too much polling time for current benchmarks, if we update each frame 
	// later on with fixed framerates of < 250 FPS perhaps not that big of a deal
	uint64_t projected_dt = RendererPollTime.projected_dt();
	std::cout << "Projected dt: " << projected_dt << "ms" << std::endl;
	if(projected_dt > 10){
		std::cout << "Updating inputs, dt: " << projected_dt << "ms" << std::endl;

		// Updating inputs
		RendererPollTime.update();
		SDL_PumpEvents();
		write_current_and_delta_inputs();

		// Reset delta values on next update
		// Since we only write input current and delta every 10ms,
		// we dont want to reset delta values every frame
		// This would take too much computational time for no reason
		reset_delta_on_next_update = true;
	}
}

//------------------------------------------
// Private Functions

void Nebulite::DomainModule::GlobalSpace::Input::map_key_names() {
	for (int scancode = SDL_SCANCODE_UNKNOWN; scancode < SDL_NUM_SCANCODES; ++scancode) {
		const char* nameRaw = SDL_GetScancodeName(static_cast<SDL_Scancode>(scancode));
		if (nameRaw && nameRaw[0] != '\0') {
			std::string keyName = nameRaw;
			for (char& c : keyName) c = std::tolower(c);
			for (char& c : keyName) if (c == ' ') c = '_';

			// Don't add if there are special chars in Nebulite::Constants::keyName
			if(!Nebulite::Utility::StringHandler::containsAnyOf(keyName,Nebulite::Utility::JSON::reservedCharacters)){
				keyNames[scancode] = keyName;
			}
		}
	}
}

void Nebulite::DomainModule::GlobalSpace::Input::write_current_and_delta_inputs() {
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
    const Uint8* keyState = SDL_GetKeyboardState(NULL);
    for (int scancode = SDL_SCANCODE_UNKNOWN; scancode < SDL_NUM_SCANCODES; ++scancode) {
		if(keyNames[scancode] != ""){
			// Paths
			std::string currentPath = "input.keyboard.current." + keyNames[scancode];
			std::string deltaPath   = "input.keyboard.delta."   + keyNames[scancode];

			// Retrieve state, store previous state
			// If key is currently pressed
			bool currentPressed = keyState[scancode] != 0;
			bool prevPressed    = prevKey[scancode];
			prevKey[scancode]   = currentPressed;

			// Compute delta: 
			// ->  1 = pressed now but not before 
			// -> -1 = released now but was pressed before, 
			// ->  0 = no change
			int delta = 0;
			if      ( currentPressed && !prevPressed) delta =  1;
			else if (!currentPressed &&  prevPressed) delta = -1;

			// Set current state (true/false as int)
			domain->global.set<int>(currentPath.c_str(), currentPressed);

			// Set delta
			domain->global.set<int>(deltaPath.c_str(), delta);
        }
    }
}

void Nebulite::DomainModule::GlobalSpace::Input::reset_delta_values() {
	// 1.) Mouse
	domain->global.set("input.mouse.delta.X",0);
	domain->global.set("input.mouse.delta.Y",0);
	domain->global.set("input.mouse.delta.left",0);
	domain->global.set("input.mouse.delta.right",0);

	// 2.) Keyboard
	for (int scancode = SDL_SCANCODE_UNKNOWN; scancode < SDL_NUM_SCANCODES; ++scancode) {
		if(keyNames[scancode] != ""){
			std::string deltaPath = "input.keyboard.delta." + keyNames[scancode];
			domain->global.set<int>(deltaPath.c_str(), 0);
		}
	}
}
