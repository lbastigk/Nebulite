// This file contains the option menues for each test type

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

void TestEnv::_FileManagement::testMenue() {
    // Create options menu
	OptionsMenu optM;
	optM.setOption(0);	//Start option
	optM.attachFunction(TestEnv::_FileManagement::testFileCollector,	    "Show files in ressource folder");
	optM.attachFunction(TestEnv::_FileManagement::testFileCollectorMenue,   "Open file in ressource folder");
	optM.render();		//First render for display
	
	//Loop until exit is chosen
	volatile int status = 0;
	while(status != OptionsMenu::statusExit){status = optM.update();}
}


//---------------------------------------------------------------
// General

void TestEnv::_General::testMenue() {
    // Create options menu
	OptionsMenu optM;
	optM.setOption(0);	//Start option
	optM.attachFunction(TestEnv::_General::benchmarkStringConversion,	    "String conversion Benchmark");
	optM.render();		//First render for display
	
	//Loop until exit is chosen
	volatile int status = 0;
	while(status != OptionsMenu::statusExit){status = optM.update();}
}


//---------------------------------------------------------------
// InventarObjekt

void TestEnv::_InventarObjekt::testMenue() {
    // Create options menu
	OptionsMenu optM;
	optM.setOption(0);	//Start option
	optM.attachFunction(TestEnv::_InventarObjekt::serialize,	    "Serialize an inventory object");
	optM.render();		//First render for display
	
	//Loop until exit is chosen
	volatile int status = 0;
	while(status != OptionsMenu::statusExit){status = optM.update();}
}


//---------------------------------------------------------------
// JSONHandler

void TestEnv::_JSONHandler::testMenue() {
    // Create options menu
	OptionsMenu optM;
	optM.setOption(0);	//Start option
	optM.attachFunction(TestEnv::_JSONHandler::listOfKeys,	    "List keys in an object");
	optM.render();		//First render for display
	
	//Loop until exit is chosen
	volatile int status = 0;
	while(status != OptionsMenu::statusExit){status = optM.update();}
}


//---------------------------------------------------------------
// Kreatur

void TestEnv::_Kreatur::testMenue() {
    // Create options menu
	OptionsMenu optM;
	optM.setOption(0);	//Start option
	optM.attachFunction(TestEnv::_Kreatur::serialize,	    "Serialize Creature");
	optM.render();		//First render for display
	
	//Loop until exit is chosen
	volatile int status = 0;
	while(status != OptionsMenu::statusExit){status = optM.update();}
}


//---------------------------------------------------------------
// MoveRuleSet

void TestEnv::_MoveRuleSet::testMenue() {
    // Create options menu
	OptionsMenu optM;
	optM.setOption(0);	//Start option
	optM.attachFunction( TestEnv::_MoveRuleSet::serialize ,	                    "Serialize MoveRuleSet");
    optM.attachFunction( TestEnv::_MoveRuleSet::testMoveSetWithoutThreads ,	    "Test MoveRuleSet without threads");
    optM.attachFunction( TestEnv::_MoveRuleSet::testMoveSetWithThreads ,	    "Test MoveRuleSet with threads");
    optM.attachFunction( TestEnv::_MoveRuleSet::testSingleObject ,	            "Test single object");
	optM.render();		//First render for display
	
	//Loop until exit is chosen
	volatile int status = 0;
	while(status != OptionsMenu::statusExit){status = optM.update();}
}


//---------------------------------------------------------------
// RenderObject

void TestEnv::_RenderObject::testMenue() {
    // Create options menu
	OptionsMenu optM;
	optM.setOption(0);	//Start option
	optM.attachFunction( TestEnv::_RenderObject::testSpriteSheets ,	    "Test Spritesheets");
    optM.attachFunction( TestEnv::_RenderObject::testPokemonSprites ,   "Test Pokemon Sprite");
    optM.attachFunction( TestEnv::_RenderObject::testRuleSets ,	        "Test Rulesets");
	optM.render();		//First render for display
	
	//Loop until exit is chosen
	volatile int status = 0;
	while(status != OptionsMenu::statusExit){status = optM.update();}
}


//---------------------------------------------------------------
// Renderer

void TestEnv::_Renderer::testMenue() {
    // Create options menu
	OptionsMenu optM;
	optM.setOption(0);	//Start option
	optM.attachFunction( TestEnv::_Renderer::simpleSdlWindow ,	    "Simple SDL Window with debug in each step");
    optM.attachFunction( TestEnv::_Renderer::randomImages ,	        "Random images");
    optM.attachFunction( TestEnv::_Renderer::circleBenchmark ,	    "Circle Benchmark");
    optM.attachFunction( TestEnv::_Renderer::ThreadTest ,	        "Thread Test");
    optM.attachFunction( TestEnv::_Renderer::testRendererMemLeak ,  "Test for memory leak");
    optM.attachFunction( TestEnv::_Renderer::movement ,	            "Movement test");
	optM.render();		//First render for display
	
	//Loop until exit is chosen
	volatile int status = 0;
	while(status != OptionsMenu::statusExit){status = optM.update();}
}


//---------------------------------------------------------------
// Talente
    
void TestEnv::_Talente::testMenue() {
    // Create options menu
	OptionsMenu optM;
	optM.setOption(0);	//Start option
	optM.attachFunction( TestEnv::_Talente::testTalentJson ,	    "Turn talent into JSON");
	optM.render();		//First render for display
	
	//Loop until exit is chosen
	volatile int status = 0;
	while(status != OptionsMenu::statusExit){status = optM.update();}
}


//---------------------------------------------------------------
// Template

void TestEnv::_Template::testMenue() {
    // Create options menu
	OptionsMenu optM;
	optM.setOption(0);	//Start option
	optM.attachFunction( TestEnv::_Template::testMyTemplate ,	    "Test template");
	optM.render();		//First render for display
	
	//Loop until exit is chosen
	volatile int status = 0;
	while(status != OptionsMenu::statusExit){status = optM.update();}
}


