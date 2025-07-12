#include "Renderer.h"



Nebulite::Renderer::Renderer(Nebulite::Invoke& invoke, Nebulite::JSON& global, bool flag_hidden, unsigned int zoom, unsigned int X, unsigned int Y)
: 	rngA(hashString("Seed for RNG A")),
	rngB(hashString("Seed for RNG B")),
	dist(0, 32767)
	{
	//--------------------------------------------
	// Linkages
	invoke_ptr = &invoke;
	env.linkGlobal(global);

	//--------------------------------------------
	// Initialize internal variables
	RenderZoom=zoom;
	tileXpos = 0;
	tileYpos = 0;
	event = SDL_Event();
	rect = SDL_Rect();
	directory = FileManagement::currentDir();
    currentTime = Time::gettime();
    lastTime = Time::gettime();
	last_poll = Time::gettime();

	//--------------------------------------------
	// SDL Renderer

	//Create SDL window
	if (SDL_Init(SDL_INIT_VIDEO) != 0) {
		// SDL initialization failed
		std::cerr << "SDL_Init Error: " << SDL_GetError() << std::endl;
	}
	window = SDL_CreateWindow(
		"Nebulite",            // Window title
		SDL_WINDOWPOS_CENTERED,
		SDL_WINDOWPOS_CENTERED,
		invoke_ptr->getGlobalPointer()->get<int>("display.resolution.X",X)*zoom,                        // Width
		invoke_ptr->getGlobalPointer()->get<int>("display.resolution.Y",Y)*zoom,                        // Height
		flag_hidden ? SDL_WINDOW_HIDDEN :SDL_WINDOW_SHOWN
	);
	if (!window) {
		// Window creation failed
		std::cerr << "SDL_CreateWindow Error: " << SDL_GetError() << std::endl;
		SDL_Quit();
	}

	// Initialize SDL_ttf
	if (TTF_Init() < 0) {
		// Handle SDL_ttf initialization error
		SDL_Quit(); // Clean up SDL
	}

	//Load font
	std::string sep = FileManagement::preferred_separator();
	std::string fontDir = std::string("Resources") + sep + std::string("Fonts") + sep + std::string("Arimo-Regular.ttf");
	std::string fontpath = FileManagement::CombinePaths(directory, fontDir);
	
	font = TTF_OpenFont(fontpath.c_str(), 60); // Adjust size as needed
	if (font == NULL) {
		// Handle font loading error
		std::cerr << TTF_GetError() << " | " << fontpath << "\n";
	}

	consoleFont = TTF_OpenFont(fontpath.c_str(), 20); // Adjust size as needed
	if (consoleFont == NULL) {
		// Handle font loading error
		std::cerr << TTF_GetError() << " | " << fontpath << "\n";
	}

	// Create a renderer
	renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
	if (!renderer) {
		std::cerr << "Renderer creation failed: << SDL_GetError()" << std::endl;
	}

	// Set virtual rendering size
	SDL_RenderSetLogicalSize(
		renderer, 
		invoke_ptr->getGlobalPointer()->get<int>("display.resolution.X",X), 
		invoke_ptr->getGlobalPointer()->get<int>("display.resolution.Y",Y)
	);

	//--------------------------------------------
	// Set basic values inside global doc
	invoke_ptr->getGlobalPointer()->set<int>("display.resolution.X",X);	
	invoke_ptr->getGlobalPointer()->set<int>("display.resolution.Y",Y);
	invoke_ptr->getGlobalPointer()->set<int>("display.position.X",0);
	invoke_ptr->getGlobalPointer()->set<int>("display.position.Y",0);

	invoke_ptr->getGlobalPointer()->set<Uint64>("time.fixed_dt_ms",0);
	invoke_ptr->getGlobalPointer()->set<double>("time.t",0);
	invoke_ptr->getGlobalPointer()->set<Uint64>("time.t_ms",0);
	invoke_ptr->getGlobalPointer()->set<double>("time.dt",0);
	invoke_ptr->getGlobalPointer()->set<Uint64>("time.dt_ms",0);
    invoke_ptr->getGlobalPointer()->set<double>("physics.G",0.1 * 100);
}

//-----------------------------------------------------------
//Marshalling
std::string Nebulite::Renderer::serialize() {
	return env.serialize();
}

