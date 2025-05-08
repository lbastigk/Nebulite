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

// TODO: Delete. Now completely described in gravity.json
int TestEnv::_Invoke::gravity(int argc, char* argv[]) {
    std::cout << std::endl << std::endl;
    std::cout << "//------------------------------------------------" << std::endl;
    std::cout << "Checking grav with invoke fully inside render loop" << std::endl;


    //------------------------------------------------
    // Renderer Settings
    Renderer Renderer;
    Renderer.setFPS(1000);
    


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
    obj1.valueSet<double>("physics.vX",  30.0);
    obj1.valueSet<double>("physics.vY",-120.0);
    obj1.valueSet<std::string>(namenKonvention.renderObject.textStr,"OBJ1");
    obj1.valueSet<double>(namenKonvention.renderObject.textFontsize,16);
    obj1.valueSet<double>(namenKonvention.renderObject.textDx,0);
    obj1.valueSet<double>(namenKonvention.renderObject.textDy,-30);
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
    obj2.valueSet<double>("physics.vX", -30.0);
    obj2.valueSet<double>("physics.vY", 120.0);
    obj2.valueSet<std::string>(namenKonvention.renderObject.textStr,"OBJ2");
    obj2.valueSet<double>(namenKonvention.renderObject.textFontsize,16);
    obj2.valueSet<double>(namenKonvention.renderObject.textDx,0);
    obj2.valueSet<double>(namenKonvention.renderObject.textDy,-30);
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
    obj3.valueSet<std::string>(namenKonvention.renderObject.textStr,"SUN");
    obj3.valueSet<double>(namenKonvention.renderObject.textFontsize,16);
    obj3.valueSet<double>(namenKonvention.renderObject.textDx,0);
    obj3.valueSet<double>(namenKonvention.renderObject.textDy,-30);

    InvokeCommand sun_dX;
    sun_dX.type="loop";
    sun_dX.logicalArg="1";
    sun_dX.selfKey=namenKonvention.renderObject.positionX;
    sun_dX.selfChangeType="add";
    sun_dX.selfValue=   "$(global.keyboard.d) - $(global.keyboard.a)";
    InvokeCommand sun_dY;
    sun_dY.type="loop";
    sun_dY.logicalArg="1";
    sun_dY.selfKey=namenKonvention.renderObject.positionY;
    sun_dY.selfChangeType="add";
    sun_dY.selfValue=   "$(global.keyboard.s) - $(global.keyboard.w)";
    obj3.appendInvoke(sun_dX);
    obj3.appendInvoke(sun_dY);
    bodies.push_back(obj3);

    // Add some smaller to the simulation for performance testing:
    /*
    for (int i=1;i<10;i++){
        std::string name = "AST_" + std::to_string(i);

        RenderObject obj;
        obj.valueSet<int>(namenKonvention.renderObject.pixelSizeX,5);
        obj.valueSet<int>(namenKonvention.renderObject.pixelSizeY,5);
        obj.valueSet<double>("physics.mass",5.0);
        obj.valueSet<double>("posX",1000.0*((float)random()/(float)RAND_MAX));
        obj.valueSet<double>("posY",1000.0*((float)random()/(float)RAND_MAX));
        obj.valueSet<double>("physics.isGrav",1.0);
        obj.valueSet<double>("physics.aX", 0.0);
        obj.valueSet<double>("physics.aY", 0.0);
        obj.valueSet<double>("physics.vX", 10.0-20.0*((float)random()/(float)RAND_MAX));
        obj.valueSet<double>("physics.vY", 10.0-20.0*((float)random()/(float)RAND_MAX));
        obj.valueSet<std::string>(namenKonvention.renderObject.textStr,name.c_str());
        obj.valueSet<double>(namenKonvention.renderObject.textFontsize,16);
        obj.valueSet<double>(namenKonvention.renderObject.textDx,0);
        obj.valueSet<double>(namenKonvention.renderObject.textDy,-30);
        bodies.push_back(obj);
    }
    //*/
    

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

    FileManagement::WriteFile("gravity.json",Renderer.serialize());

    // Main Render loop
    while (!Renderer.isQuit()) {
        // for now ,see max fps
        //if (true) {
        if (true || Renderer.timeToRender()) {
            // Update
            Renderer.update();  // Updates Renderer:
                                // - update invoke
                                // - Update Renderobjects
                                // - Each RO is checked against invokes
                                // - draw ROs new position

            

            //Render Current instances
            Renderer.renderFrame();

            //Render FPS
            Renderer.renderFPS();

            // Present the renderer
            Renderer.showFrame();
            
            // Debug:
            //Platform::clearScreen();
            //std::cout << Renderer.serializeGlobal();
        }
    }
    //End of Program!
    Renderer.destroy();

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