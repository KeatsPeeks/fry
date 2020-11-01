#include "game.h"

#include "../deps/nuklear/nuklear.h"
#include "../deps/tinydir.h"

#include "colors.h"
#include "pattern.h"

#include <algorithm>
#include <filesystem>
#include <fmt/format.h>

namespace app {

namespace {

    constexpr Size simSize{11264, 6336};
    constexpr double minFps = 45.;

    bool isMouseEvent(const SDL_Event& e) {
        return e.type == SDL_MOUSEWHEEL || e.type == SDL_MOUSEBUTTONDOWN || e.type == SDL_MOUSEBUTTONUP || e.type == SDL_MOUSEMOTION;
    }

    sdl::Texture createRenderTexture(const sdl::Renderer& renderer, Coordinates coords) {
        sdl::Texture texture{SDL_CreateTexture(renderer.getRaw(), SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_TARGET, coords.grid().w, coords.grid().h)};
        texture.setBlendMode(SDL_BLENDMODE_BLEND);
        return texture;
    }

    sdl::Texture createGridTexture(const sdl::Renderer& renderer, Coordinates coords) {
        const Size textureSize = {coords.window().w - coords.window().w % coords.gridCellSize(), coords.window().h - coords.window().h % coords.gridCellSize()};
        const Size nbLines = coords.grid() + 1;

        sdl::Texture texture{SDL_CreateTexture(renderer.getRaw(), SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_TARGET, textureSize.w, textureSize.h)};
        texture.setBlendMode(SDL_BLENDMODE_BLEND);
        renderer.setTarget(texture.getRaw());
        renderer.setDrawColor(Color::Transparent);
        renderer.clear();
        renderer.setDrawColor(Color::Grid);
        for (int y = 1; y <= nbLines.h; ++y) {
            renderer.drawLine(
               {0, y * coords.gridCellSize()},
               {nbLines.w * coords.gridCellSize(), y * coords.gridCellSize()}
            );
        }
        for (int x = 1; x <= nbLines.w; ++x) {
            renderer.drawLine(
               {x * coords.gridCellSize(), 0},
               {x * coords.gridCellSize(), nbLines.h * coords.gridCellSize()});
        }

        renderer.setTarget(nullptr);
        return texture;
    }

    std::vector<Pattern> loadAllPatterns() {
        std::vector<Pattern> patterns = Patterns::defaultPatterns();

        // std::filesystem requires macos 10.15+, so we use tinydir
        if (tinydir_dir dir; 0 == tinydir_open_sorted(&dir, "assets/patterns")) {
            std::unique_ptr<tinydir_dir, decltype(&tinydir_close)> finally{&dir, tinydir_close};
            for (size_t i = 0; i < dir.n_files; i++) {
                tinydir_file file;
                tinydir_readfile_n(&dir, &file, i);
                const std::optional<Pattern>& pattern = loadFromFile(&file.name[0], &file.path[0]);
                if (pattern) {
                    patterns.push_back(pattern.value());
                }
            }
        }

        return patterns;
    }

} // anonymous namespace

Game::Game(sdl::Window* window) :
    window{window},
    cursor{SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_CROSSHAIR)},
    guiCursor{SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_ARROW)},
    pointerCursor{SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_HAND)},
    renderer{SDL_CreateRenderer(window->getRaw(), -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC)},
    coordinates(simSize, renderer.getOutputSize(), cellSize),
    gridTexture{createGridTexture(renderer, coordinates)},
    renderTexture{createRenderTexture(renderer, coordinates)},
    simulation{std::make_unique<Simulation>(simSize, Patterns::acorn())},
    nuklearSdl{window->getRaw(), renderer.getRaw(), "assets/Cousine-Regular.ttf", 16},
    bindings{&displayGrid, &updateSpeedPower, &paused, &cellSize, &selectedPattern, &modalGui, &step, &clear, &iteration},
    gui{&nuklearSdl.getContext(), loadAllPatterns(), bindings} {
    resetSimClock();
}

