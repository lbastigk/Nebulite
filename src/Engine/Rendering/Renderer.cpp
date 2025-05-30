#include "Renderer.h"



Renderer::Renderer(std::deque<std::string>& tasks, Invoke& invoke, bool flag_hidden, unsigned int zoom, unsigned int X, unsigned int Y){
	//--------------------------------------------
	// Linkages
	invoke_ptr = &invoke;
	invoke_ptr->linkGlobal(env.getGlobal());
	invoke_ptr->linkQueue(tasks);

	//--------------------------------------------
	// Initialize internal variables
	RenderZoom=zoom;
	tileXpos = 0;
	tileYpos = 0;
	event = SDL_Event();
	rect = SDL_Rect();
	directory = FileManagement::currentDir();
	starttime = Time::gettime();
    currentTime = Time::gettime();
    lastTime = Time::gettime();
	

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
		JSONHandler::Get::Any<int>(*invoke_ptr->getGlobalPointer(),"display.resolution.X",X)*zoom,                        // Width
		JSONHandler::Get::Any<int>(*invoke_ptr->getGlobalPointer(),"display.resolution.Y",Y)*zoom,                        // Height
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

	// Create a renderer
	renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
	if (!renderer) {
		std::cerr << "Renderer creation failed: << SDL_GetError()" << std::endl;
	}

	// Set virtual rendering size
	SDL_RenderSetLogicalSize(
		renderer, 
		JSONHandler::Get::Any<int>(*invoke_ptr->getGlobalPointer(),"display.resolution.X",X), 
		JSONHandler::Get::Any<int>(*invoke_ptr->getGlobalPointer(),"display.resolution.Y",Y)
	);

	//--------------------------------------------
	// Set basic values inside global doc

	JSONHandler::Set::Any<int>(*invoke_ptr->getGlobalPointer(),"display.resolution.X",X);	
	JSONHandler::Set::Any<int>(*invoke_ptr->getGlobalPointer(),"display.resolution.Y",Y);
	JSONHandler::Set::Any<int>(*invoke_ptr->getGlobalPointer(),"display.position.X",0);
	JSONHandler::Set::Any<int>(*invoke_ptr->getGlobalPointer(),"display.position.Y",0);

	JSONHandler::Set::Any(env.getGlobal(),"input.keyboard.w",0);
	JSONHandler::Set::Any(env.getGlobal(),"input.keyboard.a",0);
	JSONHandler::Set::Any(env.getGlobal(),"input.keyboard.s",0);
	JSONHandler::Set::Any(env.getGlobal(),"input.keyboard.d",0);

	JSONHandler::Set::Any<Uint64>(env.getGlobal(),"time.fixed_dt_ms",0);
	JSONHandler::Set::Any<double>(env.getGlobal(),"time.t",0);
	JSONHandler::Set::Any<Uint64>(env.getGlobal(),"time.t_ms",0);
	JSONHandler::Set::Any<double>(env.getGlobal(),"time.dt",0);
	JSONHandler::Set::Any<Uint64>(env.getGlobal(),"time.dt_ms",0);
    JSONHandler::Set::Any<double>(env.getGlobal(),"physics.G",0.1 * 100);
}


//Marshalling
std::string Renderer::serialize() {
	return env.serialize();
}

void Renderer::deserialize(std::string serialOrLink) {
	env.deserialize(
		serialOrLink, 
		JSONHandler::Get::Any<int>(*invoke_ptr->getGlobalPointer(),"display.resolution.X",0), 
		JSONHandler::Get::Any<int>(*invoke_ptr->getGlobalPointer(),"display.resolution.Y",0), 
		THREADSIZE
	);
}

//-----------------------------------------------------------
// Pipeline
void Renderer::append(std::shared_ptr<RenderObject> toAppend) {
	// Set ID
	toAppend.get()->valueSet<uint32_t>(namenKonvention.renderObject.id,id_counter);
	id_counter++;

	//Append to environment, based on layer
	env.append(
		toAppend, 
		JSONHandler::Get::Any<int>(*invoke_ptr->getGlobalPointer(),"display.resolution.X",0), 
		JSONHandler::Get::Any<int>(*invoke_ptr->getGlobalPointer(),"display.resolution.Y",0), 
		THREADSIZE, 
		toAppend.get()->valueGet(namenKonvention.renderObject.layer, 0)
	);

	//Load texture
	loadTexture(toAppend.get()->valueGet<std::string>(namenKonvention.renderObject.imageLocation));
}

void Renderer::reinsertAllObjects(){
	env.reinsertAllObjects(
		JSONHandler::Get::Any<int>(*invoke_ptr->getGlobalPointer(),"display.resolution.X",0),
		JSONHandler::Get::Any<int>(*invoke_ptr->getGlobalPointer(),"display.resolution.Y",0),
		THREADSIZE
	);
}

