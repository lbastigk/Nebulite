#include "Interaction/Execution/FuncTreeErrorMessages.hpp"

#include "Nebulite.hpp"

namespace Nebulite::Interaction::Execution {

//------------------------------------------
// Bind error messages

void BindErrorMessage::invalidFunctionName(std::string_view const& name) {
    Error::println("---------------------------------------------------------------");
    Error::println("A Nebulite FuncTree binding failed!");
    Error::println("Error: Function name '", name, "' is invalid.");
    Error::println("Function names cannot be empty or contain whitespace characters.");
    throw std::runtime_error("FuncTree binding failed due to invalid function name.");
}

void BindErrorMessage::missingCategory(std::string_view const& tree, std::string_view const& category, std::string_view const& function) {
    Error::println("---------------------------------------------------------------");
    Error::println("A Nebulite FuncTree binding failed!");
    Error::println("Error: Category '", category, "' does not exist when trying to bind function '", function, "'.");
    Error::println("Please create the category hierarchy first using bindCategory().");
    Error::println("This Tree: ", tree);
    throw std::runtime_error("FuncTree binding failed due to missing category.");
}

void BindErrorMessage::functionShadowsCategory(std::string_view const& function) {
    Error::println("---------------------------------------------------------------");
    Error::println("A Nebulite FuncTree binding failed!");
    Error::println("Error: Cannot bind function '", function, "' because a category with the same name already exists.");
    throw std::runtime_error("FuncTree binding failed due to function shadowing category.");
}

void BindErrorMessage::functionExistsInInheritedTree(std::string_view const& tree, std::string_view const& inheritedTree, std::string_view const& function) {
    Error::println("---------------------------------------------------------------");
    Error::println("A Nebulite FuncTree binding failed!");
    Error::println("Error: A bound Function already exists in the inherited FuncTree.");
    Error::println("Function overwrite is heavily discouraged and thus not allowed.");
    Error::println("Please choose a different name or remove the existing function.");
    Error::println("This Tree: ", tree);
    Error::println("inherited FuncTree:   ", inheritedTree);
    Error::println("Function:  ", function);
    throw std::runtime_error("FuncTree binding failed due to function existing in inherited tree.");
}

void BindErrorMessage::parentCategoryDoesNotExist(std::string_view const& name, std::string_view const& currentCategoryName) {
    Error::println("---------------------------------------------------------------");
    Error::println("A Nebulite FuncTree binding failed!");
    Error::println("Error: Cannot create category '", name, "' because parent category '", currentCategoryName, "' does not exist.");
    throw std::runtime_error("FuncTree binding failed due to missing parent category.");
}

void BindErrorMessage::categoryExists(std::string_view const& name) {
    Error::println("---------------------------------------------------------------");
    Error::println("A Nebulite FuncTree binding failed!");
    Error::println("Error: Cannot create category '", name, "' because it already exists.");
    throw std::runtime_error("FuncTree binding failed due to category already existing.");
}

void BindErrorMessage::functionExists(std::string_view const& tree, std::string_view const& function) {
    Error::println("---------------------------------------------------------------");
    Error::println("A Nebulite FuncTree binding failed!");
    Error::println("Error: A bound Function already exists in this tree.");
    Error::println("Function overwrite is heavily discouraged and thus not allowed.");
    Error::println("Please choose a different name or remove the existing function.");
    Error::println("This Tree: ", tree);
    Error::println("Function:  ", function);
    throw std::runtime_error("FuncTree binding failed due to function already existing.");
}

void BindErrorMessage::variableHasWhitespace(std::string_view const& tree, std::string_view const& variable) {
    Error::println("---------------------------------------------------------------");
    Error::println("A Nebulite FuncTree binding failed!");
    Error::println("Error: Variable name '", variable, "' cannot contain whitespaces.");
    Error::println("This Tree: ", tree);
    throw std::runtime_error("Failed to bind variable due to invalid name.");
}

void BindErrorMessage::variableExists(std::string_view const& tree, std::string_view const& variable) {
    Error::println("---------------------------------------------------------------");
    Error::println("A Nebulite FuncTree binding failed!");
    Error::println("Error: Variable '", variable, "' is already bound.");
    Error::println("This Tree: ", tree);
    throw std::runtime_error("Failed to bind variable due to name conflict.");
}

//------------------------------------------
// Execution error messages

void ExecutionErrorMessage::unknownVariable(std::string_view const& tree, std::string_view const& variable) {
    Error::println("Unknown variable '", variable, "' in FuncTree '", tree, "'!");
    Error::println("Make sure the variable is bound to the FuncTree before using it.");
}

void ExecutionErrorMessage::functionNotFound(std::string_view const& tree, std::string_view const& function, std::string_view const& arguments) {
    Error::println("Function '", function, "' not found in FuncTree '", tree, "', its inherited FuncTrees or their categories!");
    Error::println("Arguments: ", arguments);
}



} // namespace Nebulite::Interaction::Execution
