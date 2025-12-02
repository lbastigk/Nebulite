# Nebulite Command Documentation

This documentation is automatically generated from the Nebulite executable.

Generated on: Tue Dec  2 18:50:40 CET 2025

## Table of Contents

- [GlobalSpace Commands](#globalspace-commands)
- [RenderObject Commands](#renderobject-commands)

## GlobalSpace Commands

These commands are available in the global Nebulite namespace.

Help for Nebulite

Available Functions

| Function | Description |
|----------|-------------|
| `add-clock` | Adds a clock with specified interval (ms) to the global clock system |
| `always` | Attach a command to the always-taskqueue that is executed on each tick. |
| `always-clear` | Clears the entire always-taskqueue. |
| `assert` | Asserts a condition and throws a custom error if false. |
| `beep` | Make a beep noise. |
| `cam` | Renderer Camera Functions |
| `clear` | Clears the console screen. |
| `console` | Console commands and settings. |
| `copy` | Copy data from one key to another. |
| `crash` | Crashes the program, useful for checking if the testing suite can catch crashes. |
| `critical` | Returns a critical error. |
| `debug` | DomainModule for special debugging capabilities within the GlobalSpace. |
| `draft` | Functions to manipulate and spawn RenderObjects in draft state |
| `echo` | Echoes all arguments as string to the standard output. |
| `ensure-array` | Ensure that a key is an array, converting a value to an array if necessary. |
| `env` | Environment management functions |
| `error` | Echoes all arguments as string to the standard error. |
| `errorlog` | Activates or deactivates error logging to a file. |
| `eval` | Evaluates an expression string and executes it. |
| `exit` | Exits the entire program. |
| `feature-test` | Functions for testing features in the GlobalSpace |
| `for` | Executes a for-loop with a function call. |
| `help` | Show available commands and their descriptions |
| `if` | Executes a block of code if a condition is true. |
| `input-wait` | Waits for user input before continuing. |
| `json` | Functions to manipulate JSON data via read-only JSON documents |
| `keyDelete` | Delete a key from the JSON document. |
| `log` | Functions to log various data to files |
| `move` | Move data from one key to another. |
| `pop-back` | Pop a value from the back of an array. |
| `pop-front` | Pop a value from the front of an array. |
| `print` | Prints the JSON document to the console for debugging purposes. |
| `push-back` | Push a value to the back of an array. |
| `push-front` | Push a value to the front of an array. |
| `query` | Functions to manipulate JSON data via SQL query results |
| `return` | Returns a custom value as a Critical Error. |
| `selected-object` | Functions to select and interact with a selected RenderObject |
| `set` | Set a key to a value in the JSON document. |
| `set-fps` | Set FPS of renderer. |
| `set-res` | Set resolution of renderer. |
| `show-fps` | Show FPS of renderer. |
| `snapshot` | Create a snapshot of the current renderer state. |
| `spawn` | Spawn a RenderObject from a json/jsonc file. |
| `standardfile` | Functions to generate standard files |
| `task` | Loads tasks from a file into the taskQueue. |
| `time` | Commands for time management |
| `wait` | Sets the waitCounter to the given value to halt all script tasks for a given amount of frames. |
| `warn` | Returns a warning: a custom, noncritical error. |

Available Variables

| Variable | Description |
|----------|-------------|
| `--headless` | Set headless mode (no renderer) |
| `--recover` | Enable recoverable error mode |

#### `add-clock`

```
Adds a clock with specified interval (ms) to the global clock system
```

#### `always`

```
Attach a command to the always-taskqueue that is executed on each tick.

Usage: always <command>

Example:
always echo This command runs every frame!
This will output "This command runs every frame!" on every frame.
```

#### `always-clear`

```
Clears the entire always-taskqueue.

Usage: always-clear

Example:
always-clear
This will remove all commands from the always-taskqueue.
```

#### `assert`

```
Asserts a condition and throws a custom error if false.

Usage: assert <condition>

It is recommended to wrap the condition in quotes to prevent parsing issues.

Example:
assert '$(eq(1+1,2))'    // No error
assert '$(eq(1+1,3))'    // Critical Error: A custom assertion failed.
Assertion failed: $(eq(1+1,3)) is not true.
```

#### `beep`

```
Make a beep noise.

Usage: beep
```

#### `cam`

Available Functions

| Function | Description |
|----------|-------------|
| `help` | Show available commands and their descriptions |
| `move` | Move camera by a given delta. |
| `set` | Set camera to concrete position. |

##### `cam move`

```
Move camera by a given delta.

Usage: cam move <dx> <dy>

<dx> : Delta x to move camera by
<dy> : Delta y to move camera by
```

##### `cam set`

```
Set camera to concrete position.

Usage: cam set <x> <y> [c]

<x> : X position to set camera to
<y> : Y position to set camera to
[c] : Optional. If provided, sets the camera's center to the given position.
```

#### `clear`

```
Clears the console screen.

Usage: clear

Note: This function attempts to clear the console screen using system-specific commands.
        It may not work in all environments or IDEs.
```

#### `console`

Available Functions

| Function | Description |
|----------|-------------|
| `help` | Show available commands and their descriptions |
| `set-background` | Sets a background image for the console. |
| `zoom` | Reduces or increases the console font size. |

##### `console set-background`

```
Sets a background image for the console.

Usage: set-background <image_path>
```

##### `console zoom`

```
Reduces or increases the console font size.

Usage: zoom [in/out/+/-]
- in  / + : Zooms in  (increases font size)
- out / - : Zooms out (decreases font size)
```

#### `copy`

```
Copy data from one key to another.

Usage: copy <source_key> <destination_key>
```

#### `crash`

```
Crashes the program, useful for checking if the testing suite can catch crashes.

Usage: crash [<type>]

- <type>: Optional. The type of crash to induce. Options are:
    - segfault   : Causes a segmentation fault (default)
    - abort      : Calls std::abort()
    - terminate  : Calls std::terminate()
    - throw      : Throws an uncaught exception
```

#### `critical`

```
Returns a critical error.

Usage: critical <string>

- <string>: The critical error message.
```

#### `debug`

Available Functions

| Function | Description |
|----------|-------------|
| `collision-detect` | Utilities for testing collision detection functionalities. |
| `help` | Show available commands and their descriptions |

##### `debug collision-detect`

Available Functions

| Function | Description |
|----------|-------------|
| `category` | Tests collision detection of categories |
| `function` | Tests collision detection of functions names |
| `help` | Show available commands and their descriptions |
| `variable` | Tests collision detection of variable names |

###### `debug collision-detect category`

```
Tests collision detection of categories

Tries to bind a category that is already registered in globalspace, expecting a collision error.

Usage: debug collision-detect category [fail/succeed]

Defaults to fail
```

###### `debug collision-detect function`

```
Tests collision detection of functions names

Tries to bind a function name to globalspace that is already registered, expecting a collision error.

Usage: debug collision-detect function [fail/succeed]

Defaults to fail
```

###### `debug collision-detect variable`

```
Tests collision detection of variable names

Tries to bind a variable name to globalspace that is already registered, expecting a collision error.

Usage: debug collision-detect variable [fail/succeed]

Defaults to fail
```

#### `echo`

```
Echoes all arguments as string to the standard output.

Usage: echo <string>

This command concatenates all arguments with a whitespace and outputs them to the standard output (cout).
Example:
./bin/Nebulite echo Hello World!
Outputs:
Hello World!
```

#### `ensure-array`

```
Ensure that a key is an array, converting a value to an array if necessary.

Usage: ensure-array <key>
```

#### `env`

Available Functions

| Function | Description |
|----------|-------------|
| `deload` | Deload entire environment, leaving an empty renderer. |
| `help` | Show available commands and their descriptions |
| `load` | Load an environment/level from a json/jsonc file. |

##### `env deload`

```
Deload entire environment, leaving an empty renderer.

Usage: env deload
```

##### `env load`

```
Load an environment/level from a json/jsonc file.

Usage: env load <path/to/file.jsonc>

If no argument is provided, an empty environment is loaded.
```

#### `error`

```
Echoes all arguments as string to the standard error.

Usage: error <string...>

- <string...>: One or more strings to echo to the standard error.
```

#### `errorlog`

```
Activates or deactivates error logging to a file.

Usage: errorlog <on/off>

- on:  Activates error logging to 'error.log' in the working directory.
- off: Deactivates error logging, reverting to standard error output.
Note: Ensure you have write permissions in the working directory when activating error logging.
```

#### `eval`

```
Evaluates an expression string and executes it.
Every argument after eval is concatenated with a whitespace to form the expression to be evaluated and then reparsed.

Usage: eval <expression>

Examples:

eval echo $(1+1)
outputs: 2.000000
First, eval evaluates every argument, then concatenates them with a whitespace,
and finally executes the resulting string as a command.
The string 'echo $(1+1)' is evaluated to "echo 2.000000", which is then executed.

eval spawn ./Resources/RenderObjects/{global.ToSpawn}.json
This evaluates to 'spawn ./Resources/RenderObjects/NAME.json',
where NAME is the current value of the global variable ToSpawn
```

#### `exit`

```
Exits the entire program.

Usage: exit

Closes the program with exit code 0 (no error)
Any queued tasks will be discarded.
```

#### `feature-test`

Available Functions

| Function | Description |
|----------|-------------|
| `functree` | Builds a funcTree with extra arguments and tests it |
| `help` | Show available commands and their descriptions |

##### `feature-test functree`

```
Builds a funcTree with extra arguments and tests it
```

#### `for`

```
Executes a for-loop with a function call.

Usage: for <var> <start> <end> <functioncall>

Example:
for i 1 5 echo Iteration {i}
This will output:
    Iteration 1
    Iteration 2
    Iteration 3
    Iteration 4
    Iteration 5

This is useful for:
- Repeating actions a specific number of times.
- Iterating over a range of values.
- Creating complex control flows in scripts.
```

#### `if`

```
Executes a block of code if a condition is true.

Usage: if <condition> <functioncall>

It is recommended to wrap the condition in quotes to prevent parsing issues.

However, This is not supported for in-console usage.
This is because the console itself removes quotes before passing the arguments to the FuncTree,
rendering them useless.

Example:
if '$(eq(1+1,2))' echo Condition is true!
```

#### `input-wait`

```
Waits for user input before continuing.

Usage: input-wait [prompt]

Note: This function pauses execution until the user presses Enter.
```

#### `json`

Available Functions

| Function | Description |
|----------|-------------|
| `help` | Show available commands and their descriptions |
| `set` | Sets a key from a JSON document. |

##### `json set`

```
Sets a key from a JSON document.

Usage: json set <key_to_set> <link:key>

Where <link:key> is a link to a JSON document.
The document is dynamically loaded and cached for future use.
```

#### `keyDelete`

```
Delete a key from the JSON document.

Usage: keyDelete <key>
```

#### `log`

Available Functions

| Function | Description |
|----------|-------------|
| `global` | Logs the global document to a file. |
| `help` | Show available commands and their descriptions |
| `state` | Logs the current state of the renderer to a file. |

##### `log global`

```
Logs the global document to a file.

Usage: log global [<filenames>...]

- <filenames>: Optional. One or more filenames to log the global document to.
                If no filenames are provided, defaults to 'global.log.jsonc'.
```

##### `log state`

```
Logs the current state of the renderer to a file.

Usage: log state [<filenames>...]

- <filenames>: Optional. One or more filenames to log the renderer state to.
                If no filenames are provided, defaults to 'state.log.jsonc'.
```

#### `move`

```
Move data from one key to another.

Usage: move <source_key> <destination_key>
```

#### `pop-back`

```
Pop a value from the back of an array.

Usage: pop-back <key>
```

#### `pop-front`

```
Pop a value from the front of an array.

Usage: pop-front <key>
```

#### `print`

```
Prints the JSON document to the console for debugging purposes.
If key is empty, prints the entire document.

Usage: print [key]
```

#### `push-back`

```
Push a value to the back of an array.

Usage: push-back <key> <value>
```

#### `push-front`

```
Push a value to the front of an array.

Usage: push-front <key> <value>
```

#### `query`

Available Functions

| Function | Description |
|----------|-------------|
| `help` | Show available commands and their descriptions |
| `set` | Sets a key from a SQL query result. |

##### `query set`

```
Sets a key from a SQL query result.
Not implemented yet.
```

#### `return`

```
Returns a custom value as a Critical Error.

Usage: return <string>

This command creates a custom critical error with the given string as description.
This can be used to exit from a task queue with a custom message.

Example:

./bin/Nebulite return We did not anticipate this happening, weird.
Outputs:
We did not anticipate this happening, weird.
Critical Error: We did not anticipate this happening, weird.
```

#### `selected-object`

Available Functions

| Function | Description |
|----------|-------------|
| `get` | Get a renderobject by its ID. |
| `help` | Show available commands and their descriptions |
| `parse` | Parse a command on the selected RenderObject. |

##### `selected-object get`

```
Get a renderobject by its ID.

Usage: selected-object get <id>
```

##### `selected-object parse`

```
Parse a command on the selected RenderObject.

Usage: selected-object parse <command>

Use 'selected-object get <id>' to select a RenderObject first.
Use 'selected-object parse help' to see available commands for the selected object.
```

#### `set`

```
Set a key to a value in the JSON document.

Usage: set <key> <value>

Note: All values are stored as strings.
```

#### `set-fps`

```
Set FPS of renderer.

Usage: set-fps [fps]

Defaults to 60 fps if no argument is provided
```

#### `set-res`

```
Set resolution of renderer.

Usage: set-res [Width] [Height] [Scale]

Defaults to 1000  for width if argument count < 1
Defaults to 1000  for height if argument count < 2
Defaults to 1     for scale if argument count < 3
```

#### `show-fps`

```
Show FPS of renderer.

Usage: show-fps [on|off]

Defaults to on if no argument is provided
```

#### `snapshot`

```
Create a snapshot of the current renderer state.

Usage: snapshot [filename]

Defaults to "./Resources/Snapshots/snapshot.png" if no argument is provided
```

#### `spawn`

```
Spawn a RenderObject from a json/jsonc file.

Usage: spawn <path/to/file.jsonc>

Supports lookups in standard resource directories:
- ./Resources/RenderObjects/
- ./Resources/Renderobjects/

Example: 'spawn Planets/sun.jsonc|set text.str This is a sun'
Looks for object 'sun.jsonc' in the standard directories
- './Planets/sun.jsonc'
- './Resources/RenderObjects/Planets/sun.jsonc'
- './Resources/Renderobjects/Planets/sun.jsonc'
and spawns the first found object.
```

#### `standardfile`

Available Functions

| Function | Description |
|----------|-------------|
| `help` | Show available commands and their descriptions |
| `renderobject` | Logs a standard render object to a file: ./Resources/Renderobjects/standard.jsonc. |

##### `standardfile renderobject`

```
Logs a standard render object to a file: ./Resources/Renderobjects/standard.jsonc.

Usage: standardfile renderobject

Note: This function creates or overwrites the file 'standard.jsonc' in the './Resources/Renderobjects/' directory.
```

#### `task`

```
Loads tasks from a file into the taskQueue.

Usage: task <filename>

This command loads a list of tasks from the specified file into the task queue.
Each line in the file is treated as a separate task.

Task files are not appended at the end, but right after the current task.
This ensures that tasks can be loaded within task files themselves and being executed immediately.

This example shows how tasks are loaded and executed:

Main task:
    mainCommand1
    mainCommand2
    task subTaskFile.txt:
        subCommand1
        subCommand2
    mainCommand4
```

#### `time`

Available Functions

| Function | Description |
|----------|-------------|
| `halt-once` | Halts time for one frame |
| `help` | Show available commands and their descriptions |
| `lock` | Locks time with lock provided,  |
| `master-unlock` | Removes all time locks. |
| `set-fixed-dt` | Sets a fixed delta time in milliseconds for the simulation time. |
| `unlock` | Removes a time lock. |

##### `time halt-once`

```
Halts time for one frame
Meaning you can halt time by continuously calling this function.

Usage: time halt-once
```

##### `time lock`

```
Locks time with lock provided, 
Time can only progress if no locks are present.

Usage: time lock <lock_name>

<lock_name> : Name of the lock to add. Any string without whitespace is valid.
```

##### `time master-unlock`

```
Removes all time locks.
Time can only progress if no locks are present.

Usage: time master-unlock
```

##### `time set-fixed-dt`

```
Sets a fixed delta time in milliseconds for the simulation time.
Use 0 to disable fixed dt.

Usage: time set-fixed-dt <dt_ms>
```

##### `time unlock`

```
Removes a time lock.
Time can only progress if no locks are present.

Usage: time unlock <lock_name>

<lock_name> : Name of the lock to remove. Must match an existing lock.
```

#### `wait`

```
Sets the waitCounter to the given value to halt all script tasks for a given amount of frames.

Usage: wait <frames>

This command pauses the execution of all script tasks for the specified number of frames.
This does not halt any tasks coming from objects within the environment and cannot be used by them.

The wait-command is intended for scripts only, allowing for timed delays between commands.

This is useful for:
- Creating pauses in scripts to wait for certain conditions to be met.
- Timing events in a sequence.
- Tool assisted speedruns (TAS)
```

#### `warn`

```
Returns a warning: a custom, noncritical error.

Usage: warn <string>

- <string>: The warning message.
```

## RenderObject Commands

These commands are available in the RenderObject domain (accessed via `draft parse`).

Help for RenderObject

Available Functions

| Function | Description |
|----------|-------------|
| `copy` | Copy data from one key to another. |
| `debug` | Debugging functions for RenderObject |
| `delete` | Marks object for deletion |
| `echo` | Echoes all arguments as string to the standard output. |
| `ensure-array` | Ensure that a key is an array, converting a value to an array if necessary. |
| `eval` | Evaluate an expression and execute the result.  |
| `fill` | Fill the texture with a color |
| `help` | Show available commands and their descriptions |
| `json` | Functions to manipulate JSON data via read-only JSON documents |
| `keyDelete` | Delete a key from the JSON document. |
| `log` | Logging utilities |
| `mirror` | Mirror utilities for RenderObject to GlobalSpace synchronization |
| `move` | Move data from one key to another. |
| `pop-back` | Pop a value from the back of an array. |
| `pop-front` | Pop a value from the front of an array. |
| `print` | Prints the JSON document to the console for debugging purposes. |
| `push-back` | Push a value to the back of an array. |
| `push-front` | Push a value to the front of an array. |
| `query` | Functions to manipulate JSON data via SQL query results |
| `reload-texture` | Reload the texture from the document. |
| `rotate` | Rotate the texture by a given angle |
| `set` | Set a key to a value in the JSON document. |
| `update-text` | Calculate text texture |

#### `copy`

```
Copy data from one key to another.

Usage: copy <source_key> <destination_key>
```

#### `debug`

Available Functions

| Function | Description |
|----------|-------------|
| `help` | Show available commands and their descriptions |
| `print-dst-rect` | Prints debug information about the destination rectangle to console |
| `print-src-rect` | Prints debug information about the source rectangle to console |
| `texture-status` | Prints debug information about the texture to console |

##### `debug print-dst-rect`

```
Prints debug information about the destination rectangle to console

Usage: debug print-dst-rect

Outputs:
Destination Rectangle: { x: ..., y: ..., w: ..., h: ... }
If the RenderObject is not a spritesheet, indicates that instead:
Destination rectangle is not set.
```

##### `debug print-src-rect`

```
Prints debug information about the source rectangle to console

Usage: debug print-src-rect

Outputs:
Source Rectangle: { x: ..., y: ..., w: ..., h: ... }
If the RenderObject is not a spritesheet, indicates that instead:
This RenderObject is not a spritesheet.
```

##### `debug texture-status`

```
Prints debug information about the texture to console

Usage: debug texture-status

Outputs various details about the texture, including:
 - Texture Key
 - Valid Texture
 - Local Texture
 - SDL Texture Info (Width, Height, Access, Format)
```

#### `delete`

```
Marks object for deletion

Usage: delete

Marks the object for deletion on the next update cycle.
```

#### `echo`

```
Echoes all arguments as string to the standard output.

Usage: echo <string>

This command concatenates all arguments with a whitespace and outputs them to the standard output (cout).
Example:
./bin/Nebulite echo Hello World!
Outputs:
Hello World!
```

#### `ensure-array`

```
Ensure that a key is an array, converting a value to an array if necessary.

Usage: ensure-array <key>
```

#### `eval`

```
Evaluate an expression and execute the result. 
Example: eval echo $(1+1)

Examples:
     
eval echo $(1+1)    outputs:    2.000000
eval spawn ./Resources/RenderObjects/{global.ToSpawn}.json
```

#### `fill`

```
Fill the texture with a color

Usage:
fill <color> 
fill [R] [G] [B]
- <color>: Predefined color name (e.g., "red", "green", "blue")
- [R] [G] [B]: RGB color values (0-255)
```

#### `json`

Available Functions

| Function | Description |
|----------|-------------|
| `help` | Show available commands and their descriptions |
| `set` | Sets a key from a JSON document. |

##### `json set`

```
Sets a key from a JSON document.

Usage: json set <key_to_set> <link:key>

Where <link:key> is a link to a JSON document.
The document is dynamically loaded and cached for future use.
```

#### `keyDelete`

```
Delete a key from the JSON document.

Usage: keyDelete <key>
```

#### `log`

Available Functions

| Function | Description |
|----------|-------------|
| `all` | Logs the entire RenderObject to a file. |
| `help` | Show available commands and their descriptions |
| `key` | Logs a specific value from the RenderObject to a file. |

##### `log all`

```
Logs the entire RenderObject to a file.

Usage: log [filename]

Logs to `RenderObject_id<id>.log.jsonc` if no filename is provided.
```

##### `log key`

```
Logs a specific value from the RenderObject to a file.

Usage: log key <key> [filename]

Logs the value associated with <key> to the specified [filename], 
or to `RenderObject_id<id>.log.jsonc` if no filename is provided.
```

#### `mirror`

Available Functions

| Function | Description |
|----------|-------------|
| `delete` | Deletes the GlobalSpace document entry for this RenderObject |
| `fetch` | Deserializes the RenderObject from the GlobalSpace document entry |
| `help` | Show available commands and their descriptions |
| `off` | Disables mirroring to the GlobalSpace document |
| `on` | Enables mirroring to the GlobalSpace document |
| `once` | Mirrors the object to the GlobalSpace document once on next update |

##### `mirror delete`

```
Deletes the GlobalSpace document entry for this RenderObject

Usage: mirror delete

Mirrors are removed from the GlobalSpace document under key "mirror.renderObject.id<id>
```

##### `mirror fetch`

```
Deserializes the RenderObject from the GlobalSpace document entry

Usage: mirror fetch

Mirrors are fetched from the GlobalSpace document under key "mirror.renderObject.id<id>
```

##### `mirror off`

```
Disables mirroring to the GlobalSpace document

Usage: mirror off

Constant mirroring is inactive until turned on again with 'mirror on'

Mirrors are stored in the GlobalSpace document under key "mirror.renderObject.id<id>
```

##### `mirror on`

```
Enables mirroring to the GlobalSpace document

Usage: mirror on

Constant mirroring is active until turned off with 'mirror off'

Mirrors are stored in the GlobalSpace document under key "mirror.renderObject.id<id>
```

##### `mirror once`

```
Mirrors the object to the GlobalSpace document once on next update

Usage: mirror once

Mirroring is only done for one frame.
Mirrors are stored in the GlobalSpace document under key "mirror.renderObject.id<id>
```

#### `move`

```
Move data from one key to another.

Usage: move <source_key> <destination_key>
```

#### `pop-back`

```
Pop a value from the back of an array.

Usage: pop-back <key>
```

#### `pop-front`

```
Pop a value from the front of an array.

Usage: pop-front <key>
```

#### `print`

```
Prints the JSON document to the console for debugging purposes.
If key is empty, prints the entire document.

Usage: print [key]
```

#### `push-back`

```
Push a value to the back of an array.

Usage: push-back <key> <value>
```

#### `push-front`

```
Push a value to the front of an array.

Usage: push-front <key> <value>
```

#### `query`

Available Functions

| Function | Description |
|----------|-------------|
| `help` | Show available commands and their descriptions |
| `set` | Sets a key from a SQL query result. |

##### `query set`

```
Sets a key from a SQL query result.
Not implemented yet.
```

#### `reload-texture`

```
Reload the texture from the document.

Usage: reload-texture
```

#### `rotate`

```
Rotate the texture by a given angle

Usage: rotate <angle>
```

#### `set`

```
Set a key to a value in the JSON document.

Usage: set <key> <value>

Note: All values are stored as strings.
```

#### `update-text`

```
Calculate text texture

Usage: update-text

Triggers a recalculation of the text texture.
```

