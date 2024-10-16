#include "TestEnv.h"

void TestEnv::_Talente::testTalentJson() {
    Talente talente;
    std::cout << talente.serialize();

    Time::wait(5000);
}