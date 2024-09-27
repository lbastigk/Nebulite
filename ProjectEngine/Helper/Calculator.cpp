#pragma once

#include "NamenKonventionen.cpp"

#include <iostream>
#include <functional>
#include <map>
#include <string>

class CalculatorOld {
public:
    template <typename T>
    static T performOperation(const std::string& rule, T operand1, T operand2) {
        auto it = operationMap<T>().find(rule);
        if (it != operationMap<T>().end()) {
            return it->second(operand1, operand2);
        }
        return static_cast<T>(0); // Default value if the operation is not found
    }

private:
    template <typename T>
    static std::map<std::string, std::function<T(T, T)>>& operationMap() {
        static std::map<std::string, std::function<T(T, T)>> instance;
        if (instance.empty()) {
            instance[namenKonvention.calculator.add.c_str()] = [](T a, T b) { return a + b; };
            instance[namenKonvention.calculator.a_sin_b.c_str()] = [](T a, T b) {return a * std::sin(b); };
            instance[namenKonvention.calculator.a_cos_b.c_str()] = [](T a, T b) {return a * std::sin(b); };
            instance[namenKonvention.calculator.b_sin_a.c_str()] = [](T a, T b) {return b * std::sin(a); };
            instance[namenKonvention.calculator.b_cos_a.c_str()] = [](T a, T b) {return b * std::sin(a); };
        }
        return instance;
    }
};

class Calculator {
public:
    static double performOperation(const std::string& rule, double operand1, double operand2) {
        auto it = operationMap().find(rule);
        if (it != operationMap().end()) {
            return it->second(operand1, operand2);
        }
        return 0.0; // Default value if the operation is not found
    }

    static bool operationExists(std::string key) {
        return (operationMap().find(key)!= operationMap().end());
    }

private:
    static std::map<std::string, std::function<double(double, double)>>& operationMap() {
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
};

class CALC {
    //Take string like:
    // 
    // "$self-posX$+$self-posY$"
    // 
    // or even for setting:
    // 
    // "$player-talente-handwerk-schmieden$:=10"
    //
    static double string(std::string toProcess) {

    }
};