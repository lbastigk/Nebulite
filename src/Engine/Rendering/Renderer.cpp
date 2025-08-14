#include "Renderer.h"

Nebulite::Renderer::Renderer(Nebulite::Invoke& invoke, Nebulite::JSON& global, bool flag_headless, unsigned int zoom, unsigned int X, unsigned int Y)
: 	rngA(hashString("Seed for RNG A")),
	rngB(hashString("Seed for RNG B")),
	dist(0, 32767),
	env(&invoke)
	{
	//--------------------------------------------
	// Linkages
	invoke_ptr = &invoke;
	env.linkGlobal(global);

	//--------------------------------------------
	// Depending on platform, set Global key "platform":
	#ifdef _WIN32
		invoke_ptr->getGlobalPointer()->set<std::string>("platform","windows");
	#elif __linux__
		invoke_ptr->getGlobalPointer()->set<std::string>("platform","linux");
	#elif __APPLE__
		invoke_ptr->getGlobalPointer()->set<std::string>("platform","macos");
	#elif __FreeBSD__
		invoke_ptr->getGlobalPointer()->set<std::string>("platform","freebsd");
	#elif __unix__
		invoke_ptr->getGlobalPointer()->set<std::string>("platform","unix");
	#elif __ANDROID__
		invoke_ptr->getGlobalPointer()->set<std::string>("platform","android");
	#elif __TEMPLEOS__
		invoke_ptr->getGlobalPointer()->set<std::string>("platform","templeos");
	#else
		invoke_ptr->getGlobalPointer()->set<std::string>("platform","unknown");
	#endif

	//--------------------------------------------
	// Initialize internal variables

	// Window
	RenderZoom = zoom;

	// Position
	tileXpos = 0;
	tileYpos = 0;

	// State
	event = SDL_Event();
	directory = FileManagement::currentDir();

	//--------------------------------------------
	// SDL Renderer

	//Create SDL window
	if (SDL_Init(SDL_INIT_VIDEO) != 0) {
		// SDL initialization failed
		std::cerr << "SDL_Init Error: " << SDL_GetError() << std::endl;
	}
	// Define window via x|y|w|h
	int x = SDL_WINDOWPOS_CENTERED;
	int y = SDL_WINDOWPOS_CENTERED;
	int w = invoke_ptr->getGlobalPointer()->get<int>(keyName.renderer.dispResX.c_str(),X)*zoom;
	int h = invoke_ptr->getGlobalPointer()->get<int>(keyName.renderer.dispResY.c_str(),Y)*zoom;
	window = SDL_CreateWindow("Nebulite",x,y,w,h,flag_headless ? SDL_WINDOW_HIDDEN :SDL_WINDOW_SHOWN);
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
	std::string sep(1,FileManagement::preferred_separator());
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
		invoke_ptr->getGlobalPointer()->get<int>(keyName.renderer.dispResX.c_str(),X), 
		invoke_ptr->getGlobalPointer()->get<int>(keyName.renderer.dispResY.c_str(),Y)
	);

	//--------------------------------------------
	// Initialize Audio
	if (SDL_Init(SDL_INIT_AUDIO) < 0) {
		std::cerr << "SDL_Init Error: " << SDL_GetError() << std::endl;
	}else{
		SDL_AudioSpec desired, obtained;
		desired.freq = 44100;
		desired.format = AUDIO_S16SYS;
		desired.channels = 1;
		desired.samples = 1024;
		desired.callback = nullptr;
		
		audioDevice = SDL_OpenAudioDevice(nullptr, 0, &desired, &obtained, 0);
		if (audioDevice == 0) {
			std::cerr << "Failed to open audio device: " << SDL_GetError() << std::endl;
		}
		else{
			audioInitialized = true;
		}
	}

	//--------------------------------------------
	// Waveform buffers
	
	// Sine wave buffer
	sineBuffer = new std::vector<Sint16>(samples);
	for (int i = 0; i < samples; i++) {
		double time = (double)i / sampleRate;
		(*sineBuffer)[i] = (Sint16)(32767 * 0.3 * sin(2.0 * M_PI * frequency * time));
	}

	// Square wave buffer
	squareBuffer = new std::vector<Sint16>(samples);
	for (int i = 0; i < samples; i++) {
		double time = (double)i / sampleRate;
    
		// Square wave: alternates between +1 and -1
		double phase = 2.0 * M_PI * frequency * time;
		double squareValue = (sin(phase) >= 0) ? 1.0 : -1.0;
		
		(*squareBuffer)[i] = (Sint16)(32767 * 0.3 * squareValue);
	}

	// Triangle wave buffer
	triangleBuffer = new std::vector<Sint16>(samples);
	for (int i = 0; i < samples; i++) {
		double time = (double)i / sampleRate;
    
		// Triangle wave: linear ramp up and down
		double phase = fmod(frequency * time, 1.0);  // 0 to 1
		double triangleValue;
		
		if (phase < 0.5) {
			triangleValue = 4.0 * phase - 1.0;      // -1 to +1 (rising)
		} else {
			triangleValue = 3.0 - 4.0 * phase;      // +1 to -1 (falling)
		}
		
		(*triangleBuffer)[i] = (Sint16)(32767 * 0.3 * triangleValue);
	}

	//--------------------------------------------
	// Set basic values inside global doc
	invoke_ptr->getGlobalPointer()->set<int>(keyName.renderer.dispResX.c_str(),X);	
	invoke_ptr->getGlobalPointer()->set<int>(keyName.renderer.dispResY.c_str(),Y);
	invoke_ptr->getGlobalPointer()->set<int>(keyName.renderer.positionX.c_str(),0);
	invoke_ptr->getGlobalPointer()->set<int>(keyName.renderer.positionY.c_str(),0);

	invoke_ptr->getGlobalPointer()->set<Uint64>(keyName.renderer.time_fixed_dt_ms.c_str(),0);
	invoke_ptr->getGlobalPointer()->set<double>(keyName.renderer.time_t.c_str(),0);
	invoke_ptr->getGlobalPointer()->set<Uint64>(keyName.renderer.time_t_ms.c_str(),0);
	invoke_ptr->getGlobalPointer()->set<double>(keyName.renderer.time_dt.c_str(),0);
	invoke_ptr->getGlobalPointer()->set<Uint64>(keyName.renderer.time_dt_ms.c_str(),0);

	//---------
	// Start timer
	RendererLoopTime.start();
	RendererPollTime.start();
	RendererFullTime.start();
}


