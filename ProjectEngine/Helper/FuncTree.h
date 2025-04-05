// This header defines the FuncTree class, which is responsible for parsing command-line
// arguments and executing the corresponding functions in the Nebulite project. The main goal
// of this class is to manage hierarchical commands and arguments for modular and flexible execution.

// Example usage in the terminal:
// nebulite test --debuglevel=2 JSONHandler keyNesting --level=2
//
// Breakdown of the execution flow:
// - nebulite -> the binary itself, invoked to start the application
// - test -> triggers the 'test' function, which parses additional arguments (e.g., --debuglevel=2)
//   and sets global parameters (e.g., global debug level to 2)
// - Next, the remaining arguments are parsed:
//   - JSONHandler -> invokes the associated debug function within the 'test' function context
//   - keyNesting -> calls a test within 'JSONHandler', setting the level of nesting to 2
//
// Each argument processed results in modifying the context or invoking specific tests, with the first
// argument and any associated options being removed before moving to the next set of arguments.


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

    // Parse is the entry to execution after all functions and arguments are attached
    // takes argc and argv, function name and executes
    // - The first argument is the function name
    // - parse all arguments, set global values
    // - call executeFunction on first argv (besides argv[0] obviously) that starts without '-' or '--'
    //   plus, pass all argv afterwards. Making sure argc is adjusted accordingly
    int parse(int argc, char* argv[]);

    // Attach an argument (like --count or -c) to a specific function or action
    void attachArgument(std::string* stringPtr, const std::string& argLong, const std::string& argShort, const std::string& helpDescription);

    // Attach a function to the menu
    void attachFunction(FunctionPtr func, const std::string& name, const std::string& helpDescription);

    // Execute the function
    int executeFunction(const std::string& name, int argc, char* argv[]);

private:
    int help(int argc, char* argv[]);
    std::map<std::string, std::pair<FunctionPtr,  std::string>> functions;
    std::map<std::string, std::pair<std::string*, std::string>> argumentPtrs;

};