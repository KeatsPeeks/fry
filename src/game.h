#pragma once

#include "clock.h"
#include "sdl_wrappers.h"

#include <vector>

namespace app {

    class Game final {
    public:
        explicit Game(SDL_Window *pWindow);

        void mainLoop();

    private:
        sdl::Renderer renderer;
        sdl::Texture gridTexture;

        void handleEvents(const std::vector<SDL_Event>& events);

        void update(const GameTime& gameTime);

        void render(const GameTime& gameTime);

        sdl::Texture createNewGridTexture();
    };

}  // namespace app