//-----------------------------------------------------------
// Pipeline
//# define debug_on_each_step 1
void Nebulite::Renderer::tick(){
	#ifdef debug_on_each_step
    	std::cout << "clear:" << std::endl;
	#endif
    clear();           				// 1.) Clear screen FIRST, so that functions like snapshot have acces to the latest frame
	#ifdef debug_on_each_step
    	std::cout << "update:" << std::endl;
	#endif
    update();          				// 2.) Update objects
	#ifdef debug_on_each_step
    	std::cout << "renderFrame:" << std::endl;
	#endif
    renderFrame();     				// 3.) Render frame
	#ifdef debug_on_each_step
    	std::cout << "renderFPS:" << std::endl;
	#endif
	if(showFPS)renderFPS();       	// 4.) Render fps count
	#ifdef debug_on_each_step
    	std::cout << "showFrame:" << std::endl;
	#endif
    showFrame();       				// 5.) Show Frame
	#ifdef debug_on_each_step
		std::cout << "done!:" << std::endl;
	#endif
}

void Nebulite::Renderer::append(Nebulite::RenderObject* toAppend) {
	// Set ID
	toAppend->valueSet<uint32_t>(Nebulite::keyName.renderObject.id.c_str(),id_counter);
	id_counter++;

	//Append to environment, based on layer
	env.append(
		toAppend, 
		invoke_ptr->getGlobalPointer()->get<int>(keyName.renderer.dispResX.c_str(),0), 
		invoke_ptr->getGlobalPointer()->get<int>(keyName.renderer.dispResY.c_str(),0), 
		toAppend->valueGet(Nebulite::keyName.renderObject.layer.c_str(), 0)
	);

	//Load texture
	loadTexture(toAppend->valueGet<std::string>(Nebulite::keyName.renderObject.imageLocation.c_str()));

	// Update rolling rand
	update_rrand();
}

void Nebulite::Renderer::reinsertAllObjects(){
	env.reinsertAllObjects(
		invoke_ptr->getGlobalPointer()->get<int>(keyName.renderer.dispResX.c_str(),0),
		invoke_ptr->getGlobalPointer()->get<int>(keyName.renderer.dispResY.c_str(),0)
	);
}

