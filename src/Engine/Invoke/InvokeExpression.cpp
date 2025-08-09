#include "InvokeExpression.h"

Nebulite::InvokeExpression::InvokeExpression() {
    // Initialize with default expression "0"
    parse("0", documentCache);
}

void Nebulite::InvokeExpression::compileIfExpression(Entry& entry) {
    if (entry.type == Entry::Type::eval) {
        // Compile the expression using TinyExpr
        int error;
        entry.expression = te_compile(entry.str.c_str(), variables.data(), variables.size(), &error);
        if (error) {
            std::cerr << "Error compiling expression: " << entry.str << " Error code: " << error << std::endl;
        }
    }
}


void Nebulite::InvokeExpression::registerIfVariable(Entry& entry){
    if(entry.type == Entry::Type::variable) {
        // TODO: Using the Virtual Double to register pointers...
        // Example:
        if(entry.type == Entry::Type::variable) {
            Nebulite::JSON* json_pointer = nullptr;
            if(entry.from == Entry::From::self && self != nullptr) {
                json_pointer = self;
            } else if (entry.from == Entry::From::other && other != nullptr) {
                json_pointer = other;
            } else if (entry.from == Entry::From::global && global != nullptr) {
                json_pointer = global;
            } else if (entry.from == Entry::From::resource) {
                json_pointer = nullptr;
            }

            // Initialize with reference to document and cache register
            std::shared_ptr<Nebulite::VirtualDouble> vd = std::make_shared<Nebulite::VirtualDouble>(json_pointer, entry.str, documentCache);
            virtualDoubles.push_back(vd);

            // Push back into variable entries
            variables.push_back({ entry.str.c_str(), &vd });
        }
    }
}

void Nebulite::InvokeExpression::modifyTextToTeConform(Entry& entry) {
    // TODO:    Modify text: $(global.myKey) -> global_myKey
}

void Nebulite::InvokeExpression::setEntryContext(Entry& entry) {
    // If entry contains $ inside, it is now an eval entry
    if(entry.type == Entry::Type::variable && entry.str.find("$") != std::string::npos){
        entry.type = Entry::Type::eval;
        return;
    }

    // Check what kind of variable it is
    if (entry.type == Entry::Type::variable) {
        // set type, remove self/other/global from beginning:
        if (entry.str.find("self.") == 0) {
            entry.from = Entry::From::self;
        } else if (entry.str.find("other.") == 0) {
            entry.from = Entry::From::other;
        } else if (entry.str.find("global.") == 0) {
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

void Nebulite::InvokeExpression::parse(const std::string& expr, Nebulite::DocumentCache* documentCache){
    fullExpression = expr;
    this->documentCache = documentCache;

    Entry currentEntry;
    int depth = 0;
    char lastChar = '\0';
    for(int i = 0; i < expr.size(); i++) {
        char currentChar = expr[i];
        // Determine the type of entry based on the character
        if (currentChar == '$' && depth == 0) {
            currentEntry.type = Entry::Type::variable;
        }
        else if(lastChar == '$' && currentChar == 'i' && depth == 0) {
            currentEntry.cast = Entry::CastType::to_int;
        }
        else if(lastChar == '$' && currentChar == 'f' && depth == 0) {
            currentEntry.cast = Entry::CastType::to_float;
        }
        else if(currentChar == '(') {
            depth++;
            currentEntry.str += currentChar;
        }
        else if(currentChar == ')') {
            depth--;
            currentEntry.str += currentChar;
            if(depth == 0){
                setEntryContext(currentEntry);          // Sets context: Variable:self-other-global-resource / eval
                modifyTextToTeConform(currentEntry);    // Modify text: $(global.myKey) -> global_myKey
                registerIfVariable(currentEntry);       // Register variable for TinyExpr evaluation

                // Push back
                entries.push_back(currentEntry);
                currentEntry = Entry();
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
        // If there is any remaining entry, add it as a text entry
        setEntryContext(currentEntry);          // Sets context: Variable:self-other-global-resource / eval
        modifyTextToTeConform(currentEntry);    // Modify text: $(global.myKey) -> global_myKey
        registerIfVariable(currentEntry);       // Register variable for TinyExpr evaluation

        // Push back
        currentEntry.type = Entry::Type::text;
        entries.push_back(currentEntry);
    }

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

    std::string token;
    for (const auto& entry : entries) {
        token = "";
        switch (entry.type){
            case Entry::variable:
                if (entry.from == Entry::self && self != nullptr) {
                    token = self->get<std::string>(entry.str.c_str(), "");
                } else if (entry.from == Entry::other && other != nullptr) {
                    token = other->get<std::string>(entry.str.c_str(), "");
                } else if (entry.from == Entry::global && global != nullptr) {
                    token = global->get<std::string>(entry.str.c_str(), "");
                } else if (entry.from == Entry::resource && globalCache != nullptr) {
                    token = globalCache->getData<std::string>(entry.str, "");
                }
                break;

            case Entry::eval:
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

    return result;
}