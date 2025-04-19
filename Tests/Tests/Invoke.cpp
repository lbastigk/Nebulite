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
    std::string resolved = Invoke::resolveVars(expr, selfDoc, otherDoc, globalDoc);
    std::cout << "Resolved expression: " << resolved << std::endl;

    //------------------------------------------------
    std::cout << std::endl << std::endl;
    std::cout << "Another test: logic gate" << std::endl << std::endl;
    std::string logicalExpr = "$( $(other.isPlayer) and $($(other.closestObjectRight) > 15) and $(not($(other.Moving))) )";
    std::cout << logicalExpr << std::endl;
    std::string logicalResult = Invoke::resolveVars(logicalExpr, selfDoc, otherDoc, globalDoc);
    std::cout << "Logical result: " << logicalResult << std::endl << std::endl;

    std::cout << "$(other.isPlayer):" 
    << Invoke::resolveVars("$(other.isPlayer)",             selfDoc, otherDoc, globalDoc) << std::endl;
    std::cout << "$(other.closestObjectRight):" 
    << Invoke::resolveVars("$(other.closestObjectRight)",   selfDoc, otherDoc, globalDoc) << std::endl;
    std::cout << "$(not($(other.Moving))):" 
    << Invoke::resolveVars("$(not($(other.Moving)))",       selfDoc, otherDoc, globalDoc) << std::endl;

    //------------------------------------------------
    // Expect "15" and "1" as outputs
    std::cout << "Output should be 15 and 1" << std::endl;
    return 0;
}

