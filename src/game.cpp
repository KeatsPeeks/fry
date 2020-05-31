#include "game.h"
#include "colors.h"

namespace app {

    Game::Game(SDL_Window *pWindow) :
        renderer{SDL_CreateRenderer(pWindow, -1, SDL_RENDERER_ACCELERATED)},
        gridTexture(createNewGridTexture())
    {}

    void Game::handleEvents(const std::vector<SDL_Event>& events) {
        for (auto event : events) {
            if (SDL_WINDOWEVENT == event.type && SDL_WINDOWEVENT_RESIZED == event.window.event) {
                gridTexture = createNewGridTexture();
            }
        }
    }

    void Game::update(const GameTime& /*gameTime*/) {

    }

    sdl::Texture Game::createNewGridTexture() {

        SDL_Point textureSize = renderer.getOutputSize();
        const int cellSize = 16;
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

    void Game::render(const GameTime& /*gameTime*/) {
        renderer.setDrawColor(BG_COLOR);
        renderer.clear();
        renderer.copy(gridTexture.getRaw(), nullptr, nullptr);
        renderer.present();
    }

    void Game::mainLoop() {

        GameClock clock{};

        for (bool exit{}; !exit;) {
            std::vector<SDL_Event> events;
            for (SDL_Event event; SDL_PollEvent(&event) != 0;) {
                if (SDL_QUIT == event.type) {
                    exit = true;
                }
                events.push_back(event);
            }
            handleEvents(events);

            GameTime gameTime = clock.update();
            update(gameTime);
            render(gameTime);
        }

    }


} // namespace app

