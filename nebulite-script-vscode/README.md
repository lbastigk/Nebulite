# Nebulite Script Language Extension

This VS Code extension provides syntax highlighting for Nebulite Script files (`.nebs`).

## Features
- Highlights Nebulite commands, expressions, variables, numbers, and strings
- Supports `.nebs` file extension

## Usage
1. Open this folder in VS Code.
2. Press `F5` to launch a new Extension Development Host window.
3. Open a `.nebs` file to see Nebulite Script highlighting.

## Customization
- Edit `syntaxes/nebulite-script.tmLanguage.json` to improve or extend highlighting rules.
- Edit `language-configuration.json` for brackets, comments, and auto-closing pairs.

## Example Nebulite Script
```nebs
# Gravity benchmark
set-fps 60
spawn Resources/Renderobjects/standard.json
wait 1
snapshot
```
