#include "UI/PluginBase.hpp"

namespace Nebulite::UI {

PluginBase::PluginBase(Utility::Capture& c) : capture(c) {}

void PluginBase::update() {};

} // namespace Nebulite::UI