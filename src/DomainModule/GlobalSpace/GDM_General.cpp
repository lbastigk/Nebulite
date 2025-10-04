#include "DomainModule/GlobalSpace/GDM_General.hpp"
#include "Core/GlobalSpace.hpp"       // Global Space for Nebulite
#include "Interaction/Invoke.hpp"            // Invoke for parsing expressions

namespace Nebulite::DomainModule::GlobalSpace{

//------------------------------------------
// Update
void General::update() {
    // Add Domain-specific updates here!
    // General rule:
    // This is used to update all variables/states that are INTERNAL ONLY
}

//------------------------------------------
// Domain-Bound Functions

Nebulite::Constants::Error General::eval(int argc, char* argv[]){
    // argc/argv to string for evaluation
    std::string args = "";
    for (int i = 0; i < argc; ++i) {
        args += argv[i];
        if (i < argc - 1) {
            args += " ";
        }
    }

    // eval all $(...)
    std::string args_evaled = domain->invoke->evaluateStandaloneExpression(args);

    // reparse
    return domain->parseStr(args_evaled);
}
const std::string General::eval_name = "eval";
const std::string General::eval_desc = R"(Evaluates an expression string and executes it.
Every argument after eval is concatenated with a whitespace to form the expression to be evaluated and then reparsed.

Usage: eval <expression>

Examples:

eval echo $(1+1)    
outputs: 2.000000
First, eval evaluates every argument, then concatenates them with a whitespace, 
and finally executes the resulting string as a command.
The string 'echo $(1+1)' is evaluated to "echo 2.000000", which is then executed.

eval spawn ./Resources/RenderObjects/{global.ToSpawn}.json
This evaluates to 'spawn ./Resources/RenderObjects/NAME.json', 
where NAME is the current value of the global variable ToSpawn
)";

Nebulite::Constants::Error General::exit(int argc, char* argv[]){
    // Clear all task queues to prevent further execution
    domain->tasks.script.taskQueue.clear();
    domain->tasks.internal.taskQueue.clear();
    domain->tasks.always.taskQueue.clear();

    // Set the renderer to quit
    domain->getRenderer()->setQuit();
    return Nebulite::Constants::ErrorTable::NONE();
}
const std::string General::exit_name = "exit";
const std::string General::exit_desc = R"(Exits the entire program.

Usage: exit

Closes the program with exit code 0 (no error)
Any queued tasks will be discarded.
)";

Nebulite::Constants::Error General::wait(int argc, char* argv[]){
    if(argc == 2){
        std::istringstream iss(argv[1]);
        iss >> domain->scriptWaitCounter;
        if (domain->scriptWaitCounter < 0){
            domain->scriptWaitCounter = 0;
        }
        return Nebulite::Constants::ErrorTable::NONE();
    }
    else if(argc < 2){
       return Nebulite::Constants::ErrorTable::FUNCTIONAL::TOO_FEW_ARGS();
    }
    else{
        return Nebulite::Constants::ErrorTable::FUNCTIONAL::TOO_MANY_ARGS();
    }
}
const std::string General::wait_name = "wait";
const std::string General::wait_desc = R"(Sets the waitCounter to the given value to halt all script tasks for a given amount of frames.

Usage: wait <frames>

This command pauses the execution of all script tasks for the specified number of frames.
This does not halt any tasks comming from objects within the environment and cannot be used by them.

The wait-command is intended for scripts only, allowing for timed delays between commands.

This is useful for:
- Creating pauses in scripts to wait for certain conditions to be met.
- Timing events in a sequence.
- Tool assisted speedruns (TAS)
)";

