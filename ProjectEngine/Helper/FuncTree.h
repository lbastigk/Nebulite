// TODO:
// This will later implement the FuncTree class used to start Nebulite with arguments.
// e.g:

// argv is:
//nebulite test --debuglevel=2 JSONHandler keyNesting --level=2
//
// - nebulite -> binary itself
// - test -> call test function, set global debug to 2
// - JSONHandler: call JSONHandler debug function attached to test function
// - keyNesting : call keynesting test inside JSONHandler test function, set the level of nesting to 2
// With each passing, the first argv is removed

#pragma once

#include <iostream>
#include <vector>
#include <map>
#include <functional>

class FuncTree{
public:
    FuncTree();

    // functions get std::string as args and return an int
    using FunctionPtr = std::function<int(int argc, char* argv[])>;  // Define the function pointer type

    // Parse is the entry to execution after all functions are attached
    // takes argc and argv, function name and executes
    // as this is passed from e.g. main, argv[0] should be the binary name itself
    // and only argv[1] should be the one used for executeFunction as name?
    int parse(int argc, char* argv[]);

    // Attach a function to the menu
    void attachFunction(FunctionPtr func, const std::string& name, const std::string& helpDescription);

    int executeFunction(const std::string& name, int argc, char* argv[]);
    

private:
    int help(int argc, char* argv[]);
    std::map<std::string, std::pair<FunctionPtr, std::string>> functions;
};