#pragma once

#include "../deps/nuklear/nuklear.h"
#include "../deps/SDL2_gfx/SDL2_gfxPrimitives.h"

#include "sdl_wrappers.h"

#include <span>
#include <string>
#include <SDL2/SDL.h>
#include <SDL_ttf/SDL_ttf.h>

namespace app {

class NuklearSdl {
public:
    NuklearSdl(SDL_Window *win, SDL_Renderer *renderer, const std::string& font, int fontSize);
    ~NuklearSdl();

    [[nodiscard]] nk_context& getContext() { return context; }

    void handleEvents(std::span<SDL_Event> events);
    void render();

    NuklearSdl(const NuklearSdl&) = delete;
    NuklearSdl& operator=(const NuklearSdl&) = delete;
    NuklearSdl(const NuklearSdl&&) = delete;
    NuklearSdl& operator=(const NuklearSdl&&) = delete;

private:
    void handleEvent(const SDL_Event& evt);

    SDL_Window *win;
    SDL_Renderer *renderer;
    sdl::unique_ptr<TTF_Font> ttf_font;
    nk_user_font nk_font;
    nk_context context{};
};

} // namespace app
