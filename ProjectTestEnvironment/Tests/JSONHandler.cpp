#include "TestEnv.h"


void TestEnv::_JSONHandler::speed(int loopAmount) {
    uint64_t starttime = Time::gettime();

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
    for (volatile int i = 0; i < loopAmount; i++) {
        if (doc.IsObject()) {
            doc.RemoveAllMembers();
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

    //--------------------------------------------
    //Printing doc once
    std::cout << JSONHandler::serialize(doc);

    //--------------------------------------------
    //Show elapsed time
    printf("\n\nTime taken: %i ms\n", (int)(Time::gettime() - starttime));
};

void TestEnv::_JSONHandler::setGet() {
    rapidjson::Document doc;

    JSONHandler::Set::Any(doc, namenKonvention.MyTemplate._self, namenKonvention.MyTemplate.bsp1);
    std::cout << JSONHandler::Get::Any<std::string>(doc, namenKonvention.MyTemplate._self, "");
}

void TestEnv::_JSONHandler::KeyNesting() {
    rapidjson::Document mainDoc;
    
    //insert intop main doc
    JSONHandler::Set::Any<double>(mainDoc, std::string("level1-double"), 3.14);
    JSONHandler::Set::Any<int>(mainDoc, std::string("level1-level2-int"), 10);

    //first step: get
    std::cout << "Getting:\n-------------------------------------------------\n";
    std::cout << "Main Doc:\n\n" << JSONHandler::serialize(mainDoc) << "\n\n";
    std::cout << "level 2 int:" << JSONHandler::Get::Any<int>(mainDoc, std::string("level1-level2-int"),0);
    
    //second step, setting int inside level2 to another value
    std::cout << "\n\nSetting int to 11:\n-------------------------------------------------\n";
    JSONHandler::Set::Any<int>(mainDoc, std::string("level1-level2-int"),11);

    //Check if it's correct, show full doc and test get function again.
    std::cout << "Main Doc:\n\n" << JSONHandler::serialize(mainDoc) << "\n\n";
    std::cout << "level 2 int:" << JSONHandler::Get::Any<int>(mainDoc, std::string("level1-level2-int"), 0);

    //For viewing result
    Time::wait(20000);

}

void TestEnv::_JSONHandler::listOfKeys() {
    rapidjson::Document doc;
    JSONHandler::Set::Any(doc, "test1", "lol");
    JSONHandler::Set::Any(doc, "test2", 123);
    JSONHandler::Set::Any(doc, "test3", 3.14);

    std::vector<std::string> keys;

    JSONHandler::Get::listOfKeys(doc, keys);

    for (const auto& key : keys) {
        std::cout << key << "\n";
    }

    Time::wait(5000);
}


//Sets a value of the doc for 'amount' many times. Value increases by 1 with each iteration
//Return value should match amount on return!
//Value type is double
//Debug to see value each 100 iterations
double TestEnv::_JSONHandler::Memoryleak::setAny(UINT64 amount, bool debug) {
    rapidjson::Document doc;
    double fromDoc = 0;
    for (UINT64 i = 0; i < amount; i++) {
        JSONHandler::Set::Any<double>(doc, "tee", (double)i+1.0);
        fromDoc = JSONHandler::Get::Any<double>(doc, "tee");
        if (debug && !(i % 100)) {
            std::cout << "Value after " << i << " iterations: " << fromDoc << "\n";
        }
    }
    return fromDoc;
}

//Sets a value of the doc for 'amount' many times. Value in subdoc increases by 1 with each Iteration
//Subdoc is rebuild from main doc with each iteration
//Return string is entire doc
//Debug to see full doc each 100 iterations
std::string TestEnv::_JSONHandler::Memoryleak::addGetSubDoc(UINT64 amount, bool debug) {
    //main Doc with static Variable
    rapidjson::Document doc;
    JSONHandler::Set::Any<double>(doc, "staticVar", 3.14);

    //Subdoc
    rapidjson::Document subDoc;

    for (UINT64 i = 0; i < amount; i++) {
        //Get back subdoc
        JSONHandler::Get::subDoc(doc, "subDoc", subDoc);

        //Set subdoc
        JSONHandler::Set::Any<int>(subDoc, "subvar", (double)i+1.0);
        
        //Add to main doc
        JSONHandler::Set::subDoc(doc, "subDoc", subDoc);    

        if (debug && !(i % 100)) {
            std::cout << "Value after " << i << " iterations: " << JSONHandler::serialize(doc) << "\n";
        }
    }

    //Return entire doc
    return JSONHandler::serialize(doc);
}

//Full memory leak test
void TestEnv::_JSONHandler::Memoryleak::fullTest(UINT64 amount) {
    uint64_t starttime;

    std::cout << "Testing for memory leaks\n";
    
    std::cout << "Total memory used: " << DsaDebug::getMemoryUsagekB() << "kB\n";
    
    std::cout << "---------------------------\n\n";

    std::cout << "Test 1: setting double,        \t" << StringHandler::uint64ToStringWithGroupingAndPadding(amount, 12) << " Iterations...\n";
    starttime = Time::gettime();
    double checksum = setAny(amount);
    std::cout << "\tTime taken:" << Time::getruntime(starttime) << "\n";
    std::cout << "\tTotal memory used after test: " << DsaDebug::getMemoryUsagekB() << "kB\n";
    std::cout << "\tChecksum: " << checksum << "\n\n";

    std::cout << "Test 2: setting/getting subdoc,\t" << StringHandler::uint64ToStringWithGroupingAndPadding(amount,12) << " Iterations...\n";
    starttime = Time::gettime();
    std::string checkstring = addGetSubDoc(amount);
    std::cout << "\tTime taken:" << Time::getruntime(starttime) << "\n";
    std::cout << "\tTotal memory used after test: " << DsaDebug::getMemoryUsagekB() << "kB\n";
    std::cout << "\tCheckstring:\n";
    DsaDebug::printWithPrefix(checkstring, "\t");
    std::cout << "\n\n";
    
}

//Also good for benchmarking
// Times in ms for 1 000 000 iterations:
//[2024-02-14] : 2564
void TestEnv::_JSONHandler::Retired::testMemLeak() {
    //main doc
    rapidjson::Document doc;
    JSONHandler::Set::Any<double>(doc, "double", 3.14);

    rapidjson::Document loc;

    //Copy a subdoc into doc many times to check for leakage
    auto start = Time::gettime();


    for (UINT64 i = 0; i < 1000000; i++) {
        //create subdoc, fill it with some stuff
        //needs to be recreated each time, as JSONHandler::Set::subDoc will delete this one
        rapidjson::Document subdoc;
        JSONHandler::Set::Any<int>(subdoc, "subvar", 0);
        JSONHandler::Set::Any<int>(subdoc, "subvar2", 0);

        //DEBUG, for checking docs
        if (false) {
            std::cout << "DEBUG before set\n";
            std::cout << "---------------------------\n";
            std::cout << "SubDoc Template: \n" << JSONHandler::serialize(subdoc) << "\n\n";
            std::cout << "Full doc: \n" << JSONHandler::serialize(doc) << "\n\n";
            std::cout << "SubDoc Copy: \n" << JSONHandler::serialize(loc) << "\n\n";
        }
        JSONHandler::empty(loc);

        //Add subdoc
        JSONHandler::Set::subDoc(doc, "sd", subdoc);

        //Get subdoc back
        JSONHandler::Get::subDoc(doc, "sd", loc);
    }
    std::cout << Time::getruntime(start) << "\n\n\n";
    std::cout << JSONHandler::serialize(doc) << "\n";
    std::cout << JSONHandler::serialize(loc) << "\n";
    Time::wait(10000);                       //Just to wait some seconds as function ends
}
