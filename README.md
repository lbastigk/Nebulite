# Nebulite Game Engine
Created by **lbastigk**.

## Overview

**Nebulite** is a C++ based game engine for the _"Das Schwarze Auge"_ (The Dark Eye) role-playing game system. This engine is built using **SDL** for rendering and **RapidJSON** to handle the structuring of game data such as `RenderObjects`, `Levels`, and their respective attributes.

## Projects

Source files are split into 3 distinct directories to support the full game development cycle:

### Main Engine (ProjectEngine)

The main engine provides the core functionality of the game, handling:

- Rendering with **SDL**.
- Game logic and entity management.
- Level loading and object management using **RapidJSON** for structured data.

---

### Editor

The Editor is designed for developers to create and manage game assets and levels:

- Allows users to define `RenderObjects`, configure their attributes.
- Create environments.

---

### Test Environment

- Run automated and manual tests on the core engine.
- Validate performance, rendering, and game logic under various scenarios.
- Experiment with new features before merging them into the main engine.

Current implementation is done inside the main Engine binary through argument passing, e.g.:
`Nebulite test renderer circle-benchmark`

---

## Dependencies

Nebulite relies on the following libraries:

- **SDL2** (Simple DirectMedia Layer) for rendering and input handling.
- **SDL_ttf** for fonts.
- **SDL_image** for handling more image formats.
- **RapidJSON** for parsing and structuring game data in JSON format.
- **QT5** for editor GUI.



