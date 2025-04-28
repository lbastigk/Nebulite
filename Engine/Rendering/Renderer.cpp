#include "Renderer.h"



Renderer::Renderer(bool flag_hidden, unsigned int zoom, unsigned int X, unsigned int Y){

	RenderZoom=zoom;

	//Basic variables
	Xpos = 0;
	Ypos = 0;
	tileXpos = 0;
	tileYpos = 0;

	// Init Event
	event = SDL_Event();

	// Init Rect
	rect = SDL_Rect();

	// Get the current directory
	directory = FileManagement::currentDir();

	// Get screen resolution
	std::cout << "Resolution setting\n"; 	
	dispResX = X;
	dispResY = Y;

	//Create SDL window
	if (SDL_Init(SDL_INIT_VIDEO) != 0) {
		// SDL initialization failed
		std::cerr << "SDL_Init Error: " << SDL_GetError() << std::endl;
	}
	window = SDL_CreateWindow(
		"Nebulite",            // Window title
		SDL_WINDOWPOS_CENTERED,
		SDL_WINDOWPOS_CENTERED,
		dispResX*zoom,                        // Width
		dispResY*zoom,                        // Height
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
		std::cerr << "\nCould not load Font: " << TTF_GetError() << "\n";
	}

	// Create a renderer
	renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
	if (!renderer) {
		std::cerr << "Renderer creation failed: << SDL_GetError()" << std::endl;
	}

	// Set virtual rendering size
	SDL_RenderSetLogicalSize(renderer, (int)dispResX, (int)dispResY);

	starttime = Time::gettime();
    currentTime = Time::gettime();
    lastTime = Time::gettime();

	Invoke.linkGlobal(env.getGlobal());
}

//Destructor
Renderer::~Renderer() {
	purgeObjects();
	purgeTextures();

	// Release resources for SDL objects
	if (window) {
		SDL_DestroyWindow(window);
	}

	if (renderer) {
		SDL_DestroyRenderer(renderer);
	}

	if (font) {
		TTF_CloseFont(font);
	}
}

//Marshalling
std::string Renderer::serialize() {
	//TODO?+
	return env.serialize();
}

std::string Renderer::serializeEnvironment() {
	return env.serialize();
}

void Renderer::deserializeEnvironment(std::string serialOrLink) {
	env.deserialize(serialOrLink, dispResX, dispResY, THREADSIZE);


}

//-----------------------------------------------------------
// Pipeline

void Renderer::append(RenderObject toAppend) {
	// Set ID
	toAppend.valueSet<uint32_t>(namenKonvention.renderObject.id,id_counter);
	id_counter++;

	//Append to environment, based on layer
	env.append(toAppend, dispResX, dispResY, THREADSIZE, toAppend.valueGet(namenKonvention.renderObject.layer, 0));

	//Load texture
	loadTexture(toAppend);
}

void Renderer::reinsertAllObjects(){
	env.reinsertAllObjects(dispResX,dispResY,THREADSIZE);
}

void Renderer::update() {
	pollEvent();
	setGlobalValues();
	Invoke.update();
	env.update(tileXpos,tileYpos,dispResX, dispResY, THREADSIZE,&Invoke);
	Invoke.getNewInvokes();
}


void Renderer::update_withThreads() {
	pollEvent();
	setGlobalValues();
	Invoke.update();
	env.update_withThreads(tileXpos, tileYpos, dispResX, dispResY, THREADSIZE,&Invoke);
	Invoke.getNewInvokes();
}