void Renderer::update() {
	pollEvent();
	setGlobalValues();
	invoke_ptr->update();
	env.update(
		tileXpos,
		tileYpos,
		JSONHandler::Get::Any<int>(*invoke_ptr->getGlobalPointer(),"display.resolution.X",0), 
		JSONHandler::Get::Any<int>(*invoke_ptr->getGlobalPointer(),"display.resolution.Y",0), 
		THREADSIZE,
		invoke_ptr
	);
	invoke_ptr->getNewInvokes();
}

void Renderer::update_withThreads() {
	pollEvent();
	setGlobalValues();
	invoke_ptr->update();
	env.update_withThreads(
		tileXpos, 
		tileYpos, 
		JSONHandler::Get::Any<int>(*invoke_ptr->getGlobalPointer(),"display.resolution.X",0), 
		JSONHandler::Get::Any<int>(*invoke_ptr->getGlobalPointer(),"display.resolution.Y",0), 
		THREADSIZE,
		invoke_ptr
	);
	invoke_ptr->getNewInvokes();
}


//-----------------------------------------------------------
// Purge
void Renderer::purgeObjects() {
	invoke_ptr->clear();
	env.purgeObjects();
}

void Renderer::purgeObjectsAt(int x, int y){
	env.purgeObjectsAt(
		x,
		y,
		JSONHandler::Get::Any<int>(*invoke_ptr->getGlobalPointer(),"display.resolution.X",0),
		JSONHandler::Get::Any<int>(*invoke_ptr->getGlobalPointer(),"display.resolution.Y",0)
	);
}

void Renderer::purgeLayer(int layer) {
	env.purgeLayer(layer);
}

void Renderer::purgeTextures() {
	// Release resources for TextureContainer
	for (auto& pair : TextureContainer) {
		SDL_DestroyTexture(pair.second);
	}
	TextureContainer.clear(); // Clear the map to release resources
}

void Renderer::destroy() {
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

void Renderer::changeWindowSize(int w, int h) {
	if(w < 64 || w > 16384){
		std::cerr << "Selected resolution is not supported" << std::endl;
		return;
	}
	if(w < 64 || w > 16384){
		std::cerr << "Selected resolution is not supported" << std::endl;
		return;
	}

	JSONHandler::Set::Any<int>(*invoke_ptr->getGlobalPointer(),"display.resolution.X",w);
	JSONHandler::Set::Any<int>(*invoke_ptr->getGlobalPointer(),"display.resolution.Y",h);
    
    // Update the window size
    SDL_SetWindowSize(
		window, 
		JSONHandler::Get::Any<int>(*invoke_ptr->getGlobalPointer(),"display.resolution.X",0), 
		JSONHandler::Get::Any<int>(*invoke_ptr->getGlobalPointer(),"display.resolution.Y",0)
	);
	SDL_RenderSetLogicalSize(
		renderer,
		JSONHandler::Get::Any<int>(*invoke_ptr->getGlobalPointer(),"display.resolution.X",0), 
		JSONHandler::Get::Any<int>(*invoke_ptr->getGlobalPointer(),"display.resolution.Y",0)
	);

    // Reinsert objects or do any additional resizing logic here
    reinsertAllObjects();
}

void Renderer::moveCam(int dX, int dY, bool isMiddle) {
	JSONHandler::Set::Any<int>(
		*invoke_ptr->getGlobalPointer(),
		"display.position.X",
		JSONHandler::Get::Any<int>(*invoke_ptr->getGlobalPointer(),"display.position.X",0) + dX
	);
	JSONHandler::Set::Any<int>(
		*invoke_ptr->getGlobalPointer(),
		"display.position.Y",
		JSONHandler::Get::Any<int>(*invoke_ptr->getGlobalPointer(),"display.position.X",0) + dY
	);
	tileXpos = JSONHandler::Get::Any<int>(*invoke_ptr->getGlobalPointer(),"display.position.X",0) / JSONHandler::Get::Any<int>(*invoke_ptr->getGlobalPointer(),"display.resolution.X",0);
	tileYpos = JSONHandler::Get::Any<int>(*invoke_ptr->getGlobalPointer(),"display.position.Y",0) / JSONHandler::Get::Any<int>(*invoke_ptr->getGlobalPointer(),"display.resolution.Y",0);
};

void Renderer::setCam(int X, int Y, bool isMiddle) {
	if(isMiddle){
		JSONHandler::Set::Any<int>(
			*invoke_ptr->getGlobalPointer(),
			"display.position.X",
			X - JSONHandler::Get::Any<int>(*invoke_ptr->getGlobalPointer(),"display.resolution.X",0) / 2
		);
		JSONHandler::Set::Any<int>(
			*invoke_ptr->getGlobalPointer(),
			"display.position.Y",
			Y - JSONHandler::Get::Any<int>(*invoke_ptr->getGlobalPointer(),"display.resolution.Y",0) / 2
		);
	}
	else{
		JSONHandler::Set::Any<int>(*invoke_ptr->getGlobalPointer(),"display.position.X",X);
		JSONHandler::Set::Any<int>(*invoke_ptr->getGlobalPointer(),"display.position.X",Y);
	}
	tileXpos = X / JSONHandler::Get::Any<int>(*invoke_ptr->getGlobalPointer(),"display.resolution.X",0);
	tileYpos = Y / JSONHandler::Get::Any<int>(*invoke_ptr->getGlobalPointer(),"display.resolution.Y",0);
};


//-----------------------------------------------------------
// Rendering

bool Renderer::timeToRender() {
	if (control_fps) {
		// FPS control is currently buggy, perhaps overflow?
		//if (epsillon < 0) {
		//	return SDL_GetTicks64() >= (prevTicks + SCREEN_TICKS_PER_FRAME + epsillon / 1000);
		//}
		return SDL_GetTicks64() >= (prevTicks + SCREEN_TICKS_PER_FRAME);
	}
	else {
		return true;
	}
}

void Renderer::clear(){
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255); // RGB values (black)
	SDL_RenderClear(renderer);
}


