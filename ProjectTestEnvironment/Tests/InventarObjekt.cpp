#include "TestEnv.h"

int TestEnv::_InventarObjekt::serialize(int argc, char* argv[]) {
    InventarObjekt tee;
    std::cout << tee.serialize();

    Time::wait(5000);
}
