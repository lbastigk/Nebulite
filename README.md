# Nebulite Game Engine
Created by **lbastigk**.

## Overview

**Nebulite** is a C++ based 2D game engine for arbitrary rulesets. Renderobjects within the system are allowed to interact with each other in a self-other relationship defined by the `Invoke` class, 
if logical conditions between two objects is true. Global attributes can be read and written via the `global` tag. 
This non-hierarchial structure allows for complex interactive systems and an easy breakdown into subsystems such as:
- movement being triggered by $(global.input.keyboard.w) -> sets velocity or request to move a distane
- animation being triggered by attributes: `$(self.isMoving)` or `$(self.physics.vX > 0)` -> set posX to `$($(self.posX) + $(self.physics.vX) * $(global.time.dt))`
- boundary check being triggered by $(other.isSolid) -> forces velocity of self to 0

This engine is built using **SDL** for rendering and **RapidJSON** as well as a custom caching system to handle the fast structuring of game data such as `RenderObjects`, `Levels`, and their respective attributes.

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
- **RapidJSON** for loading and storing game data in JSON format.
- **abseil** for fast caching of variables
- **Tinyexpr** for expressions




