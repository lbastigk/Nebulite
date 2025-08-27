# Nebulite Dictionary

All definitions used within the Nebulite framework. While this includes definitions of some classes, it's not intended as a full overview of all of them. Rather, it reflects commonly used names within its documentation and shall be used to further streamline its wording.

-----------------

## A

## B

## C

**Context** - In Nebulite, each Expression has access to 3 domains defined by keywords: *self*: the `RenderObject` broadcasting the logic, *other*: the `RenderObject` listening, *global*: the `GlobalSpace`.

## D

**Document** - A `JSON` document.

**Domain** - A class on which a FuncTree acts upon. Supported domains are: `GlobalSpace`, `RenderObject`, `JSON`.

**DomainModule** - Class that contains a list of functions and variables that are bound to a specific domain as well as an update-routine. See `GDM_*.h`, `RDM_*.h` and `JDM_*.h` for examples. DomainModules allow for easy separation of functionality. Example: one class for audio, another for inputs etc.

## E

**Expression** - Inside the Nebulite Framework, an expression is any string that `Nebulite::Interaction::Logic::Expression` can compile. This includes any mix of:
- Text
- Variable: `{self.var}`
- Expression `$(1+1)`

Expressions may include variables. Certain conditions must be met for an expression to compile. See the documentation of the `Expression` class for more information.

## F

**Functioncall** - Execution of a bound function with its arguments through `FuncTree` parsing.

**FuncTree** - Central function parsing logic. Allows for Functions of signature `RETURN_TYPE foo(int argc, char** argv)` to be bound to a keyword and executed with this keyword and its arguments. Example: `funcTree.parse("<from> myFunction arg1 arg2")`. Where `<from>` is `argv[0]`; useful for parsing where the `Functioncall` came from so we can create error messages like: `"Error on execution from" + argv[0] + "..."`

## G

## H

## I

**Invoke** - Central object manipulation Class. Invokes may be defined as `ParsedEntries` through `JSON` files and attached to any RenderObject. On valid broadcast, listen and logical condition, its Assignments and `FunctionCalls` are executed. The wording `Invoke` is also sometimes used to describe those JSON `ParsedEntries`.

## J

**JSON** - In Nebulites context, a custom wrapper around `rapidjson` for faster variable access. Used for arbitrary variable storing and retrieval in Nebulite `Domains`.

## K

## L

## M

## N

**nebs** - Nebulite Script. Used for FuncTree parsing. All attached functions from `DomainModules` may be used. Either through a `TaskFile` or an `Invoke` 

## O

## O

## P

**ParsedEntry** - A single instance of a parse `Invoke` `JSON` file. See `Invoke`.

## Q

## R

## S

## T

## U

## V

## W

## X

## Y

## Z

## \#