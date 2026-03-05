# Nebulite Command Documentation

This documentation is automatically generated.

Generated on: Thu Mar  5 12:18:44 CET 2026

## Table of Contents

- [GlobalSpace Commands](#globalspace-commands)
- [RenderObject Commands](#renderobject-commands)
- [JSON Transformations](#json-transformations)
- [Expression Functions](#expression-functions)

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
| `error-log` | Activates or deactivates error logging to a file. |
| `eval` | Evaluates an expression string and executes it. |
| `evaluation` | Tests evaluation of self and other globals in one expression |
| `exit` | Exits the entire program. |
| `expression-help` | Lists all available expression functions with their descriptions. |
| `feature-test` | Functions for testing features in the GlobalSpace |
| `for` | Executes a for-loop with a function call. |
| `help` | Show available commands and their descriptions |
| `if` | Executes a block of code if a condition is true. |
| `input-mapping` | Functions for mapping inputs to actions within the GlobalSpace. |
| `input-wait` | Waits for user input before continuing. |
| `json` | Functions to manipulate JSON data via read-only JSON documents |
| `json` | Functions to manipulate JSON data via read-only JSON documents |
| `keyDelete` | Delete a key from the JSON document. |
| `log` | Functions for logging various states and documents to files. |
| `move` | Move data from one key to another. |
| `nop` | No operation. Does nothing. |
| `pop-back` | Pop a value from the back of an array. |
| `pop-front` | Pop a value from the front of an array. |
| `print` | Prints the JSON document to the console for debugging purposes. |
| `push-back` | Push a value to the back of an array. |
| `push-front` | Push a value to the front of an array. |
| `query` | Functions to manipulate JSON data via SQL query results |
| `query` | Functions to manipulate JSON data via SQL query results |
| `return` | Returns a custom value as a Critical Error. |
| `ruleset` | Functions for managing rulesets in the GlobalSpace. |
| `selected-object` | Functions to select and interact with a selected RenderObject |
| `set` | Set a key to a value in the JSON document. |
| `set-fps` | Set FPS of renderer. |
| `set-res` | Set resolution of renderer. |
| `settings` | Functions for managing global settings. |
| `show-fps` | Show FPS of renderer. |
| `snapshot` | Create a snapshot of the current renderer state. |
| `spawn` | Spawn a RenderObject from a json/jsonc file. |
| `standard-file` | Functions for generating standard files for common resources. |
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

Usage: add-clock <interval_ms>

Example:
add-clock 100
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
| `autotype` | Utilities to automate typing in the console |
| `close` | Closes the console, hiding it and preventing it from receiving input. |
| `help` | Show available commands and their descriptions |
| `open` | Opens the console, allowing it to be rendered and receive input. |
| `set-background` | Sets a background image for the console. |
| `zoom` | Reduces or increases the console font size. |

##### `console autotype`

Available Functions

| Function | Description |
|----------|-------------|
| `clear` | Clears the autotype queue without executing the commands. |
| `close` | Closes the console if the autotype wait counter reaches zero |
| `down` | Adds a history down command into the autotype queue. |
| `enter` | Puts an enter command into the autotype queue. |
| `execute` | Executes all autotype commands in the queue. |
| `help` | Show available commands and their descriptions |
| `text` | Adds a text input command into the autotype queue. |
| `up` | Adds a history up command into the autotype queue. |
| `wait` | Waits for a specified amount of ms before executing the next autotype command. |

###### `console autotype clear`

```
Clears the autotype queue without executing the commands.
Clears both the queue pending for execution and the active queue currently being executed.

Usage: console autotype clear
```

###### `console autotype close`

```
Closes the console if the autotype wait counter reaches zero
Usage: console autotype close
```

###### `console autotype down`

```
Adds a history down command into the autotype queue.

Usage: console autotype down
```

###### `console autotype enter`

```
Puts an enter command into the autotype queue.

Usage: console autotype enter
```

###### `console autotype execute`

```
Executes all autotype commands in the queue.

Usage: console autotype execute
```

###### `console autotype text`

```
Adds a text input command into the autotype queue.

Usage: console autotype text <text>
```

###### `console autotype up`

```
Adds a history up command into the autotype queue.

Usage: console autotype up
```

###### `console autotype wait`

```
Waits for a specified amount of ms before executing the next autotype command.

Usage: console autotype wait <milliseconds>
```

##### `console close`

```
Closes the console, hiding it and preventing it from receiving input.

Usage: console close
```

##### `console open`

```
Opens the console, allowing it to be rendered and receive input.

Usage: console open
```

##### `console set-background`

```
Sets a background image for the console.

Usage: console set-background <image_path>
```

##### `console zoom`

```
Reduces or increases the console font size.

Usage: console zoom [in/out/+/-]
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
| `categoryLevel1` | Test category for debugging purposes. |
| `collision-detect` | Utilities for testing collision detection functionalities. |
| `help` | Show available commands and their descriptions |

##### `debug categoryLevel1`

Available Functions

| Function | Description |
|----------|-------------|
| `categoryLevel2` | Nested test category for debugging purposes. |
| `help` | Show available commands and their descriptions |

###### `debug categoryLevel1 categoryLevel2`

Available Functions

| Function | Description |
|----------|-------------|
| `help` | Show available commands and their descriptions |

##### `debug collision-detect`

Available Functions

| Function | Description |
|----------|-------------|
| `category` | Tests collision detection of category names |
| `function` | Tests collision detection of function names |
| `help` | Show available commands and their descriptions |
| `variable` | Tests collision detection of variable names |

###### `debug collision-detect category`

```
Tests collision detection of category names

Tries to bind a category name to globalspace that is already registered, expecting a collision error.

Usage: debug collision-detect category [fail/succeed]

Defaults to fail
```

###### `debug collision-detect function`

```
Tests collision detection of function names

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

#### `error-log`

```
Activates or deactivates error logging to a file.
Usage: error-log <on/off>

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

#### `evaluation`

```
Tests evaluation of self and other globals in one expression
Usage: evaluation
```

#### `exit`

```
Exits the entire program.

Usage: exit

Closes the program
Any queued tasks will be discarded.
```

#### `expression-help`

```
Lists all available expression functions with their descriptions.
Usage: expression-help

Note: This function provides a comprehensive list of all functions that can be used within expressions, along with their usage and descriptions.
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
Usage: feature-test functree
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

Example:
if '$(eq(1+1,2))' echo Condition is true!
```

#### `input-mapping`

Available Functions

| Function | Description |
|----------|-------------|
| `help` | Show available commands and their descriptions |
| `lock` | Functions for locking and unlocking input actions. |

##### `input-mapping lock`

Available Functions

| Function | Description |
|----------|-------------|
| `help` | Show available commands and their descriptions |
| `off` | Unlocks an action, allowing it to be triggered by its associated keys again. |
| `on` | Locks an action until it is unlocked, preventing it from being triggered by any of its associated keys. |
| `once` | Locks an action for the current frame, preventing it from being triggered by any of its associated keys. |

###### `input-mapping lock off`

```
Unlocks an action, allowing it to be triggered by its associated keys again.
Allows locking of entire categories of actions by using a structured action name, e.g. 'movement::up' or 'combat::primaryAttack' can be locked with 'movement' or 'combat'.
Usage: input-mapping unlock <actionName>
```

###### `input-mapping lock on`

```
Locks an action until it is unlocked, preventing it from being triggered by any of its associated keys.
Allows locking of entire categories of actions by using a structured action name, e.g. 'movement::up' or 'combat::primaryAttack' can be locked with 'movement' or 'combat'.
Usage: input-mapping lock on <actionName>
```

###### `input-mapping lock once`

```
Locks an action for the current frame, preventing it from being triggered by any of its associated keys.
Allows locking of entire categories of actions by using a structured action name, e.g. 'movement::up' or 'combat::primaryAttack' can be locked with 'movement' or 'combat'.
Usage: input-mapping lock once <actionName>
```

#### `input-wait`

```
Waits for user input before continuing.
Usage: input-wait [prompt]

Note: This function pauses execution until the user presses Enter
```

#### `json`

Available Functions

| Function | Description |
|----------|-------------|
| `evaluate` | Functions to evaluate and set JSON data as expressions |
| `help` | Show available commands and their descriptions |
| `set` | Sets a key from a given expression evaluated as JSON, allowing for complex objects to be set. |

##### `json evaluate`

Available Functions

| Function | Description |
|----------|-------------|
| `help` | Show available commands and their descriptions |
| `member` | If the member is a string or number, treats it as an expression and evaluates it as JSON, setting the member to the result. |
| `recursive` | Recursively evaluates all string members in the JSON object as expressions, allowing for complex nested structures to be evaluated and set in one command. |

###### `json evaluate member`

```
If the member is a string or number, treats it as an expression and evaluates it as JSON, setting the member to the result.
If the member is an array or object, it will do nothing.
Usage: json evaluate member <key>

Examples:
evaluate member myExpression
If the member myExpression is a string, for example "{global.names|filterGlob F*}", myExpression will be set to an array of names starting with F from the global scope.
```

###### `json evaluate recursive`

```
Recursively evaluates all string members in the JSON object as expressions, allowing for complex nested structures to be evaluated and set in one command.
Usage: json evaluate recursive <key>

Examples:
evaluate recursive myObject
MyObject could be an array of expressions, that are all evaluated and replaced with their results,
or an object with nested objects and arrays containing expressions, all of which are evaluated and replaced with their results.
```

##### `json set`

```
Sets a key from a given expression evaluated as JSON, allowing for complex objects to be set.
Usage: json set <key> <expression>

Examples:
json set namesStartingWithF {global.names|filterGlob F*}
json set userInfo {global.users|filterRegex {!^user[0-9]+$}}
json set readOnlyDoc {./Resources/sample.json:key1.key2}
json set sizeCopy {self.size}
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

#### `nop`

```
No operation. Does nothing.

Usage: nop <blind arguments>

Useful for testing or as a placeholder in scripts where no action is required,
but a command is syntactically necessary.
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

#### `ruleset`

Available Functions

| Function | Description |
|----------|-------------|
| `broadcast` | Broadcasts a ruleset to its specified topic. |
| `help` | Show available commands and their descriptions |
| `listen` | Listens for rulesets on a specified topic. |

##### `ruleset broadcast`

```
Broadcasts a ruleset to its specified topic.
Usage: broadcast <ruleset>

- ruleset: The ruleset content to be broadcasted.
```

##### `ruleset listen`

```
Listens for rulesets on a specified topic.
Usage: listen <topic>

- topic: The topic to listen for incoming rulesets.
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

#### `settings`

Available Functions

| Function | Description |
|----------|-------------|
| `help` | Show available commands and their descriptions |
| `save` | Saves the current global settings to the default filename. |
| `save-standards` | Overwrites the settings file with default settings. |
| `set-integer` | Sets a global setting to a specified integer value. |
| `set-string` | Sets a global setting to a specified value. |

##### `settings save`

```
Saves the current global settings to the default filename.

Usage: settings save
```

##### `settings save-standards`

```
Overwrites the settings file with default settings.

Usage: settings save-standards
```

##### `settings set-integer`

```
Sets a global setting to a specified integer value.

Usage: settings set-integer <key> <value>
```

##### `settings set-string`

```
Sets a global setting to a specified value.

Usage: settings set-string <key> <value>
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

#### `standard-file`

Available Functions

| Function | Description |
|----------|-------------|
| `help` | Show available commands and their descriptions |
| `render-object` | Logs a standard render object to a file: ./Resources/Renderobjects/standard.jsonc. |

##### `standard-file render-object`

```
Logs a standard render object to a file: ./Resources/Renderobjects/standard.jsonc.
Usage: standard-file render-object

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
| `lock` | Locks time with lock provided, |
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

Usage: time set-fixed-dt <dt_ms>

<dt_ms> : Fixed delta time in milliseconds. Use 0 to reset to real delta time.
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

This is useful for:- Creating pauses in scripts to wait for certain conditions to be met.
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
| `delete` | Marks object for deletion |
| `echo` | Echoes all arguments as string to the standard output. |
| `ensure-array` | Ensure that a key is an array, converting a value to an array if necessary. |
| `eval` | Evaluate an expression and execute the result. |
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
| `ruleset` | Ruleset management functions for the RenderObject domain. |
| `set` | Set a key to a value in the JSON document. |

#### `copy`

```
Copy data from one key to another.

Usage: copy <source_key> <destination_key>
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
eval echo {self.id} outputs this objects id
```

#### `json`

Available Functions

| Function | Description |
|----------|-------------|
| `evaluate` | Functions to evaluate and set JSON data as expressions |
| `help` | Show available commands and their descriptions |
| `set` | Sets a key from a given expression evaluated as JSON, allowing for complex objects to be set. |

##### `json evaluate`

Available Functions

| Function | Description |
|----------|-------------|
| `help` | Show available commands and their descriptions |
| `member` | If the member is a string or number, treats it as an expression and evaluates it as JSON, setting the member to the result. |
| `recursive` | Recursively evaluates all string members in the JSON object as expressions, allowing for complex nested structures to be evaluated and set in one command. |

###### `json evaluate member`

```
If the member is a string or number, treats it as an expression and evaluates it as JSON, setting the member to the result.
If the member is an array or object, it will do nothing.
Usage: json evaluate member <key>

Examples:
evaluate member myExpression
If the member myExpression is a string, for example "{global.names|filterGlob F*}", myExpression will be set to an array of names starting with F from the global scope.
```

###### `json evaluate recursive`

```
Recursively evaluates all string members in the JSON object as expressions, allowing for complex nested structures to be evaluated and set in one command.
Usage: json evaluate recursive <key>

Examples:
evaluate recursive myObject
MyObject could be an array of expressions, that are all evaluated and replaced with their results,
or an object with nested objects and arrays containing expressions, all of which are evaluated and replaced with their results.
```

##### `json set`

```
Sets a key from a given expression evaluated as JSON, allowing for complex objects to be set.
Usage: json set <key> <expression>

Examples:
json set namesStartingWithF {global.names|filterGlob F*}
json set userInfo {global.users|filterRegex {!^user[0-9]+$}}
json set readOnlyDoc {./Resources/sample.json:key1.key2}
json set sizeCopy {self.size}
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
| `key` | Logs a specific key's value to a file. |

##### `log all`

```
Logs the entire RenderObject to a file.

Usage: log [filename]

Logs to `RenderObject_id<id>.log.jsonc` if no filename is provided.
```

##### `log key`

```
Logs a specific key's value to a file.

Usage: log key <key> [filename]

Logs to `RenderObject_id<id>.log.jsonc` if no filename is provided.
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

Mirrors are removed from the GlobalSpace document under key "mirror.renderObject.id<id>"
```

##### `mirror fetch`

```
Deserializes the RenderObject from the GlobalSpace document entry

Usage: mirror fetch

Mirrors are fetched from the GlobalSpace document under key "mirror.renderObject.id<id>"
```

##### `mirror off`

```
Disables mirroring to the GlobalSpace document

Usage: mirror off

Constant mirroring is inactive until turned on again with 'mirror on'

Mirrors are stored in the GlobalSpace document under key "mirror.renderObject.id<id>"
```

##### `mirror on`

```
Enables mirroring to the GlobalSpace document

Usage: mirror on

Constant mirroring is active until turned off with 'mirror off'

Mirrors are stored in the GlobalSpace document under key "mirror.renderObject.id<id>"
```

##### `mirror once`

```
Mirrors the object to the GlobalSpace document once on next update

Usage: mirror once

Mirroring is only done for one frame.
Mirrors are stored in the GlobalSpace document under key "mirror.renderObject.id<id>"
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

#### `ruleset`

Available Functions

| Function | Description |
|----------|-------------|
| `help` | Show available commands and their descriptions |
| `once` | Applies all rulesets once on the next update |
| `reload` | Reloads all rulesets for this RenderObject on the next update. |

##### `ruleset once`

```
Applies all rulesets once on the next update

Usage: ruleset once

All rulesets are applied once on the next update cycle.
```

##### `ruleset reload`

```
Reloads all rulesets for this RenderObject on the next update.

Usage: ruleset reload

All rulesets are re-evaluated and reloaded on the next update cycle.
```

#### `set`

```
Set a key to a value in the JSON document.

Usage: set <key> <value>

Note: All values are stored as strings.
```

## JSON Transformations

These commands are available during JSON value retrieval with the transformation operator '|'.
Example: `{global.var|length}`

Help for JSON rvalue transformation FuncTree

Available Functions

| Function | Description |
|----------|-------------|
| `add` | Adds a numeric value to the current JSON value. |
| `assert` | Assertion transformations that validate JSON values and throw exceptions on failure. |
| `at` | Gets the element at the specified index from the array in the current JSON value. |
| `average` | Calculates the average of the elements of the array in the current JSON value. |
| `deserialize` | Deserializes the current JSON string value stored in root. |
| `div` | Divides the current JSON value by a numeric value. |
| `echo` | Echoes the provided arguments to the console, with newline. |
| `ensureArray` | Ensures the current JSON value is an array. |
| `filterGlob` | Filters members in the current JSON array/object based on a glob pattern. |
| `filterNulls` | Filters out null values, empty objects, and empty arrays from the current JSON |
| `filterRegex` | Filters members in the current JSON array/object based on a regular expression pattern. |
| `first` | Gets the first element of the array in the current JSON value. |
| `get` | Gets the value at the specified key from the current JSON object. |
| `getMultiple` | Gets multiple values at the specified keys from the current JSON object. |
| `help` | Show available commands and their descriptions |
| `last` | Gets the last element of the array in the current JSON value. |
| `length` | Gets the length of the array in the current JSON value. |
| `listMembers` | Lists all members of the current JSON object as an array. |
| `lStrip` | Strips whitespace from the left end of the current JSON string value. |
| `map` | Applies a mapping function to each element in the array of the current JSON value. |
| `max` | Finds the maximum value among the elements of the array in the current JSON value. |
| `median` | Calculates the median of the elements of the array in the current JSON value. |
| `min` | Finds the minimum value among the elements of the array in the current JSON value. |
| `mod` | Calculates the modulo of the current JSON value by a numeric value. |
| `mul` | Multiplies the current JSON value by a numeric value. |
| `pow` | Raises the current JSON value to the power of a numeric value. |
| `print` | Prints the current JSON value to the console. |
| `product` | Multiplies the elements of the array in the current JSON value. |
| `push` | Pushes a string value to the end of the array in the current JSON value. |
| `pushNumber` | Pushes a numeric value to the end of the array in the current JSON value. |
| `removeMember` | Removes the member at the specified key from the JSON document. |
| `replace` | Replaces all occurrences of a target substring with a replacement substring in the current JSON string value. |
| `require` | Requirement transformations that validate JSON values and return false on failure (fallback to default value in get-call, usually 'null' or 0.0). |
| `reverse` | Reverses the array in the current JSON value. |
| `root` | Calculates the n-th root of the current JSON value. |
| `round` | Rounds the current JSON numeric value to the nearest integer. |
| `roundDown` | Rounds the current JSON numeric value down to the nearest integer. |
| `roundUp` | Rounds the current JSON numeric value up to the nearest integer. |
| `rStrip` | Strips whitespace from the right end of the current JSON string value. |
| `serialize` | Serializes the current JSON value to a string. |
| `setBool` | Sets a boolean value at the specified key in the JSON document. |
| `setDouble` | Sets a double value at the specified key in the JSON document. |
| `setFromResult` | Sets the value at the specified key in the JSON document from the result of another transformation. |
| `setInt` | Sets an integer value at the specified key in the JSON document. |
| `setString` | Sets a string value at the specified key in the JSON document. |
| `sqrt` | Calculates the square root of the current JSON value. |
| `stddev` | Calculates the standard deviation of the elements of the array in the current JSON value. |
| `strCompare` | Functions for comparing string values. |
| `strCountAppearance` | Counts the number of occurrences of a specified substring in the current JSON string value. |
| `strip` | Strips whitespace from both ends of the current JSON string value. |
| `sub` | Subtracts a numeric value from the current JSON value. |
| `subspan` | Gets a subarray from the array in the current JSON value. |
| `substring` | Extracts a substring from the current JSON string value. |
| `sum` | Sums the elements of the array in the current JSON value. |
| `toBool` | Converts the current JSON value to a boolean. |
| `toBoolString` | Converts the current JSON value to a boolean string. |
| `toDouble` | Converts the current JSON value to a double. |
| `toInt` | Casts the current JSON value to an integer. |
| `toLower` | Converts the current JSON string value to lowercase. |
| `toString` | Converts the current JSON value to a string. |
| `toUpper` | Converts the current JSON string value to uppercase. |
| `typeAsNumber` | Converts the current JSON type value to a number. |
| `typeAsString` | Converts the current JSON type value to a string. |

#### `add`

```
Adds a numeric value to the current JSON value.
If multiple numbers are provided, the result is an array, one element for each provided argument.
Usage: |add <number1> <number2> ... -> {number/array}
```

#### `assert`

Available Functions

| Function | Description |
|----------|-------------|
| `help` | Show available commands and their descriptions |
| `nonEmpty` | Asserts that the current JSON value is non-empty. |
| `type` | Assertion transformations that validate JSON value types and throw exceptions on failure. |

##### `assert nonEmpty`

```
Asserts that the current JSON value is non-empty.
If the value is empty, the transformation fails and the program exits
Accepts an optional user-defined error message as additional arguments.
Usage: |assertNonEmpty -> {value,<Exception thrown if empty>}
```

##### `assert type`

Available Functions

| Function | Description |
|----------|-------------|
| `array` | Asserts that the current JSON value is of type array. |
| `basicValue` | Asserts that the current JSON value is a basic value (not object or array or null). |
| `help` | Show available commands and their descriptions |
| `object` | Asserts that the current JSON value is of type object. |

###### `assert type array`

```
Asserts that the current JSON value is of type array.
If the value is not an array, the transformation fails and the program exits
Accepts an optional user-defined error message as additional arguments.
Usage: |assertTypeArray -> {value,<Exception thrown if not array>}
```

###### `assert type basicValue`

```
Asserts that the current JSON value is a basic value (not object or array or null).
If the value is not a basic value, the transformation fails and the program exits
Accepts an optional user-defined error message as additional arguments.
Usage: |assertTypeValue -> {value,<Exception thrown if not value>}
```

###### `assert type object`

```
Asserts that the current JSON value is of type object.
If the value is not an object, the transformation fails and the program exits
Accepts an optional user-defined error message as additional arguments.
Usage: |assertTypeObject -> {value,<Exception thrown if not object>}
```

#### `at`

```
Gets the element at the specified index from the array in the current JSON value.
If the index is out of bounds, the transformation fails.
Usage: |at <index> -> {value}
```

#### `average`

```
Calculates the average of the elements of the array in the current JSON value.
Input must be an array of numbers. If any element is not a number, the current value is not an array or the array is empty, the transformation fails.
Usage: |average -> {number}
```

#### `deserialize`

```
Deserializes the current JSON string value stored in root.
Usage: |deserialize -> {value}
```

#### `div`

```
Divides the current JSON value by a numeric value.
If multiple numbers are provided, the result is an array, one element for each provided argument.
Usage: |div <number1> <number2> ... -> {number/array}
```

#### `echo`

```
Echoes the provided arguments to the console, with newline.
Usage: |echo <arg1> <arg2> -> {unchanged-json}
```

#### `ensureArray`

```
Ensures the current JSON value is an array.
If the current value is not an array, it is wrapped into a single-element array.
Usage: |ensureArray -> {array}
```

#### `filterGlob`

```
Filters members in the current JSON array/object based on a glob pattern.
For arrays, the member names are the indices as strings: [0], [1], [2], ...
Usage: |filterGlob <pattern> -> {filtered array}
```

#### `filterNulls`

```
Filters out null values, empty objects, and empty arrays from the current JSON
Usage: |filterNulls -> {filtered json}
```

#### `filterRegex`

```
Filters members in the current JSON array/object based on a regular expression pattern.
For arrays, the member names are the indices as strings: [0], [1], [2], ...
Wrap the pattern inside {!...} to avoid conflicts with piping characterUsage: |filterRegex {!<pattern>} -> {filtered array}
       |filterRegex <pattern>    -> {filtered array}
```

#### `first`

```
Gets the first element of the array in the current JSON value.
If the current value is not an array, it is first wrapped into a single-element array.
Usage: |first -> {value}
```

#### `get`

```
Gets the value at the specified key from the current JSON object.
Usage: |get <key> -> {value}
```

#### `getMultiple`

```
Gets multiple values at the specified keys from the current JSON object.
Usage: |getMultiple <key1> <key2> ... -> {array of values}
```

#### `last`

```
Gets the last element of the array in the current JSON value.
If the current value is not an array, it is first wrapped into a single-element array.
Usage: |last -> {value}
```

#### `length`

```
Gets the length of the array in the current JSON value.
Usage: |length -> {number}
```

#### `listMembers`

```
Lists all members of the current JSON object as an array.
If the current value is an array, it lists the indices as strings.
Usage: |listKeys -> {array of keys}
```

#### `lStrip`

```
Strips whitespace from the left end of the current JSON string value.
Usage: |lStrip -> {string}
```

#### `map`

```
Applies a mapping function to each element in the array of the current JSON value.
If the current value is not an array, it is first wrapped into a single-element array.
Usage: |map <function> -> {array}
```

#### `max`

```
Finds the maximum value among the elements of the array in the current JSON value.
Input must be an array of numbers. If any element is not a number, the current value is not an array or the array is empty, the transformation fails.
Usage: |max -> {number}
```

#### `median`

```
Calculates the median of the elements of the array in the current JSON value.
Input must be an array of numbers. If any element is not a number, the current value is not an array or the array is empty, the transformation fails.
Usage: |median -> {number}
```

#### `min`

```
Finds the minimum value among the elements of the array in the current JSON value.
Input must be an array of numbers. If any element is not a number, the current value is not an array or the array is empty, the transformation fails.
Usage: |min -> {number}
```

#### `mod`

```
Calculates the modulo of the current JSON value by a numeric value.
If multiple numbers are provided, the result is an array, one element for each provided argument.
Usage: |mod <number1> <number2> ... -> {number/array}
```

#### `mul`

```
Multiplies the current JSON value by a numeric value.
If multiple numbers are provided, the result is an array, one element for each provided argument.
Usage: |multiply <number1> <number2> ... -> {number/array}
```

#### `pow`

```
Raises the current JSON value to the power of a numeric value.
If multiple numbers are provided, the result is an array, one element for each provided argument.
Usage: |pow <exponent1> <exponent2> ... -> {number/array}
```

#### `print`

```
Prints the current JSON value to the console.
Usage: |print -> {unchanged-json}
```

#### `product`

```
Multiplies the elements of the array in the current JSON value.
Input must be an array of numbers. If any element is not a number, the current value is not an array or the array is empty, the transformation fails.
Usage: |product -> {number}
```

#### `push`

```
Pushes a string value to the end of the array in the current JSON value.
If the current value is not an array, it is first wrapped into a single-element array.
Usage: |push <value> -> {array}
```

#### `pushNumber`

```
Pushes a numeric value to the end of the array in the current JSON value.
If the current value is not an array, it is first wrapped into a single-element array.
Usage: |pushNumber <value> -> {array}
```

#### `removeMember`

```
Removes the member at the specified key from the JSON document.
Usage: |removeMember <key1> <key2> ... -> {json}
```

#### `replace`

```
Replaces all occurrences of a target substring with a replacement substring in the current JSON string value.
Usage: |replace {target} {replacement} -> {string}
{target}: Substring to be replaced
{replacement}: Substring to replace with
```

#### `require`

Available Functions

| Function | Description |
|----------|-------------|
| `help` | Show available commands and their descriptions |
| `nonEmpty` | Requires that the current JSON value is non-empty. |
| `type` | Requirement transformations that validate JSON value types |

##### `require nonEmpty`

```
Requires that the current JSON value is non-empty.
If the value is empty, the transformation fails
Accepts an optional user-defined error message as additional arguments.
Usage: |requireNonEmpty -> {value,<Returns false if empty>}
```

##### `require type`

Available Functions

| Function | Description |
|----------|-------------|
| `array` | Requires that the current JSON value is of type array. |
| `basicValue` | Requires that the current JSON value is a basic value (not object or array or null). |
| `help` | Show available commands and their descriptions |
| `object` | Requires that the current JSON value is of type object. |

###### `require type array`

```
Requires that the current JSON value is of type array.
If the value is not an array, the transformation fails
Accepts an optional user-defined error message as additional arguments.
Usage: |requireTypeArray -> {value,<Returns false if not array>}
```

###### `require type basicValue`

```
Requires that the current JSON value is a basic value (not object or array or null).
If the value is not a basic value, the transformation fails
Accepts an optional user-defined error message as additional arguments.
Usage: |requireTypeValue -> {value,<Returns false if not value>}
```

###### `require type object`

```
Requires that the current JSON value is of type object.
If the value is not an object, the transformation fails
Accepts an optional user-defined error message as additional arguments.
Usage: |requireTypeObject -> {value,<Returns false if not object>}
```

#### `reverse`

```
Reverses the array in the current JSON value.
If the current value is not an array, it is first wrapped into a single-element array.
Usage: |reverse -> {array}
```

#### `root`

```
Calculates the n-th root of the current JSON value.
If multiple numbers are provided, the result is an array, one element for each provided argument.
Usage: |root <number1> <number2> ... -> {number/array}
```

#### `round`

```
Rounds the current JSON numeric value to the nearest integer.
Usage: |round -> {value:int}
Non-numeric values default to 0. Fails if the value is null.
```

#### `roundDown`

```
Rounds the current JSON numeric value down to the nearest integer.
Usage: |roundDown -> {value:int}
Non-numeric values default to 0. Fails if the value is null.
```

#### `roundUp`

```
Rounds the current JSON numeric value up to the nearest integer.
Usage: |roundUp -> {value:int}
Non-numeric values default to 0.
```

#### `rStrip`

```
Strips whitespace from the right end of the current JSON string value.
Usage: |rStrip -> {string}
```

#### `serialize`

```
Serializes the current JSON value to a string.
Usage: |serialize -> {string}
```

#### `setBool`

```
Sets a boolean value at the specified key in the JSON document.
Expects two arguments: <key> and <value> (true/false).
Any other value will be considered false.
Usage: |setBool <key> <value> -> {json}
```

#### `setDouble`

```
Sets a double value at the specified key in the JSON document.
Expects two arguments: <key> and <value>.
Usage: |setDouble <key> <value> -> {json}
```

#### `setFromResult`

```
Sets the value at the specified key in the JSON document from the result of another transformation.
Usage: |setFromResult <key> {!transformation} -> {json}
The '!' is required, otherwise the nested variable is evaluated by the expression class before the transformation is applied!
```

#### `setInt`

```
Sets an integer value at the specified key in the JSON document.
Expects two arguments: <key> and <value>.
Usage: |setInt <key> <value> -> {json}
```

#### `setString`

```
Sets a string value at the specified key in the JSON document.
Expects two arguments: <key> and <value>.
Usage: |setString <key> <value> -> {json}
```

#### `sqrt`

```
Calculates the square root of the current JSON value.
No arguments are allowed, as it's an operator with a single operand (the current JSON value).
Usage: |sqrt -> {number}
```

#### `stddev`

```
Calculates the standard deviation of the elements of the array in the current JSON value.
Input must be an array of numbers. If any element is not a number, the current value is not an array or the array is empty, the transformation fails.
Usage: |stddev -> {number}
```

#### `strCompare`

Available Functions

| Function | Description |
|----------|-------------|
| `contains` | Checks if the current JSON string value contains a specified substring. |
| `endsWith` | Checks if the current JSON string value ends with a specified substring. |
| `equals` | Compares the current JSON string value with a specified string for equality. |
| `help` | Show available commands and their descriptions |
| `startsWith` | Checks if the current JSON string value starts with a specified substring. |

##### `strCompare contains`

```
Checks if the current JSON string value contains a specified substring.
Usage: |strCompare contains {string} -> {bool}
{string}: Substring to check for
```

##### `strCompare endsWith`

```
Checks if the current JSON string value ends with a specified substring.
Usage: |strCompare endsWith {string} -> {bool}
{string}: Substring to check for
```

##### `strCompare equals`

```
Compares the current JSON string value with a specified string for equality.
Usage: |strCompare equals {string} -> {bool}
{string}: String to compare with
```

##### `strCompare startsWith`

```
Checks if the current JSON string value starts with a specified substring.
Usage: |strCompare startsWith {string} -> {bool}
{string}: Substring to check for
```

#### `strCountAppearance`

```
Counts the number of occurrences of a specified substring in the current JSON string value.
Usage: |strCountAppearance {substring} -> {number}
{substring}: Substring to count
```

#### `strip`

```
Strips whitespace from both ends of the current JSON string value.
Usage: |trim -> {string}
```

#### `sub`

```
Subtracts a numeric value from the current JSON value.
If multiple numbers are provided, the result is an array, one element for each provided argument.
Usage: |sub <number1> <number2> ... -> {number/array}
```

#### `subspan`

```
Gets a subarray from the array in the current JSON value.
Usage: |subspan <start> [<length>] -> {array}
```

#### `substring`

```
Extracts a substring from the current JSON string value.
Usage: |substring {start} {length} -> {string}
{start}: Starting index (0-based)
{length}: Length of the substring
```

#### `sum`

```
Sums the elements of the array in the current JSON value.
Input must be an array of numbers. If any element is not a number, the current value is not an array or the array is empty, the transformation fails.
Usage: |sum -> {number}
```

#### `toBool`

```
Converts the current JSON value to a boolean.
Usage: |toBool -> {value:bool}
'true' values: true, 1, '1', 'true', 'yes', 'on' (case-insensitive)
'false' values: false, 0, '0', 'false', 'no', 'off' (case-insensitive)
Any other value defaults to false, but fails if the value is null.
```

#### `toBoolString`

```
Converts the current JSON value to a boolean string.
Usage: |toBoolString -> {value:string}
Either 'true' or 'false'
```

#### `toDouble`

```
Converts the current JSON value to a double.
Usage: |toDouble -> {value:double}
Non-numeric values default to 0.0, but fails if the value is null.
```

#### `toInt`

```
Casts the current JSON value to an integer.
Usage: |toInt -> {value:int}
Non-numeric values default to 0, but fails if the value is null.
```

#### `toLower`

```
Converts the current JSON string value to lowercase.
Usage: |toLower -> {string}
```

#### `toString`

```
Converts the current JSON value to a string.
Defaults to an empty string if no conversion is possible, but fails if the value is null.
Usage: |toString -> {value:string}
```

#### `toUpper`

```
Converts the current JSON string value to uppercase.
Usage: |toUpper -> {string}
```

#### `typeAsNumber`

```
Converts the current JSON type value to a number.
Usage: |typeAsNumber -> {number}where the number reflects the enum value KeyType.
```

#### `typeAsString`

```
Converts the current JSON type value to a string.
Usage: |typeAsString -> {value,array,object}
```

## Expression Functions

These functions are available in expressions (e.g. in `eval` or `if` conditions).

Help for Nebulite Expressions

Available Functions

| Function | Description |
|----------|-------------|
| `and` | Returns 1 if both a and b are logically true, otherwise returns 0. |
| `constrain` | Constrains a value to lie between a minimum and maximum value. |
| `eq` | Returns 1 if a is equal to b. |
| `geq` | Returns 1 if a is greater than or equal to b. |
| `gt` | Returns 1 if a is greater than b. |
| `help` | Show available commands and their descriptions |
| `leq` | Returns 1 if a is less than or equal to b. |
| `lt` | Returns 1 if a is less than b. |
| `map` | Linearly maps a value from one range to another. |
| `max` | Returns the greater of a and b. |
| `min` | Returns the lesser of a and b. |
| `nand` | Returns 1 if at least one of a or b is logically false (i.e. not both true), otherwise returns 0. |
| `neq` | Returns 1 if a is not equal to b. |
| `nor` | Returns 1 if both a and b are logically false, otherwise returns 0. |
| `not` | Returns 1 if a is logically false (close to zero), otherwise returns 0. |
| `or` | Returns 1 if either a or b is logically true, otherwise returns 0. |
| `rng2arg` | Returns a pseudo-random number between 0 and 1, deterministically seeded from the input values a and b. |
| `rng2argInt16` | Returns a pseudo-random integer between 0 and 32767, deterministically seeded from the input values a and b. |
| `rng3arg` | Returns a pseudo-random number between 0 and 1, deterministically seeded from the input values a, b, and c. |
| `rng3argInt16` | Returns a pseudo-random integer between 0 and 32767, deterministically seeded from the input values a, b, and c. |
| `sgn` | Returns the sign of a. |
| `to_bipolar` | Converts a numeric value to bipolar form. |
| `xnor` | Returns 1 if a and b are both logically true or both logically false, otherwise returns 0. |
| `xor` | Returns 1 if exactly one of a or b is logically true, otherwise returns 0. |

#### `and`

```
Returns 1 if both a and b are logically true, otherwise returns 0.
A value is considered true when its absolute value is greater than DBL_EPSILON.
Usage: and(a, b)
```

#### `constrain`

```
Constrains a value to lie between a minimum and maximum value.
Usage: constrain(value, min, max)
```

#### `eq`

```
Returns 1 if a is equal to b.
Equality is tested within a small epsilon (DBL_EPSILON) to handle floating point imprecision.
Usage: eq(a, b)
```

#### `geq`

```
Returns 1 if a is greater than or equal to b.
Otherwise returns 0.
Usage: geq(a, b)
```

#### `gt`

```
Returns 1 if a is greater than b.
Otherwise returns 0.
Usage: gt(a, b)
```

#### `leq`

```
Returns 1 if a is less than or equal to b.
Otherwise returns 0.
Usage: leq(a, b)
```

#### `lt`

```
Returns 1 if a is less than b.
Otherwise returns 0.
Usage: lt(a, b)
```

#### `map`

```
Linearly maps a value from one range to another.
Usage: map(value, in_min, in_max, out_min, out_max)
```

#### `max`

```
Returns the greater of a and b.
Usage: max(a, b)
```

#### `min`

```
Returns the lesser of a and b.
Usage: min(a, b)
```

#### `nand`

```
Returns 1 if at least one of a or b is logically false (i.e. not both true), otherwise returns 0.
Uses DBL_EPSILON to determine logical truthiness.
Usage: nand(a, b)
```

#### `neq`

```
Returns 1 if a is not equal to b.
Inequality is determined beyond a small epsilon (DBL_EPSILON).
Usage: neq(a, b)
```

#### `nor`

```
Returns 1 if both a and b are logically false, otherwise returns 0.
Values with absolute value <= DBL_EPSILON are treated as false.
Usage: nor(a, b)
```

#### `not`

```
Returns 1 if a is logically false (close to zero), otherwise returns 0.
Values with absolute value <= DBL_EPSILON are treated as false.
Usage: not(a)
```

#### `or`

```
Returns 1 if either a or b is logically true, otherwise returns 0.
A value is considered true when its absolute value is greater than DBL_EPSILON.
Usage: or(a, b)
```

#### `rng2arg`

```
Returns a pseudo-random number between 0 and 1, deterministically seeded from the input values a and b.
The same input values will always produce the same output, making it suitable for deterministic procedural generation.
Usage: rng2arg(a, b)
```

#### `rng2argInt16`

```
Returns a pseudo-random integer between 0 and 32767, deterministically seeded from the input values a and b.
The same input values will always produce the same output, making it suitable for deterministic procedural generation.
Usage: rng2argInt16(a, b)
```

#### `rng3arg`

```
Returns a pseudo-random number between 0 and 1, deterministically seeded from the input values a, b, and c.
The same input values will always produce the same output, making it suitable for deterministic procedural generation.
Usage: rng3arg(a, b, c)
```

#### `rng3argInt16`

```
Returns a pseudo-random integer between 0 and 32767, deterministically seeded from the input values a, b, and c.
The same input values will always produce the same output, making it suitable for deterministic procedural generation.
Usage: rng3argInt16(a, b, c)
```

#### `sgn`

```
Returns the sign of a.
Returns 1 if a is positive, -1 if a is negative, and 0 if a is zero.
Usage: sgn(a)
```

#### `to_bipolar`

```
Converts a numeric value to bipolar form.
output is 1 or -1
Returns 1 if a is logically true (absolute value > DBL_EPSILON), otherwise returns -1.
Usage: to_bipolar(a)
```

#### `xnor`

```
Returns 1 if a and b are both logically true or both logically false, otherwise returns 0.
Uses DBL_EPSILON threshold to determine logical equality.
Usage: xnor(a, b)
```

#### `xor`

```
Returns 1 if exactly one of a or b is logically true, otherwise returns 0.
Uses DBL_EPSILON threshold to determine logical truthiness.
Usage: xor(a, b)
```

