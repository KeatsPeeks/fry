#include "game.h"

namespace app {

    Game::Game(SDL_Window *pWindow) :
        pRenderer{make_unique(SDL_CreateRenderer(pWindow, -1, SDL_RENDERER_ACCELERATED), SDL_DestroyRenderer)} {}

    void Game::handleEvents(const std::vector<SDL_Event>& /*events*/) {

    }

    void Game::update(const GameTime& /*gameTime*/) {

    }

    void Game::render(const GameTime& /*gameTime*/) {

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

