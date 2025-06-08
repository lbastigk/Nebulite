// This header defines the FuncTree class, which is responsible for parsing command-line
// arguments and executing the corresponding functions in the Nebulite project. The main goal
// of this class is to manage hierarchical commands and arguments for modular and flexible execution.


#pragma once

#include <iostream>
#include <vector>
#include "absl/container/flat_hash_map.h"
#include <functional>
#include <cstring>

class FuncTree{
public:
    FuncTree(std::string treeName);

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

    // Converting an std::string into argc/argv
    void convertStrToArgcArgv(const std::string& cmd, int& argc, char**& argv);


private:
    int help(int argc, char* argv[]);
    absl::flat_hash_map<std::string, std::pair<FunctionPtr,  std::string>> functions;
    absl::flat_hash_map<std::string, std::pair<std::string*, std::string>> argumentPtrs;
    std::string TreeName; 
};