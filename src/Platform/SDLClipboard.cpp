#include <Flux/Platform/SDLClipboard.hpp>
#include <SDL3/SDL.h>

namespace flux {

void SDLClipboard::setText(const std::string& text) {
    SDL_SetClipboardText(text.c_str());
}

bool SDLClipboard::hasText() {
    return SDL_HasClipboardText();
}

std::string SDLClipboard::getText() {
    if (!SDL_HasClipboardText()) return {};
    char* text = SDL_GetClipboardText();
    if (!text) return {};
    std::string out(text);
    SDL_free(text);
    return out;
}

} // namespace flux
