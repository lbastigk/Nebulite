#include "TestEnv.h"

#include "Time.h"
#include "Invoke.h"

int TestEnv::_Invoke::example(int argc, char* argv[]) {
    std::cout << std::endl << std::endl;
    std::cout << "//------------------------------------------------" << std::endl;
    std::cout << "This test is used to check logical expression handling with invoke..." << std::endl << std::endl;


    //------------------------------------------------
    // Create docs

    // Create and populate self document
    rapidjson::Document selfDoc;
    JSONHandler::Set::Any<int>(selfDoc, "X",                    25);
    JSONHandler::Set::Any<int>(selfDoc, "Moving",               0);

    // Create and populate other document
    rapidjson::Document otherDoc;
    otherDoc.SetObject();
    JSONHandler::Set::Any<int>(otherDoc,"X",                    10);
    JSONHandler::Set::Any<int>(otherDoc,"isPlayer",             1);
    JSONHandler::Set::Any<int>(otherDoc,"closestObjectRight",   20);
    JSONHandler::Set::Any<int>(otherDoc,"Moving",               0);


    // Create and populate global document
    rapidjson::Document globalDoc;
    JSONHandler::Set::Any<int>(globalDoc,"quest.stage",2);

    // Invoke
    Invoke invoke;

    //------------------------------------------------
    // Show docs
    std::cout << "Self:" << std::endl;
    std::cout << JSONHandler::serialize(selfDoc)   << std::endl;
    std::cout << "Other:" << std::endl;
    std::cout << JSONHandler::serialize(otherDoc)  << std::endl;
    std::cout << "Global:" << std::endl;
    std::cout << JSONHandler::serialize(globalDoc) << std::endl;

    //------------------------------------------------
    std::cout << std::endl << std::endl;
    std::cout << "Sample expression to evaluate" << std::endl << std::endl;
    std::string expr = "$( $(self.X) - $(other.X) )";  // Should evaluate to 15
    std::cout << expr << std::endl;
    std::string resolved = invoke.resolveVars(expr, selfDoc, otherDoc, globalDoc);
    std::cout << "Resolved expression: " << resolved << std::endl;

    //------------------------------------------------
    std::cout << std::endl << std::endl;
    std::cout << "Another test: logic gate" << std::endl << std::endl;
    std::string logicalExpr = "$( $(other.isPlayer) and $($(other.closestObjectRight) > 15) and $(not($(other.Moving))) )";
    std::cout << logicalExpr << std::endl;
    std::string logicalResult = invoke.resolveVars(logicalExpr, selfDoc, otherDoc, globalDoc);
    std::cout << "Logical result: " << logicalResult << std::endl << std::endl;

    std::cout << "$(other.isPlayer):" 
    << invoke.resolveVars("$(other.isPlayer)",             selfDoc, otherDoc, globalDoc) << std::endl;
    std::cout << "$(other.closestObjectRight):" 
    << invoke.resolveVars("$(other.closestObjectRight)",   selfDoc, otherDoc, globalDoc) << std::endl;
    std::cout << "$(not($(other.Moving))):" 
    << invoke.resolveVars("$(not($(other.Moving)))",       selfDoc, otherDoc, globalDoc) << std::endl;

    //------------------------------------------------
    // Expect "15" and "1" as outputs
    std::cout << "Output should be 15 and 1" << std::endl;
    return 0;
}


int TestEnv::_Invoke::resolveVars(int argc, char* argv[]){
    rapidjson::Document self,other,global;

    Invoke invoke;

    JSONHandler::Set::Any<double>(global,"pi",3.141);
    JSONHandler::Set::Any<double>(global,"t",1.0);
    JSONHandler::Set::Any<double>(self,"U",100.0);
    JSONHandler::Set::Any<double>(self,"f",50.0);
    JSONHandler::Set::Any<double>(other,"U",100.0);
    JSONHandler::Set::Any<double>(other,"f",50.0);

    std::cout << JSONHandler::serialize(self);
    std::cout << JSONHandler::serialize(other);
    std::cout << JSONHandler::serialize(global);

    std::string toSolve = "$(self.U) * sin(2*$(global.pi)*$(self.f)*$(global.t))";
    //toSolve = "$(self.U)";
    //toSolve = "100";

    int amt = 10000;
    std::cout << std::endl << std::endl << "----------------------------------" << std::endl;
    std::cout << "Solving:" << std::endl;
    std::cout << toSolve << std::endl;
    std::cout << amt << " times..." << std::endl;

    

    auto t = Time::gettime();
    for (int i=1;i<amt;i++){
        std::string result = invoke.resolveVars(toSolve,self,other,global);
        volatile double val = invoke.evaluateExpression(result);
    }
    std::string result = invoke.resolveVars(toSolve,self,other,global);
    std::cout << "After var input:   " << result << std::endl;
    std::cout << "After using exprtk: " << invoke.evaluateExpression(result) << std::endl;
    std::cout << "Runtime: " << Time::getruntime(t) << std::endl;

    return 0;
}