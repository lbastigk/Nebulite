#include "Core/Renderer.hpp"

Nebulite::Core::Renderer::Renderer(Nebulite::Interaction::Invoke& invoke, Nebulite::Utility::JSON& global, bool flag_headless, unsigned int X, unsigned int Y)
: 	rngA(hashString("Seed for RNG A")),
	rngB(hashString("Seed for RNG B")),
	dist(0, 32767),
	env(&invoke)
	{
	//------------------------------------------
	// Linkages
	invoke_ptr = &invoke;

	//------------------------------------------
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
		printf("Glory be to TempleOS!\n");
		invoke_ptr->getGlobalPointer()->set<std::string>("platform","templeos");
	#else
		invoke_ptr->getGlobalPointer()->set<std::string>("platform","unknown");
	#endif

	//------------------------------------------
	// Initialize internal variables

	// Window
	WindowScale = 1;

	// Position
	tileXpos = 0;
	tileYpos = 0;

	// State
	event = SDL_Event();
	baseDirectory = Nebulite::Utility::FileManagement::currentDir();

	//------------------------------------------
	// Window

	//Create SDL window
	if (SDL_Init(SDL_INIT_VIDEO) != 0) {
		// SDL initialization failed
		std::cerr << "SDL_Init Error: " << SDL_GetError() << std::endl;
	}
	// Define window via x|y|w|h
	int x = SDL_WINDOWPOS_CENTERED;
	int y = SDL_WINDOWPOS_CENTERED;
	int w = invoke_ptr->getGlobalPointer()->get<int>(Nebulite::Constants::keyName.renderer.dispResX.c_str(),X);
	int h = invoke_ptr->getGlobalPointer()->get<int>(Nebulite::Constants::keyName.renderer.dispResY.c_str(),Y);
	window = SDL_CreateWindow("Nebulite",x,y,w,h,flag_headless ? SDL_WINDOW_HIDDEN :SDL_WINDOW_SHOWN);
	if (!window) {
		// Window creation failed
		std::cerr << "SDL_CreateWindow Error: " << SDL_GetError() << std::endl;
		SDL_Quit();
	}

	//------------------------------------------
	// Renderer

	// Create a renderer
	renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
	if (!renderer) {
		std::cerr << "Renderer creation failed: " << SDL_GetError() << std::endl;
	}

	// Set virtual rendering size
	SDL_RenderSetLogicalSize(
		renderer, 
		invoke_ptr->getGlobalPointer()->get<int>(Nebulite::Constants::keyName.renderer.dispResX.c_str(),X), 
		invoke_ptr->getGlobalPointer()->get<int>(Nebulite::Constants::keyName.renderer.dispResY.c_str(),Y)
	);

	//------------------------------------------
	// Fonts

	// Initialize SDL_ttf
	if (TTF_Init() < 0) {
		// Handle SDL_ttf initialization error
		SDL_Quit(); // Clean up SDL
	}

	loadFonts();

	//------------------------------------------
	// Audio

	// Init
	if (SDL_Init(SDL_INIT_AUDIO) < 0) {
		std::cerr << "SDL_Init Error: " << SDL_GetError() << std::endl;
	} else {
		SDL_AudioSpec desired, obtained;
		desired.freq = 44100;
		desired.format = AUDIO_S16SYS;
		desired.channels = 1;
		desired.samples = 1024;
		desired.callback = nullptr;
		
		audioDevice = SDL_OpenAudioDevice(nullptr, 0, &desired, &obtained, 0);
		if (audioDevice == 0) {
			std::cerr << "Failed to open audio device: " << SDL_GetError() << std::endl;
		} else{
			audioInitialized = true;
		}
	}

	// Waveform buffers: Sine wave buffer
	sineBuffer = new std::vector<Sint16>(samples);
	for (int i = 0; i < samples; i++) {
		double time = (double)i / sampleRate;
		(*sineBuffer)[i] = (Sint16)(32767 * 0.3 * sin(2.0 * M_PI * frequency * time));
	}

	// Waveform buffers: Square wave buffer
	squareBuffer = new std::vector<Sint16>(samples);
	for (int i = 0; i < samples; i++) {
		double time = (double)i / sampleRate;
    
		// Square wave: alternates between +1 and -1
		double phase = 2.0 * M_PI * frequency * time;
		double squareValue = (sin(phase) >= 0) ? 1.0 : -1.0;
		
		(*squareBuffer)[i] = (Sint16)(32767 * 0.3 * squareValue);
	}

	// Waveform buffers: Triangle wave buffer
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

	//------------------------------------------
	// Set basic values inside global doc
	invoke_ptr->getGlobalPointer()->set<int>(Nebulite::Constants::keyName.renderer.dispResX.c_str(),X);	
	invoke_ptr->getGlobalPointer()->set<int>(Nebulite::Constants::keyName.renderer.dispResY.c_str(),Y);

	//------------------------------------------
	// Start timer
	RendererLoopTime.start();
	RendererFullTime.start();
}