void Nebulite::Renderer::deserialize(std::string serialOrLink) {
	env.deserialize(
		serialOrLink, 
		invoke_ptr->getGlobalPointer()->get<int>("display.resolution.X",0), 
		invoke_ptr->getGlobalPointer()->get<int>("display.resolution.Y",0)
	);
}

//-----------------------------------------------------------
// Pipeline
void Nebulite::Renderer::append(Nebulite::RenderObject* toAppend) {
	// Set ID
	toAppend->valueSet<uint32_t>(Nebulite::keyName.renderObject.id.c_str(),id_counter);
	id_counter++;

	//Append to environment, based on layer
	env.append(
		toAppend, 
		invoke_ptr->getGlobalPointer()->get<int>("display.resolution.X",0), 
		invoke_ptr->getGlobalPointer()->get<int>("display.resolution.Y",0), 
		toAppend->valueGet(Nebulite::keyName.renderObject.layer.c_str(), 0)
	);

	//Load texture
	loadTexture(toAppend->valueGet<std::string>(Nebulite::keyName.renderObject.imageLocation.c_str()));

	// Update rolling rand
	update_rrand();
}

void Nebulite::Renderer::reinsertAllObjects(){
	env.reinsertAllObjects(
		invoke_ptr->getGlobalPointer()->get<int>("display.resolution.X",0),
		invoke_ptr->getGlobalPointer()->get<int>("display.resolution.Y",0)
	);
}

void Nebulite::Renderer::update() {
	//--------------------------------------------
	// Key Polling

	// Update keyboard events every n ms
	// Too much polling time for current benchmarks, 
	// later on with fixed framerates of < 250 FPS not that big of a deal
	if(Time::gettime() - RendererPollTime.t_ms > 10){
		RendererPollTime.update();
		pollEvent();

		// Toggling console mode
		if(invoke_ptr->getGlobalPointer()->get<int>("input.keyboard.delta.`",0) == 1){
			consoleMode = !consoleMode;
			if(consoleMode){
				SDL_StartTextInput();
				SDL_FlushEvents(SDL_FIRSTEVENT, SDL_LASTEVENT); // Flush all pending events
			}
			else{
				SDL_StopTextInput();
			}
			

			// Make sure the first call to console mode is registered
			if(consoleTime_last == 0){
				consoleTime_last = Time::gettime();
			}
		}
	}

	//--------------------------------------------
	// Log time spend in console
	if(consoleMode){
		// Integrate dt from last update call
		consoleTime += Time::gettime() - consoleTime_last;
		
	}
	consoleTime_last = Time::gettime();

	//--------------------------------------------
	// Internal container state

	// Only do container updates if not in console mode
	if(!consoleMode){
		// Set global values like time
		setGlobalValues();

		// Update invoke pairs, getting broadcast-listen-pairs from last env update
		invoke_ptr->update();

		// Update environment
		env.update(
			tileXpos,
			tileYpos,
			invoke_ptr->getGlobalPointer()->get<int>("display.resolution.X",0), 
			invoke_ptr->getGlobalPointer()->get<int>("display.resolution.Y",0), 
			invoke_ptr
		);
	}
}


//-----------------------------------------------------------
// Purge
void Nebulite::Renderer::purgeObjects() {
	invoke_ptr->clear();
	env.purgeObjects();
}

void Nebulite::Renderer::purgeObjectsAt(int x, int y){
	env.purgeObjectsAt(
		x,
		y,
		invoke_ptr->getGlobalPointer()->get<int>("display.resolution.X",0),
		invoke_ptr->getGlobalPointer()->get<int>("display.resolution.Y",0)
	);
}

void Nebulite::Renderer::purgeLayer(int layer) {
	env.purgeLayer(layer);
}

void Nebulite::Renderer::purgeTextures() {
	// Release resources for TextureContainer
	for (auto& pair : TextureContainer) {
		SDL_DestroyTexture(pair.second);
	}
	TextureContainer.clear(); // Clear the map to release resources
}

void Nebulite::Renderer::destroy() {
    if (window) {
        SDL_DestroyWindow(window);
        window = nullptr;
    }
    if (renderer) {
        SDL_DestroyRenderer(renderer);
        renderer = nullptr;
    }
    if (font) {
        TTF_CloseFont(font);
        font = nullptr;
    }
}

//-----------------------------------------------------------
// Manipulation

