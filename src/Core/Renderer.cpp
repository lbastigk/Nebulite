#include "Core/Renderer.hpp"

#include "Core/GlobalSpace.hpp"
#include "DomainModule/RRDM.hpp"
#include <cstdint>
#include <sys/types.h>

Nebulite::Core::Renderer::Renderer(Nebulite::Core::GlobalSpace* globalSpace, bool* flag_headless, unsigned int X, unsigned int Y)
: 	Nebulite::Interaction::Execution::Domain<Nebulite::Core::Renderer>("Renderer", this, globalSpace->getDoc(), globalSpace),
	env(globalSpace),
	rngA(hashString("Seed for RNG A")),
	rngB(hashString("Seed for RNG B"))
	{
	//------------------------------------------
	// Depending on platform, set Global key "platform":
	#ifdef _WIN32
		getDoc()->set<std::string>("platform","windows");
	#elif __linux__
		getDoc()->set<std::string>("platform","linux");
	#elif __APPLE__
		getDoc()->set<std::string>("platform","macos");
	#elif __FreeBSD__
		getDoc()->set<std::string>("platform","freebsd");
	#elif __unix__
		getDoc()->set<std::string>("platform","unix");
	#elif __ANDROID__
		getDoc()->set<std::string>("platform","android");
	#elif __TEMPLEOS__
		printf("Glory be to TempleOS!\n");
		getDoc()->set<std::string>("platform","templeos");
	#else
		getDoc()->set<std::string>("platform","unknown");
	#endif

	//------------------------------------------
	// Initialize internal variables

	// Window
	WindowScale = 1;
	headless = flag_headless;

	// Position
	tileXpos = 0;
	tileYpos = 0;

	// State
	event = SDL_Event();
	baseDirectory = Nebulite::Utility::FileManagement::currentDir();

	// Waveform buffers: Sine wave buffer
	basicAudioWaveforms.sineBuffer = new std::vector<Sint16>(basicAudioWaveforms.samples);
	for (size_t i = 0; i < basicAudioWaveforms.samples; i++) {
		double time = static_cast<double>(i) / basicAudioWaveforms.sampleRate;
		(*basicAudioWaveforms.sineBuffer)[i] = static_cast<int16_t>(32767 * 0.3 * sin(2.0 * M_PI * basicAudioWaveforms.frequency * time));
	}

	// Waveform buffers: Square wave buffer
	basicAudioWaveforms.squareBuffer = new std::vector<Sint16>(basicAudioWaveforms.samples);
	for (size_t i = 0; i < basicAudioWaveforms.samples; i++){
		double time = static_cast<double>(i) / basicAudioWaveforms.sampleRate;

		// Square wave: alternates between +1 and -1
		double phase = 2.0 * M_PI * basicAudioWaveforms.frequency * time;
		double squareValue = (sin(phase) >= 0) ? 1.0 : -1.0;

		(*basicAudioWaveforms.squareBuffer)[i] = static_cast<int16_t>(32767 * 0.3 * squareValue);
	}

	// Waveform buffers: Triangle wave buffer
	basicAudioWaveforms.triangleBuffer = new std::vector<Sint16>(basicAudioWaveforms.samples);
	for (size_t i = 0; i < basicAudioWaveforms.samples; i++){
		double time = static_cast<double>(i) / basicAudioWaveforms.sampleRate;

		// Triangle wave: linear ramp up and down
		double phase = fmod(basicAudioWaveforms.frequency * time, 1.0);  // 0 to 1
		double triangleValue;
		
		if (phase < 0.5){
			triangleValue = 4.0 * phase - 1.0;      // -1 to +1 (rising)
		} else {
			triangleValue = 3.0 - 4.0 * phase;      // +1 to -1 (falling)
		}

		(*basicAudioWaveforms.triangleBuffer)[i] = static_cast<int16_t>(32767 * 0.3 * triangleValue);
	}

	//------------------------------------------
	// Set basic values inside global doc
	getDoc()->set<unsigned int>(Nebulite::Constants::keyName.renderer.dispResX.c_str(),X);	
	getDoc()->set<unsigned int>(Nebulite::Constants::keyName.renderer.dispResY.c_str(),Y);

	getDoc()->set<unsigned int>(Nebulite::Constants::keyName.renderer.positionX.c_str(),0);	
	getDoc()->set<unsigned int>(Nebulite::Constants::keyName.renderer.positionY.c_str(),0);

	//------------------------------------------
	// Start timers
	fpsControlTimer.start();
	fpsRenderTimer.start();

	//------------------------------------------
	// Pre-parse initialization
	setPreParse(std::bind(&Nebulite::Core::Renderer::preParse, this));

	//------------------------------------------
	// Domain Modules
	Nebulite::DomainModule::RRDM_init(this);
}

