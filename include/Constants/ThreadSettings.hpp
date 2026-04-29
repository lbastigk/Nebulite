/**
 * @file ThreadSettings.hpp
 * @brief Threading settings for Nebulite's rendering and expression evaluation.
 */

#ifndef NEBULITE_CONSTANTS_THREAD_SETTINGS_HPP
#define NEBULITE_CONSTANTS_THREAD_SETTINGS_HPP

//------------------------------------------
// Includes

// Standard Library
#include <cmath>
#include <thread>

//------------------------------------------
// TODO: Move to another location

/**
 * @def EXPRESSION_POOL_SIZE
 * @brief Size of the expression pool for each expression.
 * @todo more benchmarks necessary, initial tests show that
 *       A pool size of 1 (no pool) is minimally faster.
 *       Perhaps a small overhaul is needed to actually make use of the pool?
 *       Or we remove the expression pool entirely...
 */
#define EXPRESSION_POOL_SIZE 1

/**
 * @def BATCH_COST_GOAL
 * @brief Target cost of each Render::update thread batch.
 * @details Set to 0 to disable dynamic batching and process all members per tile in a single thread
 */
static auto constexpr batchCostGoal = 256;

//------------------------------------------

namespace Nebulite::Constants {
/**
 * @class Nebulite::Constants::ThreadSettings
 * @brief Threading settings for Nebulite's Ruleset processing
 */
class ThreadSettings {
    static size_t getThreadCount() {
        size_t const threadCount = std::min(
            std::max(static_cast<size_t>(std::thread::hardware_concurrency()), static_cast<size_t>(1)),
            Maximum::totalThreadCount
        );
        return threadCount;
    }

    struct Spreading {
        static double constexpr invokeWorker = 0.75; // Percentage of threads dedicated to global ruleset processing
        static double constexpr rendererWorker = 0.125; // Percentage of threads dedicated to local ruleset processing
    };

    static_assert(Spreading::invokeWorker + Spreading::rendererWorker < 1.0, "Thread spreading percentages must sum to less than 1.0 to not take up all available threads.");
public:
    static size_t getInvokeWorkerCount() {
        return std::max(
            static_cast<size_t>(1),
            static_cast<size_t>(std::floor(static_cast<double>(getThreadCount()) * Spreading::invokeWorker))
        );
    }

    static size_t getRendererWorkerCount() {
        return std::max(
            static_cast<size_t>(1),
            static_cast<size_t>(std::floor(static_cast<double>(getThreadCount()) * Spreading::rendererWorker))
        );
    }

    // Maximum values, for array initialization
    struct Maximum {
        // Absolute maximum thread count that gets considered for spreading, to prevent overflow in extreme cases
        static size_t constexpr totalThreadCount = 32;

        // Maximum invoke worker count
        static size_t constexpr invokeWorkerCount = static_cast<size_t>(static_cast<double>(totalThreadCount) * Spreading::invokeWorker);

        // Maximum renderer worker count
        static size_t constexpr rendererWorkerCount = static_cast<size_t>(static_cast<double>(totalThreadCount) * Spreading::rendererWorker);
    };
};
} // namespace Nebulite::Constants

#endif // NEBULITE_CONSTANTS_THREAD_SETTINGS_HPP
