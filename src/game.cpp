#include "game.h"
#include "colors.h"
#include "nuklear/nuklear.h"
#include "patterns.h"
#include <fmt/format.h>

// #include "nuklear/overview.c"

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
        simulation{simSize, defaultPattern()},
        nuklearSdl(window->getRaw(), renderer.getRaw(), "assets/Cousine-Regular.ttf", 16),
        pNuklearCtx(&nuklearSdl.getContext())
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
//            benchmark = false;
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



      //  overview(pNuklearCtx);

//        if (nk_begin(pNuklearCtx, "Demo", nk_rect(50, 50, 230, 250),
//                     NK_WINDOW_BORDER|NK_WINDOW_MOVABLE|NK_WINDOW_SCALABLE|
//                     NK_WINDOW_MINIMIZABLE|NK_WINDOW_TITLE))
//        {
//            enum {EASY, HARD};
//            static int op = EASY;
//            static int property = 20;
//
//            nk_layout_row_static(pNuklearCtx, 30, 80, 1);
//            if (nk_button_label(pNuklearCtx, "button"))
//                fprintf(stdout, "button pressed\n");
//            nk_layout_row_dynamic(pNuklearCtx, 30, 2);
//            if (nk_option_label(pNuklearCtx, "easy", op == EASY)) op = EASY;
//            if (nk_option_label(pNuklearCtx, "hard", op == HARD)) op = HARD;
//            nk_layout_row_dynamic(pNuklearCtx, 25, 1);
//            nk_property_int(pNuklearCtx, "Compression:", 0, &property, 100, 10, 1);
//
//            nk_layout_row_dynamic(pNuklearCtx, 20, 1);
//            nk_label(pNuklearCtx, "background:", NK_TEXT_LEFT);
//            nk_layout_row_dynamic(pNuklearCtx, 25, 1);
//        }
//        nk_end(pNuklearCtx);



        if (0 != nk_begin(pNuklearCtx, "Controls", nk_rect(50, 50, 230, 250),
                     NK_WINDOW_BORDER|NK_WINDOW_MOVABLE|NK_WINDOW_SCALABLE|
                     NK_WINDOW_MINIMIZABLE|NK_WINDOW_TITLE))
        {
            static int check = nk_true;
            nk_layout_row_static(pNuklearCtx, 30, 80, 1);
            nk_checkbox_label(pNuklearCtx, "check", &check);

        }
        nk_end(pNuklearCtx);





        nuklearSdl.render();

        renderer.present();
    }

    bool Game::mainLoop() {
        bool exit = false;

        std::vector<SDL_Event> events;
        nk_input_begin(pNuklearCtx);
        for (SDL_Event event; SDL_PollEvent(&event) != 0;) {
            if (SDL_QUIT == event.type) {
                exit = true;
            } else {
                events.push_back(event);
            }
            nuklearSdl.handleEvent(&event);
        }
        if (nk_window_is_any_hovered(pNuklearCtx) == 0) {
            handleEvents(events);
        }
        nk_input_end(pNuklearCtx);

        const GameTime gameTime = clock.update();
        for (int i = 0; i< speed; ++i) {
            update(gameTime);
        }
        render(gameTime);

        return exit;
    }


} // namespace app