Nebulite::Constants::Error Nebulite::Core::Renderer::preParse(){
	// Initialize SDL and related subsystems
	initSDL();
	return Nebulite::Constants::ErrorTable::NONE();
}

void Nebulite::Core::Renderer::initSDL(){
	if(SDL_initialized)return;

	//------------------------------------------
	// Window

	//Create SDL window
	if (SDL_Init(SDL_INIT_VIDEO) != 0){
		// SDL initialization failed
		Nebulite::Utility::Capture::cerr() << "SDL_Init Error: " << SDL_GetError() << Nebulite::Utility::Capture::endl;
	}
	// Define window via x|y|w|h
	int x = SDL_WINDOWPOS_CENTERED;
	int y = SDL_WINDOWPOS_CENTERED;
	int w = getDoc()->get<int>(Nebulite::Constants::keyName.renderer.dispResX.c_str(),0);
	int h = getDoc()->get<int>(Nebulite::Constants::keyName.renderer.dispResY.c_str(),0);

	uint32_t flags;
	flags = *headless ? SDL_WINDOW_HIDDEN : SDL_WINDOW_SHOWN;
	//flags = flags | SDL_WINDOW_RESIZABLE; // Disabled for now, as it causes issues with the logical size rendering
	flags = flags | SDL_WINDOW_OPENGL;
	window = SDL_CreateWindow("Nebulite",x,y,w,h,flags);
	if (!window){
		// Window creation failed
		Nebulite::Utility::Capture::cerr() << "SDL_CreateWindow Error: " << SDL_GetError() << Nebulite::Utility::Capture::endl;
		SDL_Quit();
	}

	//------------------------------------------
	// Renderer

	// Create a renderer
	renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
	if (!renderer){
		Nebulite::Utility::Capture::cerr() << "Renderer creation failed: " << SDL_GetError() << Nebulite::Utility::Capture::endl;
	}

	// Set virtual rendering size
	SDL_RenderSetLogicalSize(
		renderer, 
		w, 
		h
	);

	//------------------------------------------
	// Fonts

	// Initialize SDL_ttf
	if (TTF_Init() < 0){
		// Handle SDL_ttf initialization error
		SDL_Quit(); // Clean up SDL
	}
	loadFonts();

	//------------------------------------------
	// Audio

	// Init
	if (SDL_Init(SDL_INIT_AUDIO) < 0){
		Nebulite::Utility::Capture::cerr() << "SDL_Init Error: " << SDL_GetError() << Nebulite::Utility::Capture::endl;
	} else {
		audio.desired.freq = 44100;
		audio.desired.format = AUDIO_S16SYS;
		audio.desired.channels = 1;
		audio.desired.samples = 1024;
		audio.desired.callback = nullptr;

		audio.device = SDL_OpenAudioDevice(nullptr, 0, &audio.desired, &audio.obtained, 0);
		if (audio.device == 0){
			Nebulite::Utility::Capture::cerr() << "Failed to open audio device: " << SDL_GetError() << Nebulite::Utility::Capture::endl;
		} else{
			audioInitialized = true;
		}
	}

	SDL_initialized = true;
}

void Nebulite::Core::Renderer::loadFonts(){
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
	font = TTF_OpenFont(fontpath.c_str(), static_cast<int>(FontSizeGeneral)); // Adjust size as needed
	if (font == NULL){
		// Handle font loading error
		Nebulite::Utility::Capture::cerr() << TTF_GetError() << " | " << fontpath << "\n";
	}
}