void Nebulite::Core::Renderer::loadFonts() {
	//------------------------------------------
	// Sizes
	uint32_t FontSizeGeneral = 60; 			// Does not need to scale
	uint32_t FontSizeConsole = 60;			// Does not need to scale

	//------------------------------------------
	// Font location
	std::string sep(1,Nebulite::Utility::FileManagement::preferredSeparator());
	std::string fontDir = std::string("Resources") + sep + std::string("Fonts") + sep + std::string("Arimo-Regular.ttf");
	std::string fontpath = Nebulite::Utility::FileManagement::CombinePaths(baseDirectory, fontDir);
	
	//------------------------------------------
	// Load general font
	font = TTF_OpenFont(fontpath.c_str(), FontSizeGeneral); // Adjust size as needed
	if (font == NULL) {
		// Handle font loading error
		std::cerr << TTF_GetError() << " | " << fontpath << "\n";
	}

	//------------------------------------------
	// Load console font
	consoleFont = TTF_OpenFont(fontpath.c_str(), FontSizeConsole); // Adjust size as needed
	if (consoleFont == NULL) {
		// Handle font loading error
		std::cerr << TTF_GetError() << " | " << fontpath << "\n";
	}
}

//------------------------------------------
// Pipeline

// For quick and dirty debugging, in case the rendering pipeline breaks somewhere
//# define debug_on_each_step 1

void Nebulite::Core::Renderer::tick(){
	#ifdef debug_on_each_step
    	std::cout << "clear..." << std::endl;
	#endif
    clear();           				// 1.) Clear screen FIRST, so that functions like snapshot have acces to the latest frame
	#ifdef debug_on_each_step
    	std::cout << "update..." << std::endl;
	#endif
    updateState();          		// 2.) Update objects, states, etc.
	#ifdef debug_on_each_step
    	std::cout << "renderFrame..." << std::endl;
	#endif
    renderFrame();     				// 3.) Render frame
	#ifdef debug_on_each_step
    	std::cout << "renderFPS..." << std::endl;
	#endif
	if(showFPS)renderFPS();       	// 4.) Render fps count
	#ifdef debug_on_each_step
    	std::cout << "showFrame..." << std::endl;
	#endif
    showFrame();       				// 5.) Show Frame
	#ifdef debug_on_each_step
		std::cout << "done!" << std::endl;
	#endif
}

bool Nebulite::Core::Renderer::timeToRender() {
	return SDL_GetTicks64() >= (prevTicks + TARGET_TICKS_PER_FRAME);
}

void Nebulite::Core::Renderer::append(Nebulite::Core::RenderObject* toAppend) {
	// Set ID
	toAppend->set<uint32_t>(Nebulite::Constants::keyName.renderObject.id.c_str(),renderobject_id_counter);
	renderobject_id_counter++;

	//Append to environment, based on layer
	env.append(
		toAppend, 
		invoke_ptr->getGlobalPointer()->get<int>(Nebulite::Constants::keyName.renderer.dispResX.c_str(),0), 
		invoke_ptr->getGlobalPointer()->get<int>(Nebulite::Constants::keyName.renderer.dispResY.c_str(),0), 
		toAppend->get(Nebulite::Constants::keyName.renderObject.layer.c_str(), 0)
	);

	//Load texture
	loadTexture(toAppend->get<std::string>(Nebulite::Constants::keyName.renderObject.imageLocation.c_str()));

	// Update rolling rand
	update_rrand();
}

void Nebulite::Core::Renderer::reinsertAllObjects(){
	env.reinsertAllObjects(
		invoke_ptr->getGlobalPointer()->get<int>(Nebulite::Constants::keyName.renderer.dispResX.c_str(),0),
		invoke_ptr->getGlobalPointer()->get<int>(Nebulite::Constants::keyName.renderer.dispResY.c_str(),0)
	);
}

