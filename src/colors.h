#pragma once

namespace app::Color {
    constexpr static SDL_Color Grid{0xF0, 0xF0, 0xF0, SDL_ALPHA_OPAQUE};
    constexpr static SDL_Color Transparent{0, 0, 0, SDL_ALPHA_TRANSPARENT};
    constexpr static SDL_Color DeadCell{0xB8, 0xD0, 0xF8, SDL_ALPHA_OPAQUE};
    constexpr static SDL_Color AliveCell{0x34, 0x77, 0xEB, SDL_ALPHA_OPAQUE};
} // namespace app::Color
