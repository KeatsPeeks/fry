#include "game.h"

#include "../deps/nuklear/nuklear.h"

#include "colors.h"
#include "patterns.h"

#include <algorithm>
#include <fmt/format.h>

namespace app {

    namespace {

        constexpr Size simSize{2048, 2048};
        constexpr int benchIters = 4000;
        constexpr auto defaultPattern = []() { return Patterns::acorn(); };
        constexpr double minFps = 45.;

        bool isMouseEvent(const SDL_Event& e) {
            return e.type == SDL_MOUSEWHEEL || e.type == SDL_MOUSEBUTTONDOWN || e.type == SDL_MOUSEBUTTONUP || e.type == SDL_MOUSEMOTION;
        }

        sdl::Texture createRenderTexture(const sdl::Renderer& renderer, Coordinates coords) {
            return sdl::Texture{SDL_CreateTexture(renderer.getRaw(), SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_TARGET, coords.grid().w, coords.grid().h)};
        }

        sdl::Texture createGridTexture(const sdl::Renderer& renderer, Coordinates coords) {
            const Size textureSize{coords.window().w - coords.window().w % coords.gridCellSize(), coords.window().h - coords.window().h % coords.gridCellSize()};
            const Size nbLines = coords.grid() + 1;

            sdl::Texture texture{SDL_CreateTexture(renderer.getRaw(), SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_TARGET, textureSize.w, textureSize.h)};
            texture.setBlendMode(SDL_BLENDMODE_BLEND);
            renderer.setTarget(texture.getRaw());
            renderer.setDrawColor(Color::Transparent);
            renderer.clear();
            renderer.setDrawColor(Color::Grid);
            for (int y = 1; y <= nbLines.h; ++y) {
                renderer.drawLine({0, y * coords.gridCellSize()}, {nbLines.w * coords.gridCellSize(), y * coords.gridCellSize()});
            }
            for (int x = 1; x <= nbLines.w; ++x) {
                renderer.drawLine({x * coords.gridCellSize(), 0}, {x * coords.gridCellSize(), nbLines.h * coords.gridCellSize()});
            }

            renderer.setTarget(nullptr);
            return texture;
        }

    } // anonymous namespace

