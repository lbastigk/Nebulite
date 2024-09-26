
/*
#include <SDL.h>
#include <SDL_ttf.h>

//Libraries_Self
#include "Optionsloader.cpp"
#include "Environment.cpp"
#include "FileManagement.cpp"

#include "VAE/Functions_readwrite_standalone.h"

#define WINDOWNAME "coolgame"

Options createGeneralOptions(std::string filename) {
	std::map <std::string, std::string> initialOptions;

	initialOptions["dispResX"] = "1920";
	initialOptions["dispResY"] = "1080";
	initialOptions["windowName"] = "coolgame";

	Options generalOptions(filename, initialOptions);
	return generalOptions;
}

int sdlRender(std::string directory, SDL_Renderer* renderer, SDL_Window* window,std::vector <ObjectInstance> ObjectInstances) {
	// Clear the window with a specified color (e.g., black)
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255); // RGB values (black)
	SDL_RenderClear(renderer);

	//Render Objects
	for (const auto& Object : ObjectInstances) {
		std::string path = FileManagement::CombinePaths(directory, Object.RenderObject.imageLocation);

		// Load the image
		SDL_Surface* surface = SDL_LoadBMP(path.c_str());
		if (!surface) {
			// Handle image loading error
			SDL_DestroyRenderer(renderer);
			SDL_DestroyWindow(window);
			SDL_Quit();
			std::cout << "Image load error" << "\n\n";
			return 1;
		}

		// Create a texture from the loaded surface
		SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
		SDL_FreeSurface(surface); // Free the surface as it's no longer needed

		if (!texture) {
			// Handle texture creation error
			SDL_DestroyRenderer(renderer);
			SDL_DestroyWindow(window);
			SDL_Quit();
			std::cout << "Texture creation error" << "\n\n";
			return 1;
		}

		// Calculate the destination rectangle based on pixelSizeX and pixelSizeY
		SDL_Rect dstRect = {
			Object.RenderObject.positionX,
			Object.RenderObject.positionY,
			Object.RenderObject.pixelSizeX, // Set the desired width
			Object.RenderObject.pixelSizeY, // Set the desired height
		};

		// Render the texture to the window with scaling
		SDL_RenderCopy(renderer, texture, nullptr, &dstRect);
	}
}

// Function to render an FPS counter
void sdlRenderFPS(SDL_Renderer* renderer, int frameCount, TTF_Font* font) {
	// Define font properties
	int fontSize = 16; // Adjust as needed
	SDL_Color textColor = { 255, 255, 255, 255 }; // White color

	// Create a string with the FPS value
	std::string fpsText = "FPS: " + std::to_string(frameCount);

	// Define the destination rectangle for rendering the text
	SDL_Rect textRect = { 10, 10, 0, 0 }; // Adjust position as needed
	textRect.w = fontSize * fpsText.length(); // Width based on text length
	textRect.h = fontSize * 1.5;

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

void sdlRenderText(SDL_Renderer* renderer, std::string text, TTF_Font* font, int fontSize, int posX, int posY) {
	SDL_Color textColor = { 255, 255, 255, 255 }; // White color

	// Define the destination rectangle for rendering the text
	SDL_Rect textRect = { posX, posY, 0, 0 }; // Adjust position as needed
	textRect.w = fontSize * text.length(); // Width based on text length
	textRect.h = fontSize * 1.5;	//Scalar needed!!

	// Clear the area where the FPS text will be rendered
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255); // Set background color (black)
	SDL_RenderFillRect(renderer, &textRect);

	// Create a surface with the text
	SDL_Surface* textSurface = TTF_RenderText_Solid(font, text.c_str(), textColor);

	// Create a texture from the text surface
	SDL_Texture* textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);

	// Render the text texture
	SDL_RenderCopy(renderer, textTexture, NULL, &textRect);

	// Free the text surface and texture
	SDL_FreeSurface(textSurface);
	SDL_DestroyTexture(textTexture);
}

int mainn(int argc, char* argv[]) {
	// For testing purposes...
	Environment Level1;
	ObjectInstance Object1;
	Object1.RenderObject.positionX = 400;
	Object1.RenderObject.positionY = 400;
	Object1.RenderObject.pixelSizeX = 100;
	Object1.RenderObject.pixelSizeY = 100;
	Level1.ObjectInstances.push_back(Object1);

	Kreatur tee;
	//----------------------------------------------------

	// Load Options
	Options generalOptions = createGeneralOptions("options.txt");

	// Check for Ressources Folder
	if (!FileManagement::folderExists("Resources")) {
		std::cout << "Warning: Directory Resources not found!" << "\n\n";
	}

	// Get the current directory as a std::string
	std::string currentDir = FileManagement::currentDir();

	//----------------------------------------------------
	//SDL STUFF

	//Create SDL window
	if (SDL_Init(SDL_INIT_VIDEO) != 0) {
		// SDL initialization failed
		std::cerr << "SDL_Init Error: " << SDL_GetError() << std::endl;
		return 1;
	}
	SDL_Window* window = SDL_CreateWindow(
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
		return 1;
	}

	// Initialize SDL_ttf
	if (TTF_Init() < 0) {
		// Handle SDL_ttf initialization error
		SDL_Quit(); // Clean up SDL
		return 1;
	}

	//Load font
	std::string fontpath = FileManagement::CombinePaths(currentDir, "Resources\\Fonts\\Arimo-Regular.ttf");
	//std::cout << fontpath;
	TTF_Font* font = TTF_OpenFont(fontpath.c_str(), 60); // Adjust size as needed
	if (font == NULL) {
		// Handle font loading error
		std::cout << "\nCould not load Font: " << TTF_GetError() << "\n\n";
		return 1;
	}

	//General Variables
	bool quit = false;
	SDL_Event event;
	const int SCREEN_FPS = 60; // Target framerate (e.g., 60 FPS)
	const int SCREEN_TICKS_PER_FRAME = 1000 / SCREEN_FPS; // Milliseconds per frame
	Uint32 prevTicks = SDL_GetTicks();

	// Create a renderer
	SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
	if (!renderer) {
		printf("Renderer creation failed: %s\n", SDL_GetError());
		return 1;
	}

	int fpsCount = 0;
	int fps = 0;
	Uint32 lastFPSRender = SDL_GetTicks();


	while (!quit) {
		//Poll Keys
		while (SDL_PollEvent(&event)) {
			if (event.type == SDL_QUIT) {
				std::cout << "SDL_QUIT" << "\n\n";
				quit = true; // Exit the main loop when the window is closed
			}
			else if (event.type == SDL_KEYDOWN) {
				// Check for multiple button presses
				const Uint8* state = SDL_GetKeyboardState(NULL);

				//todo

			}
		}

		//Tick calc
		Uint32 currentTicks = SDL_GetTicks();
		Uint32 frameTicks = currentTicks - prevTicks;

		//Render Current instances
		sdlRender(currentDir,renderer,window,Level1.ObjectInstances);

		//Render fps
		if (currentTicks - lastFPSRender >= 1000) {
			fps = fpsCount;
			fpsCount = 0;
			lastFPSRender = currentTicks;
		}
		sdlRenderFPS(renderer, fps, font);

		//Test Render
		sdlRenderText(renderer, "Test", font, 26, 100, 100);

		// Delay to limit the framerate
		if (frameTicks < SCREEN_TICKS_PER_FRAME) {
			SDL_Delay(SCREEN_TICKS_PER_FRAME - frameTicks);
		}

		// Present the renderer
		SDL_RenderPresent(renderer);

		//Ticks and FPS
		fpsCount++;
		prevTicks = currentTicks;
	}


	//End of Program!
	SDL_DestroyWindow(window);
	SDL_Quit();

	return 0;
}
*/