//------------------------------------------
// Pipeline

// For quick and dirty debugging, in case the rendering pipeline breaks somewhere
//# define debug_on_each_step 1
bool Nebulite::Core::Renderer::tick(Nebulite::Interaction::Invoke* invoke_ptr){
	//------------------------------------------
	// Do all the steps of the rendering pipeline
    clear();           				// 1.) Clear screen FIRST, so that functions like snapshot have acces to the latest frame
    updateState(invoke_ptr);        // 2.) Update objects, states, etc.
    renderFrame();     				// 3.) Render frame
	if(showFPS)renderFPS();       	// 4.) Render fps count
    showFrame();       				// 5.) Show Frame

	//------------------------------------------
	// SDL Polling at the end of the frame
	events.clear();
	while (SDL_PollEvent(&event)){
		// Store events for other processes, e.g. domain modules
		events.push_back(event);

		// Handle quit event
        switch (event.type){
            case SDL_QUIT:
                quit = true;
                break;
        }
    }

	//------------------------------------------
	// Manage frame skipping
	skippedUpdateLastFrame = skipUpdate;
	skipUpdate = false;

	//------------------------------------------
	// Update modules
	updateModules();

	return !skippedUpdateLastFrame;
}

bool Nebulite::Core::Renderer::timeToRender(){
	return fpsControlTimer.projected_dt() >= TARGET_TICKS_PER_FRAME;
}

void Nebulite::Core::Renderer::append(Nebulite::Core::RenderObject* toAppend){
	// Set ID
	toAppend->set<uint32_t>(Nebulite::Constants::keyName.renderObject.id.c_str(),renderobject_id_counter);
	renderobject_id_counter++;

	//Append to environment, based on layer
	env.append(
		toAppend, 
		getDoc()->get<uint16_t>(Nebulite::Constants::keyName.renderer.dispResX.c_str(),0), 
		getDoc()->get<uint16_t>(Nebulite::Constants::keyName.renderer.dispResY.c_str(),0), 
		toAppend->get<uint8_t>(Nebulite::Constants::keyName.renderObject.layer.c_str(), 0)
	);

	//Load texture
	loadTexture(toAppend->get<std::string>(Nebulite::Constants::keyName.renderObject.imageLocation.c_str()));
}

void Nebulite::Core::Renderer::reinsertAllObjects(){
	env.reinsertAllObjects(
		getDoc()->get<uint16_t>(Nebulite::Constants::keyName.renderer.dispResX.c_str(),0),
		getDoc()->get<uint16_t>(Nebulite::Constants::keyName.renderer.dispResY.c_str(),0)
	);
}

//------------------------------------------
// Special Functions

void Nebulite::Core::Renderer::beep(){
	// Beep sound effect
	if(audioInitialized){
		uint32_t audioLength = static_cast<uint32_t>(basicAudioWaveforms.samples * sizeof(int16_t));
		SDL_QueueAudio(audio.device, basicAudioWaveforms.squareBuffer->data(), audioLength);
		SDL_PauseAudioDevice(audio.device, 0);  // Start playing
	}
}

