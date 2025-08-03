/**
 * @file FuncTreeWrapper.h
 * @brief Extension wrapper for FuncTree to simplify binding class member functions.
 *
 * @details
 * The FuncTree class provides a flexible framework for attaching and executing
 * command-line style functions that follow the signature:
 * 
 *     T func(int argc, char* argv[]);
 * 
 * where `T` is the return type (usually an error or status code).
 *
 * However, FuncTree only supports binding free functions or stateless lambdas
 * matching the required signature directly. Binding class member functions
 * requires manual wrapping of the `this` pointer and member function pointers
 * into a compatible `std::function`. This wrapping can be repetitive and error-prone,
 * especially when multiple member functions need to be attached.
 *
 * The FuncTreeWrapper class extends FuncTree by providing convenient helper functions
 * to bind member functions of a specific class instance (typically the wrapper itself)
 * in a straightforward manner. This eliminates boilerplate code and improves
 * maintainability by centralizing the binding logic.
 *
 * Benefits of using FuncTreeWrapper:
 * - Simplifies attaching member functions without manual lambda creation each time.
 * - Supports clean, readable code: `bind(funcTree, this, &Class::method, "name", "desc")`.
 * - Encapsulates common binding patterns, reducing bugs and improving consistency.
 * - Makes it easier to extend or customize the function binding mechanism in one place.
 * 
 */


#pragma once

#include "FuncTree.h"

template<typename T>
class FuncTreeWrapper {
public:
    FuncTreeWrapper(const std::string& name, T standard, T error)
        : funcTree(name, standard, error) {}

    // Basic forwarding
    T parse(int argc, char* argv[]) {
        return funcTree.parse(argc, argv);
    }

    T parseStr(const std::string& cmd) {
        return funcTree.parseStr(cmd);
    }

    // Binding helper
    template<typename ClassType, typename ReturnType>
    void bindFunction(FuncTree<ReturnType>& tree,
            ClassType* obj,
            ReturnType (ClassType::*method)(int, char**),
            const std::string& name,
            const std::string& help) {
        tree.attachFunction(
            [obj, method](int argc, char** argv) {
                return (obj->*method)(argc, argv);
            },
            name,
            help
        );
    }

    // Attach a variable as string pointer to the FuncTree
    void bindVariable(std::string* varPtr, const std::string& name, const std::string& helpDescription) {
        funcTree.attachVariable(varPtr, name, helpDescription);
    }

    // Check if a function is attached
    bool hasFunction(const std::string& nameOrCommand) {
        return funcTree.hasFunction(nameOrCommand);
    }

protected:
    FuncTree<T> funcTree;
};