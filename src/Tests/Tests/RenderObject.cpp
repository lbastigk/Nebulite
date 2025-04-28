#include "TestEnv.h"


int TestEnv::_RenderObject::basic(int argc, char* argv[]) {
    Renderer Renderer;
    
    RenderObject obj[20];
    
    for (int i = 0; i < 20; i++) {

        obj[i].valueSet(namenKonvention.renderObject.positionX, 500);
        obj[i].valueSet(namenKonvention.renderObject.positionY, 500);

        obj[i].valueSet(namenKonvention.renderObject.pixelSizeX, 5);
        obj[i].valueSet(namenKonvention.renderObject.pixelSizeY, 5);
        Renderer.append(obj[i]);
    }
    

    //Renderer.serializeObjects();

    while (true) {
        if (Renderer.timeToRender()) {
            //Render Current instances
            Renderer.renderFrame();

            //Render FPS
            Renderer.renderFPS();

            // Present the renderer
            Renderer.showFrame();

            //Update all visible
            Renderer.update();
        }
    }
    //End of Program!
    Renderer.destroy();
}

