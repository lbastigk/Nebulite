#include "Core/Renderer.hpp"

#include "Core/GlobalSpace.hpp"
#include "DomainModule/RRDM.hpp"

Nebulite::Core::Renderer::Renderer(Nebulite::Core::GlobalSpace* globalSpace, bool flag_headless, unsigned int X, unsigned int Y)
: 	Nebulite::Interaction::Execution::Domain<Nebulite::Core::Renderer>("Renderer", this, globalSpace->getDoc()),
	rngA(hashString("Seed for RNG A")),
	rngB(hashString("Seed for RNG B")),
	env(globalSpace)
	{
	//------------------------------------------
	// Linkages
	invoke_ptr = globalSpace->invoke.get();

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
	invoke_ptr->getGlobalPointer()->set<int>(Nebulite::Constants::keyName.renderer.dispResX.c_str(),X);
	invoke_ptr->getGlobalPointer()->set<int>(Nebulite::Constants::keyName.renderer.dispResY.c_str(),Y);
	int x = SDL_WINDOWPOS_CENTERED;
	int y = SDL_WINDOWPOS_CENTERED;
	int w = X;
	int h = Y;

	uint32_t flags;
	flags = flag_headless ? SDL_WINDOW_HIDDEN : SDL_WINDOW_SHOWN;
	//flags = flags | SDL_WINDOW_RESIZABLE; // Disabled for now, as it causes issues with the logical size rendering
	flags = flags | SDL_WINDOW_OPENGL;
	window = SDL_CreateWindow("Nebulite",x,y,w,h,flags);
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

	invoke_ptr->getGlobalPointer()->set<int>(Nebulite::Constants::keyName.renderer.positionX.c_str(),0);	
	invoke_ptr->getGlobalPointer()->set<int>(Nebulite::Constants::keyName.renderer.positionY.c_str(),0);

	//------------------------------------------
	// Start timers
	fpsControlTimer.start();
	fpsRenderTimer.start();

	//------------------------------------------
	// Domain Modules
	Nebulite::DomainModule::RRDM_init(this);
}

void Nebulite::Core::Renderer::loadFonts() {
	//------------------------------------------
	// Sizes
	uint32_t FontSizeGeneral = 60; 			// Does not need to scale

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
}

//------------------------------------------
// Pipeline

// For quick and dirty debugging, in case the rendering pipeline breaks somewhere
//# define debug_on_each_step 1
bool Nebulite::Core::Renderer::tick(){
	//------------------------------------------
	// Do all the steps of the rendering pipeline
    clear();           				// 1.) Clear screen FIRST, so that functions like snapshot have acces to the latest frame
    updateState();          		// 2.) Update objects, states, etc.
    renderFrame();     				// 3.) Render frame
	if(showFPS)renderFPS();       	// 4.) Render fps count
    showFrame();       				// 5.) Show Frame

	//------------------------------------------
	// SDL Polling at the end of the frame
	events.clear();
	while (SDL_PollEvent(&event)) {
		// Store events for other processes, e.g. domain modules
		events.push_back(event);

		// Handle quit event
        switch (event.type) {
            case SDL_QUIT:
                quit = true;
                break;
        }
    }

	//------------------------------------------
	// Check if resolution changed
	/*
	int w = invoke_ptr->getGlobalPointer()->get<int>(Nebulite::Constants::keyName.renderer.dispResX.c_str(),0);
	int h = invoke_ptr->getGlobalPointer()->get<int>(Nebulite::Constants::keyName.renderer.dispResY.c_str(),0);
	int current_w, current_h;
	SDL_GetWindowSize(window, &current_w, &current_h);
	if (current_w != w || current_h != h) {
		// Resolution changed, update internal state
		invoke_ptr->getGlobalPointer()->set<int>(Nebulite::Constants::keyName.renderer.dispResX.c_str(),current_w);
		invoke_ptr->getGlobalPointer()->set<int>(Nebulite::Constants::keyName.renderer.dispResY.c_str(),current_h);
		
		// Since tiles scale with resolution, we need to reinsert all objects
		reinsertAllObjects();
	}
	*/

	skippedUpdateLastFrame = skipUpdate;
	skipUpdate = false;
	return !skippedUpdateLastFrame;
}

