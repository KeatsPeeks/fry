#include "game.h"
#include "colors.h"
#include "nuklear/nuklear.h"
#include "patterns.h"
#include <algorithm>
#include <fmt/format.h>

//#include "nuklear/overview.c"

namespace app {

    namespace {
        bool isMouseEvent(const SDL_Event& e) {
            return e.type == SDL_MOUSEWHEEL || e.type == SDL_MOUSEBUTTONDOWN || e.type == SDL_MOUSEBUTTONUP || e.type == SDL_MOUSEMOTION;
        }

        sdl::Texture createGridTexture(const sdl::Renderer& renderer, int cellSize) {

            const Size viewport = renderer.getOutputSize();
            const Size textureSize{viewport.w - viewport.w % cellSize, viewport.h - viewport.h % cellSize};
            const Size gridSize = viewport / cellSize + 1;

            sdl::Texture texture{SDL_CreateTexture(renderer.getRaw(), SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_TARGET, textureSize.w, textureSize.h)};
            texture.setBlendMode(SDL_BLENDMODE_BLEND);
            renderer.setTarget(texture.getRaw());
            renderer.setDrawColor(TRANSPARENT_COLOR);
            renderer.clear();
            renderer.setDrawColor(GRID_COLOR);
            for (int y = 1; y <= gridSize.h; ++y) {
                renderer.drawLine({0, y * cellSize}, {gridSize.w * cellSize, y * cellSize});
            }
            for (int x = 1; x <= gridSize.w; ++x) {
                renderer.drawLine({x * cellSize, 0}, {x * cellSize, gridSize.h * cellSize});
            }

            renderer.setTarget(nullptr);
            return texture;
        }

        sdl::Texture createRenderTexture(const sdl::Renderer& renderer, int cellSize) {
            const Size viewport = renderer.getOutputSize();
            const Size textureSize = viewport / cellSize;
            return sdl::Texture{SDL_CreateTexture(renderer.getRaw(), SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, textureSize.w, textureSize.h)};
        }

        std::vector<uint32_t> createPixelsMatrix(const sdl::Renderer& renderer, int cellSize) {
            const Size viewport = renderer.getOutputSize();
            const Size textureSize = viewport / cellSize;
            return std::vector<uint32_t>(textureSize.w * textureSize.h);
        }
    } // anonymous namespace

    static const int simSize = 2048;
    static const int benchIters = 4000;
    static const auto defaultPattern = []() { return Patterns::acorn(); };

    static const uint32_t ALIVE_COLOR = 0xFF0A6EDC;
    static const uint32_t DEAD_COLOR = 0xFF808080;

    static int displayGrid = 1;
    static const int speed = 1;
    static const int cellSize = 1;

    static bool step = false;

    Game::Game(sdl::Window* window) :
        renderer{SDL_CreateRenderer(window->getRaw(), -1, SDL_RENDERER_ACCELERATED)},
        gridTexture{createGridTexture(renderer, cellSize)},
        renderTexture{createRenderTexture(renderer, cellSize)},
        pixels{createPixelsMatrix(renderer, cellSize)},
        simulation{simSize, defaultPattern()},
        nuklearSdl(window->getRaw(), renderer.getRaw(), "assets/Cousine-Regular.ttf", 16),
        gui(&nuklearSdl.getContext(), {&displayGrid}) {
    }

    void Game::handleEvents(std::span<SDL_Event> events) {
        for (auto event : events) {
            if (SDL_WINDOWEVENT == event.type && SDL_WINDOWEVENT_RESIZED == event.window.event) {
                onViewportChanged();
            } else if (SDL_MOUSEBUTTONDOWN == event.type || SDL_MOUSEMOTION == event.type) {
                if (event.button.button == SDL_BUTTON_LEFT || event.button.button == SDL_BUTTON_RIGHT) {
                    mouseEdit({event.button.x, event.button.y}, event.button.button == SDL_BUTTON_LEFT);
                }
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
    }

    void Game::update(const GameTime& /*gameTime*/) {
        for (int i = 0; i< speed; ++i) {
            if (benchmark) {
                clock.update();
                for (int j = 0; j < benchIters; j++) {
                    simulation.nextStep();
                }
                const GameTime gameTime = clock.update();
                const auto message = fmt::format("{} iterations per second", std::lround(benchIters / gameTime.elapsedTime.count()));
                throw std::runtime_error(message);
//            benchmark = false;
            }
            if (!paused || step) {
                simulation.nextStep();
                step = false;
            }
        }
    }

    void Game::mouseEdit(Point mouse, bool alive) {
        if (paused) {
            const Size textureSize = renderer.getOutputSize();
            const Size gridSize = textureSize / cellSize;

            const Vector offset{(simSize - gridSize.w) / 2, (simSize - gridSize.h) / 2};
            const Point point{mouse.x / cellSize + offset.x, mouse.y / cellSize + offset.y};
            simulation.set(point.x, point.y, alive);
        }
    }

    void Game::onViewportChanged() {
        gridTexture = createGridTexture(renderer, cellSize);
        renderTexture = createRenderTexture(renderer, cellSize);
        pixels = createPixelsMatrix(renderer, cellSize);
    }


    void Game::render(const GameTime& /*gameTime*/) {
        const Size viewport = renderer.getOutputSize();

        const Size gridSize = viewport / cellSize;
        const Vector offset{(simSize - gridSize.w) / 2, (simSize - gridSize.h) / 2};

        for (Point tex{}; tex.y < gridSize.h; ++tex.y) {
            for (tex.x = 0; tex.x < gridSize.w; ++tex.x) {
                const Point xy = tex + offset;
                const bool alive = xy.y >= 0 && xy.x >= 0 && xy.y < simSize && xy.x < simSize && simulation.get(xy.x, xy.y);
                pixels[tex.y * gridSize.w + tex.x] = alive ? ALIVE_COLOR : DEAD_COLOR;
            }
        }
        renderTexture.update(nullptr, &pixels[0], gridSize.w * SDL_BYTESPERPIXEL(SDL_PIXELFORMAT_ARGB8888));
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

