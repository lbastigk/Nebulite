//------------------------------------------
// Forward declarations
namespace Nebulite::Core {
class RenderObject;
class GlobalSpace;
}   // namespace Nebulite::Core

namespace Nebulite::Interaction::Execution {
class DomainBase;
}   // namespace Nebulite::Interaction::Execution

namespace Nebulite::Interaction {

struct ContextBase {
    Interaction::Execution::DomainBase& self;
    Interaction::Execution::DomainBase& other;
    Interaction::Execution::DomainBase& global;
    // TODO: Parent context?
};

struct Context {
    Nebulite::Core::RenderObject& self;
    Nebulite::Core::RenderObject& other;
    Nebulite::Core::GlobalSpace& global;
    // TODO: Parent context?
};


} // namespace Nebulite::Interaction