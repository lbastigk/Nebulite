#include "InvokeExpression.h"

Nebulite::InvokeExpression::InvokeExpression() {
    // Initialize with default expression "0"
    parse("0", *documentCache);

    
    //-------------------------------------------------
    // Register functions
    te_variable gt_var =  {"gt",    (void*)expr_custom::gt,             TE_FUNCTION2};
    variables.push_back(gt_var);
    te_variable lt_var =  {"lt",    (void*)expr_custom::lt,             TE_FUNCTION2};
    variables.push_back(lt_var);
    te_variable geq_var = {"geq",   (void*)expr_custom::geq,            TE_FUNCTION2};
    variables.push_back(geq_var);
    te_variable leq_var = {"leq",   (void*)expr_custom::leq,            TE_FUNCTION2};
    variables.push_back(leq_var);
    te_variable eq_var =  {"eq",    (void*)expr_custom::eq,             TE_FUNCTION2};
    variables.push_back(eq_var);
    te_variable neq_var = {"neq",   (void*)expr_custom::neq,            TE_FUNCTION2};
    variables.push_back(neq_var);
    te_variable and_var = {"and",   (void*)expr_custom::logical_and,    TE_FUNCTION2};
    variables.push_back(and_var);
    te_variable or_var =  {"or",    (void*)expr_custom::logical_or,     TE_FUNCTION2};
    variables.push_back(or_var);
    te_variable not_var = {"not",   (void*)expr_custom::logical_not,    TE_FUNCTION1};
    variables.push_back(not_var);
    te_variable sgn_var = {"sgn",   (void*)expr_custom::sgn,            TE_FUNCTION1};
    variables.push_back(sgn_var);
}

void Nebulite::InvokeExpression::compileIfExpression(Entry& entry) {
    if (entry.type == Entry::Type::eval) {
        entry.str = modifyTextToTeConform(entry.str);

        // Compile the expression using TinyExpr
        int error;
        entry.expression = te_compile(entry.str.c_str(), variables.data(), variables.size(), &error);
        if (error) {
            std::cerr << "Error compiling expression: '" << entry.str << "' Error code: " << error << std::endl;
            std::cerr << "Registered functions and variables:\n";
            for (const auto& var : variables) {
                std::cerr << "\t'" << var.name << "'\n";
            }
        }
    }
}

void Nebulite::InvokeExpression::registerIfVariable(Entry& entry){
    // Using the Virtual Double to register pointers...
    if(entry.type == Entry::Type::variable) {
        // Remove paranthesis at front and end
        if (entry.str.front() == '(' && entry.str.back() == ')') {
            entry.str = entry.str.substr(1, entry.str.size() - 2);
        }

        Nebulite::JSON** json_dual_pointer = nullptr;
        if(entry.from == Entry::From::self) {
            json_dual_pointer = &self;
        } else if (entry.from == Entry::From::other) {
            json_dual_pointer = &other;
        } else if (entry.from == Entry::From::global) {
            json_dual_pointer = &global;
        } else if (entry.from == Entry::From::resource) {
            json_dual_pointer = nullptr;
        }

        // Check if variable exists in variables vector:
        bool found = false;
        for(auto var : variables) {
            if(var.name == entry.str) {
                found = true;
                break;
            }
        }
        if(!found) {
            // Entry for pointer with dots
            std::string key = entry.str;

            // Replace dots in name for entry
            std::replace(entry.str.begin(), entry.str.end(), '.', '_');

            // Store the variable name string so it persists beyond this scope
            variableNames.push_back(entry.str);
            const char* persistentName = variableNames.back().c_str();

            // Initialize with reference to document and cache register
            std::shared_ptr<Nebulite::VirtualDouble> vd = std::make_shared<Nebulite::VirtualDouble>(json_dual_pointer, key, documentCache);
            virtualDoubles.push_back(vd);

            // Push back into variable entries - use the persistent string and VirtualDouble pointer
            variables.push_back({ persistentName, vd->ptr(), TE_VARIABLE, nullptr });
        }
    }
    if(entry.type == Entry::Type::eval) {
        // Idea here is to create a dummy vector of entries, that we ignore
        // We only care about the registration
        std::vector<Entry> dummyEntries;

        // Remove all paranteses at front and back
        std::string substring = entry.str;
        bool stillInParanthesis = true;
        while (stillInParanthesis) {
            if (substring.front() == '(' && substring.back() == ')') {
                substring = substring.substr(1, substring.size() - 2);
            } else {
                stillInParanthesis = false;
            }
        }
        parseIntoEntries(substring, dummyEntries);
    }
}

