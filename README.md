# Nebulite Game Engine
Created by **lbastigk**.

## Overview

**Nebulite** is a C++ based game engine for the _"Das Schwarze Auge"_ (The Dark Eye) role-playing game system. This engine is built using **SDL** for rendering and **RapidJSON** to handle the structuring of game data such as `RenderObjects`, `Levels`, and their respective attributes.

The main engine provides the core functionality of the game, handling:

- Rendering with **SDL**.
- Game logic and entity management.
- Level loading and object management using **RapidJSON** for structured data.
- Environments, Renderobjects etc. can be loaded with tasks. See `./Application/TaskFiles/` for examples.

---


## Dependencies

Nebulite relies on the following libraries:

- **SDL2** (Simple DirectMedia Layer) for rendering and input handling.
- **SDL_ttf** for fonts.
- **SDL_image** for handling more image formats.
- **RapidJSON** for parsing and structuring game data in JSON format.
- **Tinyexpr** for expressions



