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


    //------------------------------------------------
    // Renderer Settings
    Renderer Renderer;
    Renderer.setFPS(60);

    // Global Values
    rapidjson::Document global;
    JSONHandler::Set::Any<double>(global,"dt",0);
    JSONHandler::Set::Any<double>(global,"G",0.1);

    // Invoke Object
    Invoke Invoke(global);
    Renderer.appendInvokePtr(&Invoke);


    //------------------------------------------------
    // OBJECTS
    std::vector<RenderObject> bodies;

    RenderObject obj1;
    obj1.valueSet<int>(namenKonvention.renderObject.pixelSizeX,10);
    obj1.valueSet<int>(namenKonvention.renderObject.pixelSizeY,10);
    obj1.valueSet<double>("physics.mass",50.0);
    obj1.valueSet<double>("posX",350);
    obj1.valueSet<double>("posY",500);
    obj1.valueSet<double>("physics.isGrav",1.0);
    obj1.valueSet<double>("physics.aX", 0.0);
    obj1.valueSet<double>("physics.aY", 0.0);
    obj1.valueSet<double>("physics.vX",  3.0);
    obj1.valueSet<double>("physics.vY",-12.0);
    bodies.push_back(obj1);

    RenderObject obj2;
    obj2.valueSet<int>(namenKonvention.renderObject.pixelSizeX,10);
    obj2.valueSet<int>(namenKonvention.renderObject.pixelSizeY,10);
    obj2.valueSet<double>("physics.mass",50.0);
    obj2.valueSet<double>("posX",650);
    obj2.valueSet<double>("posY",500);
    obj2.valueSet<double>("physics.isGrav",1.0);
    obj2.valueSet<double>("physics.aX", 0.0);
    obj2.valueSet<double>("physics.aY", 0.0);
    obj2.valueSet<double>("physics.vX", -3.0);
    obj2.valueSet<double>("physics.vY", 12.0);
    bodies.push_back(obj2);

    // SUN
    RenderObject obj3;
    obj3.valueSet<std::string>(namenKonvention.renderObject.imageLocation,"./Resources/Sprites/TEST100P/17.bmp");
    obj3.valueSet<int>(namenKonvention.renderObject.pixelSizeX,50);
    obj3.valueSet<int>(namenKonvention.renderObject.pixelSizeY,50);
    obj3.valueSet<double>("physics.mass",90000.0);
    obj3.valueSet<double>("posX",500);
    obj3.valueSet<double>("posY",500);
    obj3.valueSet<double>("physics.isGrav",1.0);
    obj3.valueSet<double>("physics.aX", 0.0);
    obj3.valueSet<double>("physics.aY", 0.0);
    obj3.valueSet<double>("physics.vX", 0.0);
    obj3.valueSet<double>("physics.vY", 0.0);
    bodies.push_back(obj3);

    //------------------------------------------------
    // Gravity ruleset

    // Reset each loop
    InvokeCommand reset_aX;
    reset_aX.type="loop";
    reset_aX.logicalArg="1";
    reset_aX.selfKey="physics.aX";
    reset_aX.selfChangeType="set";
    reset_aX.selfValue=   "0";
    InvokeCommand reset_aY;
    reset_aY.type="loop";
    reset_aY.logicalArg="1";
    reset_aY.selfKey="physics.aY";
    reset_aY.selfChangeType="set";
    reset_aY.selfValue=   "0";

    // Only attract if ids are different
    // aX = G*m2*(x2-x1)/( (x2-x1)^2 + (y2-y1)^2 +eps)^(3/2)
    // aY = G*m2*(y2-y1)/( (x2-x1)^2 + (y2-y1)^2 +eps)^(3/2)
    InvokeCommand aX;
    aX.logicalArg="($(self.id) != $(other.id)) and $(other.physics.isGrav)";
    aX.selfChangeType="add";
    aX.selfKey="physics.aX";
    aX.selfValue=   "$(global.G) * $(other.physics.mass) * ( $(other.posX) - $(self.posX)  ) / ( ( ($(other.posX) - $(self.posX))^2 + ($(other.posY) - $(self.posY))^2 + 1)^(3/2) )";
    aX.otherChangeType="add";
    aX.otherKey="physics.aX";
    aX.otherValue=  "$(global.G) * $(self.physics.mass)  * ( $(self.posX)  - $(other.posX) ) / ( ( ($(other.posX) - $(self.posX))^2 + ($(other.posY) - $(self.posY))^2 + 1)^(3/2) )";

    InvokeCommand aY;
    aY.logicalArg="($(self.id) != $(other.id)) and $(other.physics.isGrav)";
    aY.selfKey="physics.aY";
    aY.selfChangeType="add";
    aY.selfValue=   "$(global.G) * $(other.physics.mass) * ( $(other.posY) - $(self.posY)  ) / ( ( ($(other.posX) - $(self.posX))^2 + ($(other.posY) - $(self.posY))^2 + 1)^(3/2) )";
    aY.otherKey="physics.aY";
    aY.otherChangeType="add";
    aY.otherValue=  "$(global.G) * $(self.physics.mass)  * ( $(self.posY)  - $(other.posY) ) / ( ( ($(other.posX) - $(self.posX))^2 + ($(other.posY) - $(self.posY))^2 + 1)^(3/2) )";

    // Velocity ruleset

    // apply only to itself (id-match)
    InvokeCommand vX;
    vX.type="loop";
    vX.logicalArg="$(self.id) == $(other.id)";
    vX.selfKey="physics.vX";
    vX.selfChangeType="add";
    vX.selfValue="$(self.physics.aX) * $(global.dt)";

    InvokeCommand vY;
    vY.type="loop";
    vY.logicalArg="$(self.id) == $(other.id)";
    vY.selfKey="physics.vY";
    vY.selfChangeType="add";
    vY.selfValue="$(self.physics.aY) * $(global.dt)";

    // Position ruleset

    // apply only to itself (id-match)
    InvokeCommand pX;
    pX.type="loop";
    pX.logicalArg="$(self.id) == $(other.id)";
    pX.selfKey="posX";
    pX.selfChangeType="add";
    pX.selfValue="$(self.physics.vX) * $(global.dt)";

    InvokeCommand pY;
    pY.type="loop";
    pY.logicalArg="$(self.id) == $(other.id)";
    pY.selfKey="posY";
    pY.selfChangeType="add";
    pY.selfValue="$(self.physics.vY) * $(global.dt)";

    // Insert rules into objects 
    // and then insert objects into Renderer
    for (auto& o : bodies) {
        // LOOP
        // integrate for velocity
        o.appendInvoke(vX);
        o.appendInvoke(vY);

        // integrate for position
        o.appendInvoke(pX);
        o.appendInvoke(pY);

        // Reset acceleration to 0
        o.appendInvoke(reset_aX);
        o.appendInvoke(reset_aY);

        // CONTINOUS
        // calculate accelerations (essentially forces)
        o.appendInvoke(aX);
        o.appendInvoke(aY);

        // Append
        Renderer.append(o);
    }
    
    //General Variables
    uint64_t starttime = Time::gettime();
    uint64_t currentTime = Time::gettime();
    uint64_t lastTime = Time::gettime();

    // Render loop variables
    bool quit = false;
    int event = 0;

    // [TODO]: add global vars to Environment!
    //FileManagement::WriteFile("gravity.json",Renderer.serializeEnvironment());

    // Main Render loop
    while (!quit) {
        // for now ,see max fps
        if (true || Renderer.timeToRender()) {
            // increase loop time
            JSONHandler::Set::Any<double>(global, "t", (currentTime-starttime)/1000.0);

            // —– compute dt —–
            currentTime = Time::gettime();
            JSONHandler::Set::Any<double>(global, "dt", (currentTime - lastTime) / 1000.0);
            lastTime = currentTime;

            // Update
            Invoke.checkLoop(); // Checks all renderobjects on screen against loop invokes for manipulation
            Renderer.update();  // Updates Renderer:
                                // - Update Renderobjects
                                // - Each RO is checked against invokes
                                // - draw ROs new position

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
            
            // Debug:
            //Platform::clearScreen();
            //std::cout << JSONHandler::serialize(global);
            //std::cout << Renderer.serializeEnvironment();
        }
    }
    //End of Program!
    Renderer.destroy();

    return 0;
}
