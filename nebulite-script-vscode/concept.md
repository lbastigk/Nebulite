# VSCODE Nebulite script language extension

Goal: Vscode language extension on custom language.
On file `.nebs`.

## Project Structure

```bash
leo@leo-x670aoruseliteax:~/Projects/Nebulite$ tree -L 1
.
├── Application
├── build
├── build.sh
├── CMakeLists.txt
├── CONTRIBUTING.md
├── doc
├── external
├── include
├── install.sh
├── LICENSE.md
├── mingw-toolchain.cmake
├── nebulite-script-vscode  # The directory of the extension project
├── README.md
├── scripts
└── src

9 directories, 7 files
leo@leo-x670aoruseliteax:~/Projects/Nebulite$ 
```

## Examples

see `Application/TaskFiles/.../...*.nebs` for example files

## Comments

Comments start with `#` and encompass everything from the keyword to the line ending.

## Keywords

using a python script or similiar to extract all function names provided by the engine

From all files `{JTE,GTE,RTE}_*.h` in `./include/` we extract lines starting with (whitespace and tab stripped) `bindFunction(...)` where we extract the function name:

`bindFunction(&Audio::listDevices,       "list-devices",         "List audio devices");` -> Keyword is `list-devices`. From line, find first string "..." for function-name

All keywords are highlighted by the extension.

## Links

Links in a `.nebs` file are always indicated by `./` and go up until, but not including: 
- a whitespace
- a '|' character

Links should be openable, but not from the current directory, but from a specified one via a settings file.
This is because all links are opened relative from the working directory of the binary, so we should specify what we mean,
in case the editor working directory differs.

## Expressions

Expressions themselfes are not directly indicated, as many function make indirect use of them. However, this isn't a problem as the sub-parts of expressions are easy to handle.

A full expression is a mix of text, a variable reference and an evaulation:

`This is a full Nebulite expression! We can get variables like {global.time.t} or evaluate them mathematically: $( {global.time.t} + 1 )`

### Variables

Variables are indicated by `{...}` and should be fully highlighted. The may link to a file and its keys as well
- `{global.time.t}`
- `{./Resources/docs/myFile.jsonc:key1.key2}`

### Evaluations

Evaluations are indicated by `$<format>(<string>)` and should be fully highlighted.
- `$i(1+1)`
- `$f( {global.time.t} + 1 )`

## Specials

- String indicators are not used. We do not need to worry about them
- Highlight everything after `echo/error/<potentially more keywords>` for a line as string
- Set configurable colors for each type, if possible:
  - Comment (after echo etc.)
  - Variable
  - Expression
  - Keywords