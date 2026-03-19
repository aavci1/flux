#include <Flux/Platform/EventLoopWake.hpp>
#include <SDL3/SDL.h>

namespace flux {

void wakePlatformEventLoop() {
    SDL_Event wakeEvent{};
    wakeEvent.type = SDL_EVENT_USER;
    SDL_PushEvent(&wakeEvent);
}

} // namespace flux