void Nebulite::Renderer::update() {
	// Update loop timer
	uint64_t fixed_dt_ms = invoke_ptr->getGlobalPointer()->get<Uint64>(keyName.renderer.time_fixed_dt_ms.c_str(),0);
	RendererLoopTime.update(fixed_dt_ms);

	//----------------------------------
	// Basic SDL event polling
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
	// 2-Step Update of Input state

	
	// 1.) Setting all delta values to 0, so they're only on delta for one frame
	if(reset_delta){
		// 1.) Mouse
		invoke_ptr->getGlobalPointer()->set("input.mouse.delta.X",MousePosX-lastMousePosX);
		invoke_ptr->getGlobalPointer()->set("input.mouse.delta.Y",MousePosY-lastMousePosY);
		invoke_ptr->getGlobalPointer()->set("input.mouse.delta.left",
			!!(SDL_BUTTON(SDL_BUTTON_LEFT) & mouseState) - 
			!!(SDL_BUTTON(SDL_BUTTON_LEFT) & lastMouseState));
		invoke_ptr->getGlobalPointer()->set("input.mouse.delta.right",
			!!(SDL_BUTTON(SDL_BUTTON_RIGHT) & mouseState) - 
			!!(SDL_BUTTON(SDL_BUTTON_RIGHT) & lastMouseState));

		// 2.) Keyboard
		for (int scancode = SDL_SCANCODE_UNKNOWN; scancode < SDL_NUM_SCANCODES; ++scancode) {
			const char* nameRaw = SDL_GetScancodeName(static_cast<SDL_Scancode>(scancode));
			if (nameRaw && nameRaw[0] != '\0') {
				std::string keyName = nameRaw;
				for (char& c : keyName) c = std::tolower(c);
				for (char& c : keyName) if (c == ' ') c = '_';

				// Don't add if there are special chars in Nebulite::keyName
				if(!StringHandler::containsAnyOf(keyName,Nebulite::JSON::reservedCharacters)){
					// Paths
					std::string deltaPath = "input.keyboard.delta." + keyName;

					invoke_ptr->getGlobalPointer()->set<int>(deltaPath.c_str(), 0);
				}
			}
		}
		reset_delta = false;
	}

	// 2.) Polling mouse and keyboard state
	// Update every 10 ms
	// Too much polling time for current benchmarks, 
	// later on with fixed framerates of < 250 FPS perhaps not that big of a deal
	uint64_t projected_dt = RendererPollTime.projected_dt();
	
	if(projected_dt > 10){
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
		}

		// Reset delta values on next update
		reset_delta = true;
	}


	//--------------------------------------------
	// Log time spend in console
	// Not important but might be nice to know
	if(consoleMode){
		// Deactivate the loop timer
		RendererLoopTime.stop();
	}
	else{
		// Reactivate the loop timer
		RendererLoopTime.start();
	}
	

	//--------------------------------------------
	// Internal container state

	// Only do container updates if not in console mode
	if(!consoleMode){
		// Set global values like time
		setGlobalValues();

		// Update invoke pairs, getting broadcast-listen-pairs from last env update
		invoke_ptr->update();

		// Update environment
		int dispResX = invoke_ptr->getGlobalPointer()->get<int>(keyName.renderer.dispResX.c_str(),0);
		int dispResY = invoke_ptr->getGlobalPointer()->get<int>(keyName.renderer.dispResY.c_str(),0);
		env.update(tileXpos,tileYpos,dispResX,dispResY,invoke_ptr);
	}
}

// TODO: Snapshot not working on windows via wine
// Image has right dimensions, but is black
//
// Nebulite Linux Release and Debug work fine
// Nebulite Windows Release and Debug are broken
bool Nebulite::Renderer::snapshot(std::string link) {
    if (!renderer) {
        std::cerr << "Cannot take snapshot: renderer not initialized" << std::endl;
        return false;
    }
    
    // Get current window/render target size
    int width, height;
    if (window) {
        // Normal windowed mode
        SDL_GetWindowSize(window, &width, &height);
    } else {
        // Headless mode - get renderer output size
        SDL_GetRendererOutputSize(renderer, &width, &height);
    }
    
    //std::cout << "Taking snapshot (" << width << "x" << height << ") to: " << link << std::endl;
    
    // Create surface to capture pixels
    SDL_Surface* surface = SDL_CreateRGBSurface(0, width, height, 32,
                                                0x00ff0000,  // Red mask
                                                0x0000ff00,  // Green mask  
                                                0x000000ff,  // Blue mask
                                                0xff000000); // Alpha mask
    
    if (!surface) {
        std::cerr << "Failed to create surface for snapshot: " << SDL_GetError() << std::endl;
        return false;
    }
    
    // Read pixels from renderer
    if (SDL_RenderReadPixels(renderer, NULL, SDL_PIXELFORMAT_ARGB8888, 
                            surface->pixels, surface->pitch) != 0) {
        std::cerr << "Failed to read pixels for snapshot: " << SDL_GetError() << std::endl;
        SDL_FreeSurface(surface);
        return false;
    }
    
    // Create directory if it doesn't exist
    std::string directory = link.substr(0, link.find_last_of("/\\"));

	// Edge case: check if link contains no directory:
	if(link.find_last_of("/\\") == std::string::npos) {
		directory = "./Resources/Snapshots";
		link = directory + "/" + link;
	}

    if (!directory.empty()) {
        // Create directory using C++17 filesystem
        try {
            std::filesystem::create_directories(directory);
        } catch (const std::exception& e) {
            //std::cerr << "Warning: Could not create directory " << directory << ": " << e.what() << std::endl;
            // Continue anyway - maybe directory already exists
        }
    }
    
    // Save surface as PNG
    int result = IMG_SavePNG(surface, link.c_str());
    
    // Cleanup
    SDL_FreeSurface(surface);
    
    if (result != 0) {
        std::cerr << "Failed to save snapshot: " << IMG_GetError() << std::endl;
        return false;
    }
    
    //std::cout << "Snapshot saved successfully to: " << link << std::endl;
    return true;
}


