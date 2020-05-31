#include "game.h"
#include "colors.h"
#include <fmt/format.h>

namespace app {

    static const int cellSize = 8;
    static const int simSize = 2048;
    static const int benchIters = 2048;
    static const std::vector<std::vector<uint8_t>> pattern = {
            {0, 1, 0, 0, 0, 0, 0},
            {0, 0, 0, 1, 0, 0, 0},
            {1, 1, 0, 0, 1, 1, 1}
    };

    sdl::Texture static createNewRenderTexture(const sdl::Renderer& renderer) {
        SDL_Point textureSize = renderer.getOutputSize();

        SDL_Point gridSize{textureSize.x / cellSize, textureSize.y / cellSize};
        sdl::Texture texture{SDL_CreateTexture(renderer.getRaw(), SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, gridSize.x, gridSize.y)};
        return texture;
    }

    sdl::Texture static createNewGridTexture(const sdl::Renderer& renderer) {

        SDL_Point textureSize = renderer.getOutputSize();
        SDL_Point gridSize{textureSize.x / cellSize + 1, textureSize.y / cellSize + 1};

        sdl::Texture texture{SDL_CreateTexture(renderer.getRaw(), SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_TARGET, textureSize.x, textureSize.y)};
        texture.setBlendMode(SDL_BLENDMODE_BLEND);
        renderer.setTarget(texture.getRaw());
        renderer.setDrawColor(TRANSPARENT_COLOR);
        renderer.clear();
        renderer.setDrawColor(GRID_COLOR);
        std::vector<SDL_Point> points;
        for (int y = 0; y < gridSize.y; ++y) {
            renderer.drawLine(0, (y + 1) * cellSize, gridSize.x * cellSize, (y + 1) * cellSize);
        }
        for (int x = 0; x < gridSize.x; ++x) {
            renderer.drawLine((x + 1) * cellSize, 0, (x + 1) * cellSize, gridSize.y * cellSize);
        }

        renderer.setTarget(nullptr);
        return texture;
    }

    Game::Game(SDL_Window *pWindow) :
        renderer{SDL_CreateRenderer(pWindow, -1, SDL_RENDERER_ACCELERATED)},
        gridTexture{createNewGridTexture(renderer)},
        renderTexture{createNewRenderTexture(renderer)},
        simulation{simSize, pattern}
    {
        SDL_Point textureSize = renderer.getOutputSize();

        SDL_Point gridSize{textureSize.x / cellSize, textureSize.y / cellSize};
        pixels = std::vector<uint32_t>(static_cast<unsigned int>(gridSize.x * gridSize.y));
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
            auto message = fmt::format("{} iterations in {} ms", benchIters, gameTime.elapsedTime.count() * 1000);
            throw std::runtime_error(message);
            benchmark = false;
        }
        if (!paused) {
            simulation.nextStep();
        }
    }

    void Game::onLeftMouse(int x, int y) {
        if (paused) {
            SDL_Point textureSize = renderer.getOutputSize();
            SDL_Point gridSize{textureSize.x / cellSize, textureSize.y / cellSize};

            const int size = static_cast<int>(simulation.getSize());
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
        SDL_Point textureSize = renderer.getOutputSize();

        SDL_Point gridSize{textureSize.x / cellSize, textureSize.y / cellSize};
        pixels = std::vector<uint32_t>(static_cast<unsigned int>(gridSize.x * gridSize.y));
    }

    void Game::render(const GameTime& /*gameTime*/) {
        const int size = static_cast<int>(simulation.getSize());
        SDL_Point textureSize = renderer.getOutputSize();

        SDL_Point gridSize{textureSize.x / cellSize, textureSize.y / cellSize};
        const int yOffset = (size - gridSize.y) / 2;
        const int xOffset = (size - gridSize.x) / 2;

        for (int yTex = 0; yTex < gridSize.y; ++yTex) {
            for (int xTex = 0; xTex < gridSize.x; ++xTex) {
                const int y = yTex + yOffset;
                const int x = xTex + xOffset;
                const size_t pixelIndex = static_cast<size_t>(yTex * gridSize.x + xTex);
                const bool alive = y >= 0 && x >= 0 && y < size && x < size && simulation.get(x, y);
                pixels[pixelIndex] = alive ? 0xAFFF33FF : 0xFF00CC00;
            }
        }
        sdl::check(SDL_UpdateTexture(renderTexture.getRaw(), nullptr, &pixels[0], gridSize.x * SDL_BYTESPERPIXEL(SDL_PIXELFORMAT_ARGB8888)));
        renderer.copy(renderTexture.getRaw(), nullptr, nullptr);

        renderer.copy(gridTexture.getRaw(), nullptr, nullptr);
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

        GameTime gameTime = clock.update();
        update(gameTime);
        render(gameTime);

        return exit;
    }


} // namespace app