void Renderer::renderFrame() {
	
	//------------------------------------------------
	// FPS Count

	//Additional microsecond delay
	Time::waitmicroseconds(epsillon);

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
	for (int i = 0; i < RENDEROBJECTCONTAINER_COUNT; i++) {
		//Between dx +-1
		for (int dX = (tileXpos == 0 ? 0 : -1); dX <= 1; dX++) {
			// And dy +-1
			for (int dY = (tileYpos == 0 ? 0 : -1); dY <= 1; dY++) {
				// If valid
				if (env.isValidPosition(tileXpos + dX, tileYpos + dY,i)) {
					// For all batches inside
					for (auto& batch : (env.getContainerAt(tileXpos + dX, tileYpos + dY, i))) {
						// For all objects inside each batch
						for (auto& obj : batch) {
							// Check for texture
							
							std::string innerdir = obj.get()->valueGet<std::string>(namenKonvention.renderObject.imageLocation);
							if (TextureContainer.find(innerdir) == TextureContainer.end()) {
								loadTexture(innerdir);
								obj.get()->calculateDstRect();
							}
							obj.get()->calculateSrcRect();
							
							
							// Calculate position rect
							rect = obj->getDstRect();
							rect.x -= JSONHandler::Get::Any<int>(*invoke_ptr->getGlobalPointer(),"display.position.X",0);	//subtract camera posX
							rect.y -= JSONHandler::Get::Any<int>(*invoke_ptr->getGlobalPointer(),"display.position.Y",0);;	//subtract camera posY
							

							// Render the texture
							error = SDL_RenderCopy(renderer, TextureContainer[innerdir], obj.get()->getSrcRect(), &rect);

							// Render the text
							if (obj.get()->valueGet<float>(namenKonvention.renderObject.textFontsize)>0){
								obj.get()->calculateText(
									renderer,
									font,
									JSONHandler::Get::Any<int>(*invoke_ptr->getGlobalPointer(),"display.position.X",0),
									JSONHandler::Get::Any<int>(*invoke_ptr->getGlobalPointer(),"display.position.Y",0)
								);
								SDL_RenderCopy(renderer,&obj.get()->getTextTexture(),NULL,obj.get()->getTextRect());
							}
							if (error != 0){
								std::cerr << "SDL Error while rendering Frame: " << error << std::endl;
							}
						}
					}
				}
			}
		}
	}

}

