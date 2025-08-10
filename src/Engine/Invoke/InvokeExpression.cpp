#include "InvokeExpression.h"



void Nebulite::InvokeExpression::compileIfExpression(Entry& entry) {
    if (entry.type == Entry::Type::eval) {
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

        // Check if variable exists in variables vector:
        bool found = false;
        for(auto var : variables) {
            if(var.name == entry.str) {
                found = true;
                break;
            }
        }
        if(!found) {
            // Replace dots in name for entry
            std::replace(entry.str.begin(), entry.str.end(), '.', '_');

            // Initialize with reference to document and cache register
            //std::cout << "Registering variable: " << entry.str  << ":" << entry.key << " : " << entry.from << std::endl;
            std::shared_ptr<Nebulite::VirtualDouble> vd = std::make_shared<Nebulite::VirtualDouble>(entry.key, documentCache);
            std::shared_ptr<vd_entry> vde = std::make_shared<vd_entry>(vd, entry.from, entry.key, entry.str);
            virtualDoubles.push_back(vde);

            // Push back into variable entries
            variables.push_back({
                vde->te_name.c_str(),
                vde->virtualDouble->ptr(),
                TE_VARIABLE,
                nullptr
            });
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

// TODO: Improvements possible
// - support for $f() and $i() possibly needed...
std::string Nebulite::InvokeExpression::modifyTextToTeConform(std::string str) {
    // Find and replace all occurrences of {...} patterns
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
            
            // Replace the entire $(...) with content
            str.replace(pos, end - pos, content );
            pos += content.length() + 2; // Move past the replacement
        } else {
            // Unmatched parentheses, move past this $
            pos += 2;
        }
    }

    // Remove all $
    str.erase(std::remove(str.begin(), str.end(), '$'), str.end());

    // Check parenthesis count
    int open = 0, close = 0;
    for (char c : str) {
        if (c == '(') open++;
        else if (c == ')') close++;
    }
    if (open != close) {
        std::cerr << "Unmatched parentheses in expression: " << str << std::endl;
        return "NaN";
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
            entry.key = entry.str.substr(6, entry.str.size() - 7); // Remove "(self." and ")"
        } else if (entry.str.find("(other.") == 0) {
            entry.from = Entry::From::other;
            entry.key = entry.str.substr(7, entry.str.size() - 8); // Remove "(other." and ")"
        } else if (entry.str.find("(global.") == 0) {
            entry.from = Entry::From::global;
            entry.key = entry.str.substr(8, entry.str.size() - 9); // Remove "(global." and ")"
        }
        else if (entry.str.find("(.") == 0) {
            entry.from = Entry::From::resource;
            entry.key = entry.str.substr(1, entry.str.size() - 2); // Remove "(" and ")"
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

//------------------------------
// Public:

Nebulite::InvokeExpression::InvokeExpression() {
    clear();
}

void Nebulite::InvokeExpression::parse(const std::string& expr, Nebulite::DocumentCache& documentCache){
    clear();

    fullExpression = expr;
    this->documentCache = &documentCache;

    parseIntoEntries(expr, entries);

    // Ensure proper naming in all eval entries
    for (auto& entry : entries) {
        if (entry.type == Entry::Type::eval && entry.str.find('$') != std::string::npos) {
            // All $(name.name1) -> (name_name1)
            // NOTE: ONLY INSIDE $() !
            // E.g:
            // '$( 0.99 * $($(other.physics.vX) - (2 * $(self.physics.mass) )'
            entry.str = modifyTextToTeConform(entry.str);
        }
    }

    // Now compile all entries:
    for (auto& entry : entries) {
        compileIfExpression(entry);
    }

    // DEBUG
    /*
    std::cout << "Parsed expression: " << fullExpression << std::endl;
    std::cout << "Entries:" << std::endl;
    for (const auto& entry : entries) {
        switch(entry.type) {
            case Entry::Type::variable:
                std::cout << std::setw(19) << "Variable: " << entry.str << " (from: " << (entry.from == Entry::From::self ? "self" : entry.from == Entry::From::other ? "other" : entry.from == Entry::From::global ? "global" : "resource") << ")" << std::endl;
                break;
            case Entry::Type::eval:
                std::cout << std::setw(19) << "Eval: " << entry.str << std::endl;
                break;
            case Entry::Type::text:
                std::cout << std::setw(19) << "Text: " << entry.str << std::endl;
                break;
            default:
                std::cout << "Unknown type" << std::endl;
        }
    }
    std::cout << "Variables:" << std::endl;
    for (const auto& var : variables) {
        if(var.type == TE_VARIABLE)std::cout << std::setw(19) << "Variable: " << var.name << std::endl;
    }

    //*/
}

std::string Nebulite::InvokeExpression::eval(Nebulite::JSON* current_self, Nebulite::JSON* current_other, Nebulite::JSON* current_global) {
    // Since this method may be called by multiple threads, 
    // we need a way where each thread has its own VirtualDouble:cache
    for (auto& entry : virtualDoubles) {
        switch(entry->from){
            case Entry::From::self:
                entry->virtualDouble->updateCache(current_self);
                break;
            case Entry::From::other:
                entry->virtualDouble->updateCache(current_other);
                break;
            case Entry::From::global:
                entry->virtualDouble->updateCache(current_global);
                break;
            case Entry::From::resource:
                entry->virtualDouble->updateCache(nullptr);
                break;
        }
    }

    std::string result = "";
    std::string token;
    for (auto& entry : entries) {
        token = "";
        switch (entry.type){
            case Entry::variable:
                // Variables default to 0

                if (entry.from == Entry::self) {
                    if(current_self == nullptr){
                        std::cerr << "Error: Null self reference in expression: " << entry.key << std::endl;
                        return "0";
                    }
                    token = current_self->get<std::string>(entry.key.c_str(), "0");
                } else if (entry.from == Entry::other) {
                    if(current_other == nullptr) {
                        std::cerr << "Error: Null other reference in expression: " << entry.key << std::endl;
                        return "0";
                    }
                    token = current_other->get<std::string>(entry.key.c_str(), "0");
                } else if (entry.from == Entry::global) {
                    if (current_global == nullptr) {
                        std::cerr << "Error: Null global reference in expression: " << entry.key << std::endl;
                        return "0";
                    }
                    token = current_global->get<std::string>(entry.key.c_str(), "0");
                } else if (entry.from == Entry::resource) {
                    if (globalCache == nullptr) {
                        std::cerr << "Error: Null globalCache reference in expression: " << entry.key << std::endl;
                        return "0";
                    }
                    token = globalCache->getData<std::string>(entry.key.c_str(), "0");
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