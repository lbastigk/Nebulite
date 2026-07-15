//------------------------------------------
// Includes

// Standard library
#include <stdexcept>
#include <string_view>

// Nebulite
#include "Nebulite/Utility/Args/FuncTreeErrorMessages.hpp"
#include "Nebulite/Utility/IO/Capture.hpp"

//------------------------------------------
namespace Nebulite::Utility::Args {

//------------------------------------------
// Bind error messages

void BindErrorMessage::invalidFunctionName(IO::Capture& c, std::string_view const name) {
    c.error.println("---------------------------------------------------------------");
    c.error.println("A Nebulite FuncTree binding failed!");
    c.error.println("Error: Function name '", name, "' is invalid.");
    c.error.println("Function names cannot be empty or contain whitespace characters.");
    throw std::runtime_error("FuncTree binding failed due to invalid function name.");
}

void BindErrorMessage::missingCategory(IO::Capture& c, std::string_view const tree, std::string_view const category, std::string_view const function) {
    c.error.println("---------------------------------------------------------------");
    c.error.println("A Nebulite FuncTree binding failed!");
    c.error.println("Error: Category '", category, "' does not exist when trying to bind function '", function, "'.");
    c.error.println("Please create the category hierarchy first using bindCategory().");
    c.error.println("This Tree: ", tree);
    throw std::runtime_error("FuncTree binding failed due to missing category.");
}

void BindErrorMessage::functionShadowsCategory(IO::Capture& c, std::string_view const function) {
    c.error.println("---------------------------------------------------------------");
    c.error.println("A Nebulite FuncTree binding failed!");
    c.error.println("Error: Cannot bind function '", function, "' because a category with the same name already exists.");
    throw std::runtime_error("FuncTree binding failed due to function shadowing category.");
}

void BindErrorMessage::functionShadowsVariable(IO::Capture& c, std::string_view const function) {
    c.error.println("---------------------------------------------------------------");
    c.error.println("A Nebulite FuncTree binding failed!");
    c.error.println("Error: Cannot bind function '", function, "' because a variable with the same name already exists.");
    throw std::runtime_error("FuncTree binding failed due to function shadowing variable.");
}

void BindErrorMessage::functionExistsInInheritedTree(IO::Capture& c, std::string_view const tree, std::string_view const inheritedTree, std::string_view const function) {
    c.error.println("---------------------------------------------------------------");
    c.error.println("A Nebulite FuncTree binding failed!");
    c.error.println("Error: A bound Function already exists in the inherited FuncTree.");
    c.error.println("Function overwrite is only allowed if the function it points to is still the same.");
    c.error.println("Please choose a different name or remove the existing function.");
    c.error.println("This Tree: ", tree);
    c.error.println("inherited FuncTree:   ", inheritedTree);
    c.error.println("Function:  ", function);
    throw std::runtime_error("FuncTree binding failed due to function existing in inherited tree.");
}

void BindErrorMessage::parentCategoryDoesNotExist(IO::Capture& c, std::string_view const name, std::string_view const currentCategoryName) {
    c.error.println("---------------------------------------------------------------");
    c.error.println("A Nebulite FuncTree binding failed!");
    c.error.println("Error: Cannot create category '", name, "' because parent category '", currentCategoryName, "' does not exist.");
    throw std::runtime_error("FuncTree binding failed due to missing parent category.");
}

void BindErrorMessage::categoryExists(IO::Capture& c, std::string_view const name) {
    c.error.println("---------------------------------------------------------------");
    c.error.println("A Nebulite FuncTree binding failed!");
    c.error.println("Error: Cannot create category '", name, "' because it already exists.");
    throw std::runtime_error("FuncTree binding failed due to category already existing.");
}

void BindErrorMessage::functionExists(IO::Capture& c, std::string_view const tree, std::string_view const function) {
    c.error.println("---------------------------------------------------------------");
    c.error.println("A Nebulite FuncTree binding failed!");
    c.error.println("Error: A bound Function already exists in this tree.");
    c.error.println("Function overwrite is heavily discouraged and thus not allowed.");
    c.error.println("Please choose a different name or remove the existing function.");
    c.error.println("This Tree: ", tree);
    c.error.println("Function:  ", function);
    throw std::runtime_error("FuncTree binding failed due to function already existing.");
}

void BindErrorMessage::variableHasWhitespace(IO::Capture& c, std::string_view const tree, std::string_view const variable) {
    c.error.println("---------------------------------------------------------------");
    c.error.println("A Nebulite FuncTree binding failed!");
    c.error.println("Error: Variable name '", variable, "' cannot contain whitespaces.");
    c.error.println("This Tree: ", tree);
    throw std::runtime_error("Failed to bind variable due to invalid name.");
}

void BindErrorMessage::variableExists(IO::Capture& c, std::string_view const tree, std::string_view const variable) {
    c.error.println("---------------------------------------------------------------");
    c.error.println("A Nebulite FuncTree binding failed!");
    c.error.println("Error: Variable '", variable, "' is already bound.");
    c.error.println("This Tree: ", tree);
    throw std::runtime_error("Failed to bind variable due to name conflict.");
}

void BindErrorMessage::commonFunctionHasWhitespace(IO::Capture& c, std::string_view const function) {
    c.error.println("---------------------------------------------------------------");
    c.error.println("A Nebulite FuncTree binding failed!");
    c.error.println("Error: Common function name '", function, "' cannot contain whitespaces.");
    throw std::runtime_error("Failed to bind common function due to invalid name.");
}

void BindErrorMessage::categoryShadowsFunction(IO::Capture& c, std::string_view const category) {
    c.error.println("---------------------------------------------------------------");
    c.error.println("A Nebulite FuncTree binding failed!");
    c.error.println("Error: Cannot bind category '", category, "' because a function with the same name already exists.");
    throw std::runtime_error("FuncTree binding failed due to category shadowing function.");
}

void BindErrorMessage::categoryShadowsVariable(IO::Capture& c, std::string_view const category) {
    c.error.println("---------------------------------------------------------------");
    c.error.println("A Nebulite FuncTree binding failed!");
    c.error.println("Error: Cannot bind category '", category, "' because a variable with the same name already exists.");
    throw std::runtime_error("FuncTree binding failed due to category shadowing variable.");
}

//------------------------------------------
// Execution error messages

void ExecutionErrorMessage::unknownVariable(IO::Capture& c, std::string_view const tree, std::string_view const variable) {
    c.error.println("Unknown variable '", variable, "' in FuncTree '", tree, "'!");
    c.error.println("Make sure the variable is bound to the FuncTree before using it.");
}

void ExecutionErrorMessage::functionNotFound(IO::Capture& c, std::string_view const tree, std::string_view const function, std::string_view const arguments) {
    c.error.println("Function '", function, "' not found in FuncTree '", tree, "', its inherited FuncTrees or their categories!");
    c.error.println("Arguments:");
    c.error.println(arguments);
}

} // namespace Nebulite::Utility::Args
