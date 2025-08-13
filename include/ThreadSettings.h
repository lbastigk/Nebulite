#pragma once


// INVOKE THREADING SETTINGS
#define THREADED_MIN_BATCHSIZE 500

// RENDERER UPDATE THREADING SETTINGS
#define BATCH_COST_GOAL 500

// EXPRESSION POOL SETTINGS
#define INVOKE_EXPR_POOL_SIZE 10



/*
RESULTS using ./bin/Nebulite task TaskFiles/Benchmarks/spawn_constantly.txt 

Date            Result      THREADED_MIN_BATCHSIZE  BATCH_COST_GOAL INVOKE_EXPR_POOL_SIZE
------------------------------------------------------------------------------------------
2025-08-10:     36.6s       200                     500             10
2025-08-14:     20.9s       100                     500             10                      # improved caching
2025-08-14:     14.5s       500                     500             20
2025-08-14:     14.4s       500                     500             10
*/
