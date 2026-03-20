#include <Flux/Platform/SDLEventLoopWake.hpp>
#include <SDL3/SDL.h>

namespace flux {

void SDLEventLoopWake::wake() {
    SDL_Event wakeEvent{};
    wakeEvent.type = SDL_EVENT_USER;
    SDL_PushEvent(&wakeEvent);
}

} // namespace flux
