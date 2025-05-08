#include "TestEnv.h"


int TestEnv::_JSONHandler::speed(int argc, char* argv[]) {
    // Args standard values
    std::string count = "100000";

    // Parse args
    FuncTree ft("speed");
    ft.attachArgument(&count,"--count","-c","Loop count for speed test");
    ft.parse(argc, argv);

    int loopAmount = atoi(count.c_str());

    

    //--------------------------------------------
    //Test Variables
    int testInt = 1;
    float testFloat = 2.718;
    double testDouble = 3.141;
    char testString[20] = "testString";
    std::string testStdString = "testStdString";

    std::map<std::string, std::string> testMap;
    testMap["key1"] = "value1";
    testMap["key2"] = "value2";

    std::vector<double> testVector;
    testVector.push_back(1.0);
    testVector.push_back(2.0);

    std::pair<int, std::string> testPair = std::make_pair(0, "pair");

    //Creating doc
    rapidjson::Document doc;

    //Looping for speed test
    std::cout << "Used memory in MB: " << DsaDebug::getMemoryUsagekB() / 1024.0 << std::endl;
    uint64_t starttime = Time::gettime();
    for (volatile int i = 0; i < loopAmount; i++) {
        if (doc.IsObject()) {
            JSONHandler::empty(doc);
        }

        //--------------------------------------------
        //Adding to doc
        JSONHandler::Set::Any(doc, "Int", testInt);
        JSONHandler::Set::Any(doc, "Float", testFloat);
        JSONHandler::Set::Any(doc, "Double", testDouble);
        JSONHandler::Set::Any(doc, "String", testString);
        JSONHandler::Set::Any(doc, "StdString", testStdString);
        JSONHandler::Set::Any(doc, "Map", testMap);
        JSONHandler::Set::Any(doc, "Vector", testVector);
        JSONHandler::Set::Any(doc, "Pair", testPair);

        //--------------------------------------------
        //Adding doc intself to doc
        rapidjson::Document doc2;
        doc2.CopyFrom(doc, doc.GetAllocator());
        JSONHandler::Set::Any(doc, "JSON", &doc2);
    }
    uint64_t endtime = Time::gettime();

    //--------------------------------------------
    //Printing doc once
    std::cout << JSONHandler::serialize(doc);

    //--------------------------------------------
    //Show elapsed time
    std::cout << std::endl;
    std::cout << std::endl;
    std::cout << "For: " << count << " many sets and gets" << std::endl;
    printf("Time taken: %i ms\n", (int)(endtime - starttime));
    std::cout << "Used memory in MB: " << DsaDebug::getMemoryUsagekB() / 1024.0 << std::endl;
    return 0;
};

int TestEnv::_JSONHandler::setGet(int argc, char* argv[]) {
    rapidjson::Document doc;

    JSONHandler::Set::Any(doc, namenKonvention.MyTemplate._self, namenKonvention.MyTemplate.bsp1);
    std::cout << JSONHandler::Get::Any<std::string>(doc, namenKonvention.MyTemplate._self, "");

    return 0;
}

int TestEnv::_JSONHandler::KeyNesting(int argc, char* argv[]) {
    rapidjson::Document mainDoc;
    
    //insert intop main doc
    JSONHandler::Set::Any<double>(mainDoc, std::string("level1.double"), 3.14);
    JSONHandler::Set::Any<int>(mainDoc, std::string("level1.level2.int"), 10);

    //first step: get
    std::cout << "Getting:\n-------------------------------------------------\n";
    std::cout << "Main Doc:\n\n" << JSONHandler::serialize(mainDoc) << "\n\n";
    std::cout << "level 2 int:" << JSONHandler::Get::Any<int>(mainDoc, std::string("level1.level2.int"),0);
    
    //second step, setting int inside level2 to another value
    std::cout << "\n\nSetting int to 11:\n-------------------------------------------------\n";
    JSONHandler::Set::Any<int>(mainDoc, std::string("level1.level2.int"),11);

    //Check if it's correct, show full doc and test get function again.
    std::cout << "Main Doc:\n\n" << JSONHandler::serialize(mainDoc) << "\n\n";
    std::cout << "level 2 int:" << JSONHandler::Get::Any<int>(mainDoc, std::string("level1.level2.int"), 0);

    std::cout << "Result should be: \n";
    std::cout << "level1.double = 3.14 \n";
    std::cout << "level1.level2.int = 10 or 11 \n";
    return 0;
}

int TestEnv::_JSONHandler::listOfKeys(int argc, char* argv[]) {
    rapidjson::Document doc;
    JSONHandler::Set::Any(doc, "test1", "lol");
    JSONHandler::Set::Any(doc, "test2", 123);
    JSONHandler::Set::Any(doc, "test3", 3.14);

    std::vector<std::string> keys;

    JSONHandler::Get::listOfKeys(doc, keys);

    for (const auto& key : keys) {
        std::cout << key << "\n";
    }

    return 0;
}

int TestEnv::_JSONHandler::leaks(int argc, char* argv[]) {
    // Start values
    std::string count = "100000";

    // Argument parsing
    FuncTree ft("leaks");
    ft.attachArgument(&count,"count","c","Loop count for leak test");
    ft.parse(argc, argv);

    int loopAmount = atoi(count.c_str());

    // Test data
    int testInt = 123;
    std::string testString = "leakTest";
    double testDouble = 3.14159;

    rapidjson::Document doc;

    double mem = DsaDebug::getMemoryUsagekB();
    std::cout << "Testing " << loopAmount << " times" << std::endl; 
    std::cout << "Initial memory in MiB: " << DsaDebug::getMemoryUsagekB() / 1024.0 << std::endl;

    for (volatile int i = 0; i < loopAmount; ++i) {
        // Set tests
        JSONHandler::Set::Any(doc, "intVal", testInt);
        JSONHandler::Set::Any(doc, "strVal", testString);
        JSONHandler::Set::Any(doc, "dblVal", testDouble);
    }
    double memSet = DsaDebug::getMemoryUsagekB();
    std::cout << "[SET] Memory increase in MiB: " << (memSet - mem) / 1024.0 << std::endl;
    for (volatile int i = 0; i < loopAmount; ++i) {
        // Get tests (optionally check retrieval works)
        int outInt = JSONHandler::Get::Any<int>(doc, "intVal", 0);
        std::string outStr = JSONHandler::Get::Any<std::string>(doc, "strVal", "");
        double outDbl = JSONHandler::Get::Any<double>(doc, "dblVal", 0.0);

        // Optionally validate result to force compiler to retain code
        volatile int tmp = outInt;
        volatile double tmp2 = outDbl;
        (void)tmp;
        (void)tmp2;
    }
    double memGet = DsaDebug::getMemoryUsagekB();
    std::cout << "[Get] Memory increase in MiB: " << (memSet - memGet) / 1024.0 << std::endl;


    return 0;
}


int TestEnv::_JSONHandler::full(int argc, char* argv[]){
    std::cout << "Full JSONHandler test..." << std::endl;
    std::cout << "--------------------------------------" << std::endl;
    std::cout << "Test 1: Speed" << std::endl;
    (void) speed(argc,argv);
    std::cout << "Test 2: Key nesting" << std::endl;
    (void) KeyNesting(argc,argv);
    std::cout << "Test 3: Leaks" << std::endl;
    (void) leaks(argc,argv);

    return 0;
}