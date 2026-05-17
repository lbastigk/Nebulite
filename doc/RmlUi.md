# Nebulite RmlUi integration

## Nebulite Data/Conditional Attributes

Nebulite offers support for various Rml-Attributes to use the context-provided data in meaningful ways:
please ensure that the body of your document has the `data-model="nebuliteDataSync"` attribute, 
as some of the attributes rely on the data synchronization between Rml and Nebulite to work properly.

- `data-eval`: Evaluates the inner Rml of the element and replaces the content of the element with the result.
The syntax for the value is: `true/false`
- `data-value`: Provides Rml with a variable from the context to sync. This is required for text inputs.
The syntax for the value is: `context:key`.
- `data-if`: The inner Rml of the element will only be rendered if the value of the attribute is truthy.
The syntax for the value is: `context:key|optionalTransformations`.
- `if`: A more powerful version of `data-if` that supports Nebulite expressions.
The syntax for the value is any valid Nebulite expression. Note that the result is converted to a boolean.
- `data-reflect`: The inner Rml of the element is repeated for each item in the provided list.
The syntax for the value is any Nebulite expression. But as the expected output is an array, we must use: 
`{context:key|optionalTransformations}`. The braces are required to indicate a variable access.

## Supported Rml attribute commands

Nebulites RmlUi integration allows you to easily create interactive UI elements 
without having to write a lot of code. Attribute-commands are a powerful way to add interactivity to your UI elements.

Each attribute command is split into a trigger and a specialization: `<trigger>#<specialization>`.
The trigger specifies when the command should be executed, and the specialization specifies what should be executed.
The context of the action is determined by the element first, then the document if no element context is available.

### Triggers

- `onClick`: The command will be executed when the element is clicked.
- `onDestroy`: The command will be executed when the element is destroyed. This includes when the document is deleted, or when the element is removed from the document.
- `onEnter`: The command will be executed if the element is in focus and the user presses the enter key.

### Specializations

- `invokeRuleset` Expects a ruleset name or link as a parameter, which will be invoked
- `parse` The given parameter will be parsed into the `self` domain. Supports multiple lines of code, separated by `;`.
See [Commands.md](Commands.md) for supported domain commands.
- `special` Expects a special command as a parameter, which will be executed

### Supported special commands

Supported special commands are:
- `blurElement`: Blurs the currently focused element. Note that this might not be the element that the command is attached to, but the currently focused element in the document.
- `deleteDocument`: Deletes the document of the element

### Examples

```html
<rml>
    <head>
        <title>Rml attribute commands example</title>
        <link type="text/rcss" href="./external/RmlUi/Samples/assets/rml.rcss"/>
        <link type="text/rcss" href="./Resources/Rcss/window.rcss"/>
        <link type="text/rcss" href="./Resources/Rcss/text.rcss"/>
    </head>
    <body data-model="nebuliteDataSync" style="width: 800px; left:0%; width : 250px ; height: 100px">
    <h2>Please enter your name</h2>

    <!-- Set actions to do on destruction -->
    <!-- Any onDestroy is activated when the element is removed from the document -->
    <!-- Or, if the document itself is removed. -->
    <div onDestroy#invokeRuleset="::debug::message"></div>
    <div onDestroy#parse="set nameSet 1 ; assert $(eq({self:nameSet},1))"></div>

    <!-- Text input with onEnter actions -->
    <p>
        <input
                type="text"
                data-value="self:name"
                onEnter#parse="assign self:name = {self:name|capitalize} ; eval echo Hello {self:name}!"
                onEnter#special="deleteDocument"
        />
    </p>
    </body>
</rml>
```