void Nebulite::Renderer::changeWindowSize(int w, int h, int scalar) {
	RenderScalar = scalar;
	if(w < 64 || w > 16384){
		std::cerr << "Selected resolution is not supported:" << w << "x" << h << "x" << std::endl;
		return;
	}
	if(w < 64 || w > 16384){
		std::cerr << "Selected resolution is not supported:" << w << "x" << h << "x" << std::endl;
		return;
	}

	invoke_ptr->getGlobalPointer()->set<int>("display.resolution.X",w);
	invoke_ptr->getGlobalPointer()->set<int>("display.resolution.Y",h);
    
    // Update the window size
    SDL_SetWindowSize(
		window, 
		invoke_ptr->getGlobalPointer()->get<int>("display.resolution.X",360) * RenderScalar, 
		invoke_ptr->getGlobalPointer()->get<int>("display.resolution.Y",360) * RenderScalar
	);
	SDL_RenderSetLogicalSize(
		renderer,
		invoke_ptr->getGlobalPointer()->get<int>("display.resolution.X",360), 
		invoke_ptr->getGlobalPointer()->get<int>("display.resolution.Y",360)
	);

    // Reinsert objects, due to new tile size
    reinsertAllObjects();
}

void Nebulite::Renderer::moveCam(int dX, int dY, bool isMiddle) {
	invoke_ptr->getGlobalPointer()->set<int>(
		"display.position.X",
		invoke_ptr->getGlobalPointer()->get<int>("display.position.X",0) + dX
	);
	invoke_ptr->getGlobalPointer()->set<int>(
		"display.position.Y",
		invoke_ptr->getGlobalPointer()->get<int>("display.position.X",0) + dY
	);
};

void Nebulite::Renderer::setCam(int X, int Y, bool isMiddle) {
	if(isMiddle){
		invoke_ptr->getGlobalPointer()->set<int>(
			"display.position.X",
			X - invoke_ptr->getGlobalPointer()->get<int>("display.resolution.X",0) / 2
		);
		invoke_ptr->getGlobalPointer()->set<int>(
			"display.position.Y",
			Y - invoke_ptr->getGlobalPointer()->get<int>("display.resolution.Y",0) / 2
		);
	}
	else{
		invoke_ptr->getGlobalPointer()->set<int>("display.position.X",X);
		invoke_ptr->getGlobalPointer()->set<int>("display.position.X",Y);
	}
};


//-----------------------------------------------------------
// Rendering

bool Nebulite::Renderer::timeToRender() {
	return SDL_GetTicks64() >= (prevTicks + SCREEN_TICKS_PER_FRAME);
}

void Nebulite::Renderer::clear(){
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255); // RGB values (black)
	SDL_RenderClear(renderer);
}

