#pragma once

#include "OptionsMenu.h"

#include "_TEMPLATE_CLASS.h"
#include "FileManagement.h"
#include "JSONHandler.h"
#include "Kreatur.h"
#include "Renderer.h"
#include "Time.h"
#include "MoveRuleSet.h"
#include "DsaDebug.h"
#include "StringHandler.h"


class TestEnv {
public:
    class _FileManagement {
    public:
        static void testMenue();

        //Shows all files in ressources folder
        static void testFileCollector();

       //Same as before, but with opening
        static void testFileCollectorMenue();
    };
	class _General {
    public:
        static void testMenue();
        static void benchmarkStringConversion();
	};
    class _InventarObjekt {
    public:
        static void testMenue();
        static void serialize();
    };
    class _JSONHandler {
    public:
        static void testMenue();
        static void speed(int loopAmount);
        static void setGet();
        static void KeyNesting();
        static void listOfKeys();
        class Memoryleak {
        public:
            //Sets a value of the doc for 'amount' many times. Value increases by 1 with each iteration
            //Return value should match amount on return!
            //Value type is double
            //Debug to see value each 100 iterations
            static double setAny(UINT64 amount, bool debug = false);
            //Sets a value of the doc for 'amount' many times. Value in subdoc increases by 1 with each Iteration
            //Subdoc is rebuild from main doc with each iteration
            //Return string is entire doc
            //Debug to see full doc each 100 iterations
            static std::string addGetSubDoc(UINT64 amount, bool debug = false);
            //Full memory leak test
            static void fullTest(UINT64 amount = 10000000);
        };

        class Retired {
            //Also good for benchmarking
            // Times in ms for 1 000 000 iterations:
            //[2024-02-14] : 2564
            static void testMemLeak();
        };
    };
    class _Kreatur {
    public:
        static void testMenue();
        static void serialize();
    };
    class _MoveRuleSet {
    public:
        static void testMenue();
        static void serialize();
        static void testMoveSetWithoutThreads();
        static void testMoveSetWithThreads();
        static void testSingleObject();
    };
    class _RenderObject {
    public:
        static void testMenue();
        static void testSpriteSheets();
        static void testPokemonSprites();
        static void testRuleSets();
    };
    class _Renderer {
    public:
        static void testMenue();
        static int randomImages();
        static int circleBenchmark();
        static void ThreadTest();
        static void testRendererMemLeak();
        static int movement();
        static void simpleSdlWindow();
        static void twoSdlWindows();
    private:
        static void createRandomObjects(Renderer& Renderer, int amount, int radius);
        static void createCircleOLD(Renderer& Renderer, int amount, int radius);
        static void createCircle001P(Renderer& Renderer, int amount, int radius, int colour128, int _dX, int _dY);
        static void createCircle100P(Renderer& Renderer, int amount, int radius, int colour128, int _dX, int _dY);
        //Returns render time for x frames
        static UINT64 stressTest(int objCount = 512, int ringCount = 8, int threadSize = 1024, unsigned int framesToRender = 300, bool bypassThreads = false);
	};
    class _Talente {
    public:
        static void testMenue();
        static void testTalentJson();
    };
    class _Template {
    public:
        static void testMenue();
        static void testMyTemplate();
	};
};