void Renderer::setGlobalValues(){
	// Setup global doc with values

	// increase loop time
	JSONHandler::Set::Any<double>(env.getGlobal(), "t", (currentTime-starttime)/1000.0);

	// compute dt
	currentTime = Time::gettime();
	JSONHandler::Set::Any<double>(env.getGlobal(), "dt", (currentTime - lastTime) / 1000.0);
	lastTime = currentTime;

	// Cursor Position:
	lastMousePosX = MousePosX;
	lastMousePosY = MousePosY;
	lastMouseState = mouseState;
	mouseState = SDL_GetMouseState(&MousePosX, &MousePosY);

	JSONHandler::Set::Any(env.getGlobal(),"mouse_current.X",MousePosX);
	JSONHandler::Set::Any(env.getGlobal(),"mouse_current.Y",MousePosY);
	JSONHandler::Set::Any(env.getGlobal(),"mouse_delta.X",MousePosX-lastMousePosX);
	JSONHandler::Set::Any(env.getGlobal(),"mouse_delta.Y",MousePosY-lastMousePosY);

	JSONHandler::Set::Any(env.getGlobal(),"mouse_current.left",!!(SDL_BUTTON(SDL_BUTTON_LEFT) & mouseState));
	JSONHandler::Set::Any(env.getGlobal(),"mouse_current.right",!!(SDL_BUTTON(SDL_BUTTON_RIGHT) & mouseState));
	JSONHandler::Set::Any(env.getGlobal(),"mouse_delta.left",
		!!(SDL_BUTTON(SDL_BUTTON_LEFT) & mouseState) - 
		!!(SDL_BUTTON(SDL_BUTTON_LEFT) & lastMouseState));
	JSONHandler::Set::Any(env.getGlobal(),"mouse_delta.right",
		!!(SDL_BUTTON(SDL_BUTTON_RIGHT) & mouseState) - 
		!!(SDL_BUTTON(SDL_BUTTON_RIGHT) & lastMouseState));
}

//-----------------------------------------------------------
// Purge
void Renderer::purgeObjects() {
	env.purgeObjects();
}

void Renderer::purgeObjectsAt(int x, int y){
	env.purgeObjectsAt(x,y,dispResX,dispResY);
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
	// End of Program!
	SDL_DestroyWindow(window);
	SDL_DestroyRenderer(renderer);
	// SDL_Quit(); // Not necessary here
}

//-----------------------------------------------------------
// Manipulation

void Renderer::changeWindowSize(int w, int h) {
    dispResX = w;
    dispResY = h;
    
    // Update the window size
    SDL_SetWindowSize(window, dispResX, dispResY);
	SDL_RenderSetLogicalSize(renderer, dispResX, dispResY);

    // Reinsert objects or do any additional resizing logic here
    reinsertAllObjects();
}

void Renderer::updatePosition(int x, int y, bool isMiddle) {
	if(isMiddle){
		Xpos = x - (int)(dispResX / 2);
		Ypos = y - (int)(dispResY / 2);
	}
	else{
		Xpos = x;
		Ypos = y;
	}
	
	tileXpos = Xpos / dispResX;
	tileYpos = Ypos / dispResY;
}

