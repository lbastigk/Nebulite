#pragma once

#include "OptionsMenu.h"

#include "_TEMPLATE_CLASS.h"
#include "FileManagement.h"
#include "JSONHandler.h"
#include "Renderer.h"
#include "Time.h"
#include "MoveRuleSet.h"
#include "DsaDebug.h"
#include "StringHandler.h"


class TestEnv {
public:
    int passArgs(int argc, char* argv[]){
        FuncTree ft;

        // Check for provided args starting with '--' or '-'
        

        // Attach functions
        ft.attachFunction(_General::passArgs,"general","General Tests");
        ft.attachFunction(_FileManagement::passArgs,"file-management","Tests for File Management");
        ft.attachFunction(_JSONHandler::passArgs,"json-handler","Tests for JSONHandler");
        ft.attachFunction(_Renderer::passArgs,"renderer","Tests for Renderer");
        ft.attachFunction(_RenderObject::passArgs,"render-object","Tests for Render Objects");
        ft.attachFunction(_MoveRuleSet::passArgs,"move-rule-set","Tests for MoveRuleSets");
        
        // parse
        return ft.parse(argc, argv);
    };
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
    class _JSONHandler {
    public:
        int passArgs(int argc, char* argv[]);

        int speed(int loopAmount);
        int setGet(int argc, char* argv[]);
        int KeyNesting(int argc, char* argv[]);
        int listOfKeys(int argc, char* argv[]);
        class Memoryleak {
        public:
            //Sets a value of the doc for 'amount' many times. Value increases by 1 with each iteration
            //Return value should match amount on return!
            //Value type is double
            //Debug to see value each 100 iterations
            double setAny(int argc, char* argv[]);
            //Sets a value of the doc for 'amount' many times. Value in subdoc increases by 1 with each Iteration
            //Subdoc is rebuild from main doc with each iteration
            //Return string is entire doc
            //Debug to see full doc each 100 iterations
            std::string addGetSubDoc(int argc, char* argv[]);
            //Full memory leak test
            int fullTest(int argc, char* argv[]);
        };

        class Retired {
            //Also good for benchmarking
            // Times in ms for 1 000 000 iterations:
            //[2024-02-14] : 2564
            int testMemLeak(int argc, char* argv[]);
        };
    };
    class _MoveRuleSet {
    public:
        int passArgs(int argc, char* argv[]);

        int serialize(int argc, char* argv[]);
        int testMoveSetWithoutThreads(int argc, char* argv[]);
        int testMoveSetWithThreads(int argc, char* argv[]);
        int testSingleObject(int argc, char* argv[]);
    };
    class _RenderObject {
    public:
        int passArgs(int argc, char* argv[]);

        int testSpriteSheets(int argc, char* argv[]);
        int testPokemonSprites(int argc, char* argv[]);
        int testRuleSets(int argc, char* argv[]);
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
};