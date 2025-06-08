# Nebulite Game Engine
[![Author](https://img.shields.io/badge/Author-lbastigk-blue)](https://github.com/lbastigk)

## Overview

**Nebulite** is a C++-based 2D game engine built for arbitrary rulesets and flexible inter-object logic.
In Nebulite, `RenderObjects` can interact with each other through a self-other relationship using the `Invoke` class, 
provided logical conditions between the two are satisfied. Global attributes are accessed and modified via the `global` tag.

This non-hierarchical architecture enables complex interactive systems and modular subsystems. Examples:
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

| Library                                               | Purpose                                      |
|-------------------------------------------------------|----------------------------------------------|
| [SDL2](https://www.libsdl.org/)                       | Rendering and input handling                 |
| [SDL_ttf](https://github.com/libsdl-org/SDL_ttf)      | Font rendering                               |
| [SDL_image](https://github.com/libsdl-org/SDL_image)  | Extended image format support                |
| [RapidJSON](https://github.com/Tencent/rapidjson)     | JSON parsing for game data                   |
| [Abseil](https://abseil.io/)                          | Fast variable caching                        |
| [Tinyexpr](https://github.com/codeplea/tinyexpr)      | Parsing and evaluating expressions           |


