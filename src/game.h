#pragma once

#include "clock.h"
#include "gui.h"
#include "nuklear_sdl.h"
#include "primitives.h"
#include "sdl_wrappers.h"
#include "simulation.h"

#include <span>

namespace app {

    class Coordinates final {
    public:
        Coordinates(Size sim, Size window, int gridCellSize) : m_sim(sim), m_window(window), m_gridCellSize(gridCellSize) {}

        [[nodiscard]] int gridCellSize() const { return m_gridCellSize; }
        [[nodiscard]] Size window() const { return m_window; }
        [[nodiscard]] Size grid() const { return m_grid; }

        [[nodiscard]] Point windowToGrid(Point p) const {return {
                    static_cast<int>(p.x * windowToGridScaleX),
                    static_cast<int>(p.y * windowToGridScaleY)};
        }
        [[nodiscard]] Point gridToSim(Point p) const { return p + gridToSimOffset; }
        [[nodiscard]] Point simToGrid(Point p) const { return p - gridToSimOffset; }
        [[nodiscard]] Point windowToSim(Point p) const { return gridToSim(windowToGrid(p)); }

    private:
        Size m_sim;
        Size m_window;
        int m_gridCellSize;

        Size m_grid = m_window / m_gridCellSize;

        Vector gridToSimOffset = Vector{m_sim.w - m_grid.w, m_sim.h - m_grid.h} / 2;
        double windowToGridScaleX = static_cast<double>(m_grid.w) / m_window.w;
        double windowToGridScaleY = static_cast<double>(m_grid.h) / m_window.h;
    };

    class Game final {
    public:
        explicit Game(sdl::Window* window);

        bool mainLoop();

    private:
        sdl::Window* window;
        sdl::Renderer renderer;
        Coordinates coordinates;
        sdl::Texture gridTexture;
        sdl::Texture renderTexture;
        GameClock clock{};
        bool paused{true};
        bool benchmark{false};
        Simulation simulation;
        NuklearSdl nuklearSdl;
        Gui gui;

        void handleEvents(std::span<SDL_Event> events);
        void mouseEdit(Point mouse, CellState state);

        void update(const GameTime& gameTime);

        void render(const GameTime& gameTime);

        void onViewportChanged();
    };

}  // namespace app
