#include "TestEnv.h"


void TestEnv::_Kreatur::serialize() {
    Kreatur tee;
    std::cout << tee.serialize();
    std::cout << "\n\nPress any key to return.\n";
    Time::wait(500);
    Platform::flushKeyboardInput();
    (void)Platform::getCharacter();
}