bool Nebulite::Core::Renderer::timeToRender() {
	return fpsControlTimer.projected_dt() >= TARGET_TICKS_PER_FRAME;
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
}

void Nebulite::Core::Renderer::reinsertAllObjects(){
	env.reinsertAllObjects(
		invoke_ptr->getGlobalPointer()->get<int>(Nebulite::Constants::keyName.renderer.dispResX.c_str(),0),
		invoke_ptr->getGlobalPointer()->get<int>(Nebulite::Constants::keyName.renderer.dispResY.c_str(),0)
	);
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
	if(w < 240 || w > 16384){
		std::cerr << "Selected resolution is not supported:" << w << "x" << h << "x" << std::endl;
		return;
	}
	if(h < 240 || h > 16384){
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
	// Not needed anymore, console should dynamically adapt to new window size
	// consoleMode = false;

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
	//------------------------------------------
	// Skip update if flagged
	if(skipUpdate){
		return;
	}

	// Update invoke pairs, getting broadcast-listen-pairs from last env update
	invoke_ptr->update();

	// Update environment
	int dispResX = invoke_ptr->getGlobalPointer()->get<int>(Nebulite::Constants::keyName.renderer.dispResX.c_str(),0);
	int dispResY = invoke_ptr->getGlobalPointer()->get<int>(Nebulite::Constants::keyName.renderer.dispResY.c_str(),0);
	env.update(tileXpos,tileYpos,dispResX,dispResY,invoke_ptr);
}

void Nebulite::Core::Renderer::renderFrame() {
	//------------------------------------------
	// Store for faster access

	// Get camera position
	int dispPosX = invoke_ptr->getGlobalPointer()->get<int>(Nebulite::Constants::keyName.renderer.positionX.c_str(),0);
	int dispPosY = invoke_ptr->getGlobalPointer()->get<int>(Nebulite::Constants::keyName.renderer.positionY.c_str(),0);

	// Depending on position, set tiles to render
	tileXpos = dispPosX / invoke_ptr->getGlobalPointer()->get<int>(Nebulite::Constants::keyName.renderer.dispResX.c_str(),0);
	tileYpos = dispPosY / invoke_ptr->getGlobalPointer()->get<int>(Nebulite::Constants::keyName.renderer.dispResY.c_str(),0);
	
	//------------------------------------------
	// FPS Count and Control
	
	//Calculate fps every second
	REAL_FPS_COUNTER++;
	if (fpsRenderTimer.projected_dt() >= 1000) {
		REAL_FPS = REAL_FPS_COUNTER;
		REAL_FPS_COUNTER = 0;
		fpsRenderTimer.update();
	}

	// Control framerate
	fpsControlTimer.update();

	//------------------------------------------
	// Rendering
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255); // Black background
	int error = 0;

	//Render Objects
	//For all layers, starting at 0
	for (auto layer : *(env.getAllLayers())) {
		// Get all tile positions to render
		std::vector<std::pair<int, int>> tilesToRender;
		for (int dX = (tileXpos == 0 ? 0 : -1); dX <= 1; dX++) {
			for (int dY = (tileYpos == 0 ? 0 : -1); dY <= 1; dY++) {
				if (env.isValidPosition(tileXpos + dX, tileYpos + dY, layer)) {
					tilesToRender.emplace_back(tileXpos + dX, tileYpos + dY);
				}
			}
		}

		// For all tiles to render
		for (const auto& [tileX, tileY] : tilesToRender) {
			// For all batches inside
			for (auto& batch : env.getContainerAt(tileX, tileY, layer)) {
				// For all objects in batch
				for(auto& obj : batch.objects){
					error = renderObjectToScreen(obj, dispPosX, dispPosY);
					if(error != 0){
						std::cerr << "Error rendering object ID " << obj->get<uint32_t>(Nebulite::Constants::keyName.renderObject.id.c_str(),0) << ": " << error << std::endl;
					}
				}
			}
		}

		// Render all textures that were attached from outside processes
		for (const auto& [name, texturePair] : BetweenLayerTextures[layer]) {
			const auto& texture = texturePair.first;
			const auto& rect = texturePair.second;

			if (!texture) {
				continue; // Skip if texture is null
			}
			SDL_RenderCopy(renderer, texture, NULL, rect);
		}
	}
}

