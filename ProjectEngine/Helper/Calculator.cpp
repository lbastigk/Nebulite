#include "Calculator.h"


double Calculator::performOperation(const std::string& rule, double operand1, double operand2) {
    auto it = operationMap().find(rule);
    if (it != operationMap().end()) {
        return it->second(operand1, operand2);
    }
    return 0.0; // Default value if the operation is not found
}

bool Calculator::operationExists(std::string key) {
    return (operationMap().find(key)!= operationMap().end());
}

std::map<std::string, std::function<double(double, double)>>& Calculator::operationMap() {
    static std::map<std::string, std::function<double(double, double)>> instance;
    if (instance.empty()) {
        instance[namenKonvention.calculator.add.c_str()] = [](double a, double b) { return a + b; };
        instance[namenKonvention.calculator.sin_b.c_str()] = [](double a, double b) { return std::sin(b); };
        instance[namenKonvention.calculator.cos_b.c_str()] = [](double a, double b) { return std::cos(b); };
        instance[namenKonvention.calculator.a_sin_b.c_str()] = [](double a, double b) { return a * std::sin(b); };
        instance[namenKonvention.calculator.a_cos_b.c_str()] = [](double a, double b) { return a * std::cos(b); };
        instance[namenKonvention.calculator.b_sin_a.c_str()] = [](double a, double b) { return b * std::sin(a); };
        instance[namenKonvention.calculator.b_cos_a.c_str()] = [](double a, double b) { return b * std::cos(a); };
        instance[namenKonvention.calculator.set.c_str()] = [](double a, double b) { return b; };
        instance[namenKonvention.calculator.setNoSkip.c_str()] = [](double a, double b) { return b; };
    }
    return instance;
}

