#pragma once

#include "simulation.h"
#include "clock.h"
#include "nuklear_sdl.h"
#include "sdl_wrappers.h"

#include <vector>

namespace app {

    class Game final {
    public:
        explicit Game(sdl::Window* window);

        bool mainLoop();

    private:
        sdl::Renderer renderer;
        sdl::Texture gridTexture;
        sdl::Texture renderTexture;
        std::vector<uint32_t> pixels;
        GameClock clock{};
        bool paused{true};
        bool benchmark{false};
        Simulation simulation;
        NuklearSdl nuklearSdl;
        nk_context* pNuklearCtx;

        void handleEvents(const std::vector<SDL_Event>& events);
        void onLeftMouse(int x, int y);

        void update(const GameTime& gameTime);

        void render(const GameTime& gameTime);

        void onViewportChanged();
    };

}  // namespace app