void Nebulite::Renderer::renderFrame() {
	// Store for faster access
	int dispPosX = invoke_ptr->getGlobalPointer()->get<int>("display.position.X",0);
	int dispPosY = invoke_ptr->getGlobalPointer()->get<int>("display.position.Y",0);

	// Depending on position, set tiles to render
	tileXpos = dispPosX / invoke_ptr->getGlobalPointer()->get<int>("display.resolution.X",0);
	tileYpos = dispPosY / invoke_ptr->getGlobalPointer()->get<int>("display.resolution.Y",0);
	
	//------------------------------------------------
	// FPS Count

	//Ticks and FPS
	totalframes++;
	fpsCount++;
	prevTicks = SDL_GetTicks64();

	//Calculate fps
	if (prevTicks - lastFPSRender >= 1000) {
		fps = fpsCount;
		fpsCount = 0;
		lastFPSRender = prevTicks;

		/*
		if (control_fps) {
			//D and I summation
			int error = fps - SCREEN_FPS;
			integral += error;

			//Terms
			double pTerm = kp * error;
			double iTerm = ki * integral;
			double dTerm = kd * (error - prevError);

			// Calculate the new epsilon value
			int depsillon = static_cast<int>(pTerm + iTerm + dTerm);

			//Sum up: PID * s controller
			epsillon += depsillon;

			prevError = error;
		}
		*/
		
	}

	//------------------------------------------------
	// Rendering
	int error = 0;

	//Render Objects
	//For all layers, starting at 0
	for (int layer = 0; layer < RENDEROBJECTCONTAINER_COUNT; layer++) {
		//Between dx +-1
		for (int dX = (tileXpos == 0 ? 0 : -1); dX <= 1; dX++) {
			// And dy +-1
			for (int dY = (tileYpos == 0 ? 0 : -1); dY <= 1; dY++) {
				// If valid
				if (env.isValidPosition(tileXpos + dX, tileYpos + dY,layer)) {
					// For all objects inside
					for (auto& obj : env.getContainerAt(tileXpos + dX,tileYpos + dY,layer)) {
						// Check for texture
						std::string innerdir = obj->valueGet<std::string>(Nebulite::keyName.renderObject.imageLocation.c_str());
						if (TextureContainer.find(innerdir) == TextureContainer.end()) {
							loadTexture(innerdir);
							obj->calculateDstRect();
						}
						obj->calculateSrcRect();
						
						// Calculate position rect
						DstRect = obj->getDstRect();
						DstRect.x -= dispPosX;		//subtract camera posX
						DstRect.y -= dispPosY; 	//subtract camera posY

						// Render the texture
						error = SDL_RenderCopy(renderer, TextureContainer[innerdir], obj->getSrcRect(), &DstRect);

						// Render the text
						//*
						if (obj->valueGet<float>(Nebulite::keyName.renderObject.textFontsize.c_str())>0){
							obj->calculateText(
								renderer,
								font,
								dispPosX,
								dispPosY
							);
							SDL_Texture* texture = obj->getTextTexture();
							if(texture && obj->getTextRect()){
								SDL_RenderCopy(renderer,texture,NULL,obj->getTextRect());
							}
						}
						if (error != 0){
							std::cerr << "SDL Error while rendering Frame: " << error << std::endl;
						}
						//*/
						
					}
				}
			}
		}
	}

	//------------------------------------------------
	// Render Console if Active
	if(consoleMode){
		// Semi-transparent background
		consoleRect.x = 0;
		consoleRect.y = invoke_ptr->getGlobalPointer()->get<int>("display.resolution.Y",0) - 150;
		consoleRect.w = invoke_ptr->getGlobalPointer()->get<int>("display.resolution.X",0);
		consoleRect.h = 150;

		SDL_SetRenderDrawColor(renderer, 0, 32, 128, 180); // blue-ish with transparency
		SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
		SDL_RenderFillRect(renderer, &consoleRect);

		SDL_Color textColor = {255, 255, 255, 255}; // white
		int lineHeight = TTF_FontHeight(consoleFont);

		// 1. Draw console input line at bottom
		if (!consoleInputBuffer.empty()) {
			SDL_Surface* textSurface = TTF_RenderText_Blended(consoleFont, consoleInputBuffer.c_str(), textColor);
			SDL_Texture* textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);

			textRect.x = 10;
			textRect.y = consoleRect.y + consoleRect.h - lineHeight - 10;
			textRect.w = textSurface->w;
			textRect.h = textSurface->h;

			SDL_RenderCopy(renderer, textTexture, NULL, &textRect);

			SDL_FreeSurface(textSurface);
			SDL_DestroyTexture(textTexture);
		}

		// 2. Render previous output lines above the input
		int maxLines = (consoleRect.h - 20 - lineHeight) / lineHeight;
		int startLine = std::max(0, (int)consoleOutput.size() - maxLines);

		for (int i = 0; i < maxLines && (startLine + i) < consoleOutput.size(); ++i) {
			const std::string& line = consoleOutput[startLine + i];

			SDL_Surface* textSurface = TTF_RenderText_Blended(consoleFont, line.c_str(), textColor);
			SDL_Texture* textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);

			SDL_Rect lineRect;
			lineRect.x = 10;
			lineRect.y = consoleRect.y + 10 + i * lineHeight;
			lineRect.w = textSurface->w;
			lineRect.h = textSurface->h;

			SDL_RenderCopy(renderer, textTexture, NULL, &lineRect);

			SDL_FreeSurface(textSurface);
			SDL_DestroyTexture(textTexture);
		}
	}

}

