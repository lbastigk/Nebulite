// Toggling console mode
// TODO: insert into GDM_Console
// Using the Renderer capability to queue textures
/*
if(domain->RendererExists() && domain->getDoc()->get<int>("input.keyboard.delta.`",0) == 1){
    domain->getRenderer()->toggleConsoleMode();
    if(domain->getRenderer()->isConsoleMode()){
        SDL_StartTextInput();
        SDL_FlushEvents(SDL_FIRSTEVENT, SDL_LASTEVENT); // Flush all pending events
    }
    else{
        SDL_StopTextInput();
    }
}
*/