std::string Nebulite::InvokeExpression::modifyTextToTeConform(std::string str) {
    // Find and replace all occurrences of $(...) patterns
    size_t pos = 0;
    while ((pos = str.find("$(", pos)) != std::string::npos) {
        // Find the matching closing parenthesis
        size_t start = pos + 2; // Position after "$("
        size_t depth = 1;
        size_t end = start;
        
        while (end < str.length() && depth > 0) {
            if (str[end] == '(') {
                depth++;
            } else if (str[end] == ')') {
                depth--;
            }
            end++;
        }
        
        if (depth == 0) {
            // Extract the content between parentheses
            std::string content = str.substr(start, end - start - 1);
            
            // Replace dots with underscores in the content
            for (char& c : content) {
                if (c == '.') {
                    c = '_';
                }
            }
            
            // Replace the entire $(...) with (content)
            str.replace(pos, end - pos, "(" + content + ")");
            pos += content.length() + 2; // Move past the replacement
        } else {
            // Unmatched parentheses, move past this $
            pos += 2;
        }
    }
    return str;
}

void Nebulite::InvokeExpression::setEntryContext(Entry& entry) {
    // Check if string starts with '$', remove
    if (entry.str.find("$") == 0) {
        entry.str.erase(0, 1); // Remove "$"
    }

    // If entry contains $ inside, it is now an eval entry
    if(entry.type == Entry::Type::variable && entry.str.find("$") != std::string::npos){
        entry.type = Entry::Type::eval;
        return;
    }

    // Check what kind of variable it is
    if (entry.type == Entry::Type::variable) {
        // set type, remove self/other/global from beginning:
        if (entry.str.find("(self.") == 0) {
            entry.from = Entry::From::self;
        } else if (entry.str.find("(other.") == 0) {
            entry.from = Entry::From::other;
        } else if (entry.str.find("(global.") == 0) {
            entry.from = Entry::From::global;
        }
        else if (entry.str.find(".") == 0) {
            entry.from = Entry::From::resource;
        }
        else {
            // Is an expression like $(1+1)
            entry.type = Entry::Type::eval;
        }
    }
}

void Nebulite::InvokeExpression::make_entry(Entry& currentEntry, std::vector<Entry>& entries) {
    setEntryContext(currentEntry);          // Sets context: Variable:self-other-global-resource / eval
    registerIfVariable(currentEntry);       // Register variable for TinyExpr evaluation

    // Push back
    entries.push_back(currentEntry);

    // Reset current entry
    currentEntry = Entry();
};

void Nebulite::InvokeExpression::parseIntoEntries(const std::string& expr, std::vector<Entry>& entries){
    //std::cout << "Parsing expr: " << expr << std::endl;

    Entry currentEntry;
    int depth = 0;
    bool inEval = false;
    char lastChar = '\0';
    for(int i = 0; i < expr.size(); i++) {
        char currentChar = expr[i];
        // Determine the type of entry based on the character
        if (currentChar == '$' && depth == 0) {
            inEval = true;
            if(currentEntry.str.size() > 0) {
                make_entry(currentEntry, entries);
            }
            currentEntry.type = Entry::Type::variable;
            currentEntry.str += currentChar;
        }
        else if(lastChar == '$' && currentChar == 'i' && depth == 0) {
            currentEntry.cast = Entry::CastType::to_int;
        }
        else if(lastChar == '$' && currentChar == 'f' && depth == 0) {
            currentEntry.cast = Entry::CastType::to_float;
        }
        else if(currentChar == '(') {
            if(inEval)depth++;
            currentEntry.str += currentChar;
        }
        else if(currentChar == ')') {
            if(inEval)depth--;
            currentEntry.str += currentChar;
            if(depth == 0){
                inEval = false;
                make_entry(currentEntry, entries);
            }
        }
        else{
            currentEntry.str += currentChar;
        }
        lastChar = currentChar;
    }

    if(depth != 0) {
        // If depth is not zero, there is an unmatched parenthesis
        std::cerr << "Error: Unmatched parentheses in expression: " << expr << std::endl;
        return; // or throw an exception
    }

    if(currentEntry.str.size() > 0) {
        make_entry(currentEntry, entries);
    }
}