void Nebulite::Renderer::renderFPS(float scalar) {
	scalar = scalar / (float)RenderZoom / (float)RenderScalar;

	// Create a string with the FPS value
	std::string fpsText = "FPS: " + std::to_string(fps);

	float fontSize = 16;

	// Define the destination rectangle for rendering the text
	SDL_Rect textRect = { (int)(scalar*10.0), (int)(scalar*10.0), 0, 0 }; // Adjust position as needed
	textRect.w = scalar * fontSize * fpsText.length(); // Width based on text length
	textRect.h = (int)((float)fontSize * 1.5 * scalar);

	// Clear the area where the FPS text will be rendered
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255); // Set background color (black)
	SDL_RenderFillRect(renderer, &textRect);

	// Create a surface with the text
	SDL_Surface* textSurface = TTF_RenderText_Solid(font, fpsText.c_str(), textColor);

	// Create a texture from the text surface
	SDL_Texture* textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);

	// Render the text texture
	SDL_RenderCopy(renderer, textTexture, NULL, &textRect);

	// Free the text surface and texture
	SDL_FreeSurface(textSurface);
	SDL_DestroyTexture(textTexture);
}

void Nebulite::Renderer::showFrame() {
	SDL_RenderPresent(renderer);
}

// This function is called on each non in-console frame to set the values for the global document
void Nebulite::Renderer::setGlobalValues(){
	//---------------------------------------------
	// Time
	// logs:
	// - time in s and ms
	// - dt from last frame in s and ms
	lastTime = currentTime;
	currentTime = Time::gettime() - consoleTime;	// subtract time spend in console mode to cause time freeze

	// Get dt_ms. Either fixed value or calculate from actual time difference
	Uint64 dt_ms = invoke_ptr->getGlobalPointer()->get<Uint64>("time.fixed_dt_ms",0);
	if(dt_ms == 0){
		dt_ms = currentTime - lastTime;
	}

	// Get t_ms
	Uint64 t_ms = invoke_ptr->getGlobalPointer()->get<Uint64>( "time.t_ms",0) + dt_ms;

	// From those values, set all other values
	invoke_ptr->getGlobalPointer()->set<double>( "time.dt", dt_ms / 1000.0);
	invoke_ptr->getGlobalPointer()->set<double>( "time.t",   t_ms / 1000.0);
	invoke_ptr->getGlobalPointer()->set<Uint64>( "time.dt_ms", dt_ms);
	invoke_ptr->getGlobalPointer()->set<Uint64>( "time.t_ms", t_ms);

	// Get Frame count
	Uint64 ticks = invoke_ptr->getGlobalPointer()->get<Uint64>("frameCount",0);
	invoke_ptr->getGlobalPointer()->set<Uint64>("frameCount",ticks+1);

	//---------------------------------------------
	// Random
	update_rand();
	update_rrand();
}

