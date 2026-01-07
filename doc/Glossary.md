# Nebulite Glossary

All definitions used within the Nebulite framework. While this includes definitions of some classes, 
it's not intended as a full overview of all of them. 
Rather, it reflects commonly used names within its documentation and shall be used to further streamline its wording.

-----------------

[A](#a)
[B](#b)
[C](#c)
[D](#d)
[E](#e)
[F](#f)
[G](#g)
[H](#h)
[I](#i)
[J](#j)
[K](#k)
[L](#l)
[M](#m)
[N](#n)
[O](#o)
[P](#p)
[Q](#q)
[R](#r)
[S](#s)
[T](#t)
[U](#u)
[V](#v)
[W](#w)
[X](#x)
[Y](#y)
[Z](#z)

-----------------
## A

-----------------
## B

-----------------
## C

-----------------
**Context** - In Nebulite, each Expression/Ruleset has access to 3 domains defined by keywords: 
- *self*: the `RenderObject` broadcasting the logic
- *other*: the `RenderObject` listening
- *global*: the `GlobalSpace`
- as well as read-only Resources. 

See also: [Expression](#e), [Domain](#d), [GlobalSpace](#g)

-----------------
## D

-----------------
**Document** - A `JSON` document. 

See also: [JSON](#j)

-----------------
**Domain** - A class on which a FuncTree acts upon. Supported domains are: 
- `Environment`
- `GlobalSpace`
- `JsonScope`
- `Renderer`
- `RenderObject`
- `Texture`

Any domain may have multiple `DomainModules` attached to it, providing additional functionality.
All domains provide access to an internal `JSON` document for variable storage and retrieval.
All domains support string parsing through `parseStr` from other domains and supported classes.

See also: [DomainModule](#d) [FuncTree](#f), [GlobalSpace](#g), [JsonScope](#j)

-----------------
**DomainModule** - Special Class that contains a list of functions and variables that are bound 
to a specific Domain as well as an update-routine. 
DomainModules allow for easy separation of functionality. 
Example: one class for audio, another for inputs etc. 

See also: [Domain](#d)

-----------------
## E

-----------------
**Expression** - Inside the Nebulite Framework, an expression is any string that 
`Nebulite::Interaction::Logic::Expression` can compile. This includes any mix of:
- Text
- Variable: `{self.var}`
- Expression `$(1+1)`

Expressions may include variables. Certain conditions must be met for an expression to compile. 
See the documentation of the Expression class for more information. 

See also: [Context](#c), [VirtualDouble](#v), [Multiresolve](#m)

-----------------
**ExpressionPool** - Allows for parallel computation of Expressions with different other-instances. 

See also: [Expression](#e), [Context](#c)

-----------------
## F

-----------------
**Functioncall** - Execution of a bound function with its arguments through `FuncTree` parsing.

-----------------
**FuncTree** - Central function parsing logic. Allows for Functions of signature 
`RETURN_TYPE foo(std::span<std::string const> const& args, AdditionalArgs)` 
to be bound to a keyword and executed with this keyword and its arguments. 

Example: `funcTree.parse("<from> myFunction arg1 arg2")`. Where `<from>` is `argv[0]`; 
useful for parsing where the `Functioncall` came from so we can create error messages like: 
`"Error on execution from" + argv[0] + "..."`. 

This allows for executable logic such as `set-fps 60`, `set posX 100` or  `spawn Planets/sun.jsonc|set posX 500|set posY 100`

-----------------
## G

-----------------
**GlobalSpace** - Nebulite Domain. The core engine controller class that serves as the central coordination point 
for all engine operations. Singleton access granted via `Nebulite::global()`.

-----------------
## H

-----------------
## I

-----------------
**Invoke** - Central object manipulation Class. Invokes may be defined as `ParsedEntries` through `JSON` files 
and attached to any RenderObject. 
On valid broadcast, listen and logical condition, its Assignments and `FunctionCalls` are executed. 

See also: [Ruleset](#p)

-----------------
## J

-----------------
**JSON** - A custom wrapper around `rapidjson` for faster variable access. 
Used for arbitrary variable storing and retrieval in Nebulite `Domains`.

-----------------
**JsonScope** - Nebulite Domain. Part of a `JSON` document. Any data access is performed with a key prefix.

-----------------
## K

-----------------
## L

-----------------
## M

-----------------
**Multiresolve** - A feature that allows nested variable access: 
`{global.mirror.id{self.id}.posX}` would evaluate to `{global.mirror.id1.posX}` first, 
if `{self.id}=1`. Then, `{global.mirror.id1.posX}` is evaluated.

-----------------
## N

-----------------
**nebs** - Nebulite Script. Used for FuncTree parsing. 
All attached functions from `DomainModules` may be used. 
Either through a `TaskFile` or an `Invoke` 

-----------------
## O

-----------------
## P

-----------------
## Q

-----------------
## R

-----------------
**RenderObject** - Nebulite Domain. Represents a renderable entity in the Nebulite engine. 
Stores all properties in an internal JSON document, supports dynamic property access, 
manages SDL rendering (sprites/text), handles invoke commands for interactions. 
Core building block for visual game objects. 

See also: [Domain](#d), [JSON](#j), [Invoke](#i)

-----------------
**Ruleset** - A function or JSON-File describing interactions in a self-other-global model. 

See also: [Invoke](#i)

-----------------
## S

**ScopedKey** - A key with an attached scope. If any JsonScope attempts to access a variable with a ScopedKey,
it is first checked whether the scope matches. If not, the access fails and the program exits.

See also: [JsonScope](#j)

-----------------
## T

-----------------
**Texture** - Nebulite Domain. Wrapper Domain for an SDL-Texture. 

See also: [Domain](#d)

-----------------
**Transformation** - During JSON variable access, Transformations may be applied to the retrieved value. 
Example: `{self.var|add 5}` would retrieve `self.var` and apply the `add` transformation to it before returning it. 
Transformations are applied after all variable resolving is done, including Multiresolve. 

See also: [Multiresolve](#m)

-----------------
## U

-----------------
## V

-----------------
**VirtualDouble** - Wrapper class providing consistent access to double values in JSON documents with `tinyexpr` compatibility. 
Supports both remanent contexts (stable documents like `self`, `global`) and non-remanent contexts (dynamic documents like `other`) 
for efficient expression evaluation.

-----------------
## W

-----------------
## X

-----------------
## Y

-----------------
## Z

-----------------
## \#