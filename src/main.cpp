#include "game.h"
#include "version.h"

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/spdlog.h>
#include <iostream>

constexpr int DEFAULT_WIDTH{1024};
constexpr int DEFAULT_HEIGHT{768};

void init_loggers() {
    auto file_logger{spdlog::basic_logger_mt("root", "lastexecution.log", true)};
    spdlog::set_default_logger(file_logger);
    spdlog::flush_on(spdlog::level::critical);
}

int main(int /*argc*/, char** /*argv*/) {
    init_loggers();

    spdlog::info("Startup");
    spdlog::info("Build {}", BUILD_VERSION);

    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        spdlog::critical("Unable to initialize SDL: {}", SDL_GetError());
        return 1;
    }

    try {
        auto pWindow{app::sdl::make_unique(SDL_CreateWindow(
                "Sam's App",
                SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
                DEFAULT_WIDTH, DEFAULT_HEIGHT,
                SDL_WINDOW_RESIZABLE
        ), SDL_DestroyWindow)};

        app::Game game{pWindow.get()};

        spdlog::info("Entering main loop");

#ifdef __EMSCRIPTEN__
        // Note: A lambda can only be converted to a function pointer if it does not capture, so we
        // use the callback arguments to access the real loop
        emscripten_set_main_loop_arg([](void* arg) {
            try {
                auto pGame = static_cast<app::Game*>(arg);
                auto stop = pGame->mainLoop();
                if (stop) {
                    emscripten_cancel_main_loop();
                }
            } catch (const std::exception& ex) {
                spdlog::critical("Unhandled exception : {}", ex.what());
                std::cerr << ex.what();
                SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Fatal error", ex.what(), nullptr);
                emscripten_cancel_main_loop();
            }
        }, &game, 0, 1);
#else
        while (!game.mainLoop()) {
            // no-op
        }
#endif
        spdlog::info("Exiting main main loop");
    } catch (const std::exception& ex) {
        spdlog::critical("Unhandled exception : {}", ex.what());
        std::cerr << ex.what();
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Fatal error", ex.what(), nullptr);
    }
    SDL_Quit();
    spdlog::info("Graceful Exit");
    spdlog::default_logger()->flush();
    return 0;
}