// TODO: hashmap for key names instead of constant polling?
// This also ensures cross-platform stability, note that SDL_GetScancodeName is not cross-platform stable!!!
// Manual map is therefore necessary
void Nebulite::Renderer::pollEvent() {
	//----------------------------------
	// Window state + console input
	while (SDL_PollEvent(&event)) {
        switch (event.type) {
            case SDL_QUIT:
                quit = true;
                break;
        }
		if (consoleMode) {
			switch (event.type) {
				case SDL_TEXTINPUT:
					consoleInputBuffer += event.text.text;
					break;

				case SDL_KEYDOWN:
					switch (event.key.keysym.sym) {
						case SDLK_BACKSPACE:
							if (!consoleInputBuffer.empty()) {
								consoleInputBuffer.pop_back();
							}
							break;
						case SDLK_RETURN:
						case SDLK_KP_ENTER:
							if (!consoleInputBuffer.empty()) {
								consoleOutput.emplace_back("> " + consoleInputBuffer);
								invoke_ptr->getQueue()->emplace_back(consoleInputBuffer);
								consoleInputBuffer.clear();
							}
							break;				
					}
					break;
			}

			// Early return so mouse and keyboard polling doesn’t interfere
			return;
		}
    }

	//----------------------------------
	// Mouse
    lastMousePosX = MousePosX;
    lastMousePosY = MousePosY;
    lastMouseState = mouseState;
    mouseState = SDL_GetMouseState(&MousePosX, &MousePosY);

	// Cursor Position and state
	invoke_ptr->getGlobalPointer()->set("input.mouse.current.X",MousePosX);
	invoke_ptr->getGlobalPointer()->set("input.mouse.current.Y",MousePosY);
	invoke_ptr->getGlobalPointer()->set("input.mouse.delta.X",MousePosX-lastMousePosX);
	invoke_ptr->getGlobalPointer()->set("input.mouse.delta.Y",MousePosY-lastMousePosY);

	invoke_ptr->getGlobalPointer()->set("input.mouse.current.left",!!(SDL_BUTTON(SDL_BUTTON_LEFT) & mouseState));
	invoke_ptr->getGlobalPointer()->set("input.mouse.current.right",!!(SDL_BUTTON(SDL_BUTTON_RIGHT) & mouseState));
	invoke_ptr->getGlobalPointer()->set("input.mouse.delta.left",
		!!(SDL_BUTTON(SDL_BUTTON_LEFT) & mouseState) - 
		!!(SDL_BUTTON(SDL_BUTTON_LEFT) & lastMouseState));
	invoke_ptr->getGlobalPointer()->set("input.mouse.delta.right",
		!!(SDL_BUTTON(SDL_BUTTON_RIGHT) & mouseState) - 
		!!(SDL_BUTTON(SDL_BUTTON_RIGHT) & lastMouseState));

    //----------------------------------
	// Keyboard

    // Get current keyboard state
    const Uint8* keyState = SDL_GetKeyboardState(NULL);

    // Initialize prevKeyState if empty (on first run)
    if (prevKeyState.empty()) {
        prevKeyState.assign(keyState, keyState + SDL_NUM_SCANCODES);
    }

    for (int scancode = SDL_SCANCODE_UNKNOWN; scancode < SDL_NUM_SCANCODES; ++scancode) {
        const char* nameRaw = SDL_GetScancodeName(static_cast<SDL_Scancode>(scancode));
        if (nameRaw && nameRaw[0] != '\0') {
            std::string keyName = nameRaw;
            for (char& c : keyName) c = std::tolower(c);
            for (char& c : keyName) if (c == ' ') c = '_';

            // Don't add if there are special chars in Nebulite::keyName
			if(!StringHandler::containsAnyOf(keyName,Nebulite::JSON::reservedCharacters)){
				// Paths
				std::string currentPath = "input.keyboard.current." + keyName;
            	std::string deltaPath = "input.keyboard.delta." + keyName;

				bool currentPressed = keyState[scancode] != 0;
				bool prevPressed = prevKeyState[scancode] != 0;

				// Set current state (true/false as int)
				invoke_ptr->getGlobalPointer()->set<int>(currentPath.c_str(), currentPressed);

				// Compute delta: 1 = pressed now but not before, -1 = released now but was pressed before, 0 = no change
				int delta = 0;
				if (currentPressed && !prevPressed) delta = 1;
				else if (!currentPressed && prevPressed) delta = -1;

				invoke_ptr->getGlobalPointer()->set<int>(deltaPath.c_str(), delta);
			}
        }
    }

    // Save current keyboard state for next frame
    prevKeyState.assign(keyState, keyState + SDL_NUM_SCANCODES);
}

SDL_Event Nebulite::Renderer::getEventHandle() {
	SDL_Event event;
	SDL_PollEvent(&event);
	return event;
}

//-----------------------------------------------------------
// Setting
void Nebulite::Renderer::setFPS(int fps) {
	if (fps > 0) {
		SCREEN_FPS = fps;
		SCREEN_TICKS_PER_FRAME = 1000 / SCREEN_FPS;
	}
	else {
		SCREEN_FPS = 60;
		SCREEN_TICKS_PER_FRAME = 1000 / SCREEN_FPS;
	}
}


//-----------------------------------------------------------
// Other

void Nebulite::Renderer::loadTexture(std::string link) {
	// Combine directory and innerdir to form full path
	std::string path = FileManagement::CombinePaths(directory, link);

	// Check if texture is already loaded
	if (TextureContainer.find(link) == TextureContainer.end()) {

		// Attempt to load as PNG (or other supported formats)
		SDL_Surface* surface = IMG_Load(path.c_str()); 

		// Fallback to BMP if PNG load fails
		if (!surface) {
			surface = SDL_LoadBMP(path.c_str()); 
		}

		// Unknown format or other issues with surface
		if (!surface) {
			std::cerr << "Failed to load image '" << path << "': " << SDL_GetError() << std::endl;
			return;
		}

		// Create texture from surface
		SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
		SDL_FreeSurface(surface); // Free the surface after creating texture

		// Check for texture issues
		if (!texture) {
			std::cerr << "Failed to create texture from surface: " << SDL_GetError() << std::endl;
			return;
		}

		// Store texture in container
		TextureContainer[link] = texture;
	}
}

std::size_t Nebulite::Renderer::hashString(const std::string& str) {
    return std::hash<std::string>{}(str);
}