//------------------------------------------
// Includes

// Nebulite
#include "Interaction/Execution/FuncTreeErrorMessages.hpp"
#include "Nebulite.hpp"

//------------------------------------------
namespace Nebulite::Interaction::Execution {

//------------------------------------------
// Bind error messages

void BindErrorMessage::invalidFunctionName(std::string_view const& name) {
    Global::capture().error.println("---------------------------------------------------------------");
    Global::capture().error.println("A Nebulite FuncTree binding failed!");
    Global::capture().error.println("Error: Function name '", name, "' is invalid.");
    Global::capture().error.println("Function names cannot be empty or contain whitespace characters.");
    throw std::runtime_error("FuncTree binding failed due to invalid function name.");
}

void BindErrorMessage::missingCategory(std::string_view const& tree, std::string_view const& category, std::string_view const& function) {
    Global::capture().error.println("---------------------------------------------------------------");
    Global::capture().error.println("A Nebulite FuncTree binding failed!");
    Global::capture().error.println("Error: Category '", category, "' does not exist when trying to bind function '", function, "'.");
    Global::capture().error.println("Please create the category hierarchy first using bindCategory().");
    Global::capture().error.println("This Tree: ", tree);
    throw std::runtime_error("FuncTree binding failed due to missing category.");
}

void BindErrorMessage::functionShadowsCategory(std::string_view const& function) {
    Global::capture().error.println("---------------------------------------------------------------");
    Global::capture().error.println("A Nebulite FuncTree binding failed!");
    Global::capture().error.println("Error: Cannot bind function '", function, "' because a category with the same name already exists.");
    throw std::runtime_error("FuncTree binding failed due to function shadowing category.");
}

void BindErrorMessage::functionShadowsVariable(std::string_view const& function) {
    Global::capture().error.println("---------------------------------------------------------------");
    Global::capture().error.println("A Nebulite FuncTree binding failed!");
    Global::capture().error.println("Error: Cannot bind function '", function, "' because a variable with the same name already exists.");
    throw std::runtime_error("FuncTree binding failed due to function shadowing variable.");
}

void BindErrorMessage::functionExistsInInheritedTree(std::string_view const& tree, std::string_view const& inheritedTree, std::string_view const& function) {
    Global::capture().error.println("---------------------------------------------------------------");
    Global::capture().error.println("A Nebulite FuncTree binding failed!");
    Global::capture().error.println("Error: A bound Function already exists in the inherited FuncTree.");
    Global::capture().error.println("Function overwrite is only allowed if the function it points to is still the same.");
    Global::capture().error.println("Please choose a different name or remove the existing function.");
    Global::capture().error.println("This Tree: ", tree);
    Global::capture().error.println("inherited FuncTree:   ", inheritedTree);
    Global::capture().error.println("Function:  ", function);
    throw std::runtime_error("FuncTree binding failed due to function existing in inherited tree.");
}

void BindErrorMessage::parentCategoryDoesNotExist(std::string_view const& name, std::string_view const& currentCategoryName) {
    Global::capture().error.println("---------------------------------------------------------------");
    Global::capture().error.println("A Nebulite FuncTree binding failed!");
    Global::capture().error.println("Error: Cannot create category '", name, "' because parent category '", currentCategoryName, "' does not exist.");
    throw std::runtime_error("FuncTree binding failed due to missing parent category.");
}

void BindErrorMessage::categoryExists(std::string_view const& name) {
    Global::capture().error.println("---------------------------------------------------------------");
    Global::capture().error.println("A Nebulite FuncTree binding failed!");
    Global::capture().error.println("Error: Cannot create category '", name, "' because it already exists.");
    throw std::runtime_error("FuncTree binding failed due to category already existing.");
}

void BindErrorMessage::functionExists(std::string_view const& tree, std::string_view const& function) {
    Global::capture().error.println("---------------------------------------------------------------");
    Global::capture().error.println("A Nebulite FuncTree binding failed!");
    Global::capture().error.println("Error: A bound Function already exists in this tree.");
    Global::capture().error.println("Function overwrite is heavily discouraged and thus not allowed.");
    Global::capture().error.println("Please choose a different name or remove the existing function.");
    Global::capture().error.println("This Tree: ", tree);
    Global::capture().error.println("Function:  ", function);
    throw std::runtime_error("FuncTree binding failed due to function already existing.");
}

void BindErrorMessage::variableHasWhitespace(std::string_view const& tree, std::string_view const& variable) {
    Global::capture().error.println("---------------------------------------------------------------");
    Global::capture().error.println("A Nebulite FuncTree binding failed!");
    Global::capture().error.println("Error: Variable name '", variable, "' cannot contain whitespaces.");
    Global::capture().error.println("This Tree: ", tree);
    throw std::runtime_error("Failed to bind variable due to invalid name.");
}

void BindErrorMessage::variableExists(std::string_view const& tree, std::string_view const& variable) {
    Global::capture().error.println("---------------------------------------------------------------");
    Global::capture().error.println("A Nebulite FuncTree binding failed!");
    Global::capture().error.println("Error: Variable '", variable, "' is already bound.");
    Global::capture().error.println("This Tree: ", tree);
    throw std::runtime_error("Failed to bind variable due to name conflict.");
}

void BindErrorMessage::commonFunctionHasWhitespace(std::string_view const& function) {
    Global::capture().error.println("---------------------------------------------------------------");
    Global::capture().error.println("A Nebulite FuncTree binding failed!");
    Global::capture().error.println("Error: Common function name '", function, "' cannot contain whitespaces.");
    throw std::runtime_error("Failed to bind common function due to invalid name.");
}

void BindErrorMessage::categoryShadowsFunction(std::string_view const& category) {
    Global::capture().error.println("---------------------------------------------------------------");
    Global::capture().error.println("A Nebulite FuncTree binding failed!");
    Global::capture().error.println("Error: Cannot bind category '", category, "' because a function with the same name already exists.");
    throw std::runtime_error("FuncTree binding failed due to category shadowing function.");
}

void BindErrorMessage::categoryShadowsVariable(std::string_view const& category) {
    Global::capture().error.println("---------------------------------------------------------------");
    Global::capture().error.println("A Nebulite FuncTree binding failed!");
    Global::capture().error.println("Error: Cannot bind category '", category, "' because a variable with the same name already exists.");
    throw std::runtime_error("FuncTree binding failed due to category shadowing variable.");
}

//------------------------------------------
// Execution error messages

void ExecutionErrorMessage::unknownVariable(std::string_view const& tree, std::string_view const& variable) {
    Global::capture().error.println("Unknown variable '", variable, "' in FuncTree '", tree, "'!");
    Global::capture().error.println("Make sure the variable is bound to the FuncTree before using it.");
}

void ExecutionErrorMessage::functionNotFound(std::string_view const& tree, std::string_view const& function, std::string_view const& arguments) {
    Global::capture().error.println("Function '", function, "' not found in FuncTree '", tree, "', its inherited FuncTrees or their categories!");
    Global::capture().error.println("Arguments: ", arguments);
}



} // namespace Nebulite::Interaction::Execution
