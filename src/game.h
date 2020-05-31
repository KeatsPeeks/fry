#pragma once

#include "clock.h"
#include "sdl_helpers.h"

#include <SDL2/SDL.h>
#include <memory>
#include <vector>

namespace app {

    class Game final {
    public:
        explicit Game(SDL_Window *pWindow);

        void mainLoop();

    private:
        sdl_unique_ptr<SDL_Renderer> pRenderer;

        void handleEvents(const std::vector<SDL_Event>& vector);

        void update(const GameTime& gameTime);

        void render(const GameTime& gameTime);
    };

}  // namespace app
