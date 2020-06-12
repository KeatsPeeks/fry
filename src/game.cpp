#include "game.h"

#include "../deps/nuklear/nuklear.h"

#include "colors.h"
#include "patterns.h"

#include <algorithm>
#include <fmt/format.h>

//#include "nuklear/overview.c"

namespace app {

    namespace {

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
            renderer.setDrawColor(TRANSPARENT_COLOR);
            renderer.clear();
            renderer.setDrawColor(GRID_COLOR);
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

    static const Size simSize{2048, 2048};
    static const int benchIters = 4000;
    static const auto defaultPattern = []() { return Patterns::acorn(); };

    static int displayGrid = 1;
    static const int speed = 1;
    static const int cellSize = 12;

    static bool step = false;

    Game::Game(sdl::Window* window) :
        window{window},
        renderer{SDL_CreateRenderer(window->getRaw(), -1, SDL_RENDERER_ACCELERATED)},
        coordinates(simSize, renderer.getOutputSize(), cellSize),
        gridTexture{createGridTexture(renderer, coordinates)},
        renderTexture{createRenderTexture(renderer, coordinates)},
        simulation{simSize.w, defaultPattern()},
        nuklearSdl(window->getRaw(), renderer.getRaw(), "assets/Cousine-Regular.ttf", 16),
        gui(&nuklearSdl.getContext(), {&displayGrid}) {
    }

    void Game::handleEvents(std::span<SDL_Event> events) {
        bool left = false;
        bool right = false;
        Point mouse;
        for (auto event : events) {
            if (SDL_WINDOWEVENT == event.type && SDL_WINDOWEVENT_RESIZED == event.window.event) {
                onViewportChanged();
            } else if (SDL_MOUSEBUTTONDOWN == event.type) {
                left = event.button.button == SDL_BUTTON_LEFT;
                right = event.button.button == SDL_BUTTON_RIGHT;
                mouse = {event.button.x, event.button.y};
            } else if (SDL_MOUSEMOTION == event.type) {
                left = 0 != (event.motion.state & SDL_BUTTON(SDL_BUTTON_LEFT));
                right = 0 != (event.motion.state & SDL_BUTTON(SDL_BUTTON_RIGHT));
                mouse = {event.motion.x, event.motion.y};
            } else if (SDL_KEYDOWN == event.type) {
                if (SDL_SCANCODE_SPACE == event.key.keysym.scancode) {
                    paused = !paused;
                } else if (SDL_SCANCODE_RIGHT == event.key.keysym.scancode) {
                    step = true;
                } else if (SDL_SCANCODE_B == event.key.keysym.scancode) {
                    benchmark = true;
                } else if (SDL_SCANCODE_G == event.key.keysym.scancode) {
                    displayGrid = displayGrid == 0 ? 1 : 0;
                }
            }
        }
        if (left != right) {
            mouseEdit(mouse, left ? CellState::ALIVE : CellState::DEAD);
        }
    }

    void Game::update(const GameTime& /*gameTime*/) {
        for (int i = 0; i< speed; ++i) {
            if (benchmark) {
                Simulation backup = simulation;
                clock.update();
                for (int j = 0; j < benchIters; j++) {
                    simulation.nextStep();
                }
                const GameTime gameTime = clock.update();
                const auto message = fmt::format("{} iterations per second", std::lround(benchIters / gameTime.elapsedTime.count()));
                window->showSimpleMessageBox(SDL_MESSAGEBOX_INFORMATION, "Benchmark results", message.c_str());
                benchmark = false;
                simulation = backup;
            }
            if (!paused || step) {
                simulation.nextStep();
                step = false;
            }
        }
    }

    void Game::mouseEdit(Point mouse, CellState cellState) {
        if (paused) {
            const Point point = coordinates.windowToSim(mouse);
            simulation.set(point.x, point.y, cellState);
        }
    }

    void Game::onViewportChanged() {
        coordinates = Coordinates{simSize, renderer.getOutputSize(), cellSize};
        gridTexture = createGridTexture(renderer, coordinates);
        renderTexture = createRenderTexture(renderer, coordinates);
    }


    void Game::render(const GameTime& /*gameTime*/) {
        const Simulation::TAliveList& aliveList = simulation.getAliveCells();
        std::vector<SDL_Point> pixels{};
        std::for_each(aliveList.cbegin(), aliveList.cend(), [=,&pixels](std::pair<int, int> xy) {
            const Point tex = coordinates.simToGrid({xy.first, xy.second});
            if (tex.x >= 0 && tex.x < coordinates.grid().w && tex.y >= 0 && tex.y < coordinates.grid().h) {
                pixels.push_back({tex.x, tex.y});
            }
        });

        renderer.setTarget(renderTexture.getRaw());
        renderer.setDrawColor(DEAD_COLOR);

        renderer.fillRect(nullptr);
        if (!pixels.empty()) {
            renderer.setDrawColor(ALIVE_COLOR);
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

        // First, gather all SDL events
        std::vector<SDL_Event> events;
        for (SDL_Event event; SDL_PollEvent(&event) != 0;) {
            if (SDL_QUIT == event.type) {
                return true;
            }
            events.push_back(event);
        }
        // Pass them to the GUI engine
        nuklearSdl.handleEvents(events);
        // Now the GUIs can be updated (and alter the state)
        gui.update(renderer.getOutputSize().w);
        // Filter mouse events when hovering the GUIb
        bool mouseOnGui = nk_window_is_any_hovered(&nuklearSdl.getContext()) != 0;
        const auto &iterator = std::remove_if(events.begin(), events.end(),
                [mouseOnGui](auto &e) {return isMouseEvent(e) && mouseOnGui;});
        events.erase(iterator, events.end());
        handleEvents(events);

        // UPDATE and RENDER
        const GameTime gameTime = clock.update();
        update(gameTime);
        render(gameTime);

        return false;
    }


} // namespace app

