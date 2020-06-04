#include "game.h"
#include "colors.h"
#include "patterns.h"
#include <fmt/format.h>

namespace app {

    static const int cellSize = 12;
    static const int simSize = 2048;
    static const int benchIters = 4000;
    static const bool displayGrid = true;
    static const int speed = 1;
    static const auto defaultPattern = []() { return Patterns::acorn(); };

    static const uint32_t ALIVE_COLOR = 0xFF0033FF;
    static const uint32_t DEAD_COLOR = 0xFF00A000;

    sdl::Texture static createNewRenderTexture(const sdl::Renderer& renderer) {
        const SDL_Point textureSize = renderer.getOutputSize();

        const SDL_Point gridSize{textureSize.x / cellSize, textureSize.y / cellSize};
        return sdl::Texture{SDL_CreateTexture(renderer.getRaw(), SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, gridSize.x, gridSize.y)};
    }

    sdl::Texture static createNewGridTexture(const sdl::Renderer& renderer) {

        SDL_Point textureSize = renderer.getOutputSize();
        textureSize.x = textureSize.x - textureSize.x % cellSize;
        textureSize.y = textureSize.y - textureSize.y % cellSize;
        const SDL_Point gridSize{textureSize.x / cellSize + 1, textureSize.y / cellSize + 1};

        sdl::Texture texture{SDL_CreateTexture(renderer.getRaw(), SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_TARGET, textureSize.x, textureSize.y)};
        texture.setBlendMode(SDL_BLENDMODE_BLEND);
        renderer.setTarget(texture.getRaw());
        renderer.setDrawColor(TRANSPARENT_COLOR);
        renderer.clear();
        renderer.setDrawColor(GRID_COLOR);
        std::vector<SDL_Point> points;
        for (int y = 1; y <= gridSize.y; ++y) {
            renderer.drawLine(0, y * cellSize, gridSize.x * cellSize, y * cellSize);
        }
        for (int x = 1; x <= gridSize.x; ++x) {
            renderer.drawLine(x * cellSize, 0, x * cellSize, gridSize.y * cellSize);
        }

        renderer.setTarget(nullptr);
        return texture;
    }

    Game::Game(sdl::Window* window) :
        renderer{SDL_CreateRenderer(window->getRaw(), -1, SDL_RENDERER_ACCELERATED)},
        gridTexture{createNewGridTexture(renderer)},
        renderTexture{createNewRenderTexture(renderer)},
        simulation{simSize, defaultPattern()}
    {
        const SDL_Point textureSize = renderer.getOutputSize();

        const SDL_Point gridSize{textureSize.x / cellSize, textureSize.y / cellSize};
        pixels = std::vector<uint32_t>(gridSize.x * gridSize.y);
    }

    void Game::handleEvents(const std::vector<SDL_Event>& events) {
        for (auto event : events) {
            if (SDL_WINDOWEVENT == event.type && SDL_WINDOWEVENT_RESIZED == event.window.event) {
                onViewportChanged();
            }
            if (SDL_KEYDOWN == event.type && SDL_SCANCODE_RETURN == event.key.keysym.scancode) {
                paused = false;
            }
            if (SDL_KEYDOWN == event.type && SDL_SCANCODE_B == event.key.keysym.scancode) {
                benchmark = true;
            }
            if (SDL_MOUSEBUTTONDOWN == event.type || SDL_MOUSEMOTION == event.type) {
                if (event.button.button == SDL_BUTTON_LEFT) {
                    onLeftMouse(event.button.x, event.button.y);
                }
            }
        }
    }

    void Game::update(const GameTime& /*gameTime*/) {
        if (benchmark) {
            clock.update();
            for (int i = 0; i < benchIters; i++) {
                simulation.nextStep();
            }
            const GameTime gameTime = clock.update();
            const auto message = fmt::format("{} iterations per second", std::lround(benchIters / gameTime.elapsedTime.count()));
            throw std::runtime_error(message);
            benchmark = false;
        }
        if (!paused) {
            simulation.nextStep();
        }
    }

    void Game::onLeftMouse(int x, int y) {
        if (paused) {
            const SDL_Point textureSize = renderer.getOutputSize();
            const SDL_Point gridSize{textureSize.x / cellSize, textureSize.y / cellSize};

            const int size = simulation.getSize();
            const int yOffset = (size - gridSize.y) / 2;
            const int xOffset = (size - gridSize.x) / 2;
            const int yMat = y / cellSize + yOffset;
            const int xMat = x / cellSize + xOffset;
            simulation.set(xMat, yMat, true);
        }
    }

    void Game::onViewportChanged() {
        gridTexture = createNewGridTexture(renderer);
        renderTexture = createNewRenderTexture(renderer);
        const SDL_Point textureSize = renderer.getOutputSize();

        const SDL_Point gridSize{textureSize.x / cellSize, textureSize.y / cellSize};
        pixels = std::vector<uint32_t>(gridSize.x * gridSize.y);
    }


    void Game::render(const GameTime& /*gameTime*/) {
        const int size = simulation.getSize();
        const SDL_Point textureSize = renderer.getOutputSize();

        const SDL_Point gridSize{textureSize.x / cellSize, textureSize.y / cellSize};
        const int yOffset = (size - gridSize.y) / 2;
        const int xOffset = (size - gridSize.x) / 2;

        for (int yTex = 0; yTex < gridSize.y; ++yTex) {
            for (int xTex = 0; xTex < gridSize.x; ++xTex) {
                const int y = yTex + yOffset;
                const int x = xTex + xOffset;
                const int pixelIndex = yTex * gridSize.x + xTex;
                const bool alive = y >= 0 && x >= 0 && y < size && x < size && simulation.get(x, y);
                pixels[pixelIndex] = alive ? ALIVE_COLOR : DEAD_COLOR;
            }
        }
        sdl::check(SDL_UpdateTexture(renderTexture.getRaw(), nullptr, &pixels[0], gridSize.x * SDL_BYTESPERPIXEL(SDL_PIXELFORMAT_ARGB8888)));
        renderer.copy(renderTexture.getRaw(), nullptr, nullptr);

        if (displayGrid) {
            renderer.copy(gridTexture.getRaw(), nullptr, nullptr);
        }
        renderer.present();
    }

    bool Game::mainLoop() {
        bool exit = false;

        std::vector<SDL_Event> events;
        for (SDL_Event event; SDL_PollEvent(&event) != 0;) {
            if (SDL_QUIT == event.type) {
                exit = true;
            } else {
                events.push_back(event);
            }
        }
        handleEvents(events);

        const GameTime gameTime = clock.update();
        for (int i = 0; i< speed; ++i) {
            update(gameTime);
        }
        render(gameTime);

        return exit;
    }


} // namespace app

