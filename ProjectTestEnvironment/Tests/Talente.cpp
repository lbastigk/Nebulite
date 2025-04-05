#include "TestEnv.h"

int TestEnv::_Talente::testTalentJson(int argc, char* argv[]) {
    Talente talente;
    std::cout << talente.serialize();

    Time::wait(5000);
}