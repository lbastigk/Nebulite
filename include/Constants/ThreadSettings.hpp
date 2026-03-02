/**
 * @file ThreadSettings.hpp
 * @brief Threading settings for Nebulite's rendering and expression evaluation.
 */

#ifndef NEBULITE_CONSTANTS_THREAD_SETTINGS_HPP
#define NEBULITE_CONSTANTS_THREAD_SETTINGS_HPP

//------------------------------------------
// Runtime expression evaluation settings

/**
 * @def EXPRESSION_POOL_SIZE
 * @brief Size of the expression pool for each expression.
 * @todo more benchmarks necessary, initial tests show that
 *       A pool size of 1 (no pool) is minimally faster.
 *       Perhaps a small overhaul is needed to actually make use of the pool?
 *       Or we remove the expression pool entirely...
 */
#define EXPRESSION_POOL_SIZE 1

//------------------------------------------
// General threading settings

/**
 * @def INVOKE_WORKER_COUNT
 * @brief Number of thread runners for processing broadcast-listen pairs.
 */
#define INVOKE_WORKER_COUNT 12

/**
 * @def RENDERER_WORKER_COUNT
 * @brief Number of worker threads for processing RenderObjectContainer batches.
 */
#define RENDERER_WORKER_COUNT 4

/**
 * @def BATCH_COST_GOAL
 * @brief Target cost of each Render::update thread batch.
 * @details Set to 0 to disable dynamic batching and process all members per layer in a single thread
 */
static auto constexpr batchCostGoal = 256;

#endif // NEBULITE_CONSTANTS_THREAD_SETTINGS_HPP
