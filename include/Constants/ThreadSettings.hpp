/**
 * @file ThreadSettings.hpp
 * 
 * Threading settings for Nebulite's rendering and expression evaluation.
 */

#pragma once

/**
 * @def THREADED_MIN_BATCHSIZE
 * 
 * Minimum batch size needed to create a new rendering thread of invoke-pairs to process.
 */
#define THREADED_MIN_BATCHSIZE 500

/**
 * @def BATCH_COST_GOAL
 * 
 * Target cost of each Render::update thread batch.
 */
#define BATCH_COST_GOAL 500

/**
 * @def INVOKE_EXPR_POOL_SIZE
 *
 * Size of the expression pool for each expression.
 */
#define INVOKE_EXPR_POOL_SIZE 10

/*
RESULTS using ./bin/Nebulite task TaskFiles/Benchmarks/spawn_constantly.nebs 

Date            Result      THREADED_MIN_BATCHSIZE  BATCH_COST_GOAL INVOKE_EXPR_POOL_SIZE
------------------------------------------------------------------------------------------
2025-08-10:     36.6s       200                     500             10
2025-08-14:     20.9s       100                     500             10                      # improved caching
2025-08-14:     14.5s       500                     500             20
2025-08-14:     14.4s       500                     500             10
2025-08-28:     14.2s       500                     500             10                      # smaller tweaks perhaps, or just general os fluctuations
2025-09-03:     10.2s       500                     500             10                      # Fixed flushing issue, improved cache handling, less frequent flushes
2025-09-25:      9.6s       500                     500             10                      # New SDL2 build + switch from Ubuntu 25.04 to Fedora 42
*/
