# Nebulite RmlUi integration

## Supported Rml attribute commands

Nebulites RmlUi integration allows you to easily create interactive UI elements 
without having to write a lot of code. Attribute-commands are a powerful way to add interactivity to your UI elements.

Each attribute command is split into a trigger and a specialization: `<trigger>#<specialization>`.
The trigger specifies when the command should be executed, and the specialization specifies what should be executed.
The context of the action is determined by the element first, then the document if no element context is available.

### Triggers

- `onDestroy`: The command will be executed when the element is destroyed. This includes when the document is deleted, or when the element is removed from the document.
- `onEnter`: The command will be executed if the element is in focus and the user presses the enter key.

### Specializations

- `invokeRuleset` Expects a ruleset name or link as a parameter, which will be invoked
- `parse` The given parameter will be parsed into the `self` domain
- `special` Expects a special command as a parameter, which will be executed

### Supported special commands

Supported special commands are:
- `blurElement`: Blurs the currently focused element. Note that this might not be the element that the command is attached to, but the currently focused element in the document.
- `crash`: Crashes the game immediately.
- `debugError`: Logs an error message to the console. This can be used to test error handling in your UI.
- `debugLog`: Logs a message to the console. This can be used to test logging in your UI.
- `debugWarn`: Logs a warning message to the console. This can be used to test warning handling in your UI.
- `deleteDocument`: Deletes the document of the element