void Nebulite::InvokeExpression::parse(const std::string& expr, Nebulite::DocumentCache& documentCache){
    fullExpression = expr;
    this->documentCache = &documentCache;

    // Clear existing data
    entries.clear();
    variables.clear();
    virtualDoubles.clear();
    variableNames.clear();

    // Re-register built-in functions
    te_variable gt_var =  {"gt",    (void*)expr_custom::gt,             TE_FUNCTION2};
    variables.push_back(gt_var);
    te_variable lt_var =  {"lt",    (void*)expr_custom::lt,             TE_FUNCTION2};
    variables.push_back(lt_var);
    te_variable geq_var = {"geq",   (void*)expr_custom::geq,            TE_FUNCTION2};
    variables.push_back(geq_var);
    te_variable leq_var = {"leq",   (void*)expr_custom::leq,            TE_FUNCTION2};
    variables.push_back(leq_var);
    te_variable eq_var =  {"eq",    (void*)expr_custom::eq,             TE_FUNCTION2};
    variables.push_back(eq_var);
    te_variable neq_var = {"neq",   (void*)expr_custom::neq,            TE_FUNCTION2};
    variables.push_back(neq_var);
    te_variable and_var = {"and",   (void*)expr_custom::logical_and,    TE_FUNCTION2};
    variables.push_back(and_var);
    te_variable or_var =  {"or",    (void*)expr_custom::logical_or,     TE_FUNCTION2};
    variables.push_back(or_var);
    te_variable not_var = {"not",   (void*)expr_custom::logical_not,    TE_FUNCTION1};
    variables.push_back(not_var);
    te_variable sgn_var = {"sgn",   (void*)expr_custom::sgn,            TE_FUNCTION1};
    variables.push_back(sgn_var);

    parseIntoEntries(expr, entries);

    // Now compile all entries:
    for (auto& entry : entries) {
        compileIfExpression(entry);
    }
}

std::string Nebulite::InvokeExpression::eval(Nebulite::JSON* current_self, Nebulite::JSON* current_other, Nebulite::JSON* current_global) {
    // Setting new referenes
    self = current_self;
    other = current_other;
    global = current_global;

    std::string result = "";

    std::string key;
    std::string token;
    for (const auto& entry : entries) {
        token = "";
        switch (entry.type){
            case Entry::variable:

                if (entry.from == Entry::self) {
                    if(self == nullptr){
                        std::cerr << "Error: Null self reference in expression: " << entry.str << std::endl;
                        return "";
                    }
                    // Removing "self." at front
                    token = self->get<std::string>(entry.str.substr(5).c_str(), "");
                } else if (entry.from == Entry::other) {
                    if(other == nullptr) {
                        std::cerr << "Error: Null other reference in expression: " << entry.str << std::endl;
                        return "";
                    }
                    // Removing "other." at front
                    token = other->get<std::string>(entry.str.substr(6).c_str(), "");
                } else if (entry.from == Entry::global) {
                    if (global == nullptr) {
                        std::cerr << "Error: Null global reference in expression: " << entry.str << std::endl;
                        return "";
                    }
                    // Removing "global." at front
                    token = global->get<std::string>(entry.str.substr(7).c_str(), "");
                } else if (entry.from == Entry::resource) {
                    if (globalCache == nullptr) {
                        std::cerr << "Error: Null globalCache reference in expression: " << entry.str << std::endl;
                        return "";
                    }
                    token = globalCache->getData<std::string>(entry.str.c_str(), "");
                }
                break;

            case Entry::eval:
                // Update all VirtualDouble cache values before evaluation
                for (auto& vd : virtualDoubles) {
                    vd->ptr(); // This will update the cache
                }
                token = std::to_string(te_eval(entry.expression));
                break;

            case Entry::text:
                token = entry.str;
                break;

            default:
                break;
        }
        switch (entry.cast){
            case Entry::CastType::none:
                result += token;
                break;

            case Entry::CastType::to_int:
                result += std::to_string(static_cast<int>(std::stof(token)));
                break;

            case Entry::CastType::to_float:
                result += std::to_string(static_cast<float>(std::stof(token)));
                break;

            default:
                break;
        }
    }

    //std::cout << "Evaluated expression: " << fullExpression << " -> Result: " << result << std::endl;
    return result;
}