void Game::handleEvents(std::span<SDL_Event> events, bool mouseOnGui) {
    bool left = false;
    bool right = false;
    for (auto event : events) {
        if (isMouseEvent(event) && mouseOnGui) {
            continue;
        }

        switch (event.type) {
            case SDL_WINDOWEVENT: {
                if (SDL_WINDOWEVENT_RESIZED == event.window.event) {
                    onCoordinatesChanged();
                }
            } break;

            case SDL_MOUSEWHEEL: {
                if (!modalGui && event.wheel.y != 0) {
                    cellSize = std::min(32, std::max(1, cellSize + (event.wheel.y > 0 ? 1 : -1)));
                }
            } break;

            case SDL_MOUSEBUTTONDOWN: {
                left = event.button.button == SDL_BUTTON_LEFT;
                right = event.button.button == SDL_BUTTON_RIGHT;
            } break;

            case SDL_MOUSEMOTION: {
                left = 0 != (event.motion.state & SDL_BUTTON(SDL_BUTTON_LEFT));
                right = 0 != (event.motion.state & SDL_BUTTON(SDL_BUTTON_RIGHT));
            } break;

            case SDL_KEYDOWN: {
                SDL_Scancode scancode = event.key.keysym.scancode;
                if (!modalGui && selectedPattern == nullptr && SDL_SCANCODE_SPACE == scancode) {
                    paused = !paused;
                } else if (!modalGui && selectedPattern == nullptr && SDL_SCANCODE_RIGHT == scancode) {
                    step = true;
                } else if (!modalGui && selectedPattern == nullptr && SDL_SCANCODE_B == scancode) {
                    benchmark = true;
                } else if (!modalGui && SDL_SCANCODE_G == scancode) {
                    displayGrid = displayGrid == 0 ? 1 : 0;
                } else if (!modalGui && selectedPattern == nullptr && SDL_SCANCODE_UP == scancode) {
                    updateSpeedPower = std::min(10, updateSpeedPower + 1);
                } else if (!modalGui && selectedPattern == nullptr && SDL_SCANCODE_DOWN == scancode) {
                    updateSpeedPower = std::max(0, updateSpeedPower - 1);
                } else if (SDL_SCANCODE_ESCAPE == scancode) {
                    selectedPattern = nullptr;
                    modalGui = false;
                }
            } break;

            default:
                break;
        }
    }
    SDL_GetMouseState(&mouse.x, &mouse.y);

    if (coordinates.gridCellSize() != cellSize) {
        if (cellSize < coordinates.gridCellSize() && cellSize < 4 && displayGrid == 1) {
            // disable grid automatically on zoom-out
            displayGrid = 0;
            gridAutoDisabled = true;
        } else if (cellSize > coordinates.gridCellSize() && cellSize >= 4 && gridAutoDisabled) {
            // re-enable the grid on zoom-in if it was disabled automatically
            displayGrid = 1;
            gridAutoDisabled = false;
        }
        onCoordinatesChanged();
    }
    if (!modalGui && selectedPattern == nullptr && left != right) {
        mouseEdit(left ? CellState::ALIVE : CellState::DEAD);
    }
    if (selectedPattern != nullptr) {
        if (left) {
            placeSelectedPattern();
        } else if (right) {
            selectedPattern = nullptr;
        }
    }

    // cursor
    if (mouseOnGui) {
        guiCursor.set();
    } else if (selectedPattern != nullptr) {
        pointerCursor.set();
    } else {
        cursor.set();
    }
}

void Game::update() {
    if (benchmark) {
        runBenchmark();
        return;
    }

    if (clear) {
        simulation = std::make_unique<Simulation>(simSize);
        lastUpdates.clear();
        iteration = 0;
        clear = false;
        paused = true;
        forceFullRedraw = true;
    }

    if (paused) {
        resetSimClock();

        if (step) {
            simulation->nextStep();
            forceFullRedraw = true;
            iteration++;
            step = false;
        }
        return;
    }

    GameTime time = simClock.update();
    while (time.totalTime.count() >= nextSimUpdate) {
        simulation->nextStep();
        lastUpdates.push_back(simulation->updatedCells());
        iteration++;
        nextSimUpdate += 1. / (1 << updateSpeedPower);
        if (minFpsClock.update().totalTime.count() > 1. / minFps) {
            // frame time is too large => throttle
            break;
        }
    }
    minFpsClock = GameClock{};

}

void Game::runBenchmark() {
    std::vector<std::pair<Pattern, int>> patterns = {
            { Patterns::acorn(), 4000 },
            { Patterns::infinite(), 8000 },
    };
    std::string message;
    for (const auto& pattern : patterns) {
        Simulation sim{simSize, pattern.first};
        GameClock benchClock;
        for (int j = 0; j < pattern.second; j++) {
            sim.nextStep();
        }
        const GameTime gameTime = benchClock.update();
        auto result = std::lround(pattern.second / gameTime.elapsedTime.count());
        message += fmt::format("{} = {} ups\n", pattern.first.name(), result);
    }

    window->showSimpleMessageBox(SDL_MESSAGEBOX_INFORMATION, "Benchmark results", message.c_str());
    benchmark = false;
    resetSimClock();
}

void Game::resetSimClock() {
    simClock = GameClock{};
    nextSimUpdate = 1. / (1 << updateSpeedPower);
}

void Game::mouseEdit(CellState cellState) {
    if (paused) {
        const Point point = coordinates.windowToSim(mouse);
        simulation->set(point.x, point.y, cellState);
    }
}

