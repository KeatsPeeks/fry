#pragma once

#include "nuklear/nuklear.h"

#include <string_view>
#include <SDL2/SDL.h>
#include <SDL_ttf/SDL_ttf.h>
#include "SDL2_gfx/SDL2_gfxPrimitives.h"
#include "sdl_wrappers.h"

namespace app {

    class NuklearSdl {
    public:
        NuklearSdl(SDL_Window *win, SDL_Renderer *renderer, std::string_view font, int fontSize);
        ~NuklearSdl();

        [[nodiscard]] nk_context& getContext() { return context; }

        void handleEvent(SDL_Event *evt);
        void render();

        NuklearSdl(const NuklearSdl&) = delete;
        NuklearSdl& operator=(const NuklearSdl&) = delete;
        NuklearSdl(const NuklearSdl&&) = delete;
        NuklearSdl& operator=(const NuklearSdl&&) = delete;

    private:
        SDL_Window *win;
        SDL_Renderer *renderer;
        sdl::unique_ptr<TTF_Font> ttf_font;
        nk_user_font nk_font;
        nk_context context{};
    };

}