void Renderer::moveCam(int dX, int dY) {
	Xpos += dX;
	Ypos += dY;
	tileXpos = Xpos / dispResX;
	tileYpos = Ypos / dispResY;
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

// TODO: Multithreading ...
// Needed changes:
// - texture container for multi threading (MUTEX?)
//
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
	}

	//------------------------------------------------
	// Rendering

	// Clear the window with a specified color (e.g., black)
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255); // RGB values (black)
	SDL_RenderClear(renderer);

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
							//Texture loading is handled in append
							std::string innerdir = obj->valueGet<std::string>(namenKonvention.renderObject.imageLocation);
							if (TextureContainer.find(innerdir) == TextureContainer.end()) {
								loadTexture(*obj);
								obj->calculateDstRect();
								//obj->calculateSrcRect();
							}
							obj->calculateSrcRect();

							rect = obj->getDstRect();
							rect.x -= Xpos;	//subtract camera posX
							rect.y -= Ypos;	//subtract camera posY

							// Render the texture to the window
							error = SDL_RenderCopy(renderer, TextureContainer[innerdir], obj->getSrcRect(), &rect);
							if (obj->valueGet<float>(namenKonvention.renderObject.textFontsize)>0){
								obj->calculateTxtRect(renderer,font);
								SDL_RenderCopy(renderer,&obj->getTextTexture(),NULL,obj->getTextRect());
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

void Renderer::renderFrameNoThreads() {
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

		if (control_fps) {
			//FPS-Control
			/*
			//int fps is current fps
			//int SCREEN_FPS is goal
			//This part is called ever second when fps is recalculated

			//Implement PID for int epsillon
			//epsillon is a �s delay to get closer to goal FPS

			//Current implementation is a simple counter
			if (fps > SCREEN_FPS) {
				epsillon += 1;
			}
			else if (fps < SCREEN_FPS) {
				//Eps cant be under 0
				if (epsillon > 0) {
					epsillon -= 1;
				}
			}
			*/

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
	}

	

	//------------------------------------------------
	// Rendering

	// Clear the window with a specified color (e.g., black)
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255); // RGB values (black)
	SDL_RenderClear(renderer);

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
							//Texture loading is handled in append
							std::string innerdir = obj->valueGet<std::string>(namenKonvention.renderObject.imageLocation);
							if (TextureContainer.find(innerdir) == TextureContainer.end()) {
								loadTexture(*obj);
								obj->calculateDstRect();
								obj->calculateSrcRect();
							}
							obj->calculateSrcRect();

							rect = obj->getDstRect();
							rect.x -= Xpos;	//subtract camera posX
							rect.y -= Ypos;	//subtract camera posY

							// Render the texture to the window
							error = SDL_RenderCopy(renderer, TextureContainer[innerdir], obj->getSrcRect(), &rect);
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

void Renderer::pollEvent() {
	while (SDL_PollEvent(&event)) {
		switch (event.type) {
		case SDL_QUIT:
			quit=true;
			break;
			// Handle other events as needed
		case SDL_KEYDOWN:
			switch(event.key.keysym.sym){
				case Renderer::SDL::KEY_W:
					JSONHandler::Set::Any(env.getGlobal(),"keyboard.w",1);
					break;
				case Renderer::SDL::KEY_A:
					JSONHandler::Set::Any(env.getGlobal(),"keyboard.a",1);
					break;
				case Renderer::SDL::KEY_S:
					JSONHandler::Set::Any(env.getGlobal(),"keyboard.s",1);
					break;
				case Renderer::SDL::KEY_D:
					JSONHandler::Set::Any(env.getGlobal(),"keyboard.d",1);
					break;
			}
			break;
		case SDL_KEYUP:
			switch(event.key.keysym.sym){
				case Renderer::SDL::KEY_W:
					JSONHandler::Set::Any(env.getGlobal(),"keyboard.w",0);
					break;
				case Renderer::SDL::KEY_A:
					JSONHandler::Set::Any(env.getGlobal(),"keyboard.a",0);
					break;
				case Renderer::SDL::KEY_S:
					JSONHandler::Set::Any(env.getGlobal(),"keyboard.s",0);
					break;
				case Renderer::SDL::KEY_D:
					JSONHandler::Set::Any(env.getGlobal(),"keyboard.d",0);
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
// Getting
int Renderer::getEps() {return epsillon;}
size_t Renderer::getTextureAmount() {return TextureContainer.size();}
size_t Renderer::getObjectCount() {return env.getObjectCount();}
int Renderer::getResX() {return dispResX;}
int Renderer::getResY() {return dispResY;}
int Renderer::getThreadSize() {return THREADSIZE;}
int Renderer::getFPS() {return fps;}
int Renderer::getPosX(){return Xpos;};
int Renderer::getPosY(){return Ypos;};
SDL_Renderer* Renderer::getSdlRenderer(){return renderer;};

//-----------------------------------------------------------
// Other
bool Renderer::windowExists(){
	return !!Renderer::window;
}

void Renderer::loadTexture(RenderObject& toAppend) {
	std::string innerdir = toAppend.valueGet<std::string>(namenKonvention.renderObject.imageLocation);

	// Combine directory and innerdir to form full path
	std::string path = FileManagement::CombinePaths(directory, innerdir);

	// Check if texture is already loaded
	if (TextureContainer.find(innerdir) == TextureContainer.end()) {
		SDL_Surface* surface = IMG_Load(path.c_str()); // Attempt to load as PNG (or other supported formats)
		if (!surface) {
			surface = SDL_LoadBMP(path.c_str()); // Fallback to BMP if PNG load fails
		}
		if (!surface) {
			std::cerr << "Failed to load image '" << path << "': " << SDL_GetError() << std::endl;
			return;
		}

		// Create texture from surface
		SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
		SDL_FreeSurface(surface); // Free the surface after creating texture
		if (!texture) {
			std::cerr << "Failed to create texture from surface: " << SDL_GetError() << std::endl;
			return;
		}

		// Store texture in container
		TextureContainer[innerdir] = texture;
	}
}