int Nebulite::Core::Renderer::renderObjectToScreen(Nebulite::Core::RenderObject* obj, int dispPosX, int dispPosY){
	//------------------------------------------
	// Texture Loading
	
	// Check for texture
	std::string innerdir = obj->get<std::string>(Nebulite::Constants::keyName.renderObject.imageLocation.c_str());

	// Load texture if not yet loaded
	if (TextureContainer.find(innerdir) == TextureContainer.end()) {
		loadTexture(innerdir);
	}

	// Link texture if not yet linked
	if(obj->isTextureValid() == false){
		obj->linkExternalTexture(TextureContainer[innerdir]);
	}
	SDL_Texture* texture = obj->getSDLTexture();

	//------------------------------------------
	// Source and Destination Rectangles

	// Calculate source rect
	obj->calculateSrcRect();
	
	// Calculate position rect
	obj->calculateDstRect();
	obj->getDstRect()->x -= dispPosX;	// Subtract X camera position
	obj->getDstRect()->y -= dispPosY;	// Subtract Y camera position

	

	//------------------------------------------
	// Error Checking
	if(!texture){
		std::cerr << "Error: RenderObject ID " << obj->get<uint32_t>(Nebulite::Constants::keyName.renderObject.id.c_str(),0) << " texture with path '" << innerdir << "' not found" << std::endl;
		return -1;
	}

	//------------------------------------------
	// Rendering

	// Render the texture
	int error_sprite = SDL_RenderCopy(renderer, texture, obj->getSrcRect(), obj->getDstRect());

	// Render the text
	//*
	int error_text = 0;
	double val = obj->get<double>(Nebulite::Constants::keyName.renderObject.textFontsize.c_str());
	if (val > 0){
		obj->calculateText(
			renderer,
			font,
			dispPosX,
			dispPosY
		);
		SDL_Texture* texture = obj->getTextTexture();
		if(texture && obj->getTextRect()){
			error_text = SDL_RenderCopy(renderer,texture,NULL,obj->getTextRect());
		}
	}
	
	//------------------------------------------
	// Return
	if(error_sprite != 0){
		return error_sprite;
	}
	return error_text;
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
	SDL_Texture* texture = loadTextureToMemory(link);
	if (texture) {
		TextureContainer[link] = texture;
	}
}

/**
 * @todo Texture not created with SDL_TEXTUREACCESS_TARGET, so cannot be used with SDL_SetRenderTarget
 */
SDL_Texture* Nebulite::Core::Renderer::loadTextureToMemory(std::string link) {
    std::string path = Nebulite::Utility::FileManagement::CombinePaths(baseDirectory, link);
    
	// Get file extension, based on last dot
	std::string extension;
	size_t dotPos = path.find_last_of('.');
	if (dotPos != std::string::npos) {
		extension = path.substr(dotPos + 1);
	}
	else {
		std::cerr << "Failed to load image '" << path << "': No file extension found." << std::endl;
		return nullptr;
	}

	// turn to lowercase
	std::transform(extension.begin(), extension.end(), extension.begin(), ::tolower);

	// Check for known image formats
	SDL_Surface* surface = nullptr;
	if(extension == "bmp"){
		surface = SDL_LoadBMP(path.c_str());
	}
	else if(extension == "png" || extension == "jpg" || extension == "jpeg" || extension == "tif" || extension == "tiff" || extension == "webp" || extension == "gif"){
		surface = IMG_Load(path.c_str());
	}

	// Unknown format or other issues with surface
	if (surface == nullptr) {
		std::cerr << "Failed to load image '" << path << "': " << SDL_GetError() << std::endl;
		return nullptr;
	}

	// Create texture from surface
	SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
	SDL_FreeSurface(surface); // Free the surface after creating texture

	// Check for texture issues
	if (!texture) {
		std::cerr << "Failed to create texture from surface: " << SDL_GetError() << std::endl;
		return nullptr;
	}

	// Store texture in container
	return texture;
}


