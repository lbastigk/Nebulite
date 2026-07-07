#ifndef NEBULITE_INTERACTION_RULES_LISTENER_HPP
#define NEBULITE_INTERACTION_RULES_LISTENER_HPP

//------------------------------------------
// Includes

// Standard library
#include <string>
#include <string_view>

//------------------------------------------
// Forward declarations

namespace Nebulite::Interaction::Execution {
class Domain;
} // namespace Nebulite::Interaction::Execution

//------------------------------------------
namespace Nebulite::Interaction::Rules {
/**
 * @struct Listener
 * @brief Represents a listener for a specific topic
 */
struct Listener {
    explicit Listener(Execution::Domain& d, std::string_view t);
    ~Listener() = default;

    Execution::Domain& domain;
    std::string topic;
    double** otr; // Pointer to the ordered cache list of the listener, for performance when evaluating rulesets

    // Listener is owned by a single Domain, no copy or move semantics

    Listener(Listener const&) = delete;
    Listener& operator=(Listener const&) = delete;
    Listener(Listener&&) = delete;
    Listener& operator=(Listener&&) = delete;
};
} // namespace Nebulite::Interaction::Rules
#endif // NEBULITE_INTERACTION_RULES_LISTENER_HPP

