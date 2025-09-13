/**
 * @file GDM_RNG.hpp
 * 
 * @todo: Implement random number generation functions
 * - include old global.rand and global.rrand for compatibility
 * - New structure: global.rng.A/B/C/D/...
 * 
 * @todo [Discussion on proper RNG -> using functioncalls as hash]
 * Biggest todo: Proper deterministic rng:
 * - we cant update after each frame unless we use a consistent dt
 * - one idea is to poll input events and use them to seed the RNG
 * - another is a fixed time, but this is likely to lead to microscopic issues 
 *   as one run might load a frame slightly faster, causing rng desync
 *   e.g.: rng update after 10ms, one might be at 9.9 and the other at 10.1 as we can never directly wait 10ms
 *   There might be some way to mitigate this with threads or other synchronization methods
 *   But this is a complex issue to a simple problem we can fix with input polling
 * - but input polling can also lead to desync!
 * - Better: we may poll function calls instead!!
 *   This way, we can ensure that the RNG is updated consistently with the game state.
 *   Since anything needing an RNG will be called in the same context, we can guarantee
 *   that the RNG is updated at the right time.
 *   optionally, we might call an rng-update after every functioncall
 *   This would ensure that the RNG is always updated after each potential usage.
 *   we can make this two-fold: 
 *   - general update call after each frame depending on functioncalls
 *   - manual update from functioncalls
 *   So we use the last functioncall to re-seed multiple RNGs.
 *   In order to better incorporate this if no functioncall is present, we may also seed with current inputs
 *   -> Deep integration into globalspace needed.
 */

#pragma once

//------------------------------------------
// Includes

// General
#include "Constants/ErrorTypes.hpp"
#include "Interaction/Execution/DomainModule.hpp"

//------------------------------------------
// Forward declarations
namespace Nebulite{
    namespace Core{
        class GlobalSpace; // Forward declaration of domain class GlobalSpace
    }
}

//------------------------------------------
namespace Nebulite {
namespace DomainModule {
namespace GlobalSpace {
/**
 * @class Nebulite::DomainModule::GlobalSpace::RNG
 * @brief DomainModule for random number generation within the GlobalSpace.
 */
class RNG : public Nebulite::Interaction::Execution::DomainModule<Nebulite::Core::GlobalSpace> {

};
} // namespace GlobalSpace
} // namespace DomainModule
} // namespace Nebulite