int TestEnv::_Invoke::gravity(int argc, char* argv[]) {
    Renderer Renderer;
    Renderer.setFPS(60);

    // OBJECTS
    RenderObject obj1;
    obj1.valueSet<int>(namenKonvention.renderObject.pixelSizeX,10);
    obj1.valueSet<int>(namenKonvention.renderObject.pixelSizeY,10);
    obj1.valueSet<double>("physics.mass",50.0);
    obj1.valueSet<double>("posX",350); //renderer expects int here, but should automatically convert
    obj1.valueSet<double>("posY",500); //renderer expects int here, but should automatically convert
    obj1.valueSet<double>("physics.isGrav",1.0);
    obj1.valueSet<double>("physics.vX",0.0);
    obj1.valueSet<double>("physics.aX",0.0);
    obj1.valueSet<double>("physics.vY",-10.0);
    obj1.valueSet<double>("physics.aY",0.0);

    RenderObject obj2;
    obj2.valueSet<int>(namenKonvention.renderObject.pixelSizeX,10);
    obj2.valueSet<int>(namenKonvention.renderObject.pixelSizeY,10);
    obj2.valueSet<double>("physics.mass",50.0);
    obj2.valueSet<double>("posX",650); //renderer expects int here, but should automatically convert
    obj2.valueSet<double>("posY",500); //renderer expects int here, but should automatically convert
    obj2.valueSet<double>("physics.isGrav",1.0);
    obj2.valueSet<double>("physics.vX", 0.0);
    obj2.valueSet<double>("physics.aX", 0.0);
    obj2.valueSet<double>("physics.vY",10.0);
    obj2.valueSet<double>("physics.aY", 0.0);

    RenderObject obj3;
    obj3.valueSet<std::string>(namenKonvention.renderObject.imageLocation,"./Resources/Sprites/TEST100P/17.bmp");
    obj3.valueSet<int>(namenKonvention.renderObject.pixelSizeX,50);
    obj3.valueSet<int>(namenKonvention.renderObject.pixelSizeY,50);
    obj3.valueSet<int>("ID",2);
    obj3.valueSet<double>("physics.mass",200000.0);
    obj3.valueSet<double>("posX",500); //renderer expects int here, but should automatically convert
    obj3.valueSet<double>("posY",500); //renderer expects int here, but should automatically convert
    obj3.valueSet<double>("physics.isGrav",1.0);
    obj3.valueSet<double>("physics.vX", 0.0);
    obj3.valueSet<double>("physics.aX", 0.0);
    obj3.valueSet<double>("physics.vY", 0.0);
    obj3.valueSet<double>("physics.aY", 0.0);

    std::vector<RenderObject*> bodies = { &obj1, &obj2, &obj3};

    // Global Values
    rapidjson::Document global;
    JSONHandler::Set::Any<double>(global,"dt",0);
    JSONHandler::Set::Any<double>(global,"G",0.1);

    // for now, invoke is done manually. Later, the invokes should be coded inside the renderobject and called by the renderer itself
    InvokeCommand gravInvokeX;
    gravInvokeX.logicalArg = "$(other.physics.isGrav)";
    gravInvokeX.selfChangeType = "add";
    gravInvokeX.selfKey = "physics.aX";
    gravInvokeX.selfValue = "$(( $(global.G)  * $(other.physics.mass) * ( $(other.posX) - $(self.posX) ))  / ( ( ($(other.posX) - $(self.posX))^2 + ($(other.posY) - $(self.posY))^2 + 1e-3) )^(3/2))";
    gravInvokeX.otherChangeType = "add";
    gravInvokeX.otherKey = "physics.aX";
    gravInvokeX.otherValue = "$(( $(global.G) * $(self.physics.mass)  * ( $(self.posX)  - $(other.posX) )) / ( ( ($(other.posX) - $(self.posX))^2 + ($(other.posY) - $(self.posY))^2 + 1e-3) )^(3/2))";

    InvokeCommand gravInvokeY;
    gravInvokeY.logicalArg = "$(other.physics.isGrav)";
    gravInvokeY.selfChangeType = "add";
    gravInvokeY.selfKey = "physics.aY";
    gravInvokeY.selfValue =  "$(( $(global.G) * $(other.physics.mass) * ( $(other.posY) - $(self.posY) ))  / ( ( ($(other.posX) - $(self.posX))^2 + ($(other.posY) - $(self.posY))^2 + 1e-3) )^(3/2))";
    gravInvokeY.otherChangeType = "add";
    gravInvokeY.otherKey = "physics.aY";
    gravInvokeY.otherValue = "$(( $(global.G) * $(self.physics.mass)  * ( $(self.posY)  - $(other.posY) )) / ( ( ($(other.posX) - $(self.posX))^2 + ($(other.posY) - $(self.posY))^2 + 1e-3) )^(3/2))";

    // Invoke Object
    Invoke Invoke(global);
    
    //General Variables
    uint64_t currentTime = Time::gettime();
    uint64_t lastTime = Time::gettime();
    double dt;
    double pX,pY,vX,vY,aX,aY;

    bool quit = false;
    int event = 0;
    while (!quit) {
        if (Renderer.timeToRender()) {
            // —– compute and read dt —–
            currentTime = Time::gettime();
            JSONHandler::Set::Any<double>(global, "dt", (currentTime - lastTime) / 1000.0);
            dt = JSONHandler::Get::Any<double>(global, "dt", 0.0);
            lastTime = currentTime;

            // —– zero out previous accelerations —–
            for (auto* o : bodies) {
            o->valueSet<double>("physics.aX", 0.0);
            o->valueSet<double>("physics.aY", 0.0);
            }

            // —– queue up gravity invokes —–
            Invoke.append(gravInvokeX);
            Invoke.append(gravInvokeY);

            // —– apply to every pair, accumulating forces —–
            for (size_t i = 0; i < bodies.size(); ++i) {
            for (size_t j = i + 1; j < bodies.size(); ++j) {
                Invoke.check(*bodies[i], *bodies[j]);
            }
            }
            Invoke.clear();

            // —– integrate velocity & position —–
            for (auto* o : bodies) {
            double aX = o->valueGet<double>("physics.aX");
            double aY = o->valueGet<double>("physics.aY");
            double vX = o->valueGet<double>("physics.vX") + aX * dt;
            double vY = o->valueGet<double>("physics.vY") + aY * dt;
            o->valueSet<double>("physics.vX", vX);
            o->valueSet<double>("physics.vY", vY);

            double pX = o->valueGet<double>("posX");
            double pY = o->valueGet<double>("posY");
            o->valueSet<double>("posX", pX + vX * dt);
            o->valueSet<double>("posY", pY + vY * dt);
            }

            // Reinsert objects
            for (auto* o : bodies) {
                Renderer.append(*o);
            }

            //Event handling
            event = Renderer.handleEvent();

            //Render Current instances
            Renderer.renderFrame();

            //Render FPS
            Renderer.renderFPS();

            // Present the renderer
            Renderer.showFrame();

            //Delete objects
            Renderer.purgeObjects();

            //Analyze event:
            switch (event) {
            case SDL_QUIT:
                quit = true;
                break;
            }
        }
    }
    //End of Program!
    Renderer.destroy();

    return 0;
}