bool Nebulite::Core::Renderer::snapshot(std::string link){
    if (!renderer){
        Nebulite::Utility::Capture::cerr() << "Cannot take snapshot: renderer not initialized" << Nebulite::Utility::Capture::endl;
        return false;
    }
    
    // Get current window/render target size
    int width, height;
    if (window){
        // Normal windowed mode
        SDL_GetWindowSize(window, &width, &height);
    } else {
        // Headless mode - get renderer output size
        SDL_GetRendererOutputSize(renderer, &width, &height);
    }
    
    // Create surface to capture pixels
    SDL_Surface* surface = SDL_CreateRGBSurface(0, width, height, 32,
                                                0x00ff0000,  // Red mask
                                                0x0000ff00,  // Green mask  
                                                0x000000ff,  // Blue mask
                                                0xff000000); // Alpha mask
    
    if (!surface){
        Nebulite::Utility::Capture::cerr() << "Failed to create surface for snapshot: " << SDL_GetError() << Nebulite::Utility::Capture::endl;
        return false;
    }
    
    // Read pixels from renderer
    if (SDL_RenderReadPixels(renderer, NULL, SDL_PIXELFORMAT_ARGB8888, 
                            surface->pixels, surface->pitch) != 0){
        Nebulite::Utility::Capture::cerr() << "Failed to read pixels for snapshot: " << SDL_GetError() << Nebulite::Utility::Capture::endl;
        SDL_FreeSurface(surface);
        return false;
    }
    
    // Create directory if it doesn't exist
    std::string directory = link.substr(0, link.find_last_of("/\\"));

	// Edge case: check if link contains no directory:
	if(link.find_last_of("/\\") == std::string::npos){
		directory = "./Resources/Snapshots";
		link = directory + "/" + link;
	}

    if (!directory.empty()){
        // Create directory using C++17 filesystem
        try {
            std::filesystem::create_directories(directory);
        } catch (const std::exception& e){
            //Nebulite::Utility::Capture::cerr() << "Warning: Could not create directory " << directory << ": " << e.what() << Nebulite::Utility::Capture::endl;
            // Continue anyway - maybe directory already exists
        }
    }
    
    // Save surface as PNG
    int result = IMG_SavePNG(surface, link.c_str());
    
    // Cleanup
    SDL_FreeSurface(surface);
    
    if (result != 0){
        Nebulite::Utility::Capture::cerr() << "Failed to save snapshot: " << IMG_GetError() << Nebulite::Utility::Capture::endl;
        return false;
    }
    return true;
}

//------------------------------------------
// Purge

void Nebulite::Core::Renderer::purgeObjects(){
	env.purgeObjects();
}

void Nebulite::Core::Renderer::purgeTextures(){
	// Release resources for TextureContainer
	for (auto& pair : TextureContainer){
		SDL_DestroyTexture(pair.second);
	}
	TextureContainer.clear(); // Clear the map to release resources
}

void Nebulite::Core::Renderer::destroy(){
	if(!SDL_initialized)return;
    if (window){
        SDL_DestroyWindow(window);
        window = nullptr;
    }
    if (renderer){
        SDL_DestroyRenderer(renderer);
        renderer = nullptr;
    }
    if (font){
        TTF_CloseFont(font);
        font = nullptr;
    }
}

//------------------------------------------
// Manipulation

void Nebulite::Core::Renderer::changeWindowSize(int w, int h, uint16_t scalar){
	WindowScale = scalar;
	if(w < 240 || w > 16384){
		Nebulite::Utility::Capture::cerr() << "Selected resolution is not supported:" << w << "x" << h << "x" << Nebulite::Utility::Capture::endl;
		return;
	}
	if(h < 240 || h > 16384){
		Nebulite::Utility::Capture::cerr() << "Selected resolution is not supported:" << w << "x" << h << "x" << Nebulite::Utility::Capture::endl;
		return;
	}

	getDoc()->set<int>(Nebulite::Constants::keyName.renderer.dispResX.c_str(),w);
	getDoc()->set<int>(Nebulite::Constants::keyName.renderer.dispResY.c_str(),h);

    // Update the window size
    SDL_SetWindowSize(
		window, 
		getDoc()->get<int>(Nebulite::Constants::keyName.renderer.dispResX.c_str(),360) * WindowScale, 
		getDoc()->get<int>(Nebulite::Constants::keyName.renderer.dispResY.c_str(),360) * WindowScale
	);
	SDL_RenderSetLogicalSize(
		renderer,
		getDoc()->get<int>(Nebulite::Constants::keyName.renderer.dispResX.c_str(),360), 
		getDoc()->get<int>(Nebulite::Constants::keyName.renderer.dispResY.c_str(),360)
	);

	// Turn off console mode
	// Not needed anymore, console should dynamically adapt to new window size
	// consoleMode = false;

    // Reinsert objects, due to new tile size
    reinsertAllObjects();
}

