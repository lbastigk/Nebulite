#include "MainTree.h"
#include "GlobalSpace.h"       // Global Space for Nebulite
#include "Invoke.h"            // Invoke for parsing expressions


//-------------------------------------
// Linking ALL Functions to MainTree
Nebulite::MainTree::MainTree(Nebulite::Invoke* invokeLinkage, Nebulite::GlobalSpace* globalSpaceLinkage)
    : FuncTreeWrapper("Nebulite", Nebulite::ERROR_TYPE::NONE, Nebulite::ERROR_TYPE::CRITICAL_FUNCTIONCALL_INVALID),
      general   (invokeLinkage, globalSpaceLinkage, &this->funcTree),
      renderer  (invokeLinkage, globalSpaceLinkage, &this->funcTree),
      debug     (invokeLinkage, globalSpaceLinkage, &this->funcTree) 
{

    invoke = invokeLinkage;
    global = globalSpaceLinkage;

    //---------------------------------------------
    // Bind global variables to tree, 
    // allowing manipulation with --var or --var=value on: this->funcTree.parseStr(command)
    bindVariable(&global->headless, "headless", "Setting for headless mode, no window will be created");

    //---------------------------------------------
    // Bind Functions: General
    bindFunction(funcTree, &general, &MainTreeCategory::General::eval,              "eval",         "Evaluate all $(...) after this keyword, parse rest as usual");
    bindFunction(funcTree, &general, &MainTreeCategory::General::setGlobal,         "set-global",   "Set any global variable: [key] [value]");
    bindFunction(funcTree, &general, &MainTreeCategory::General::exitProgram,       "exit",         "exits the program");
    bindFunction(funcTree, &general, &MainTreeCategory::General::stateSave,         "state-save",   "Saves the state");
    bindFunction(funcTree, &general, &MainTreeCategory::General::stateLoad,         "state-load",   "Loads a state");
    bindFunction(funcTree, &general, &MainTreeCategory::General::loadTaskList,      "task",         "Loads a txt file of tasks");
    bindFunction(funcTree, &general, &MainTreeCategory::General::wait,              "wait",         "Halt all commands for a set amount of frames");
    bindFunction(funcTree, &general, &MainTreeCategory::General::forLoop,           "for",          "Start for-loop. Usage: for var <iStart> <iEnd> command $var");
    bindFunction(funcTree, &general, &MainTreeCategory::General::func_assert,       "assert",       "Force a certain return value");
    bindFunction(funcTree, &general, &MainTreeCategory::General::func_return,       "return",       "Returns an assert value, stopping program");
    bindFunction(funcTree, &general, &MainTreeCategory::General::forceGlobal,       "force-global", "Force a global variable to a certain value");
    bindFunction(funcTree, &general, &MainTreeCategory::General::clearForceGlobal,  "force-global-clear", "Release all forced global values");
    bindFunction(funcTree, &general, &MainTreeCategory::General::echo,              "echo",         "Echos all args provided to cout");
    bindFunction(funcTree, &general, &MainTreeCategory::General::error,             "error",        "Echos all args provided to cerr");

    //---------------------------------------------
    // Bind Functions: Renderer Settings / Funtions
    bindFunction(funcTree, &renderer, &MainTreeCategory::Renderer::envload,         "env-load",     "Loads an environment");
    bindFunction(funcTree, &renderer, &MainTreeCategory::Renderer::envdeload,       "env-deload",   "Deloads an environment");
    bindFunction(funcTree, &renderer, &MainTreeCategory::Renderer::spawn,           "spawn",        "Spawn a renderobject");
    bindFunction(funcTree, &renderer, &MainTreeCategory::Renderer::setFPS,          "set-fps",      "Sets FPS to an integer between 1 and 10000. 60 if no arg is provided");
    bindFunction(funcTree, &renderer, &MainTreeCategory::Renderer::setResolution,   "set-res",      "Sets resolution size:  [w] [h]");
    bindFunction(funcTree, &renderer, &MainTreeCategory::Renderer::setCam,          "cam-set",      "Sets Camera position:  [x] [y] <c>");
    bindFunction(funcTree, &renderer, &MainTreeCategory::Renderer::moveCam,         "cam-move",     "Moves Camera position: [dx] [dy]");
    bindFunction(funcTree, &renderer, &MainTreeCategory::Renderer::snapshot,        "snapshot",     "Take screenshot:       <link>");
    bindFunction(funcTree, &renderer, &MainTreeCategory::Renderer::beep,            "beep",         "Simple Beep tone");
    
    //---------------------------------------------
    // Bind Functions: Debug
    bindFunction(funcTree, &debug, &MainTreeCategory::Debug::printGlobal,           "print-global", "Prints global doc to cout");
    bindFunction(funcTree, &debug, &MainTreeCategory::Debug::printState,            "print-state",  "Prints state to cout");
    bindFunction(funcTree, &debug, &MainTreeCategory::Debug::logGlobal,             "log-global",   "Logs global doc to file");
    bindFunction(funcTree, &debug, &MainTreeCategory::Debug::logState,              "log-state",    "Logs state to file");
    bindFunction(funcTree, &debug, &MainTreeCategory::Debug::errorlog,              "log",          "Activate/Deactivate error log");
    bindFunction(funcTree, &debug, &MainTreeCategory::Debug::always,                "always",       "Attach functioncall that is executed on each tick");
    bindFunction(funcTree, &debug, &MainTreeCategory::Debug::alwaysClear,           "always-clear", "Clear all always-functioncalls");
    // Temporary Helper
    bindFunction(funcTree, &debug, &MainTreeCategory::Debug::render_object,         "standard-render-object",  "Serializes standard renderobject to ./Resources/Renderobjects/standard.json");
    bindFunction(funcTree, &debug, &MainTreeCategory::Debug::printVar,              "print-var",               "Prints the value of the all internal values");
}