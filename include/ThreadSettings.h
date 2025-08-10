#pragma once


// INVOKE THREADING SETTINGS
#define THREADED_MIN_BATCHSIZE 100

// RENDERER UPDATE THREADING SETTINGS
#define BATCH_COST_GOAL 500

// EXPRESSION POOL SETTINGS
#define INVOKE_EXPR_POOL_SIZE 10



/*
RESULTS using ./bin/Nebulite task TaskFiles/Benchmarks/spawn_constantly.txt 

Date            Result      THREADED_MIN_BATCHSIZE  BATCH_COST_GOAL INVOKE_EXPR_POOL_SIZE
------------------------------------------------------------------------------------------
2025-08-10:     36.6s       200                     500             10

*/
