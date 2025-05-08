#pragma once

#include "FileManagement.h"
#include "JSONHandler.h"
#include "Renderer.h"
#include "Time.h"
#include "DsaDebug.h"
#include "StringHandler.h"
#include "FuncTree.h"

class TestEnv {
public:
    // Main function to call when attaching args and functions
    int passArgs(int argc, char* argv[]) {
        FuncTree ft("Test Environment");

        // Attach functions with existing instances
        ft.attachFunction(
            [this](int argc, char* argv[]) -> int {
                return fileManagement.passArgs(argc, argv);
            },
            "file-management", "Tests for File Management"
        );
        ft.attachFunction(
            [this](int argc, char* argv[]) -> int {
                return general.passArgs(argc, argv);
            }, 
            "general", "General Tests"
        );
        ft.attachFunction(
            [this](int argc, char* argv[]) -> int {
                return invoke.passArgs(argc, argv);
            },
            "invoke", "Tests for Invoke"
        );
        ft.attachFunction(
            [this](int argc, char* argv[]) -> int {
                return jsonHandler.passArgs(argc, argv);
            },
            "json-handler", "Tests for JSONHandler"
        );
        ft.attachFunction(
            [this](int argc, char* argv[]) -> int {
                return renderer.passArgs(argc, argv);
            },
            "renderer", "Tests for Renderer"
        );
        ft.attachFunction(
            [this](int argc, char* argv[]) -> int {
                return renderObject.passArgs(argc, argv);
            },
            "render-object", "Tests for Render Objects"
        );

        // Parse arguments
        return ft.parse(argc, argv);
    }

private:
    class _FileManagement {
    public:
        int passArgs(int argc, char* argv[]);

        //Shows all files in ressources folder
        int testFileCollector(int argc, char* argv[]);

       //Same as before, but with opening
        int testFileCollectorMenue(int argc, char* argv[]);
    };
	class _General {
    public:
        int passArgs(int argc, char* argv[]);

        int benchmarkStringConversion(int argc, char* argv[]);
	};
    
    class _Invoke{
    public:
        int passArgs(int argc, char* argv[]);

        int example(int argc, char* argv[]);

        int gravity(int argc, char* argv[]);

        int gravityV2(int argc, char* argv[]);

        int resolveVars(int argc, char* argv[]);
    };
    class _JSONHandler {
    public:
        int passArgs(int argc, char* argv[]);

        int speed(int argc, char* argv[]);
        int setGet(int argc, char* argv[]);
        int KeyNesting(int argc, char* argv[]);
        int listOfKeys(int argc, char* argv[]);
        int full(int argc, char* argv[]);
        int leaks(int argc, char* argv[]);
    };
    class _RenderObject {
    public:
        int passArgs(int argc, char* argv[]);

        int basic(int argc, char* argv[]);
    };
    class _Renderer {
    public:
        int passArgs(int argc, char* argv[]);

        int randomImages(int argc, char* argv[]);
        int circleBenchmark(int argc, char* argv[]);
        int ThreadTest(int argc, char* argv[]);
        int testRendererMemLeak(int argc, char* argv[]);
        int movement(int argc, char* argv[]);
        int simpleSdlWindow(int argc, char* argv[]);
        int twoSdlWindows(int argc, char* argv[]);
    private:
        void createRandomObjects(Renderer& Renderer, int amount, int radius);
        void createCircleOLD(Renderer& Renderer, int amount, int radius);
        void createCircle001P(Renderer& Renderer, int amount, int radius, int colour128, int _dX, int _dY);
        void createCircle100P(Renderer& Renderer, int amount, int radius, int colour128, int _dX, int _dY);
        //Returns render time for x frames
        UINT64 stressTest(int objCount = 512, int ringCount = 8, int threadSize = 1024, unsigned int framesToRender = 300, bool bypassThreads = false);
	};

    // Member variables for nested classes
    _FileManagement fileManagement;
    _General general;
    _Invoke invoke;
    _JSONHandler jsonHandler;
    _Renderer renderer;
    _RenderObject renderObject;

};