#include <Flux/Core/ClipboardUtil.hpp>
#include <SDL3/SDL.h>

namespace flux {

void setClipboardText(const std::string& text) {
    SDL_SetClipboardText(text.c_str());
}

bool hasClipboardText() {
    return SDL_HasClipboardText();
}

std::string getClipboardText() {
    if (!SDL_HasClipboardText()) return {};
    char* text = SDL_GetClipboardText();
    if (!text) return {};
    std::string out(text);
    SDL_free(text);
    return out;
}

} // namespace flux