Nebulite::Constants::Error General::task(int argc, char* argv[]) {
    std::cout << "Loading task list from file: " << (argc > 1 ? argv[1] : "none") << std::endl;

    if (argc < 2) {
        return Nebulite::Constants::ErrorTable::FUNCTIONAL::TOO_FEW_ARGS();
    }
    if (argc > 2) {
        return Nebulite::Constants::ErrorTable::FUNCTIONAL::TOO_MANY_ARGS();
    }

    std::string file = Nebulite::Utility::FileManagement::LoadFile(argv[1]);
    if (file.empty()) {
        std::cerr << "Error: "<< argv[0] <<" Could not open file '" << argv[1] << "'" << std::endl;
        return Nebulite::Constants::ErrorTable::FILE::CRITICAL_INVALID_FILE();
    }

    std::vector<std::string> lines;

    // Split std::string file into lines and remove comments
    std::istringstream stream(file);
    std::string line;
    while (std::getline(stream, line)) {
        line = Nebulite::Utility::StringHandler::untilSpecialChar(line,'#');   // Remove comments.
        line = Nebulite::Utility::StringHandler::lstrip(line,' ');             // Remove whitespaces at start
        if(line.length() == 0){
            // line is empty
            continue;
        }
        else{
            // Insert line backwards, so we can process them in the order they were written later on:
            lines.insert(lines.begin(), line);
        }
    }

    // Now insert all lines into the task queue
    for (const auto& line : lines){
        domain->tasks.script.taskQueue.push_front(line);
    }
    return Nebulite::Constants::ErrorTable::NONE();
}
const std::string General::task_name = "task";
const std::string General::task_desc = R"(Loads tasks from a file into the taskQueue.

Usage: task <filename>

This command loads a list of tasks from the specified file into the task queue.
Each line in the file is treated as a separate task.

Task files are not appended at the end, but right after the current task. 
This ensures that tasks can be loaded within task files themselves and being executed immediately.

This pseudo-example shows how tasks are loaded and executed:

MAIN_TASK{
    maincommand1
    maincommand2
    task subtaskfile.txt{
        subcommand1
        subcommand2
    }
    maincommand4
}
)";

Nebulite::Constants::Error General::echo(int argc, char* argv[]) {
    for (int i = 1; i < argc; ++i) {
        std::cout << argv[i];
        if (i < argc - 1) {
            std::cout << " ";
        }
    }
    std::cout << std::endl;
    return Nebulite::Constants::ErrorTable::NONE();
}
const std::string General::echo_name = "echo";
const std::string General::echo_desc = R"(Echoes all arguments as string to the standard output.

Usage: echo <string>

This command concatenates all arguments with a whitespace and outputs them to the standard output (cout).
Example:
./bin/Nebulite echo Hello World!
Outputs:
Hello World!
)";

Nebulite::Constants::Error General::func_for(int argc, char* argv[]){
    std::string funcName = argv[0];
    if(argc > 4){
        std::string varName = argv[1];

        int iStart = std::stoi(domain->invoke->evaluateStandaloneExpression(argv[2]));
        int iEnd   = std::stoi(domain->invoke->evaluateStandaloneExpression(argv[3]));

        std::string args = "";
        for (int i = 4; i < argc; ++i) {
            args += argv[i];
            if (i < argc - 1) {
                args += " ";
            }
        }
        std::string args_replaced;
        for(int i = iStart; i <= iEnd; i++){
            // for + args
            args_replaced = funcName + " " + Nebulite::Utility::StringHandler::replaceAll(args, '{' + varName + '}', std::to_string(i));
            domain->parseStr(args_replaced);
        }
    }
    return Nebulite::Constants::ErrorTable::NONE();
}
const std::string General::func_for_name = "for";
const std::string General::func_for_desc = R"(Executes a for-loop with a function call.

Usage: for <var> <start> <end> <functioncall>

Example:
for i 1 5 echo Iteration {i}
This will output:
    Iteration 1
    Iteration 2
    Iteration 3
    Iteration 4
    Iteration 5

This is useful for:
- Repeating actions a specific number of times.
- Iterating over a range of values.
- Creating complex control flows in scripts.
)";

Nebulite::Constants::Error General::func_if(int argc, char* argv[]) {
    if (argc < 3) {
        return Nebulite::Constants::ErrorTable::FUNCTIONAL::TOO_FEW_ARGS();
    }

    std::string result = domain->invoke->evaluateStandaloneExpression(argv[1]);
    double condition_potentially_nan = std::stod(result);

    bool condition = !isnan(condition_potentially_nan) && (condition_potentially_nan != 0);

    if (!condition) {
        // If the condition is false, skip the following commands
        return Nebulite::Constants::ErrorTable::NONE();
    }

    // Build the command string from rest
    std::string commands = "";
    for (int i = 2; i < argc; i++) {
        commands += argv[i];
        if (i < argc - 1) {
            commands += " ";
        }
    }
    commands = __FUNCTION__ + std::string(" ") + commands;
    return domain->parseStr(commands);
}
const std::string General::func_if_name = "if";
const std::string General::func_if_desc = R"(Executes a block of code if a condition is true.

