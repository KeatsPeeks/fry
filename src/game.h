#pragma once

#include "clock.h"
#include "gui.h"
#include "nuklear_sdl.h"
#include "primitives.h"
#include "sdl_wrappers.h"
#include "simulation.h"

#include <span>

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
        Gui gui;

        void handleEvents(std::span<SDL_Event> events);
        void mouseEdit(Point mouse, bool alive);

        void update(const GameTime& gameTime);

        void render(const GameTime& gameTime);

        void onViewportChanged();

    };

}  // namespace app
