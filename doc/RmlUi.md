# Nebulite RmlUi integration

## Supported Rml attribute commands

Nebulites RmlUi integration allows you to easily create interactive UI elements 
without having to write a lot of code. Attribute-commands are a powerful way to add interactivity to your UI elements.

Each attribute command is split into a trigger and a specialization: <trigger>-<specialization> 
The trigger specifies when the command should be executed, and the specialization specifies what should be executed.
The context of the action is determined by the element first, then the document if no element context is available.

### Triggers

- `on-destroy`: The command will be executed when the element is destroyed. This includes when the document is deleted, or when the element is removed from the document.
- `on-enter`: The command will be executed if the element is in focus and the user presses the enter key.

### Specializations

- `invoke-ruleset` Expects a ruleset name or link as a parameter, which will be invoked
- `parse` The given parameter will be parsed into the `self` domain
- `special` Expects a special command as a parameter, which will be executed

### Supported special commands

Supported special commands are:
- `blurElement`: Blurs the element, causing it to lose focus if it has it.
- `deleteDocument`: Deletes the document of the element
