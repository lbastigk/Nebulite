// This file contains the option menues for each test type

// Include FunctionTree creator
#include "FuncTree.h"

//Include Header
#include "TestEnv.h"

// Include Tests
#include "Tests/FileManagement.cpp"
#include "Tests/General.cpp"
#include "Tests/JSONHandler.cpp"
#include "Tests/MoveRuleSet.cpp"
#include "Tests/Renderer.cpp"
#include "Tests/RenderObject.cpp"

//---------------------------------------------------------------
// FileManagement

int TestEnv::_FileManagement::passArgs(int argc, char* argv[]) {
	FuncTree ft;

	// Check for provided args starting with '--' or '-'
	

	// Attach functions
	ft.attachFunction(testFileCollector,"file-collector","Show files in ressource folder");
	
	// parse
	return ft.parse(argc, argv);
}


//---------------------------------------------------------------
// General

int TestEnv::_General::passArgs(int argc, char* argv[]) {
	FuncTree ft;

	// Check for provided args starting with '--' or '-'
	

	// Attach functions
	ft.attachFunction(benchmarkStringConversion,"benchmark-String-Conversion","String conversion Benchmark");
	
	// parse
	return ft.parse(argc, argv);
}


//---------------------------------------------------------------
// JSONHandler

int TestEnv::_JSONHandler::passArgs(int argc, char* argv[]) {
	FuncTree ft;

	// Check for provided args starting with '--' or '-'
	

	// Attach functions
	ft.attachFunction(listOfKeys,"list-keys",	"List keys in an object");
	ft.attachFunction(KeyNesting,"key-nesting",	"Test key nesting");
	
	// parse
	return ft.parse(argc, argv);
}


//---------------------------------------------------------------
// MoveRuleSet

int TestEnv::_MoveRuleSet::passArgs(int argc, char* argv[]) {
	FuncTree ft;

	// Check for provided args starting with '--' or '-'
	

	// Attach functions
	ft.attachFunction(serialize,				"serialize",				"Serialize MoveRuleSet");
	ft.attachFunction(testMoveSetWithoutThreads,"moveset-without-threads",	"Test MoveRuleSet without threads");
	ft.attachFunction(testMoveSetWithThreads,	"moveset-with-threads",		"Test MoveRuleSet with threads");
	ft.attachFunction(testSingleObject,			"single-object",			"Test single object");
	
	// parse
	return ft.parse(argc, argv);
}


//---------------------------------------------------------------
// RenderObject

int TestEnv::_RenderObject::passArgs(int argc, char* argv[]) {
	FuncTree ft;

	// Check for provided args starting with '--' or '-'
	

	// Attach functions
	ft.attachFunction(testSpriteSheets,		"sprite-sheets",	"Test Spritesheets");
	ft.attachFunction(testPokemonSprites,	"pokemon-sprites",	"Test Pokemon Sprite");
	ft.attachFunction(testRuleSets,			"rule-sets",		"Test Rulesets");
	
	// parse
	return ft.parse(argc, argv);
}


//---------------------------------------------------------------
// Renderer

int TestEnv::_Renderer::passArgs(int argc, char* argv[]) {
	FuncTree ft;

	// Check for provided args starting with '--' or '-'
	

	// Attach functions
	ft.attachFunction(simpleSdlWindow,		"simple-window",		"Simple SDL Window with debug in each step");
	ft.attachFunction(twoSdlWindows,		"two-windows",			"Two SDL Windows with debug in each step");
	ft.attachFunction(randomImages,			"random-images",		"Random images");
	ft.attachFunction(circleBenchmark,		"circle-benchmark",		"Circle Benchmark");
	ft.attachFunction(ThreadTest,			"thread-test",			"Thread Test");
	ft.attachFunction(testRendererMemLeak,	"renderer-memory-leak",	"Test for memory leak");
	ft.attachFunction(movement,				"movement",				"Movement test");
	
	// parse
	return ft.parse(argc, argv);
}

