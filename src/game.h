#pragma once

#include "clock.h"
#include "gui.h"
#include "nuklear_sdl.h"
#include "pattern.h"
#include "primitives.h"
#include "sdl_wrappers.h"
#include "simulation.h"

#include <span>
#include <vector>

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
    // time
    GameClock simClock{};
    GameClock minFpsClock{};
    double nextSimUpdate{};

    // actions
    bool paused = true;
    bool benchmark = false;
    bool step = false;
    bool clear = false;

    // status
    const Pattern* selectedPattern = nullptr;
    bool modalGui = false;
    bool gridAutoDisabled = false;
    int iteration = 0;
    bool forceFullRedraw = true;
    std::vector<std::shared_ptr<std::vector<Cell>>> lastUpdates;

    // options
    int displayGrid = 1;
    int updateSpeedPower = 5;
    int cellSize = 12;

    sdl::Window* window;
    sdl::Cursor cursor;
    sdl::Cursor guiCursor;
    sdl::Cursor pointerCursor;
    sdl::Renderer renderer;
    Coordinates coordinates;
    sdl::Texture gridTexture;
    sdl::Texture renderTexture;
    std::unique_ptr<Simulation> simulation;
    NuklearSdl nuklearSdl;
    GuiBindings bindings;
    Gui gui;
    Point mouse;

    void handleEvents(std::span<SDL_Event> events, bool mouseOnGui);

    void mouseEdit(CellState state);

    void update();

    void render();

    void onCoordinatesChanged();

    void resetSimClock();

    void renderCells();
    void renderGrid() const;
    void renderSelectedPattern() const;

    void placeSelectedPattern();

    void runBenchmark();
};

}  // namespace app
