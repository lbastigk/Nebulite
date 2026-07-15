#ifndef NEBULITE_DATA_TASKQUEUERESULT_HPP
#define NEBULITE_DATA_TASKQUEUERESULT_HPP

//------------------------------------------
// Includes

// Standard library
#include <vector>

// Nebulite
#include "Nebulite/Constants/Event.hpp"

//------------------------------------------
namespace Nebulite::Data {
/**
 * @brief Represents the result of resolving a task queue.
 *        This structure holds the outcome of processing a task queue, including any errors
 *        encountered during resolution and whether the process was halted due to a critical error.
 *        Does not include successful results
 * @note Since Events just hold the type (Warning, Error), returning a vector of results might be unnecessary.
 *       Although it's sort of useful, as we can count the amount of events and inform: "Task encountered X warnings and Y errors"
 *       Later on we might change that to 2 std::size_t that count the amount of warnings and errors, instead of returning a vector of events.
 */
struct TaskQueueResult {
    bool encounteredCriticalResult = false; // Indicates if a critical error was encountered during task resolution
    std::vector<Constants::Event> events;   // List of events encountered during task resolution

    Constants::Event worstEvent();
};
}// namespace Nebulite::Data
#endif // NEBULITE_DATA_TASKQUEUERESULT_HPP
