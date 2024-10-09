▗▖  ▗▖▗▄▄▄▖▗▄▄▖ ▗▖ ▗▖▗▖   ▗▄▄▄▖▗▄▄▄▖▗▄▄▄▖    ▗▄▄▄▖▗▖  ▗▖ ▗▄▄▖▗▄▄▄▖▗▖  ▗▖▗▄▄▄▖
▐▛▚▖▐▌▐▌   ▐▌ ▐▌▐▌ ▐▌▐▌     █    █  ▐▌       ▐▌   ▐▛▚▖▐▌▐▌     █  ▐▛▚▖▐▌▐▌
▐▌ ▝▜▌▐▛▀▀▘▐▛▀▚▖▐▌ ▐▌▐▌     █    █  ▐▛▀▀▘    ▐▛▀▀▘▐▌ ▝▜▌▐▌▝▜▌  █  ▐▌ ▝▜▌▐▛▀▀▘
▐▌  ▐▌▐▙▄▄▖▐▙▄▞▘▝▚▄▞▘▐▙▄▄▖▗▄█▄▖  █  ▐▙▄▄▖    ▐▙▄▄▖▐▌  ▐▌▝▚▄▞▘▗▄█▄▖▐▌  ▐▌▐▙▄▄▖

▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄

by lbastigk



A C++ based game engine for the 'Das Schwarze Auge' (The Dark Eye) role-playing game system. This engine is built using SDL for rendering, and uses RapidJSON to handle the structuring of game data such as RenderObjects, Levels, and their respective attributes.



This repository contains 3 distinct projects that work together to support the full game development cycle:
|
| Main Engine (ProjectEngine)
| |
| | The main engine provides the core functionality of the game, handling:
| |
| | - Rendering with SDL.
| | - Game logic and entity management.
| | - Level loading and object management using RapidJSON for structured data.
| -----------------------------------------------------------------------------------------------------------------
|
|
| Editor (ProjectEditor)
| |
| | The Editor is designed for developers to create and manage game assets and levels:
| |
| | - Allows users to define RenderObjects, configure their attributes,
| | - Create Environments
| -----------------------------------------------------------------------------------------------------------------
|
|
| Test Environment (ProjectTestEnvironment)
| |
| | A dedicated testing environment where developers can:
| |
| | - Run automated and manual tests on the core engine.
| | - Validate performance, rendering, and game logic under various scenarios.
| | - Experiment with new features before merging them into the main engine.
| |
| | Current implementation for various classes in a 2 level Menue,
| | select the class first and then the specific test you wish to perform.
| -----------------------------------------------------------------------------------------------------------------
-------------------------------------------------------------------------------------------------------------------


| Dependencies
|
| The DSA Game Engine relies on the following libraries:
|
| - SDL2 (Simple DirectMedia Layer) for rendering and input handling.
| - SDL_ttf for fonts
| - SDL_image for handling more image formats
| - RapidJSON for parsing and structuring game data in JSON format.
|
| [Todo: Add install shell script]
-------------------------------------------------------------------------------------------------------------------

