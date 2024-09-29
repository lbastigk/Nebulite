#pragma once

#include "NamenKonventionen.h"

#include <cmath>
#include <iostream>
#include <functional>
#include <map>
#include <string>

class Calculator {
public:
    static double performOperation(const std::string& rule, double operand1, double operand2);

    static bool operationExists(std::string key);

private:
    static std::map<std::string, std::function<double(double, double)>>& operationMap();
};