void Game::onCoordinatesChanged() {
    coordinates = Coordinates{simSize, renderer.getOutputSize(), cellSize};
    gridTexture = createGridTexture(renderer, coordinates);
    renderTexture = createRenderTexture(renderer, coordinates);
    forceFullRedraw = true;
}

void Game::placeSelectedPattern() {
    if (selectedPattern == nullptr) {
        return;
    }

    Size s = selectedPattern->size() / 2;
    const Vector offset = {s.w, s.h};
    const Point origin = coordinates.windowToSim(mouse) - offset;
    std::vector<SDL_Point> patternCells;
    for (const Point& p : selectedPattern->aliveCells()) {
        simulation->set(origin.x + p.x, origin.y + p.y, CellState::ALIVE);
    }
    forceFullRedraw = true;

    selectedPattern = nullptr;
}

void Game::render() {
    renderCells();
    renderSelectedPattern();
    renderGrid();

    nuklearSdl.render();
    renderer.present();
}

void Game::renderSelectedPattern() const {
    if (selectedPattern == nullptr && !modalGui) {
        return;
    }

    renderer.setTarget(renderTexture.getRaw());

    renderer.setDrawColor(Color::PatternOverlay);
    renderer.fillRect(nullptr);

    if (selectedPattern != nullptr) {
        Size s = selectedPattern->size() / 2;
        const Vector offset = {s.w, s.h};
        const Point origin = coordinates.windowToGrid(mouse) - offset;
        std::vector<SDL_Point> patternCells;
        for (const Point& p : selectedPattern->aliveCells()) {
            patternCells.push_back({origin.x + p.x, origin.y + p.y});
        }
        renderer.setDrawColor(Color::PatternCell);
        renderer.drawPoints(patternCells);
    }

    renderer.setTarget(nullptr);
    renderer.copy(renderTexture.getRaw(), nullptr, nullptr);
}

void Game::renderGrid() const {
    if (displayGrid == 1) {
        renderer.copy(gridTexture.getRaw(), nullptr, nullptr);
    }
}

void Game::renderCells() {
    renderer.setTarget(renderTexture.getRaw());

    if (forceFullRedraw || selectedPattern != nullptr) {

        // FULL mode

        renderer.setDrawColor(Color::DeadCell);
        renderer.fillRect(nullptr);

        std::vector<SDL_Point> alives;
        forceFullRedraw = false;
        for (int y = 0; y < coordinates.grid().h; y++) {
            for (int x = 0; x < coordinates.grid().w; x++) {
                Point p = coordinates.gridToSim({x, y});
                if (p.x >= 0 && p.y >= 0 && p.x < simulation->size().w && p.y < simulation->size().h && simulation->get(p.x, p.y) == CellState::ALIVE) {
                    alives.push_back({x, y});
                }
            }
        }

        if (!alives.empty()) {
            renderer.setDrawColor(Color::AliveCell);
            renderer.drawPoints(alives);
        }
    } else {

        // DELTA mode

        std::vector<SDL_Point> alives;
        std::vector<SDL_Point> deads;

        for (const auto& update : lastUpdates) {
            for (const Cell& cell : *update) {
                Point p = coordinates.simToGrid({cell.x, cell.y});
                if (p.x >= 0 && p.y >= 0 && p.x < coordinates.grid().w && p.y < coordinates.grid().h) {
                    std::vector<SDL_Point>& ref = cell.state == CellState::ALIVE ? alives : deads;
                    ref.push_back({p.x, p.y});
                }
            }

            if (!alives.empty()) {
                renderer.setDrawColor(Color::AliveCell);
                renderer.drawPoints(alives);
            }
            if (!deads.empty()) {
                renderer.setDrawColor(Color::DeadCell);
                renderer.drawPoints(deads);
            }
            alives.clear();
            deads.clear();
        }
        lastUpdates.clear();
    }

    renderer.setTarget(nullptr);
    renderer.copy(renderTexture.getRaw(), nullptr, nullptr);
}

bool Game::mainLoop() {
    // EVENTS
    std::vector<SDL_Event> events;
    for (SDL_Event event; SDL_PollEvent(&event) != 0;) {
        if (SDL_RENDER_TARGETS_RESET == event.type) {
            // This needs to be called before nuklear starts creating drawing commands
            gridTexture = createGridTexture(renderer, coordinates);
            forceFullRedraw = true;
            gui.onSdlContextLost();
        }

        if (SDL_QUIT == event.type) {
            return true;
        }
        events.push_back(event);
    }
    nuklearSdl.handleEvents(events);
    gui.update(renderer);
    bool mouseOnGui = nk_window_is_any_hovered(&nuklearSdl.getContext()) != 0;
    handleEvents(events, mouseOnGui);

    // UPDATE and RENDER
    update();
    render();

    return false;
}

} // namespace app

