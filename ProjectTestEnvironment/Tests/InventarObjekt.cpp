#include "TestEnv.h"

void TestEnv::_InventarObjekt::serialize() {
    InventarObjekt tee;
    std::cout << tee.serialize();

    Time::wait(5000);
}
