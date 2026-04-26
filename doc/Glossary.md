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
### Context / ContextScope
In Nebulite, any interaction is performed in a specific context. This context is twofold:
- Context - the domains of the interaction
- ContextScope - the JSON document scopes of the interaction

The following context members are supported:
- self - domain broadcasting the logic
- other - domain listening
- global - domain acting as a global storage and coordinator for the engine

Furthermore, access to read-only documents is supported inside expressions.
Examples for access in Expressions:
- `{self:posX}` would access the posX variable of the context self
- `{other:posX}` would access the posX variable of the context other
- `{global:someVar}` would access the someVar variable of the context global
- `{./link/to/file.json:key}` would access a read-only document

However, context may be combined during variable access:
- `{local:}` combines self and other
- `{all:}` combines self, other and global

where the name of the context member is the key prefix.

See also: [Expression](#expression), [Domain](#domain), [GlobalSpace](#globalspace)

-----------------
## D

-----------------
### Document
A JSON document.

See also: [JSON](#json)

-----------------
### Domain
A specialized class that is the basis for all interactions in the Nebulite framework.
Each domain is defined by a JSON document and a set of functions and variables acting on this document/the class itself.
Domains are used for all interactions in the engine, such as rendering, texture management, environment control etc.
The following domains are currently supported:
- Environment
- GlobalSpace
- Renderer
- RenderObject
- Texture

Any domain may have multiple DomainModules attached to it, providing additional functionality,
such as time management inside the GlobalSpace domain, debugging tools or texture modifications.
All domains support string parsing through `parseStr` from other domains and supported classes.

See also: [DomainModule](#domainmodule), [FuncTree](#functree), [GlobalSpace](#globalspace), [JsonScope](#jsonscope)

### Domain-Inheritance

Domains may inherit functions from each other, allowing for shared data, functionality and properties.

-----------------
### Domain-Serialization-Piping

A feature that allows for Functioncalls to be piped into a domain directly after deserialization.
Unlike JSON-Transformations, the calls are for domain functions and not for JSON-Transformation-functions.
Other than that, the syntax is the same.

Example: `spawn Planets/sun.jsonc|set posX 500|set posY 100` would serialize the file `Planets/sun.jsonc` into a RenderObject, 
then execute the Functioncalls `set posX 500` and `set posY 100` on it.

See also: [FuncTree](#functree), [JSON](#json), [Transformation](#transformation)

-----------------
### DomainModule
Special class that contains a list of functions and variables as well as an update-routine that are bound 
to a specific Domain. DomainModules have a restricted access to the Domain's internal JSON document, 
allowing for safe and modular functionality extensions.
DomainModules allow for easy separation of functionality. 
Example: one class for audio, another for inputs etc. 

See also: [Domain](#domain), [FuncTree](#functree), [JSON](#json), [JsonScope](#jsonscope)

-----------------
## E

-----------------
### Evaluation-delay
Allows for expression evaluation to be delayed by a set number of evaluation steps.
Example:
```
set workspace.arr[0] 1
set workspace.arr[1] 2
set workspace.arr[2] 3
eval nop {global:workspace|setFromResult arraySize {arr|length|toInt}|print}
eval nop {global:workspace|setFromResult arraySize {1!arr|length|toInt}|print}
exit
```
The first eval fails, because the expression evaluation tries to evaluate `{arr|length|toInt}`
before the `setFromResult` function is executed, leading to the setFromResult function to get the arguments:
`arraySize` and `null` instead of `arraySize` and `{arr|length|toInt}`.

See also: [Expression](#expression), [FuncTree](#functree), [Transformation](#transformation)

-----------------
### Expression
Inside the Nebulite Framework, an expression is any string that 
Nebulite::Interaction::Logic::Expression can compile. This includes any mix of:
- Text
- Variable: `{self:var}`
- Expression: `$(1+1)`

Expressions may include variables. Certain conditions must be met for an expression to compile. 
See the documentation of the Expression class for more information. 

Examples:
- `$(1+1)` would evaluate to `2`
- `My name is {self:name}` would evaluate to `My name is John` if `{self:name}` = `John`
- `I have {self:inventory|length} items` would evaluate to `I have 5 items` if `{self:inventory}` is an array of length 5.

See also: [Context](#context), [VirtualDouble](#virtualdouble), [Multiresolve](#multiresolve)

-----------------
### ExpressionPool
Allows for parallel computation of a single Expression. 

See also: [Expression](#expression), [Context](#context)

-----------------
## F

-----------------
### Functioncall
Execution of a bound function with its arguments through FuncTree string parsing.

See also: [FuncTree](#functree)

-----------------
### FuncTree
Central function parsing logic. Allows for Functions of signature 

`returnValue foo(std::span<std::string const> const& args, additionalArgs...)` 

to be bound to a keyword and executed with this keyword and its arguments. 

Example: `funcTree.parse("<from> myFunction arg1 arg2")`. Where `<from>` is argv[0]; 
useful for parsing where the Functioncall came from so we can create error messages like: 
`"Error on execution from" + argv[0] + "..."`. 

This allows for executable logic such as `set-fps 60`, `set posX 100` or `spawn Planets/sun.jsonc|set posX 500|set posY 100`

-----------------
## G

-----------------
### GlobalSpace
Nebulite Domain. The core engine controller class that serves as the central coordination point 
for all engine operations. Singleton access granted via `Global::instance()`.
Includes modules for:
- Time management
- Settings reading and writing
- Testing utilities
- Debugging tools
- Input mapping

-----------------
## H

-----------------
## I

-----------------
### Invoke
Central object manipulation class using Ruleset objects for global interactions between Domains.
On valid broadcast, listen and logical condition, the Ruleset is executed. 

See also: [Ruleset](#p)

-----------------
## J

-----------------
### JSON
A custom wrapper around rapidjson for faster variable access. 
Used for arbitrary variable storing and retrieval in Nebulite Domains.
Allows users to get a stable pointer to a numeric value (type double) for even faster access and modification.

-----------------
### JsonScope
Part of a JSON document. Any data access is performed with a key prefix.
It serves as a scoped view over an existing JSON document, used for modular data management.
If a scoped key is used on a JsonScope without a matching prefix, the program exits with an error.
This is not an issue for user-defined inputs, as they are always prefixed with the scope they are used in.
But it is a safety measure for hardcoded keys to enforce encapsulation of DomainModules 
and prevent accidental access to the wrong variables.

See also: [DomainModule](#domainmodule) [JSON](#json), [ScopedKey](#scopedkey)

-----------------
## K

-----------------
## L

-----------------
## M

-----------------
### Multiresolve
An expression feature that allows nested variable access: 
`{global:mirror.id{self:id}.posX}` would evaluate to `{global:mirror.id1.posX}` first, 
if `{self:id}` = `1`. Then, `{global:mirror.id1.posX}` is evaluated. 
Evaluation may be delayed with the Evaluation-delay feature.

See also: [Evaluation-delay](#evaluation-delay), [Context](#context), [Expression](#expression)

-----------------
## N

-----------------
### nebs
Nebulite Script. A custom script file format for executing FuncTree commands.
All attached functions from DomainModules and inherited Domains may be used. 
Either through a TaskFile or a Ruleset.

If a TaskFile is called from command line arguments or the Console, it is executed within the GlobalSpace domain.

See also: [DomainModule](#domainmodule), [FuncTree](#functree), [Invoke](#invoke), [Ruleset](#ruleset)

-----------------
## O

-----------------
## P

-----------------
## Q

-----------------
## R

-----------------
### RenderObject
Nebulite Domain. Represents a renderable entity in the Nebulite engine. 
Stores all properties in an internal JSON document, supports dynamic property access, 
manages SDL rendering (sprites/text) and handles ruleset-dispatching through a DomainModule.
Core building block for visual game objects. 

See also: [Domain](#domain), [JSON](#json), [Invoke](#invoke)

-----------------
### Ruleset
A function or JSON-File describing interactions for a given Context.
JSON based rulesets contain assignments and function calls to be executed on valid broadcast/listen conditions.
Static rulesets are hardcoded C++ functions for performance-critical logic.

See also: [Context](#context), [Invoke](#invoke), [RenderObject](#renderobject)

-----------------
## S

### ScopedKey
A key with an attached scope. If any `JsonScope` attempts to access a variable with a ScopedKey,
it is first checked whether the scope matches the required scope. If not, the access fails and the program exits.

See also: [JsonScope](#jsonscope)

-----------------
## T

-----------------
### Texture
Nebulite Domain. Wrapper Domain for an SDL-Texture. 

See also: [Domain](#domain)

-----------------
### Transformation
During JSON variable access, Transformations may be applied to the retrieved value.
Transformations are supported for any JSON variable access.
Example: `{self:var|add 5}` would retrieve `{self:var}` and apply the add transformation to it before returning the value.

See also: [Evaluation-delay](#evaluation-delay) [JSON](#json), [Multiresolve](#multiresolve)

-----------------
## U

-----------------
## V

-----------------
### VirtualDouble
Wrapper class providing consistent access to double values from arbitrary JSON documents.
The value is copied and stored internally, allowing for a stable pointer access even if the JSON document changes.
This is used in the Expression system to register variables for tinyexpr.

See also: [Expression](#expression), [JSON](#json)

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