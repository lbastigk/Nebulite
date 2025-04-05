#include "TestEnv.h"

int TestEnv::_Template::testMyTemplate(int argc, char* argv[]) {
    MyTemplate temp1;
    MyTemplate temp2;
    temp1.valueSet("testString", "zero");
    temp2.valueSet("testString2", "zero2");
    std::cout << "Obj1:\n";
    std::cout << temp1.serialize();
    std::cout << "\n";

    std::cout << "Obj2:\n";
    std::cout << temp2.serialize();
    std::cout << "\n";

    std::cout << "Obj2 copy to Obj1...\n";
    temp1 = temp2;
    std::cout << "Obj1:\n";
    std::cout << temp1.serialize();
    std::cout << "\n";
    std::cout << "Obj2:\n";
    std::cout << temp2.serialize();
    std::cout << "\n";

    Time::wait(5000);
}