void Renderer::renderFPS(float scalar) {
	scalar = scalar / (float)RenderZoom;

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

void Renderer::showFrame() {
	SDL_RenderPresent(renderer);
}

// This function is called on each frame to set the values for the global document
void Renderer::setGlobalValues(){
	// Time
	// logs:
	// - time in s and ms
	// - dt from last frame in s and ms
	lastTime = currentTime;
	currentTime = Time::gettime();

	// Get dt. Either fixed value or calculate from actual time difference
	Uint64 dt_ms = JSONHandler::Get::Any<Uint64>(env.getGlobal(),"time.fixed_dt_ms",0);
	if(dt_ms == 0){
		dt_ms = currentTime - lastTime;
	}
	Uint64 t_ms = JSONHandler::Get::Any<Uint64>(env.getGlobal(), "time.t_ms",0) + dt_ms;
	JSONHandler::Set::Any<double>(env.getGlobal(), "time.dt", dt_ms / 1000.0);
	JSONHandler::Set::Any<double>(env.getGlobal(), "time.t",   t_ms / 1000.0);
	JSONHandler::Set::Any<Uint64>(env.getGlobal(), "time.dt_ms", dt_ms);
	JSONHandler::Set::Any<Uint64>(env.getGlobal(), "time.t_ms", t_ms);

	// Get Frame count
	Uint64 ticks = JSONHandler::Get::Any<Uint64>(env.getGlobal(),"frameCount",0);
	JSONHandler::Set::Any<Uint64>(env.getGlobal(),"frameCount",ticks+1);

	// Cursor Position and state
	JSONHandler::Set::Any(env.getGlobal(),"input.mouse.current.X",MousePosX);
	JSONHandler::Set::Any(env.getGlobal(),"input.mouse.current.Y",MousePosY);
	JSONHandler::Set::Any(env.getGlobal(),"input.mouse.delta.X",MousePosX-lastMousePosX);
	JSONHandler::Set::Any(env.getGlobal(),"input.mouse.delta.Y",MousePosY-lastMousePosY);

	JSONHandler::Set::Any(env.getGlobal(),"input.mouse.current.left",!!(SDL_BUTTON(SDL_BUTTON_LEFT) & mouseState));
	JSONHandler::Set::Any(env.getGlobal(),"input.mouse.current.right",!!(SDL_BUTTON(SDL_BUTTON_RIGHT) & mouseState));
	JSONHandler::Set::Any(env.getGlobal(),"input.mouse.delta.left",
		!!(SDL_BUTTON(SDL_BUTTON_LEFT) & mouseState) - 
		!!(SDL_BUTTON(SDL_BUTTON_LEFT) & lastMouseState));
	JSONHandler::Set::Any(env.getGlobal(),"input.mouse.delta.right",
		!!(SDL_BUTTON(SDL_BUTTON_RIGHT) & mouseState) - 
		!!(SDL_BUTTON(SDL_BUTTON_RIGHT) & lastMouseState));
}


void Renderer::pollEvent() {
	lastMousePosX = MousePosX;
	lastMousePosY = MousePosY;
	lastMouseState = mouseState;
	mouseState = SDL_GetMouseState(&MousePosX, &MousePosY);

	while (SDL_PollEvent(&event)) {
		switch (event.type) {
		case SDL_QUIT:
			quit=true;
			break;
			// Handle other events as needed
		case SDL_KEYDOWN:
			switch(event.key.keysym.sym){
				case Renderer::SDL::KEY_W:
					JSONHandler::Set::Any(env.getGlobal(),"input.keyboard.w",1);
					break;
				case Renderer::SDL::KEY_A:
					JSONHandler::Set::Any(env.getGlobal(),"input.keyboard.a",1);
					break;
				case Renderer::SDL::KEY_S:
					JSONHandler::Set::Any(env.getGlobal(),"input.keyboard.s",1);
					break;
				case Renderer::SDL::KEY_D:
					JSONHandler::Set::Any(env.getGlobal(),"input.keyboard.d",1);
					break;
			}
			break;
		case SDL_KEYUP:
			switch(event.key.keysym.sym){
				case Renderer::SDL::KEY_W:
					JSONHandler::Set::Any(env.getGlobal(),"input.keyboard.w",0);
					break;
				case Renderer::SDL::KEY_A:
					JSONHandler::Set::Any(env.getGlobal(),"input.keyboard.a",0);
					break;
				case Renderer::SDL::KEY_S:
					JSONHandler::Set::Any(env.getGlobal(),"input.keyboard.s",0);
					break;
				case Renderer::SDL::KEY_D:
					JSONHandler::Set::Any(env.getGlobal(),"input.keyboard.d",0);
					break;
			}
			break;
		}
	}
}

SDL_Event Renderer::getEventHandle() {
	SDL_Event event;
	SDL_PollEvent(&event);
	return event;
}

//-----------------------------------------------------------
// Setting
void Renderer::setFPS(int fps) {
	if (fps > 0) {
		control_fps = true;
		SCREEN_FPS = fps;
		SCREEN_TICKS_PER_FRAME = 1000 / SCREEN_FPS;
	}
	else {
		control_fps = false;
	}
}

void Renderer::setThreadSize(unsigned int size) {
	THREADSIZE = size;
}

//-----------------------------------------------------------
// Other

void Renderer::loadTexture(std::string link) {
	//std::cout << "Loading Texture:" << link << std::endl;

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

