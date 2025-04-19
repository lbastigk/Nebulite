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
    std::cout << std::endl << std::endl;
    std::cout << "//------------------------------------------------" << std::endl;
    std::cout << "Checking grav with invoke fully inside render loop" << std::endl;

    Renderer Renderer;
    Renderer.setFPS(60);

    // OBJECTS
    std::vector<RenderObject> bodies;

    RenderObject obj1;
    obj1.valueSet<int>(namenKonvention.renderObject.pixelSizeX,10);
    obj1.valueSet<int>(namenKonvention.renderObject.pixelSizeY,10);
    obj1.valueSet<double>("physics.mass",50.0);
    obj1.valueSet<double>("posX",350); //renderer expects int here, but should automatically convert
    obj1.valueSet<double>("posY",500); //renderer expects int here, but should automatically convert
    obj1.valueSet<double>("physics.isGrav",1.0);
    obj1.valueSet<double>("physics.aX", 0.0);
    obj1.valueSet<double>("physics.aY", 0.0);
    obj1.valueSet<double>("physics.vX", 0.0);
    obj1.valueSet<double>("physics.vY", 0.0);
    //bodies.push_back(obj1);

    RenderObject obj2;
    obj2.valueSet<int>(namenKonvention.renderObject.pixelSizeX,10);
    obj2.valueSet<int>(namenKonvention.renderObject.pixelSizeY,10);
    obj2.valueSet<double>("physics.mass",50.0);
    obj2.valueSet<double>("posX",650); //renderer expects int here, but should automatically convert
    obj2.valueSet<double>("posY",500); //renderer expects int here, but should automatically convert
    obj2.valueSet<double>("physics.isGrav",1.0);
    obj2.valueSet<double>("physics.aX", 0.0);
    obj2.valueSet<double>("physics.aY", 0.0);
    obj2.valueSet<double>("physics.vX", 0.0);
    obj2.valueSet<double>("physics.vY", 0.0);
    //bodies.push_back(obj2);

    // SUN
    RenderObject obj3;
    obj3.valueSet<std::string>(namenKonvention.renderObject.imageLocation,"./Resources/Sprites/TEST100P/17.bmp");
    obj3.valueSet<int>(namenKonvention.renderObject.pixelSizeX,50);
    obj3.valueSet<int>(namenKonvention.renderObject.pixelSizeY,50);
    obj3.valueSet<double>("physics.mass",200000.0);
    obj3.valueSet<double>("posX",500); //renderer expects int here, but should automatically convert
    obj3.valueSet<double>("posY",500); //renderer expects int here, but should automatically convert
    obj3.valueSet<double>("physics.isGrav",1.0);
    obj3.valueSet<double>("physics.aX", 0.0);
    obj3.valueSet<double>("physics.aY", 0.0);
    obj3.valueSet<double>("physics.vX", 10.0);
    obj3.valueSet<double>("physics.vY", 0.0);
    bodies.push_back(obj3);

    std::cerr << "Start Values for Bodies:" << std::endl;
    for (auto& body : bodies){
        std::cerr << body.serialize() << std::endl;
    }

    // Global Values
    rapidjson::Document global;
    JSONHandler::Set::Any<double>(global,"dt",0);
    JSONHandler::Set::Any<double>(global,"G",0.1);

    // Grav ruleset

    // Only attract if ids are different
    InvokeCommand aX;
    aX.logicalArg="($(self.id) != $(other.id)) and $(other.physics.isGrav)";
    aX.selfChangeType="add";
    aX.selfKey="physics.aX";
    aX.selfValue=   "$(( $(global.G) * $(other.physics.mass) * ( $(other.posX) - $(self.posX) ))  / ( ( ($(other.posX) - $(self.posX))^2 + ($(other.posY) - $(self.posY))^2 + 1e-3) )^(3/2))";
    aX.otherChangeType="add";
    aX.otherKey="physics.aX";
    aX.otherValue=  "$(( $(global.G) * $(self.physics.mass)  * ( $(self.posX)  - $(other.posX) )) / ( ( ($(other.posX) - $(self.posX))^2 + ($(other.posY) - $(self.posY))^2 + 1e-3) )^(3/2))";

    InvokeCommand aY;
    aY.logicalArg="($(self.id) != $(other.id)) and $(other.physics.isGrav)";
    aY.selfKey="physics.aY";
    aY.selfChangeType="add";
    aY.selfValue=   "$(( $(global.G) * $(other.physics.mass) * ( $(other.posY) - $(self.posY) ))  / ( ( ($(other.posX) - $(self.posX))^2 + ($(other.posY) - $(self.posY))^2 + 1e-3) )^(3/2))";
    aY.otherKey="physics.aY";
    aY.otherChangeType="add";
    aY.otherValue=  "$(( $(global.G) * $(self.physics.mass)  * ( $(self.posY)  - $(other.posY) )) / ( ( ($(other.posX) - $(self.posX))^2 + ($(other.posY) - $(self.posY))^2 + 1e-3) )^(3/2))";

    // Velocity ruleset

    // apply only to itself (id-match)
    InvokeCommand vX;
    vX.logicalArg="$(self.id) == $(other.id)";
    vX.selfKey="physics.vX";
    vX.selfChangeType="add";
    vX.selfValue="$(self.physics.aX) * $(global.dt)";

    InvokeCommand vY;
    vY.logicalArg="$(self.id) == $(other.id)";
    vY.selfKey="physics.vY";
    vY.selfChangeType="add";
    vY.selfValue="$(self.physics.aY) * $(global.dt)";

    // Position ruleset

    // apply only to itself (id-match)
    InvokeCommand pX;
    pX.logicalArg="$(self.id) == $(other.id)";
    pX.selfKey="posX";
    pX.selfChangeType="add";
    pX.selfValue="$(self.physics.vX) * $(global.dt)";

    InvokeCommand pY;
    pY.logicalArg="$(self.id) == $(other.id)";
    pY.selfKey="posY";
    pY.selfChangeType="add";
    pY.selfValue="$(self.physics.vY) * $(global.dt)";

    // Insert objects
    for (auto& o : bodies) {
        // make new doc from string
        //o.appendInvoke(aX);
        //o.appendInvoke(aY);
        //o.appendInvoke(vX);
        //o.appendInvoke(vY);
        o.appendInvoke(pX);
        o.appendInvoke(pY);

        // Append
        Renderer.append(o);
    }

    // Invoke Object
    Invoke Invoke(global);
    Renderer.appendInvokePtr(&Invoke);
    
    //General Variables
    uint64_t currentTime = Time::gettime();
    uint64_t lastTime = Time::gettime();

    bool quit = false;
    int event = 0;
    while (!quit) {
        if (Renderer.timeToRender()) {
            // —– compute dt —–
            currentTime = Time::gettime();
            JSONHandler::Set::Any<double>(global, "dt", (currentTime - lastTime) / 1000.0);
            lastTime = currentTime;

            // Update
            Renderer.update();

            //Event handling
            event = Renderer.handleEvent();

            //Render Current instances
            Renderer.renderFrame();

            //Render FPS
            Renderer.renderFPS();

            // Present the renderer
            Renderer.showFrame();

            //Analyze event:
            switch (event) {
            case SDL_QUIT:
                quit = true;
                break;
            }
            
            //Platform::clearScreen();
            //std::cerr << JSONHandler::serialize(global);
            //std::cerr << Renderer.serializeEnvironment();
        }
    }
    //End of Program!
    Renderer.destroy();

    return 0;
}
