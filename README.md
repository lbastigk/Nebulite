# Nebulite Game Engine
Created by **lbastigk**.

## Overview

**Nebulite** is a C++ based game engine for the _"Das Schwarze Auge"_ (The Dark Eye) role-playing game system. This engine is built using **SDL** for rendering and **RapidJSON** to handle the structuring of game data such as `RenderObjects`, `Levels`, and their respective attributes.

## Projects

This repository contains 3 distinct projects that work together to support the full game development cycle:

### Main Engine (ProjectEngine)

The main engine provides the core functionality of the game, handling:

- Rendering with **SDL**.
- Game logic and entity management.
- Level loading and object management using **RapidJSON** for structured data.

---

### Editor (ProjectEditor)

The Editor is designed for developers to create and manage game assets and levels:

- Allows users to define `RenderObjects`, configure their attributes.
- Create environments.

---

### Test Environment (ProjectTestEnvironment)

A dedicated testing environment where developers can:

- Run automated and manual tests on the core engine.
- Validate performance, rendering, and game logic under various scenarios.
- Experiment with new features before merging them into the main engine.

Current implementation includes a 2-level menu:
- Select the class first and then the specific test you wish to perform.

---

## Dependencies

The DSA Game Engine relies on the following libraries:

- **SDL2** (Simple DirectMedia Layer) for rendering and input handling.
- **SDL_ttf** for fonts.
- **SDL_image** for handling more image formats.
- **RapidJSON** for parsing and structuring game data in JSON format.



