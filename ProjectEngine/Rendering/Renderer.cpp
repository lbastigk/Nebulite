#include <SDL.h>
#include <SDL_ttf.h>
#include <SDL_image.h>

#include "Optionsloader.cpp"
#include "Environment.cpp"
#include "FileManagement.cpp"
#include "Time.cpp"

#include <thread>

#include <stdint.h>

#define WINDOWNAME "coolgame"

class Renderer {
public:
	Renderer(){
		//Options
		generalOptions.setFileName("options.txt");

		//Basic variables
		Xpos = 0;
		Ypos = 0;
		tileXpos = 0;
		tileYpos = 0;

		event = SDL_Event();

		// Get the current directory as a std::string
		directory = FileManagement::currentDir();

		//Create SDL window
		if (SDL_Init(SDL_INIT_VIDEO) != 0) {
			// SDL initialization failed
			std::cerr << "SDL_Init Error: " << SDL_GetError() << std::endl;
		}
		window = SDL_CreateWindow(
			generalOptions.GetOption("windowName").c_str(),            // Window title
			SDL_WINDOWPOS_CENTERED,
			SDL_WINDOWPOS_CENTERED,
			atoi(generalOptions.GetOption("dispResX").c_str()),                        // Width
			atoi(generalOptions.GetOption("dispResY").c_str()),                        // Height
			SDL_WINDOW_SHOWN
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
		std::string fontpath = FileManagement::CombinePaths(directory, "Resources\\Fonts\\Arimo-Regular.ttf");
		//std::cout << fontpath;
		font = TTF_OpenFont(fontpath.c_str(), 60); // Adjust size as needed
		if (font == NULL) {
			// Handle font loading error
			std::cout << "\nCould not load Font: " << TTF_GetError() << "\n\n";
		}

		// Create a renderer
		renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
		if (!renderer) {
			printf("Renderer creation failed: %s\n", SDL_GetError());
		}

		// Get screen resolution
		dispResX = atoi(generalOptions.GetOption(namenKonvention.options.dispResX).c_str());
		dispResY = atoi(generalOptions.GetOption(namenKonvention.options.dispResY).c_str());

		//Rect
		rect = SDL_Rect();
	}

	//Destructor
	~Renderer() {
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
	std::string serialize() {
		//TODO?
	}
	std::string serializeEnvironment() {
		return env.serialize();
	}

	void deserializeEnvironment(std::string serialOrLink, int dispResX, int dispResY, int THREADSIZE) {
		env.deserialize(serialOrLink, dispResX, dispResY, THREADSIZE);
	}

	//-----------------------------------------------------------
	// Pipeline
	void append(RenderObject toAppend) {
		//Append to environment, based on layer
		env.append(toAppend, dispResX, dispResY, THREADSIZE, toAppend.valueGet(namenKonvention.renderObject.layer, 0));

		//Load texture
		loadTexture(toAppend);
	}
	void update() {
		env.update(tileXpos, tileYpos, dispResX, dispResY, THREADSIZE);
	}
	void update_withThreads() {
		env.update_withThreads(tileXpos, tileYpos, dispResX, dispResY, THREADSIZE);
	}
	

	//-----------------------------------------------------------
	// Purge
	void purgeObjects() {
		env.purgeObjects();
	}
	void purgeLayer(int layer) {
		env.purgeLayer(layer);
	}

	void purgeTextures() {
		// Release resources for TextureContainer
		for (auto& pair : TextureContainer) {
			SDL_DestroyTexture(pair.second);
		}
		TextureContainer.clear(); // Clear the map to release resources
	}

	void destroy() {
		// End of Program!
		SDL_DestroyWindow(window);
		SDL_DestroyRenderer(renderer);
		// SDL_Quit(); // Not necessary here
	}

	//-----------------------------------------------------------
	// Manipulation

	void changeWindowSize() {
		
	}

	void updatePosition(int x, int y) {
		Xpos = x;
		Ypos = y;
		tileXpos = Xpos / stoi(generalOptions.GetOption(namenKonvention.options.dispResX));
		tileYpos = Ypos / stoi(generalOptions.GetOption(namenKonvention.options.dispResY));
	}

	void moveCam(int dX, int dY) {
		Xpos += dX;
		Ypos += dY;
		tileXpos = Xpos / stoi(generalOptions.GetOption(namenKonvention.options.dispResX));
		tileYpos = Ypos / stoi(generalOptions.GetOption(namenKonvention.options.dispResY));
	};

	//-----------------------------------------------------------
	// Rendering

	bool timeToRender() {
		if (control_fps) {
			if (epsillon < 0) {
				return SDL_GetTicks64() >= (prevTicks + SCREEN_TICKS_PER_FRAME + epsillon / 1000);
			}
			return SDL_GetTicks64() >= (prevTicks + SCREEN_TICKS_PER_FRAME);
		}
		else {
			return true;
		}
	}
	int renderFrame(bool drawTileGrid = false) {
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
								std::string innerdir = obj.valueGet<std::string>(namenKonvention.renderObject.imageLocation);
								if (TextureContainer.find(innerdir) == TextureContainer.end()) {
									loadTexture(obj);
									obj.calculateDstRect();
									obj.calculateSrcRect();
								}

								rect = obj.getDstRect();
								rect.x -= Xpos;	//subtract camera posX
								rect.y -= Ypos;	//subtract camera posY

								// Render the texture to the window with scaling
								SDL_RenderCopy(renderer, TextureContainer[innerdir], obj.getSrcRect(), &rect);
							}
						}
					}
				}
			}
		}
		
		return 0;
	}
	void renderFPS() {
		// Create a string with the FPS value
		std::string fpsText = "FPS: " + std::to_string(fps);

		// Define the destination rectangle for rendering the text
		SDL_Rect textRect = { 10, 10, 0, 0 }; // Adjust position as needed
		textRect.w = fontSize * fpsText.length(); // Width based on text length
		textRect.h = (int)(fontSize * 1.5);

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
	void showFrame() {
		SDL_RenderPresent(renderer);
	}
	int handleEvent() {
		//------------------------------------------------
		// Event Return

		SDL_Event event;
		while (SDL_PollEvent(&event)) {
			switch (event.type) {
			case SDL_QUIT:
				return SDL_QUIT;
				break;
				// Handle other events as needed
			}
		}
		return 0;
	}

	SDL_Event getEventHandle() {
		SDL_Event event;
		SDL_PollEvent(&event);
		return event;
	}

	//-----------------------------------------------------------
	// Setting
	void setFPS(int fps) {
		if (fps > 0) {
			control_fps = true;
			SCREEN_FPS = fps;
			SCREEN_TICKS_PER_FRAME = 1000 / SCREEN_FPS;
		}
		else {
			control_fps = false;
		}
	}

	void setThreadSize(unsigned int size) {
		THREADSIZE = size;
	}

	//-----------------------------------------------------------
	// Getting

	int getEps() {
		return epsillon;
	}

	size_t getTextureAmount() {
		return TextureContainer.size();
	}

	size_t getObjectCount() {
		return env.getObjectCount();
	}

	int getResX() {
		return dispResX;
	}
	int getResY() {
		return dispResY;
	}
	int getThreadSize() {
		return THREADSIZE;
	}

	int getFPS() {
		return fps;
	}

	class SDL {
	public:
		const static int KEY_ESC = 27;
		const static int KEY_SPACE = 32;
		const static int KEY_ENTER = 13;

		const static int KEY_Q = 113;
		const static int KEY_W = 119;
		const static int KEY_E = 101;
		const static int KEY_R = 114;
		const static int KEY_T = 116;
		const static int KEY_Z = 122;
		const static int KEY_U = 117;
		const static int KEY_I = 105;
		const static int KEY_O = 111;
		const static int KEY_P = 112;
		const static int KEY_� = 252;

		const static int KEY_A = 97;
		const static int KEY_S = 115;
		const static int KEY_D = 100;
		const static int KEY_F = 102;
		const static int KEY_G = 103;
		const static int KEY_H = 104;
		const static int KEY_J = 106;
		const static int KEY_K = 107;
		const static int KEY_L = 108;
		const static int KEY_� = 246;
		const static int KEY_� = 228;

		const static int KEY_Y = 121;
		const static int KEY_X = 120;
		const static int KEY_C = 99;
		const static int KEY_V = 118;
		const static int KEY_B = 98;
		const static int KEY_N = 110;
		const static int KEY_M = 109;
		
		

	};

private:
	//-------------------------------------------------------------------------------------
	//General Variables

	Environment env;

	//tileY		tileX		#			Batches		of Objects
	//std::vector<std::vector<std::vector<std::vector<RenderObject>>>> ObjectContainer;
	unsigned int THREADSIZE = 2;

	

	Options generalOptions;
	std::string directory;

	int16_t Xpos;
	int16_t Ypos;
	unsigned int tileXpos;
	unsigned int tileYpos;
	unsigned int dispResX;
	unsigned int dispResY;

	SDL_Event event;
	SDL_Window* window;
	SDL_Renderer* renderer;
	SDL_Rect rect;
	TTF_Font* font;

	//-------------------------------------------------------------------------------------
	//For FPS Count

	// Define font properties
	int fontSize = 16; // Adjust as needed
	SDL_Color textColor = { 255, 255, 255, 255 }; // White color

	bool control_fps = false;
	int SCREEN_FPS = 500; // Target framerate (e.g., 60 FPS)
	int SCREEN_TICKS_PER_FRAME = 1000 / SCREEN_FPS; // Milliseconds per frame
	Uint64 prevTicks = SDL_GetTicks64();
	Uint64 lastFPSRender = SDL_GetTicks64();
	Uint64 totalframes = 0;
	int fpsCount = 0;
	int fps = 0;

	//Extra delay in microseconds
	//Start value based on experience?
	int64_t epsillon = 0;
	int kp = 2;
	int ki = 10;
	int kd = 1;
	int64_t integral = 0;
	int64_t prevError = 0;

	//-------------------------------------------------------------------------------------
	//Other

	// Function to load texture from file
	std::map<std::string, SDL_Texture*> TextureContainer;
	void loadTexture(RenderObject& toAppend) {
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
};
