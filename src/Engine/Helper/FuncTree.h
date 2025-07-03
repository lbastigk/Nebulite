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
    using FunctionPtr = std::function<int(int argc, char* argv[])>;

    // Parse is the entry to execution after all functions and arguments are attached
    // takes argc and argv, function name and executes
    // - The first argument is the function name itself
    // - call executeFunction provided by second argv
    //   pass all argv afterwards. Making sure argc is adjusted accordingly
    int parse(int argc, char* argv[]);

    // Attach a function to the menu
    void attachFunction(FunctionPtr func, const std::string& name, const std::string& helpDescription);

    // Execute the function
    int executeFunction(const std::string& name, int argc, char* argv[]);

private:
    int help(int argc, char* argv[]);
    absl::flat_hash_map<std::string, std::pair<FunctionPtr,  std::string>> functions;
    std::string TreeName; 
};