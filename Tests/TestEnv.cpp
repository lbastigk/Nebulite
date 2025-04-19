// This file contains the option menues for each test type

// Include FunctionTree creator
#include "FuncTree.h"

//Include Header
#include "TestEnv.h"

// Include Tests
#include "Tests/FileManagement.cpp"
#include "Tests/General.cpp"
#include "Tests/Invoke.cpp"
#include "Tests/JSONHandler.cpp"
#include "Tests/MoveRuleSet.cpp"
#include "Tests/Renderer.cpp"
#include "Tests/RenderObject.cpp"

//---------------------------------------------------------------
// FileManagement

int TestEnv::_FileManagement::passArgs(int argc, char* argv[]) {
	FuncTree ft("File Management");

	// Check for provided args starting with '--' or '-'
	

	// Attach functions
	ft.attachFunction(
		[this](int argc, char* argv[]) -> int {
			return testFileCollector(argc, argv);
		}, 
		"file-collector", "Show files in ressource folder"
	);
	
	// parse
	return ft.parse(argc, argv);
}


//---------------------------------------------------------------
// General

int TestEnv::_General::passArgs(int argc, char* argv[]) {
	FuncTree ft("General");

	// Check for provided args starting with '--' or '-'
	

	// Attach functions
	ft.attachFunction(
		[this](int argc, char* argv[]) -> int {
			return benchmarkStringConversion(argc, argv);
		}, 
		"benchmark-String-Conversion","String conversion Benchmark"
	);

	// parse
	return ft.parse(argc, argv);
}

//---------------------------------------------------------------
// Invoke

int TestEnv::_Invoke::passArgs(int argc, char* argv[]){
	FuncTree ft("Invoke");

	// Check for provided args starting with '--' or '-'
	

	// Attach functions
	ft.attachFunction(
		[this](int argc, char* argv[]) -> int {
			return example(argc, argv);
		}, 
		"example","Example invoke"
	);
	ft.attachFunction(
		[this](int argc, char* argv[]) -> int {
			return gravity(argc, argv);
		}, 
		"gravity","Gravity test"
	);

	// parse
	return ft.parse(argc, argv);
}

//---------------------------------------------------------------
// JSONHandler

int TestEnv::_JSONHandler::passArgs(int argc, char* argv[]) {
	FuncTree ft("JSON Handler");

	// Check for provided args starting with '--' or '-'
	

	// Attach functions
	ft.attachFunction(
		[this](int argc, char* argv[]) -> int {
			return listOfKeys(argc, argv);
		}, 
		"list-keys",	"List keys in an object"
	);
	ft.attachFunction(
		[this](int argc, char* argv[]) -> int {
			return KeyNesting(argc, argv);
		}, 
		"key-nesting",	"Test key nesting"
	);
	
	// parse
	return ft.parse(argc, argv);
}


//---------------------------------------------------------------
// MoveRuleSet

int TestEnv::_MoveRuleSet::passArgs(int argc, char* argv[]) {
	FuncTree ft("MoveRuleSet");

	// Check for provided args starting with '--' or '-'
	

	// Attach functions
	ft.attachFunction(
		[this](int argc, char* argv[]) -> int {
			return serialize(argc, argv);
		}, 
		"serialize",				"Serialize MoveRuleSet"
	);
	ft.attachFunction(
		[this](int argc, char* argv[]) -> int {
			return testMoveSetWithoutThreads(argc, argv);
		}, 
		"moveset-without-threads",	"Test MoveRuleSet without threads"
	);
	ft.attachFunction(
		[this](int argc, char* argv[]) -> int {
			return testMoveSetWithThreads(argc, argv);
		}, 
		"moveset-with-threads",		"Test MoveRuleSet with threads"
	);
	ft.attachFunction(
		[this](int argc, char* argv[]) -> int {
			return testSingleObject(argc, argv);
		}, 
		"single-object",			"Test single object"
	);
	
	// parse
	return ft.parse(argc, argv);
}


//---------------------------------------------------------------
// RenderObject

int TestEnv::_RenderObject::passArgs(int argc, char* argv[]) {
	FuncTree ft("RenderObject");

	// Check for provided args starting with '--' or '-'
	

	// Attach functions
	ft.attachFunction(
		[this](int argc, char* argv[]) -> int {
			return testSpriteSheets(argc, argv);
		}, 
		"sprite-sheets",	"Test Spritesheets"
	);
	ft.attachFunction(
		[this](int argc, char* argv[]) -> int {
			return testPokemonSprites(argc, argv);
		}, 
		"pokemon-sprites",	"Test Pokemon Sprite"
	);
	ft.attachFunction(
		[this](int argc, char* argv[]) -> int {
			return testRuleSets(argc, argv);
		}, 
		"rule-sets",		"Test Rulesets"
	);
	
	// parse
	return ft.parse(argc, argv);
}


//---------------------------------------------------------------
// Renderer

int TestEnv::_Renderer::passArgs(int argc, char* argv[]) {
	FuncTree ft("Renderer");

	// Check for provided args starting with '--' or '-'
	

	// Attach functions
	ft.attachFunction(
		[this](int argc, char* argv[]) -> int {
			return simpleSdlWindow(argc, argv);
		}, 
		"simple-window",		"Simple SDL Window with debug in each step"
	);
	ft.attachFunction(
		[this](int argc, char* argv[]) -> int {
			return twoSdlWindows(argc, argv);
		}, 
		"two-windows",			"Two SDL Windows with debug in each step"
	);
	ft.attachFunction(
		[this](int argc, char* argv[]) -> int {
			return randomImages(argc, argv);
		}, 
		"random-images",		"Random images"
	);
	ft.attachFunction(
		[this](int argc, char* argv[]) -> int {
			return circleBenchmark(argc, argv);
		}, 
		"circle-benchmark",		"Circle Benchmark"
	);
	ft.attachFunction(
		[this](int argc, char* argv[]) -> int {
			return ThreadTest(argc, argv);
		}, 
		"thread-test",			"Thread Test"
	);
	ft.attachFunction(
		[this](int argc, char* argv[]) -> int {
			return testRendererMemLeak(argc, argv);
		}, 
		"renderer-memory-leak",	"Test for memory leak"
	);
	ft.attachFunction(
		[this](int argc, char* argv[]) -> int {
			return movement(argc, argv);
		}, 
		"movement",				"Movement test"
	);
	
	// parse
	return ft.parse(argc, argv);
}