void Nebulite::Core::Renderer::moveCam(int dX, int dY){
	getDoc()->set<int>(
		Nebulite::Constants::keyName.renderer.positionX.c_str(),
		getDoc()->get<int>(Nebulite::Constants::keyName.renderer.positionX.c_str(),0) + dX
	);
	getDoc()->set<int>(
		Nebulite::Constants::keyName.renderer.positionY.c_str(),
		getDoc()->get<int>(Nebulite::Constants::keyName.renderer.positionY.c_str(),0) + dY
	);
}

void Nebulite::Core::Renderer::setCam(int X, int Y, bool isMiddle){
	logln("Setting camera position to: " + std::to_string(X) + ", " + std::to_string(Y) + ", Middle: " + std::to_string(isMiddle));

	if(isMiddle){
		int newPosX = X - getDoc()->get<int>(Nebulite::Constants::keyName.renderer.dispResX.c_str(),0) / 2;
		int newPosY = Y - getDoc()->get<int>(Nebulite::Constants::keyName.renderer.dispResY.c_str(),0) / 2;
		getDoc()->set<int>(Nebulite::Constants::keyName.renderer.positionX.c_str(),newPosX);
		getDoc()->set<int>(Nebulite::Constants::keyName.renderer.positionY.c_str(),newPosY);
	}
	else{
		getDoc()->set<int>(Nebulite::Constants::keyName.renderer.positionX.c_str(),X);
		getDoc()->set<int>(Nebulite::Constants::keyName.renderer.positionY.c_str(),Y);
	}
}

//------------------------------------------
// Setting

