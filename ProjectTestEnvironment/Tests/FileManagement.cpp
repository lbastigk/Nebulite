#include "TestEnv.h"


//Shows all files in ressources folder
void TestEnv::_FileManagement::testFileCollector(){
    FileManagement::FileTypeCollector ftc(std::string("Resources"),std::string(".bmp"));

    std::cout << StringHandler::parseArray(ftc.getFileDirectories(),"Entry %i :\t","\n");

    Time::wait(10000);
}

//Same as before, but with opening
void TestEnv::_FileManagement::testFileCollectorMenue() {
    FileManagement::FileTypeCollector ftc(std::string("Resources"), std::string(".bmp"));

    std::string options = StringHandler::parseArray(ftc.getFileDirectories(), "Entry %i :\t", "\n");

    int i = DsaDebug::menueScreen(options, 0, "Files available:\n\n", "\n\nPlease select one.",false);

    std::cout << "\n\nOpening:\t" << ftc.getFileDirectories().at(i) << " ...";

    FileManagement::openFileWithDefaultProgram(ftc.getFileDirectories().at(i));

    Time::wait(10000);
}