void Nebulite::Core::Renderer::setGlobalValues(){
	//------------------------------------------
	// Simulation time
	uint64_t dt_ms = RendererLoopTime.get_dt_ms(); // Either fixed value or calculate from actual simtime difference
	uint64_t t_ms = RendererLoopTime.get_t_ms();
	invoke_ptr->getGlobalPointer()->set<double>( "time.dt", dt_ms / 1000.0);
	invoke_ptr->getGlobalPointer()->set<double>( "time.t",   t_ms / 1000.0);
	invoke_ptr->getGlobalPointer()->set<Uint64>( "time.dt_ms", dt_ms);
	invoke_ptr->getGlobalPointer()->set<Uint64>( "time.t_ms", t_ms);

	//------------------------------------------
	// Frame count

	// Get Frame count
	Uint64 ticks = invoke_ptr->getGlobalPointer()->get<Uint64>("frameCount",0);
	invoke_ptr->getGlobalPointer()->set<Uint64>("frameCount",ticks+1);

	//------------------------------------------
	// Full time (runtime)
	RendererFullTime.update();
	dt_ms = RendererFullTime.get_dt_ms();
	t_ms  = RendererFullTime.get_t_ms();
	invoke_ptr->getGlobalPointer()->set<double>( "runtime.dt", dt_ms / 1000.0);
	invoke_ptr->getGlobalPointer()->set<double>( "runtime.t",   t_ms / 1000.0);
	invoke_ptr->getGlobalPointer()->set<Uint64>( "runtime.dt_ms", dt_ms);
	invoke_ptr->getGlobalPointer()->set<Uint64>( "runtime.t_ms", t_ms);

	//------------------------------------------
	// Random
	update_rand();
	update_rrand();
}

//------------------------------------------
// Special Functions

void Nebulite::Core::Renderer::beep() {
	// Beep sound effect
	if(audioInitialized) {
		SDL_QueueAudio(audioDevice, squareBuffer->data(), samples * sizeof(Sint16));
		SDL_PauseAudioDevice(audioDevice, 0);  // Start playing
	}
}

