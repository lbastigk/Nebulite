// This file contains the option menues for each test type

// Include FunctionTree creator
#include "FuncTree.h"

//Include Header
#include "TestEnv.h"

// Include Tests
#include "Tests/FileManagement.cpp"
#include "Tests/General.cpp"
#include "Tests/InventarObjekt.cpp"
#include "Tests/JSONHandler.cpp"
#include "Tests/Kreatur.cpp"
#include "Tests/MoveRuleSet.cpp"
#include "Tests/Renderer.cpp"
#include "Tests/RenderObject.cpp"
#include "Tests/Talente.cpp"
#include "Tests/Template.cpp"

//---------------------------------------------------------------
// FileManagement

int TestEnv::_FileManagement::passArgs(int argc, char* argv[]) {
	// Check for provided args starting with '--' or '-'
	// TODO...
	// if-else-chain for all args, setting class values

	// testFileCollector is part of TestEnv::_FileManagement class
	// use lambda to bind to ft.attachFunction
	// currently, error is:
	// 
	//"message": "no suitable constructor exists to convert from \"int (int argc, char **argv)\" to \"std::function<int (int argc, char **argv)>\"",

	FuncTree ft;
	ft.attachFunction(testFileCollector,"file-collector","Show files in ressource folder");
	return ft.parse(argc, argv);
}


//---------------------------------------------------------------
// General

int TestEnv::_General::passArgs(int argc, char* argv[]) {
    // // Create options menu
	// OptionsMenu optM;
	// optM.setOption(0);	//Start option
	// optM.attachFunction(TestEnv::_General::benchmarkStringConversion,	    "String conversion Benchmark");
	// optM.render();		//First render for display
	
	// //Loop until exit is chosen
	// volatile int status = 0;
	// while(status != OptionsMenu::statusExit){status = optM.update();}
}


//---------------------------------------------------------------
// InventarObjekt

int TestEnv::_InventarObjekt::passArgs(int argc, char* argv[]) {
    // // Create options menu
	// OptionsMenu optM;
	// optM.setOption(0);	//Start option
	// optM.attachFunction(TestEnv::_InventarObjekt::serialize,	    "Serialize an inventory object");
	// optM.render();		//First render for display
	
	// //Loop until exit is chosen
	// volatile int status = 0;
	// while(status != OptionsMenu::statusExit){status = optM.update();}
}


//---------------------------------------------------------------
// JSONHandler

int TestEnv::_JSONHandler::passArgs(int argc, char* argv[]) {
    // // Create options menu
	// OptionsMenu optM;
	// optM.setOption(0);	//Start option
	// optM.attachFunction(TestEnv::_JSONHandler::listOfKeys,	    "List keys in an object");
	// optM.attachFunction(TestEnv::_JSONHandler::KeyNesting,	    "Test key nesting");
	// optM.render();		//First render for display
	
	// //Loop until exit is chosen
	// volatile int status = 0;
	// while(status != OptionsMenu::statusExit){status = optM.update();}
}


//---------------------------------------------------------------
// Kreatur

int TestEnv::_Kreatur::passArgs(int argc, char* argv[]) {
    // // Create options menu
	// OptionsMenu optM;
	// optM.setOption(0);	//Start option
	// optM.attachFunction(TestEnv::_Kreatur::serialize,	    "Serialize Creature");
	// optM.render();		//First render for display
	
	// //Loop until exit is chosen
	// volatile int status = 0;
	// while(status != OptionsMenu::statusExit){status = optM.update();}
}


//---------------------------------------------------------------
// MoveRuleSet

int TestEnv::_MoveRuleSet::passArgs(int argc, char* argv[]) {
    // // Create options menu
	// OptionsMenu optM;
	// optM.setOption(0);	//Start option
	// optM.attachFunction( TestEnv::_MoveRuleSet::serialize ,	                    "Serialize MoveRuleSet");
    // optM.attachFunction( TestEnv::_MoveRuleSet::testMoveSetWithoutThreads ,	    "Test MoveRuleSet without threads");
    // optM.attachFunction( TestEnv::_MoveRuleSet::testMoveSetWithThreads ,	    "Test MoveRuleSet with threads");
    // optM.attachFunction( TestEnv::_MoveRuleSet::testSingleObject ,	            "Test single object");
	// optM.render();		//First render for display
	
	// //Loop until exit is chosen
	// volatile int status = 0;
	// while(status != OptionsMenu::statusExit){status = optM.update();}
}


//---------------------------------------------------------------
// RenderObject

int TestEnv::_RenderObject::passArgs(int argc, char* argv[]) {
    // // Create options menu
	// OptionsMenu optM;
	// optM.setOption(0);	//Start option
	// optM.attachFunction( TestEnv::_RenderObject::testSpriteSheets ,	    "Test Spritesheets");
    // optM.attachFunction( TestEnv::_RenderObject::testPokemonSprites ,   "Test Pokemon Sprite");
    // optM.attachFunction( TestEnv::_RenderObject::testRuleSets ,	        "Test Rulesets");
	// optM.render();		//First render for display
	
	// //Loop until exit is chosen
	// volatile int status = 0;
	// while(status != OptionsMenu::statusExit){status = optM.update();}
}


//---------------------------------------------------------------
// Renderer

int TestEnv::_Renderer::passArgs(int argc, char* argv[]) {
    // // Create options menu
	// OptionsMenu optM;
	// optM.setOption(0);	//Start option
	// optM.attachFunction( TestEnv::_Renderer::simpleSdlWindow ,	    "Simple SDL Window with debug in each step");
	// optM.attachFunction( TestEnv::_Renderer::twoSdlWindows ,	    "Two SDL Windows with debug in each step");
    // optM.attachFunction( TestEnv::_Renderer::randomImages ,	        "Random images");
    // optM.attachFunction( TestEnv::_Renderer::circleBenchmark ,	    "Circle Benchmark");
    // optM.attachFunction( TestEnv::_Renderer::ThreadTest ,	        "Thread Test");
    // optM.attachFunction( TestEnv::_Renderer::testRendererMemLeak ,  "Test for memory leak");
    // optM.attachFunction( TestEnv::_Renderer::movement ,	            "Movement test");
	// optM.render();		//First render for display
	
	// //Loop until exit is chosen
	// volatile int status = 0;
	// while(status != OptionsMenu::statusExit){status = optM.update();}
}


//---------------------------------------------------------------
// Talente
    
int TestEnv::_Talente::passArgs(int argc, char* argv[]) {
    // // Create options menu
	// OptionsMenu optM;
	// optM.setOption(0);	//Start option
	// optM.attachFunction( TestEnv::_Talente::testTalentJson ,	    "Turn talent into JSON");
	// optM.render();		//First render for display
	
	// //Loop until exit is chosen
	// volatile int status = 0;
	// while(status != OptionsMenu::statusExit){status = optM.update();}
}


//---------------------------------------------------------------
// Template

int TestEnv::_Template::passArgs(int argc, char* argv[]) {
    // // Create options menu
	// OptionsMenu optM;
	// optM.setOption(0);	//Start option
	// optM.attachFunction( TestEnv::_Template::testMyTemplate ,	    "Test template");
	// optM.render();		//First render for display
	
	// //Loop until exit is chosen
	// volatile int status = 0;
	// while(status != OptionsMenu::statusExit){status = optM.update();}
}


