/**
 * @file ThreadSettings.hpp
 * 
 * Threading settings for Nebulite's rendering and expression evaluation.
 */

#pragma once

//------------------------------------------

/**
 * @brief Number of thread runners for processing broadcast-listen pairs.
 */
#define THREADRUNNER_COUNT 30

/**
 * @def INVOKE_EXPR_POOL_SIZE
 *
 * Size of the expression pool for each expression.
 */
#define INVOKE_EXPR_POOL_SIZE 30

//------------------------------------------

/**
 * @def BATCH_COST_GOAL
 * 
 * Target cost of each Render::update thread batch.
 */
#define BATCH_COST_GOAL 250



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
2025-10-01:      9.1s       500                     750             10                      # Improved JSON cache handling in combination with Assignments
------------------------------------------------------------------------------------------
Date            Result      THREADRUNNER_COUNT      BATCH_COST_GOAL INVOKE_EXPR_POOL_SIZE
------------------------------------------------------------------------------------------
2025-10-06:      7.7s       20                      250             20                      # New threading model for Invoke
2025-10-13:      7.2s       30                      250             30                      # Increased thread count and new BroadCastListen structure
2025-10-17:      6.8s       30                      250             30                      # Improved Expression evaluation with ordered double pointer lists
*/
