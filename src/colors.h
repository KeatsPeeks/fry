#pragma once

namespace app::Color {
    constexpr static SDL_Color Grid{240, 240, 240, SDL_ALPHA_OPAQUE};
    constexpr static SDL_Color Transparent{0, 0, 0, SDL_ALPHA_TRANSPARENT};
    constexpr static SDL_Color DeadCell{203, 221, 250, SDL_ALPHA_OPAQUE};
    constexpr static SDL_Color AliveCell{52, 119, 235, SDL_ALPHA_OPAQUE};
    constexpr static SDL_Color PatternOverlay{220, 220, 220, 150};
    constexpr static SDL_Color PatternCell{52, 119, 235, SDL_ALPHA_OPAQUE};
} // namespace app::Color
