/**
 * @file ThreadSettings.hpp
 * @brief Threading settings for Nebulite's rendering and expression evaluation.
 */

#ifndef NEBULITE_CONSTANTS_THREAD_SETTINGS_HPP
#define NEBULITE_CONSTANTS_THREAD_SETTINGS_HPP

//------------------------------------------

/**
 * @def THREADRUNNER_COUNT
 * @brief Number of thread runners for processing broadcast-listen pairs.
 * @note Hypothesis:
 *       Should be approximately ORDERED_DOUBLE_POINTERS_MAPS x INVOKE_EXPR_POOL_SIZE ?
 */
#define THREADRUNNER_COUNT 25

/**
 * @def ORDERED_DOUBLE_POINTERS_MAPS
 * @brief Number of ordered double pointer maps for expression evaluations.
 * @details If set to 1, all threads share the same map.
 *          Optimized to not use any special indexing, but simple access to [0] if set to 1.
 */
#define ORDERED_DOUBLE_POINTERS_MAPS 5

/**
 * @def INVOKE_EXPR_POOL_SIZE
 * @brief Size of the expression pool for each expression.
 * @todo more benchmarks necessary, initial tests show that
 *       A pool size of 1 (no pool) is minimally faster.
 *       Perhaps a small overhaul is needed to actually make use of the pool?
 */
#define INVOKE_EXPR_POOL_SIZE ORDERED_DOUBLE_POINTERS_MAPS

/**
 * @def BATCH_COST_GOAL
 * @details Target cost of each Render::update thread batch.
 */
#define BATCH_COST_GOAL 250

#endif // NEBULITE_CONSTANTS_THREAD_SETTINGS_HPP
