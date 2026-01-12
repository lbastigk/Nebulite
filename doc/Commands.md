# Nebulite Command Documentation

This documentation is automatically generated.

Generated on: Sat Jan  3 22:57:28 CET 2026

## Table of Contents

- [GlobalSpace Commands](#globalspace-commands)
- [RenderObject Commands](#renderobject-commands)

- [JSON Transformations Commands](#json-transformations-commands)

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
| `in-scope` | Parses a command within a specific scope. |
| `input-wait` | Waits for user input before continuing. |
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
| `return` | Returns a custom value as a Critical Error. |
| `ruleset` | Functions for managing rulesets in the GlobalSpace. |
| `selected-object` | Functions to select and interact with a selected RenderObject |
| `set` | Set a key to a value in the JSON document. |
| `set-fps` | Set FPS of renderer. |
| `set-res` | Set resolution of renderer. |
| `show-fps` | Show FPS of renderer. |
| `snapshot` | Create a snapshot of the current renderer state. |
| `spawn` | Spawn a RenderObject from a json/jsonc file. |
| `standardfile` | Functions for generating standard files for common resources. |
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
Move data from one key to another.

Usage: move <source_key> <destination_key>
```

##### `cam set`

```
Set a key to a value in the JSON document.

Usage: set <key> <value>

Note: All values are stored as strings.
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

##### `console zoom`

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

##### `env load`

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

Closes the program
Any queued tasks will be discarded.
```

#### `feature-test`

Available Functions

| Function | Description |
|----------|-------------|
| `functree` | Builds a funcTree with extra arguments and tests it |
| `help` | Show available commands and their descriptions |

##### `feature-test functree`

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

#### `in-scope`

```
Parses a command within a specific scope.

Usage: in-scope <scope> <command>

Example:
in-scope global echo This is in the global scope!
This will output: This is in the global scope!
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
| `help` | Show available commands and their descriptions |
| `set` | Sets a key from a read-only JSON document. |

##### `json set`

```
Set a key to a value in the JSON document.

Usage: set <key> <value>

Note: All values are stored as strings.
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

##### `log state`

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
Set a key to a value in the JSON document.

Usage: set <key> <value>

Note: All values are stored as strings.
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

##### `ruleset listen`

#### `selected-object`

Available Functions

| Function | Description |
|----------|-------------|
| `get` | Get a renderobject by its ID. |
| `help` | Show available commands and their descriptions |
| `parse` | Parse a command on the selected RenderObject. |

##### `selected-object get`

##### `selected-object parse`

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

##### `time lock`

##### `time master-unlock`

##### `time set-fixed-dt`

##### `time unlock`

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
| `debug` | Debugging functions for RenderObject domains |
| `delete` | Marks object for deletion |
| `echo` | Echoes all arguments as string to the standard output. |
| `ensure-array` | Ensure that a key is an array, converting a value to an array if necessary. |
| `eval` | Evaluate an expression and execute the result. |
| `fill` | Fills the texture with a specified color. |
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
| `rotate` | Rotate the texture by a specified angle. |
| `ruleset` | Ruleset management functions for the RenderObject domain. |
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

##### `debug print-src-rect`

##### `debug texture-status`

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
eval spawn ./Resources/RenderObjects/{global.ToSpawn}.json
```

#### `fill`

```
Fills the texture with a specified color.
Usage:
  fill <color>
  fill <r> <g> <b>
Where <color> can be 'red', 'green', or 'blue',
and <r>, <g>, <b> are integer values (0-255) for red, green, and blue components.
```

#### `json`

Available Functions

| Function | Description |
|----------|-------------|
| `help` | Show available commands and their descriptions |
| `set` | Sets a key from a read-only JSON document. |

##### `json set`

```
Set a key to a value in the JSON document.

Usage: set <key> <value>

Note: All values are stored as strings.
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

##### `log key`

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
Marks object for deletion

Usage: delete

Marks the object for deletion on the next update cycle.
```

##### `mirror fetch`

##### `mirror off`

##### `mirror on`

##### `mirror once`

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
Set a key to a value in the JSON document.

Usage: set <key> <value>

Note: All values are stored as strings.
```

#### `reload-texture`

```
Reload the texture from the document.

Usage: reload-texture
```

#### `rotate`

```
Rotate the texture by a specified angle.

Usage: rotate <angle>

Arguments:
  angle   The angle in degrees to rotate the texture.
```

#### `ruleset`

Available Functions

| Function | Description |
|----------|-------------|
| `help` | Show available commands and their descriptions |
| `once` | Applies all rulesets once on the next update |
| `reload` | Reloads all rulesets for this RenderObject on the next update. |

##### `ruleset once`

##### `ruleset reload`

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

## JSON Transformations Commands

These commands are available during JSON value retrieval with the transformation operator '|'.
Example: `{global.var|length}`

Help for JSON rvalue transformation FuncTree

Available Functions

| Function | Description |
|----------|-------------|
| `add` | Adds a numeric value to the current JSON value. |
| `assertNonEmpty` | Asserts that the current JSON value is non-empty. |
| `at` | Gets the element at the specified index from the array in the current JSON value. |
| `deserialize` | Deserializes the current JSON string value. |
| `echo` | Echoes the provided arguments to the console, with newline. |
| `ensureArray` | Ensures the current JSON value is an array. |
| `first` | Gets the first element of the array in the current JSON value. |
| `get` | Gets the value at the specified key from the current JSON object. |
| `help` | Show available commands and their descriptions |
| `last` | Gets the last element of the array in the current JSON value. |
| `length` | Gets the length of the array in the current JSON value. |
| `map` | Applies a mapping function to each element in the array of the current JSON value. |
| `mod` | Calculates the modulo of the current JSON value by a numeric value. |
| `mul` | Multiplies the current JSON value by a numeric value. |
| `nebs` | Parses a Nebulite Script command on the JSON |
| `pow` | Raises the current JSON value to the power of a numeric value. |
| `print` | Prints the current JSON value to the console. |
| `reverse` | Reverses the array in the current JSON value. |
| `serialize` | Serializes the current JSON value to a string. |
| `toBool` | Converts the current JSON value to a boolean. |
| `toBoolString` | Converts the current JSON value to a boolean string. |
| `toDouble` | Converts the current JSON value to a double. |
| `toInt` | Converts the current JSON value to an integer. |
| `toString` | Converts the current JSON value to a string. |
| `typeAsNumber` | Converts the current JSON type value to a number. |
| `typeAsString` | Converts the current JSON type value to a string. |

#### `add`

```
Adds a numeric value to the current JSON value.
Usage: |add <number1> <number2> ... -> {number}
```

#### `assertNonEmpty`

```
Asserts that the current JSON value is non-empty.
If the value is empty, the transformation fails and the program exits
Usage: |assertNonEmpty -> {value,<Exception thrown if empty>}
```

#### `at`

```
Gets the element at the specified index from the array in the current JSON value.
If the index is out of bounds, the transformation fails.
Usage: |at <index> -> {value}
```

#### `deserialize`

```
Deserializes the current JSON string value.
Usage: |deserialize -> {value}
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

#### `map`

```
Applies a mapping function to each element in the array of the current JSON value.
If the current value is not an array, it is first wrapped into a single-element array.
Usage: |map <function> -> {array}
```

#### `mod`

```
Calculates the modulo of the current JSON value by a numeric value.
Usage: |mod <number> -> {number}
```

#### `mul`

```
Multiplies the current JSON value by a numeric value.
Usage: |multiply <number1> <number2> ...
```

#### `nebs`

```
Parses a Nebulite Script command on the JSON
Usage: |nebs <command> -> {value}
```

#### `pow`

```
Raises the current JSON value to the power of a numeric value.
Usage: |pow <exponent> -> {number}
```

#### `print`

```
Prints the current JSON value to the console.
Usage: |print -> {unchanged-json}
```

#### `reverse`

```
Reverses the array in the current JSON value.
If the current value is not an array, it is first wrapped into a single-element array.
Usage: |reverse -> {array}
```

#### `serialize`

```
Serializes the current JSON value to a string.
Usage: |serialize -> {string}
```

#### `toBool`

```
Converts the current JSON value to a boolean.
Usage: |toBool -> {bool}
'true' values: true, 1, '1', 'true', 'yes', 'on' (case-insensitive)
'false' values: false, 0, '0', 'false', 'no', 'off' (case-insensitive)
Any other value defaults to false.
```

#### `toBoolString`

```
Converts the current JSON value to a boolean string.
Usage: |toBoolString -> {string}
Either 'true' or 'false'
```

#### `toDouble`

```
Converts the current JSON value to a double.
Usage: |toDouble -> {number}
Non-numeric values default to 0.0.
```

#### `toInt`

```
Converts the current JSON value to an integer.
Never fails, defaults to 0 if the provided value is non-numeric.
Usage: |toInt -> {number}
```

#### `toString`

```
Converts the current JSON value to a string.
Never fails, defaults to an empty string if no conversion is possible.
Usage: |toString -> {string}
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

