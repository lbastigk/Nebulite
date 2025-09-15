#include "Core/Texture.hpp"
#include "DomainModule/TDM.hpp"

Nebulite::Core::Texture::Texture(Nebulite::Utility::JSON* doc, SDL_Renderer* renderer, SDL_Texture* texture) 
: Nebulite::Interaction::Execution::Domain<Texture>("Texture",this,doc), 
  renderer(renderer), 
  texture(texture) 
{
    Nebulite::DomainModule::TDM_init(this);
}

void Nebulite::Core::Texture::update() {
    for(auto& module : modules){
        module->update();
    }
}