//-----------------------------------------------------------
// Special Functions
void Nebulite::Renderer::beep() {
	// Beep sound effect
	if(audioInitialized) {
		SDL_QueueAudio(audioDevice, squareBuffer->data(), samples * sizeof(Sint16));
		SDL_PauseAudioDevice(audioDevice, 0);  // Start playing
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
		invoke_ptr->getGlobalPointer()->get<int>(keyName.renderer.dispResX.c_str(),0),
		invoke_ptr->getGlobalPointer()->get<int>(keyName.renderer.dispResY.c_str(),0)
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

	invoke_ptr->getGlobalPointer()->set<int>(keyName.renderer.dispResX.c_str(),w);
	invoke_ptr->getGlobalPointer()->set<int>(keyName.renderer.dispResY.c_str(),h);
    
    // Update the window size
    SDL_SetWindowSize(
		window, 
		invoke_ptr->getGlobalPointer()->get<int>(keyName.renderer.dispResX.c_str(),360) * RenderScalar, 
		invoke_ptr->getGlobalPointer()->get<int>(keyName.renderer.dispResY.c_str(),360) * RenderScalar
	);
	SDL_RenderSetLogicalSize(
		renderer,
		invoke_ptr->getGlobalPointer()->get<int>(keyName.renderer.dispResX.c_str(),360), 
		invoke_ptr->getGlobalPointer()->get<int>(keyName.renderer.dispResY.c_str(),360)
	);

	// Turn off console mode
	consoleMode = false;

    // Reinsert objects, due to new tile size
    reinsertAllObjects();
}

void Nebulite::Renderer::moveCam(int dX, int dY, bool isMiddle) {
	invoke_ptr->getGlobalPointer()->set<int>(
		keyName.renderer.positionX.c_str(),
		invoke_ptr->getGlobalPointer()->get<int>(keyName.renderer.positionX.c_str(),0) + dX
	);
	invoke_ptr->getGlobalPointer()->set<int>(
		keyName.renderer.positionY.c_str(),
		invoke_ptr->getGlobalPointer()->get<int>(keyName.renderer.positionX.c_str(),0) + dY
	);
};

void Nebulite::Renderer::setCam(int X, int Y, bool isMiddle) {
	std::cout << "Setting camera position to: " << X << ", " << Y << ", Middle: " << isMiddle << std::endl;

	if(isMiddle){
		int newPosX = X - invoke_ptr->getGlobalPointer()->get<int>(keyName.renderer.dispResX.c_str(),0) / 2;
		int newPosY = Y - invoke_ptr->getGlobalPointer()->get<int>(keyName.renderer.dispResY.c_str(),0) / 2;
		invoke_ptr->getGlobalPointer()->set<int>(keyName.renderer.positionX.c_str(),newPosX);
		invoke_ptr->getGlobalPointer()->set<int>(keyName.renderer.positionY.c_str(),newPosY);
	}
	else{
		invoke_ptr->getGlobalPointer()->set<int>(keyName.renderer.positionX.c_str(),X);
		invoke_ptr->getGlobalPointer()->set<int>(keyName.renderer.positionY.c_str(),Y);
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
	int dispPosX = invoke_ptr->getGlobalPointer()->get<int>(keyName.renderer.positionX.c_str(),0);
	int dispPosY = invoke_ptr->getGlobalPointer()->get<int>(keyName.renderer.positionY.c_str(),0);

	// Depending on position, set tiles to render
	tileXpos = dispPosX / invoke_ptr->getGlobalPointer()->get<int>(keyName.renderer.dispResX.c_str(),0);
	tileYpos = dispPosY / invoke_ptr->getGlobalPointer()->get<int>(keyName.renderer.dispResY.c_str(),0);
	
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
					// For all batches inside
					for (auto& batch : env.getContainerAt(tileXpos + dX,tileYpos + dY,layer)) {
						// For all objects in batch
						for(auto& obj : batch.objects){
							// Check for texture
							std::string innerdir = obj->valueGet<std::string>(Nebulite::keyName.renderObject.imageLocation.c_str());
							if (TextureContainer.find(innerdir) == TextureContainer.end()) {
								loadTexture(innerdir);
								obj->calculateDstRect();
							}
							obj->calculateSrcRect();
							
							// Calculate position rect
							DstRect = *obj->getDstRect();
							DstRect.x -= dispPosX;	//subtract camera posX
							DstRect.y -= dispPosY; 	//subtract camera posY

							// Render the texture
							error = SDL_RenderCopy(renderer, TextureContainer[innerdir], obj->getSrcRect(), &DstRect);

							// Render the text
							//*
							if (obj->valueGet<double>(Nebulite::keyName.renderObject.textFontsize.c_str())>0){
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

		// Render all textures that were attached from outside processes
		for (const auto& [name, texture] : BetweenLayerTextures[static_cast<Renderer::Layers>(layer)]) {
			SDL_RenderCopy(renderer, texture, NULL, NULL);
		}
	}

	//------------------------------------------------
	// Render Console if Active
	if(consoleMode){
		// Semi-transparent background
		consoleRect.x = 0;
		consoleRect.y = invoke_ptr->getGlobalPointer()->get<int>(keyName.renderer.dispResY.c_str(),0) - 150;
		consoleRect.w = invoke_ptr->getGlobalPointer()->get<int>(keyName.renderer.dispResX.c_str(),0);
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

void Nebulite::Renderer::renderFPS(double scalar) {
	scalar = scalar / (double)RenderZoom / (double)RenderScalar;

	// Create a string with the FPS value
	std::string fpsText = "FPS: " + std::to_string(fps);

	double fontSize = 16;

	// Define the destination rectangle for rendering the text
	SDL_Rect textRect = { (int)(scalar*10.0), (int)(scalar*10.0), 0, 0 }; // Adjust position as needed
	textRect.w = scalar * fontSize * fpsText.length(); // Width based on text length
	textRect.h = (int)((double)fontSize * 1.5 * scalar);

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
	// Simulation time
	uint64_t dt_ms = RendererLoopTime.get_dt_ms(); // Either fixed value or calculate from actual simtime difference
	uint64_t t_ms = RendererLoopTime.get_t_ms();
	invoke_ptr->getGlobalPointer()->set<double>( "time.dt", dt_ms / 1000.0);
	invoke_ptr->getGlobalPointer()->set<double>( "time.t",   t_ms / 1000.0);
	invoke_ptr->getGlobalPointer()->set<Uint64>( "time.dt_ms", dt_ms);
	invoke_ptr->getGlobalPointer()->set<Uint64>( "time.t_ms", t_ms);

	//---------------------------------------------
	// Frame count

	// Get Frame count
	Uint64 ticks = invoke_ptr->getGlobalPointer()->get<Uint64>("frameCount",0);
	invoke_ptr->getGlobalPointer()->set<Uint64>("frameCount",ticks+1);

	//---------------------------------------------
	// Full time (runtime)
	RendererFullTime.update();
	dt_ms = RendererFullTime.get_dt_ms();
	t_ms  = RendererFullTime.get_t_ms();
	invoke_ptr->getGlobalPointer()->set<double>( "runtime.dt", dt_ms / 1000.0);
	invoke_ptr->getGlobalPointer()->set<double>( "runtime.t",   t_ms / 1000.0);
	invoke_ptr->getGlobalPointer()->set<Uint64>( "runtime.dt_ms", dt_ms);
	invoke_ptr->getGlobalPointer()->set<Uint64>( "runtime.t_ms", t_ms);


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

	//----------------------------------
	// Set forced values, from internal vector
	for(const auto& pair : forced_global_values) {
		invoke_ptr->getGlobalPointer()->set(pair.first.c_str(), pair.second);
	}

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