Usage: if <condition> <functioncall>

It is recommended to wrap the condition in quotes to prevent parsing issues.

However, This is not supported for in-console usage. 
This is because the console itself removes quotes before passing the arguments to the FuncTree,
rendering them useless.

Example:
if '$(eq(1+1,2))' echo Condition is true!
)";

Nebulite::Constants::Error General::func_assert(int argc, char* argv[]){
    if (argc < 2) {
        return Nebulite::Constants::ErrorTable::FUNCTIONAL::TOO_FEW_ARGS();
    }

    if (argc > 2) {
        return Nebulite::Constants::ErrorTable::FUNCTIONAL::TOO_MANY_ARGS();
    }

    std::string condition = argv[1];

    // condition must start with $( and end with )
    if (condition.front() != '$' || condition[1] != '(' || condition.back() != ')') {
        return Nebulite::Constants::ErrorTable::FUNCTIONAL::UNKNOWN_ARG();
    }

    // Evaluate condition
    if(!std::stod(domain->invoke->evaluateStandaloneExpression(condition))){
        return Nebulite::Constants::ErrorTable::CRITICAL_CUSTOM_ASSERT();
    }

    // All good
    return Nebulite::Constants::ErrorTable::NONE();
}
const std::string General::assert_name = "assert";
const std::string General::assert_desc = R"(Asserts a condition and throws a custom error if false.

Usage: assert <condition>

It is recommended to wrap the condition in quotes to prevent parsing issues.

Example:
assert '$(eq(1+1,2))'    // No error
assert '$(eq(1+1,3))'    // Critical Error: A custom assertion failed.
Assertion failed: $(eq(1+1,3)) is not true.
)";

Nebulite::Constants::Error General::func_return(int argc, char* argv[]){
    std::string str = "";
    for(int i = 1; i < argc; ++i){
        str += argv[i];
        if(i < argc - 1){
            str += " ";
        }
    }
    return Nebulite::Constants::ErrorTable::addError(str, Nebulite::Constants::Error::CRITICAL);
}
const std::string General::func_return_name = "return";
const std::string General::func_return_desc = R"(Returns a custom value as a Critical Error.

Usage: return <string>

This command creates a custom critical error with the given string as description.
This can be used to exit from a task queue with a custom message.

Example:

./bin/Nebulite return We did not anticipate this happening, weird.
Outputs:
We did not anticipate this happening, weird.
Critical Error: We did not anticipate this happening, weird.
)";

Nebulite::Constants::Error General::always(int argc, char* argv[]){
    if (argc > 1) {
        std::ostringstream oss;
        for (int i = 1; i < argc; ++i) {
            if (i > 1) oss << ' ';
            oss << argv[i];
        }

        // Split oss.str() on ';' and push each trimmed command
        std::string argStr = oss.str();
        std::stringstream ss(argStr);
        std::string command;

        while (std::getline(ss, command, ';')) {
            // Trim whitespace from each command
            command.erase(0, command.find_first_not_of(" \t"));
            command.erase(command.find_last_not_of(" \t") + 1);
            if (!command.empty()) {
                domain->tasks.always.taskQueue.push_back(command);
            }
        }
    }
    return Nebulite::Constants::ErrorTable::NONE();
}
const std::string General::always_name = "always";
const std::string General::always_desc = R"(Attach a command to the always-taskqueue that is executed on each tick.

Usage: always <command>

Example:
always echo This command runs every frame!
This will output "This command runs every frame!" on every frame.
)";

Nebulite::Constants::Error General::alwaysClear(int argc, char* argv[]){
    domain->tasks.always.taskQueue.clear();
    return Nebulite::Constants::ErrorTable::NONE();
}
const std::string General::alwaysClear_name = "always-clear";
const std::string General::alwaysClear_desc = R"(Clears the entire always-taskqueue.

Usage: always-clear

Example:
always-clear
This will remove all commands from the always-taskqueue.
)";

} // namespace Nebulite::DomainModule::GlobalSpace