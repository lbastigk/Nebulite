#ifndef NEBULITE_INTERACTION_EXECUTION_FUNCTREE_ERROR_MESSAGES_HPP
#define NEBULITE_INTERACTION_EXECUTION_FUNCTREE_ERROR_MESSAGES_HPP

//------------------------------------------
// Includes
#include <string_view>

//------------------------------------------
// Collection of error messages for FuncTree binding issues

namespace Nebulite::Interaction::Execution::bindErrorMessage {

[[noreturn]] inline void MissingCategory(std::string_view const& tree, std::string_view const& category, std::string_view const& function) {
    Utility::Capture::cerr() << "---------------------------------------------------------------" << Utility::Capture::endl;
    Utility::Capture::cerr() << "A Nebulite FuncTree binding failed!" << Utility::Capture::endl;
    Utility::Capture::cerr() << "Error: Category '" << category << "' does not exist when trying to bind function '" << function << "'." << Utility::Capture::endl;
    Utility::Capture::cerr() << "Please create the category hierarchy first using bindCategory()." << Utility::Capture::endl;
    Utility::Capture::cerr() << "This Tree: " << tree << Utility::Capture::endl;
    throw std::runtime_error("FuncTree binding failed due to missing category.");
}

[[noreturn]] inline void FunctionShadowsCategory(std::string_view const& function) {
    Utility::Capture::cerr() << "---------------------------------------------------------------" << Utility::Capture::endl;
    Utility::Capture::cerr() << "A Nebulite FuncTree binding failed!" << Utility::Capture::endl;
    Utility::Capture::cerr() << "Error: Cannot bind function '" << function << "' because a category with the same name already exists." << Utility::Capture::endl;
    throw std::runtime_error("FuncTree binding failed due to function shadowing category.");
}

[[noreturn]] inline void FunctionExistsInInheritedTree(std::string_view const& tree, std::string_view const& inheritedTree, std::string_view const& function) {
    Utility::Capture::cerr() << "---------------------------------------------------------------\n";
    Utility::Capture::cerr() << "A Nebulite FuncTree initialization failed!\n";
    Utility::Capture::cerr() << "Error: A bound Function already exists in the inherited FuncTree.\n";
    Utility::Capture::cerr() << "Function overwrite is heavily discouraged and thus not allowed.\n";
    Utility::Capture::cerr() << "Please choose a different name or remove the existing function.\n";
    Utility::Capture::cerr() << "This Tree: " << tree << "\n";
    Utility::Capture::cerr() << "inherited FuncTree:   " << inheritedTree << "\n";
    Utility::Capture::cerr() << "Function:  " << function << "\n";
    throw std::runtime_error("FuncTree binding failed due to function existing in inherited tree.");
}

[[noreturn]] inline void ParentCategoryDoesNotExist(std::string_view const& name, std::string_view const& currentCategoryName) {
    Utility::Capture::cerr() << "---------------------------------------------------------------\n";
    Utility::Capture::cerr() << "Error: Cannot create category '" << name << "' because parent category '" << currentCategoryName << "' does not exist." << Utility::Capture::endl;
    throw std::runtime_error("FuncTree binding failed due to missing parent category.");
}

[[noreturn]] inline void CategoryExists(std::string_view const& name) {
    Utility::Capture::cerr() << "---------------------------------------------------------------\n";
    Utility::Capture::cerr() << "A Nebulite FuncTree initialization failed!\n";
    Utility::Capture::cerr() << "Error: Cannot create category '" << name << "' because it already exists." << Utility::Capture::endl;
    throw std::runtime_error("FuncTree binding failed due to category already existing.");
}

[[noreturn]] inline void FunctionExists(std::string_view const& tree, std::string_view const& function) {
    Utility::Capture::cerr() << "---------------------------------------------------------------\n";
    Utility::Capture::cerr() << "Nebulite FuncTree initialization failed!\n";
    Utility::Capture::cerr() << "Error: A bound Function already exists in this tree.\n";
    Utility::Capture::cerr() << "Function overwrite is heavily discouraged and thus not allowed.\n";
    Utility::Capture::cerr() << "Please choose a different name or remove the existing function.\n";
    Utility::Capture::cerr() << "This Tree: " << tree << "\n";
    Utility::Capture::cerr() << "Function:  " << function << "\n";
    throw std::runtime_error("FuncTree binding failed due to function already existing.");
}

} // namespace Nebulite::Interaction::Execution::bindErrorMessage
#endif // NEBULITE_INTERACTION_EXECUTION_FUNCTREE_ERROR_MESSAGES_HPP