    Game::Game(sdl::Window* window) :
        window{window},
        cursor{SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_CROSSHAIR)},
        guiCursor{SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_ARROW)},
        renderer{SDL_CreateRenderer(window->getRaw(), -1, SDL_RENDERER_ACCELERATED)},
        coordinates(simSize, renderer.getOutputSize(), cellSize),
        gridTexture{createGridTexture(renderer, coordinates)},
        renderTexture{createRenderTexture(renderer, coordinates)},
        simulation{simSize.w, defaultPattern()},
        nuklearSdl(window->getRaw(), renderer.getRaw(), "assets/Cousine-Regular.ttf", 16),
        gui(&nuklearSdl.getContext(), {&displayGrid, &updateSpeedPower, &paused})
    {
        resetSimClock();
    }

    void Game::handleEvents(std::span<SDL_Event> events, bool mouseOnGui) {
        bool left = false;
        bool right = false;
        Point mouse;
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
                    if (event.wheel.y != 0) {
                        cellSize = std::max(1, cellSize + (event.wheel.y > 0 ? 1 : -1));
                        onCoordinatesChanged();
                    }
                } break;

                case SDL_MOUSEBUTTONDOWN: {
                    left = event.button.button == SDL_BUTTON_LEFT;
                    right = event.button.button == SDL_BUTTON_RIGHT;
                    mouse = {event.button.x, event.button.y};
                } break;

                case SDL_MOUSEMOTION: {
                    left = 0 != (event.motion.state & SDL_BUTTON(SDL_BUTTON_LEFT));
                    right = 0 != (event.motion.state & SDL_BUTTON(SDL_BUTTON_RIGHT));
                    mouse = {event.motion.x, event.motion.y};
                } break;

                case SDL_KEYDOWN: {
                    SDL_Scancode scancode = event.key.keysym.scancode;
                    if (SDL_SCANCODE_SPACE == scancode) {
                        paused = !paused;
                    } else if (SDL_SCANCODE_RIGHT == scancode) {
                        step = true;
                    } else if (SDL_SCANCODE_B == scancode) {
                        benchmark = true;
                    } else if (SDL_SCANCODE_G == scancode) {
                        displayGrid = displayGrid == 0 ? 1 : 0;
                    } else if (SDL_SCANCODE_UP == scancode) {
                        updateSpeedPower = std::min(10, updateSpeedPower + 1);
                    } else if (SDL_SCANCODE_DOWN == scancode) {
                        updateSpeedPower = std::max(0, updateSpeedPower - 1);
                    }
                } break;

                default:
                    break;
            }
        }
        if (left != right) {
            mouseEdit(mouse, left ? CellState::ALIVE : CellState::DEAD);
        }
        if (mouseOnGui) {
            guiCursor.set();
        } else {
            cursor.set();
        }
    }

    void Game::update() {
        if (benchmark) {
            Simulation backup = simulation;
            GameClock benchClock{};
            for (int j = 0; j < benchIters; j++) {
                simulation.nextStep();
            }
            const GameTime gameTime = benchClock.update();
            const auto message = fmt::format("{} iterations per second", std::lround(benchIters / gameTime.elapsedTime.count()));
            window->showSimpleMessageBox(SDL_MESSAGEBOX_INFORMATION, "Benchmark results", message.c_str());
            benchmark = false;
            simulation = backup;
            resetSimClock();
            return;
        }

        if (paused) {
            resetSimClock();

            if (step) {
                simulation.nextStep();
                step = false;
            }
            return;
        }

        GameTime time = simClock.update();
        while (time.totalTime.count() >= nextSimUpdate) {
            simulation.nextStep();
            nextSimUpdate += 1. / (1 << updateSpeedPower);
            if (minFpsClock.update().totalTime.count() > 1. / minFps) {
                // frame time is too large => throttle
                break;
            }
        }
        minFpsClock = GameClock{};

    }

    void Game::resetSimClock() {
        simClock = GameClock{};
        nextSimUpdate = 1. / (1 << updateSpeedPower);
    }

    void Game::mouseEdit(Point mouse, CellState cellState) {
        if (paused) {
            const Point point = coordinates.windowToSim(mouse);
            simulation.set(point.x, point.y, cellState);
        }
    }

    void Game::onCoordinatesChanged() {
        coordinates = Coordinates{simSize, renderer.getOutputSize(), cellSize};
        gridTexture = createGridTexture(renderer, coordinates);
        renderTexture = createRenderTexture(renderer, coordinates);
    }

    void Game::render() {
        const Simulation::TAliveList& aliveList = simulation.getAliveCells();
        std::vector<SDL_Point> pixels{};
        std::for_each(aliveList.cbegin(), aliveList.cend(), [=,&pixels](std::pair<int, int> xy) {
            const Point tex = coordinates.simToGrid({xy.first, xy.second});
            if (tex.x >= 0 && tex.x < coordinates.grid().w && tex.y >= 0 && tex.y < coordinates.grid().h) {
                pixels.push_back({tex.x, tex.y});
            }
        });

        renderer.setTarget(renderTexture.getRaw());
        renderer.setDrawColor(Color::DeadCell);

        renderer.fillRect(nullptr);
        if (!pixels.empty()) {
            renderer.setDrawColor(Color::AliveCell);
            renderer.drawPoints(pixels);
        }
        renderer.setTarget(nullptr);
        renderer.copy(renderTexture.getRaw(), nullptr, nullptr);

        if (displayGrid == 1) {
            renderer.copy(gridTexture.getRaw(), nullptr, nullptr);
        }
        
        nuklearSdl.render();
        renderer.present();
    }

    bool Game::mainLoop() {
        // EVENTS
        std::vector<SDL_Event> events;
        for (SDL_Event event; SDL_PollEvent(&event) != 0;) {
            if (SDL_QUIT == event.type) {
                return true;
            }
            events.push_back(event);
        }
        nuklearSdl.handleEvents(events);
        gui.update(renderer.getOutputSize().w);
        bool mouseOnGui = nk_window_is_any_hovered(&nuklearSdl.getContext()) != 0;
        handleEvents(events, mouseOnGui);

        // UPDATE and RENDER
        update();
        render();

        return false;
    }


} // namespace app

