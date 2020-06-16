#include "game.h"
#include "version.h"

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

#include <iostream>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/spdlog.h>

#if defined(WIN32) || defined(_WIN32)

// Force dedicated GPU on laptops
extern "C" {
    __declspec(dllexport) unsigned long NvOptimusEnablement = 0x00000001;
    __declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1;
}

#endif // WIN32


namespace {

    constexpr int DEFAULT_WIDTH = 1024;
    constexpr int DEFAULT_HEIGHT = 768;

    void init_loggers() {
        auto file_logger = spdlog::basic_logger_mt("root", "lastexecution.log", true);
        spdlog::set_default_logger(file_logger);
        spdlog::flush_on(spdlog::level::critical);
    }

    std::unique_ptr<app::sdl::Window> createWindow() {
        return std::make_unique<app::sdl::Window>(SDL_CreateWindow(
                "Sam's App",
                SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
                DEFAULT_WIDTH, DEFAULT_HEIGHT,
                SDL_WINDOW_RESIZABLE
        ));
    }

    void handleUnhandled(app::sdl::Window* window, const std::exception &ex) {
        spdlog::critical("Unhandled exception : {}", ex.what());
        std::cerr << "Unhandled exception : " << ex.what() << std::endl;
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Fatal error", ex.what(), window == nullptr ? nullptr : window->getRaw());
    }

} // anonymous namespace


int main(int /*argc*/, char** /*argv*/) {
    init_loggers();
    spdlog::info("Startup");
    spdlog::info("Build {}", BUILD_VERSION);


    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        auto msg = fmt::format("Unable to initialize SDL: {}", SDL_GetError());
        spdlog::critical(msg);
        return 1;
    }
    std::atexit(&SDL_Quit);
    if (TTF_Init() != 0) {
        auto msg = fmt::format("Unable to initialize SDL_ttf: {}", SDL_GetError());
        spdlog::critical(msg);
        return 1;
    }
    std::atexit(&TTF_Quit);

    std::unique_ptr<app::sdl::Window> window;
    std::unique_ptr<app::Game> game;

#ifdef __EMSCRIPTEN__
    auto userData = std::make_pair(&window, &game);
    // NB: this lambda can't capture because it needs to convert to a function pointer
    emscripten_set_main_loop_arg([](void* userData) {
        auto refs = static_cast<std::pair<std::unique_ptr<app::sdl::Window>*, std::unique_ptr<app::Game>*>*>(userData);
        auto& windowArg = *refs->first;
        auto& gameArg = *refs->second;
        try {
            if (gameArg == nullptr) {
                windowArg = createWindow();
                gameArg = std::make_unique<app::Game>(windowArg.get());
            }
            auto stop = gameArg->mainLoop();
            if (stop) {
                emscripten_cancel_main_loop();
            }
        } catch (const std::exception& ex) {
            // exceptions can't escape the emscripten loop
            handleUnhandled(windowArg.get(), ex);
            emscripten_cancel_main_loop();
        }
    }, &userData, 0, 1);

#else
    try {
        window = createWindow();
        game = std::make_unique<app::Game>(window.get());

        spdlog::info("Entering main loop");
        while (!game->mainLoop()) {
            // no-op
        }
        spdlog::info("Exiting main main loop");
    } catch (const std::exception& ex) {
        handleUnhandled(window.get(), ex);
        return 1;
    }
#endif
    return 0;
}