bool Nebulite::Core::Renderer::snapshot(std::string link) {
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

//------------------------------------------
// Purge

void Nebulite::Core::Renderer::purgeObjects() {
	invoke_ptr->clear();
	env.purgeObjects();
}

void Nebulite::Core::Renderer::purgeTextures() {
	// Release resources for TextureContainer
	for (auto& pair : TextureContainer) {
		SDL_DestroyTexture(pair.second);
	}
	TextureContainer.clear(); // Clear the map to release resources
}

void Nebulite::Core::Renderer::destroy() {
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

//------------------------------------------
// Manipulation

void Nebulite::Core::Renderer::changeWindowSize(int w, int h, int scalar) {
	WindowScale = scalar;
	if(w < 64 || w > 16384){
		std::cerr << "Selected resolution is not supported:" << w << "x" << h << "x" << std::endl;
		return;
	}
	if(w < 64 || w > 16384){
		std::cerr << "Selected resolution is not supported:" << w << "x" << h << "x" << std::endl;
		return;
	}

	invoke_ptr->getGlobalPointer()->set<int>(Nebulite::Constants::keyName.renderer.dispResX.c_str(),w);
	invoke_ptr->getGlobalPointer()->set<int>(Nebulite::Constants::keyName.renderer.dispResY.c_str(),h);
    
    // Update the window size
    SDL_SetWindowSize(
		window, 
		invoke_ptr->getGlobalPointer()->get<int>(Nebulite::Constants::keyName.renderer.dispResX.c_str(),360) * WindowScale, 
		invoke_ptr->getGlobalPointer()->get<int>(Nebulite::Constants::keyName.renderer.dispResY.c_str(),360) * WindowScale
	);
	SDL_RenderSetLogicalSize(
		renderer,
		invoke_ptr->getGlobalPointer()->get<int>(Nebulite::Constants::keyName.renderer.dispResX.c_str(),360), 
		invoke_ptr->getGlobalPointer()->get<int>(Nebulite::Constants::keyName.renderer.dispResY.c_str(),360)
	);

	// Turn off console mode
	consoleMode = false;

    // Reinsert objects, due to new tile size
    reinsertAllObjects();
}

void Nebulite::Core::Renderer::moveCam(int dX, int dY) {
	invoke_ptr->getGlobalPointer()->set<int>(
		Nebulite::Constants::keyName.renderer.positionX.c_str(),
		invoke_ptr->getGlobalPointer()->get<int>(Nebulite::Constants::keyName.renderer.positionX.c_str(),0) + dX
	);
	invoke_ptr->getGlobalPointer()->set<int>(
		Nebulite::Constants::keyName.renderer.positionY.c_str(),
		invoke_ptr->getGlobalPointer()->get<int>(Nebulite::Constants::keyName.renderer.positionY.c_str(),0) + dY
	);
};

void Nebulite::Core::Renderer::setCam(int X, int Y, bool isMiddle) {
	std::cout << "Setting camera position to: " << X << ", " << Y << ", Middle: " << isMiddle << std::endl;

	if(isMiddle){
		int newPosX = X - invoke_ptr->getGlobalPointer()->get<int>(Nebulite::Constants::keyName.renderer.dispResX.c_str(),0) / 2;
		int newPosY = Y - invoke_ptr->getGlobalPointer()->get<int>(Nebulite::Constants::keyName.renderer.dispResY.c_str(),0) / 2;
		invoke_ptr->getGlobalPointer()->set<int>(Nebulite::Constants::keyName.renderer.positionX.c_str(),newPosX);
		invoke_ptr->getGlobalPointer()->set<int>(Nebulite::Constants::keyName.renderer.positionY.c_str(),newPosY);
	}
	else{
		invoke_ptr->getGlobalPointer()->set<int>(Nebulite::Constants::keyName.renderer.positionX.c_str(),X);
		invoke_ptr->getGlobalPointer()->set<int>(Nebulite::Constants::keyName.renderer.positionY.c_str(),Y);
	}
};

//------------------------------------------
// Event Handling



SDL_Event Nebulite::Core::Renderer::getEventHandle() {
	SDL_Event event;
	SDL_PollEvent(&event);
	return event;
}

//------------------------------------------
// Setting

void Nebulite::Core::Renderer::setTargetFPS(int fps) {
	if (fps > 0) {
		TARGET_FPS = fps;
		TARGET_TICKS_PER_FRAME = 1000 / TARGET_FPS;
	}
	else {
		TARGET_FPS = 60;
		TARGET_TICKS_PER_FRAME = 1000 / TARGET_FPS;
	}
}

//------------------------------------------
// Renderer::tick Functions

void Nebulite::Core::Renderer::clear(){
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255); // RGB values (black)
	SDL_RenderClear(renderer);
}

void Nebulite::Core::Renderer::updateState() {
	// Update loop timer
	uint64_t fixed_dt_ms = invoke_ptr->getGlobalPointer()->get<Uint64>(Nebulite::Constants::keyName.renderer.time_fixed_dt_ms.c_str(),0);
	RendererLoopTime.update(fixed_dt_ms);
	
	//------------------------------------------
	// Basic SDL event polling
	while (SDL_PollEvent(&event)) {
        switch (event.type) {
            case SDL_QUIT:
                quit = true;
                break;
        }
		// [CONSOLE MODE]
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

	//------------------------------------------
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
	

	//------------------------------------------
	// Internal container state

	// Only do container updates if not in console mode
	if(!consoleMode){
		// Set global values like time
		setGlobalValues();

		// Update invoke pairs, getting broadcast-listen-pairs from last env update
		invoke_ptr->update();

		// Update environment
		int dispResX = invoke_ptr->getGlobalPointer()->get<int>(Nebulite::Constants::keyName.renderer.dispResX.c_str(),0);
		int dispResY = invoke_ptr->getGlobalPointer()->get<int>(Nebulite::Constants::keyName.renderer.dispResY.c_str(),0);
		env.update(tileXpos,tileYpos,dispResX,dispResY,invoke_ptr);
	}
}

void Nebulite::Core::Renderer::renderFrame() {
	// Store for faster access
	int dispPosX = invoke_ptr->getGlobalPointer()->get<int>(Nebulite::Constants::keyName.renderer.positionX.c_str(),0);
	int dispPosY = invoke_ptr->getGlobalPointer()->get<int>(Nebulite::Constants::keyName.renderer.positionY.c_str(),0);

	// Depending on position, set tiles to render
	tileXpos = dispPosX / invoke_ptr->getGlobalPointer()->get<int>(Nebulite::Constants::keyName.renderer.dispResX.c_str(),0);
	tileYpos = dispPosY / invoke_ptr->getGlobalPointer()->get<int>(Nebulite::Constants::keyName.renderer.dispResY.c_str(),0);
	
	//------------------------------------------
	// FPS Count

	//Ticks and FPS
	totalframes++;
	REAL_FPS_COUNTER++;
	prevTicks = SDL_GetTicks64();

	//Calculate fps
	if (prevTicks - lastFPSRender >= 1000) {
		REAL_FPS = REAL_FPS_COUNTER;
		REAL_FPS_COUNTER = 0;
		lastFPSRender = prevTicks;
	}

	//------------------------------------------
	// Rendering
	int error = 0;

	//Render Objects
	//For all layers, starting at 0
	for (int layer = 0; layer < Nebulite::Core::Environment::LayerCount; layer++) {
		//Between dx +-1
		for (int dX = (tileXpos == 0 ? 0 : -1); dX <= 1; dX++) {
			// And dy +-1
			for (int dY = (tileYpos == 0 ? 0 : -1); dY <= 1; dY++) {
				Environment::Layer currentLayer = static_cast<Environment::Layer>(layer);
				// If valid
				if (env.isValidPosition(tileXpos + dX, tileYpos + dY,currentLayer)) {
					// For all batches inside
					for (auto& batch : env.getContainerAt(tileXpos + dX,tileYpos + dY,currentLayer)) {
						// For all objects in batch
						for(auto& obj : batch.objects){
							// Check for texture
							std::string innerdir = obj->get<std::string>(Nebulite::Constants::keyName.renderObject.imageLocation.c_str());
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
							if (obj->get<double>(Nebulite::Constants::keyName.renderObject.textFontsize.c_str())>0){
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
		for (const auto& [name, texture] : BetweenLayerTextures[static_cast<Environment::Layer>(layer)]) {
			SDL_RenderCopy(renderer, texture, NULL, NULL);
		}
	}

	//------------------------------------------
	// Render Console if Active
	if(consoleMode){
		// Semi-transparent background
		uint32_t height = invoke_ptr->getGlobalPointer()->get<int>(Nebulite::Constants::keyName.renderer.dispResY.c_str(),0) / 2;
		consoleRect.x = 0;
		consoleRect.y = invoke_ptr->getGlobalPointer()->get<int>(Nebulite::Constants::keyName.renderer.dispResY.c_str(),0) - height;
		consoleRect.w = invoke_ptr->getGlobalPointer()->get<int>(Nebulite::Constants::keyName.renderer.dispResX.c_str(),0);
		consoleRect.h = height;

		SDL_SetRenderDrawColor(renderer, 0, 32, 128, 180); // blue-ish with transparency
		SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
		SDL_RenderFillRect(renderer, &consoleRect);

		SDL_Color textColor = {255, 255, 255, 255}; // white
		int lineHeight = (double)TTF_FontHeight(consoleFont) / (double)WindowScale;

		// 1. Draw console input line at bottom
		if (!consoleInputBuffer.empty()) {
			// Create text texture
			SDL_Surface* textSurface = TTF_RenderText_Blended(consoleFont, consoleInputBuffer.c_str(), textColor);
			SDL_Texture* textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);

			// Rect size
			textRect.x = 10;
			textRect.y = consoleRect.y + consoleRect.h - lineHeight - 10;
			textRect.w = (double)textSurface->w / (double)WindowScale;
			textRect.h = (double)textSurface->h / (double)WindowScale;

			// Render the text
			SDL_RenderCopy(renderer, textTexture, NULL, &textRect);
			SDL_FreeSurface(textSurface);
			SDL_DestroyTexture(textTexture);
		}

		// 2. Render previous output lines above the input
		int maxLines = floor(consoleRect.h - 20 - lineHeight) / lineHeight;
		int startLine = std::max(0, (int)consoleOutput.size() - maxLines);
		for (int i = 0; i < maxLines && (startLine + i) < consoleOutput.size(); ++i) {
			// Create text texture
			const std::string& line = consoleOutput[startLine + i];
			SDL_Surface* textSurface = TTF_RenderText_Blended(consoleFont, line.c_str(), textColor);
			SDL_Texture* textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);

			// Rect size
			SDL_Rect lineRect;
			lineRect.x = 10;
			lineRect.y = consoleRect.y + 10 + i * lineHeight;
			lineRect.w = (double)textSurface->w / (double)WindowScale;
			lineRect.h = (double)textSurface->h / (double)WindowScale;

			// Render the text
			SDL_RenderCopy(renderer, textTexture, NULL, &lineRect);
			SDL_FreeSurface(textSurface);
			SDL_DestroyTexture(textTexture);
		}
	}

}

void Nebulite::Core::Renderer::renderFPS(double scalar) {
	scalar = scalar / (double)WindowScale;

	// Create a string with the FPS value
	std::string fpsText = "FPS: " + std::to_string(REAL_FPS);

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

void Nebulite::Core::Renderer::showFrame() {
	SDL_RenderPresent(renderer);
}

//------------------------------------------
// Texture-Related

void Nebulite::Core::Renderer::loadTexture(std::string link) {
	// Combine directory and innerdir to form full path
	std::string path = Nebulite::Utility::FileManagement::CombinePaths(baseDirectory, link);

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