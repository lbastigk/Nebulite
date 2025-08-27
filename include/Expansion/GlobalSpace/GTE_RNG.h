/**
 * @file GTE_RNG.h
 * 
 * @todo: Implement random number generation functions for GlobalSpaceTree
 * - include old global.rand and global.rrand for compatibility
 * - New structure: global.rng.A/B/C/D/...
 * 
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
 */

#include "Constants/ErrorTypes.h"
#include "Interaction/Execution/ExpansionWrapper.h"

//----------------------------------------------------------
// Forward declarations
namespace Nebulite{
    namespace Core{
        class GlobalSpace; // Forward declaration of domain class GlobalSpace
    }
}

//----------------------------------------------------------
namespace Nebulite {
namespace Expansion {
namespace GlobalSpace {
/**
 * @class Nebulite::Expansion::GlobalSpace::RNG
 * @brief Expansion for random number generation within the GlobalSpace.
 */
class RNG : public Nebulite::Interaction::Execution::ExpansionWrapper<Nebulite::Core::GlobalSpace, RNG> {

};
} // namespace GlobalSpace
} // namespace Expansion
} // namespace Nebulite