void Nebulite::Core::Renderer::setTargetFPS(uint16_t fps){
	if (fps > 0){
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

void Nebulite::Core::Renderer::updateState(Nebulite::Interaction::Invoke* invoke_ptr){
	//------------------------------------------
	// Skip update if flagged
	if(skipUpdate){
		return;
	}

	// Update invoke pairs, getting broadcast-listen-pairs from last env update
	invoke_ptr->update();

	// Update environment
	uint16_t dispResX = getDoc()->get<uint16_t>(Nebulite::Constants::keyName.renderer.dispResX.c_str(),0);
	uint16_t dispResY = getDoc()->get<uint16_t>(Nebulite::Constants::keyName.renderer.dispResY.c_str(),0);
	env.update(tileXpos,tileYpos,dispResX,dispResY);
}

void Nebulite::Core::Renderer::renderFrame(){
	//------------------------------------------
	// Store for faster access

	// Get camera position
	int16_t dispPosX = getDoc()->get<int16_t>(Nebulite::Constants::keyName.renderer.positionX.c_str(),0);
	int16_t dispPosY = getDoc()->get<int16_t>(Nebulite::Constants::keyName.renderer.positionY.c_str(),0);

	// Depending on position, set tiles to render
	tileXpos = dispPosX / getDoc()->get<int16_t>(Nebulite::Constants::keyName.renderer.dispResX.c_str(),0);
	tileYpos = dispPosY / getDoc()->get<int16_t>(Nebulite::Constants::keyName.renderer.dispResY.c_str(),0);

	//------------------------------------------
	// FPS Count and Control
	
	//Calculate fps every second
	REAL_FPS_COUNTER++;
	if (fpsRenderTimer.projected_dt() >= 1000){
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
	for (auto layer : *(env.getAllLayers())){
		// Get all tile positions to render
		std::vector<std::pair<int16_t, int16_t>> tilesToRender;
		for (int dX = (tileXpos == 0 ? 0 : -1); dX <= 1; dX++){
			for (int dY = (tileYpos == 0 ? 0 : -1); dY <= 1; dY++){
				if (env.isValidPosition(tileXpos + dX, tileYpos + dY, layer)){
					tilesToRender.emplace_back(tileXpos + dX, tileYpos + dY);
				}
			}
		}

		// For all tiles to render
		for (auto const& [tileX, tileY] : tilesToRender){
			// For all batches inside
			for (auto& batch : env.getContainerAt(tileX, tileY, layer)){
				// For all objects in batch
				for(auto& obj : batch.objects){
					error = renderObjectToScreen(obj, dispPosX, dispPosY);
					if(error != 0){
						Nebulite::Utility::Capture::cerr() << "Error rendering object ID " << obj->get<uint32_t>(Nebulite::Constants::keyName.renderObject.id.c_str(),0) << ": " << error << Nebulite::Utility::Capture::endl;
					}
				}
			}
		}

		// Render all textures that were attached from outside processes
		for (auto const& [name, texturePair] : BetweenLayerTextures[layer]){
			auto const& texture = texturePair.first;
			auto const& rect = texturePair.second;

			if (!texture){
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
	if (TextureContainer.find(innerdir) == TextureContainer.end()){
		loadTexture(innerdir);
	}

	// Link texture if not yet linked
	if(obj->isTextureValid() == false){
		obj->linkExternalTexture(TextureContainer[innerdir]);
	}

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
	if(!obj->getSDLTexture()){
		Nebulite::Utility::Capture::cerr() << "Error: RenderObject ID " << obj->get<uint32_t>(Nebulite::Constants::keyName.renderObject.id.c_str(),0) << " texture with path '" << innerdir << "' not found" << Nebulite::Utility::Capture::endl;
		return -1;
	}

	//------------------------------------------
	// Rendering

	// Render the texture
	int error_sprite = SDL_RenderCopy(renderer, obj->getSDLTexture(), obj->getSrcRect(), obj->getDstRect());

	// Render the text
	int error_text = 0;
	double val = obj->get<double>(Nebulite::Constants::keyName.renderObject.textFontsize.c_str());
	if (val > 0){
		obj->calculateText(
			renderer,
			font,
			dispPosX,
			dispPosY
		);
		if(obj->getTextTexture() && obj->getTextRect()){
			error_text = SDL_RenderCopy(renderer,obj->getTextTexture(),NULL,obj->getTextRect());
		}
	}
	
	//------------------------------------------
	// Return
	if(error_sprite != 0){
		return error_sprite;
	}
	return error_text;
}

void Nebulite::Core::Renderer::renderFPS(double scalar){
	scalar = scalar / static_cast<double>(WindowScale);

	// Create a string with the FPS value
	std::string fpsText = "FPS: " + std::to_string(REAL_FPS);

	double fontSize = 16;

	// Define the destination rectangle for rendering the text
	SDL_Rect textRect = { static_cast<int>(scalar*10.0), static_cast<int>(scalar*10.0), 0, 0 }; // Adjust position as needed
	textRect.w = static_cast<int>(scalar * fontSize * static_cast<int>(fpsText.length())); // Width based on text length
	textRect.h = static_cast<int>(fontSize * 1.5 * scalar);

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

void Nebulite::Core::Renderer::showFrame(){
	SDL_RenderPresent(renderer);
}

//------------------------------------------
// Texture-Related

void Nebulite::Core::Renderer::loadTexture(std::string const& link){
	SDL_Texture* texture = loadTextureToMemory(link);
	if (texture){
		TextureContainer[link] = texture;
	}
}

/**
 * @todo Texture not created with SDL_TEXTUREACCESS_TARGET, so cannot be used with SDL_SetRenderTarget
 */
SDL_Texture* Nebulite::Core::Renderer::loadTextureToMemory(std::string const& link){
    std::string path = Nebulite::Utility::FileManagement::CombinePaths(baseDirectory, link);
    
	// Get file extension, based on last dot
	std::string extension;
	size_t dotPos = path.find_last_of('.');
	if (dotPos != std::string::npos){
		extension = path.substr(dotPos + 1);
	}
	else {
		Nebulite::Utility::Capture::cerr() << "Failed to load image '" << path << "': No file extension found." << Nebulite::Utility::Capture::endl;
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
	if (surface == nullptr){
		Nebulite::Utility::Capture::cerr() << "Failed to load image '" << path << "': " << SDL_GetError() << Nebulite::Utility::Capture::endl;
		return nullptr;
	}

	// Create texture from surface
	SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
	SDL_FreeSurface(surface); // Free the surface after creating texture

	// Check for texture issues
	if (!texture){
		Nebulite::Utility::Capture::cerr() << "Failed to create texture from surface: " << SDL_GetError() << Nebulite::Utility::Capture::endl;
		return nullptr;
	}

	// Store texture in container
	return texture;
}


