#include "game.h"
#include "version.h"

#include <spdlog/sinks/basic_file_sink.h>

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
        auto pWindow{app::make_unique(SDL_CreateWindow(
                "Sam's App",
                SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
                DEFAULT_WIDTH, DEFAULT_HEIGHT,
                SDL_WINDOW_SHOWN
        ), SDL_DestroyWindow)};

        app::Game game{pWindow.get()};

        game.mainLoop();
    } catch (const std::exception& ex) {
        spdlog::critical("Unhandled exception : {}", ex.what());
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Fatal error", ex.what(), nullptr);
    }
    SDL_Quit();
    return 0;
}

