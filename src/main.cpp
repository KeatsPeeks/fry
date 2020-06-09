#include "game.h"
#include "version.h"

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

#include <iostream>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/spdlog.h>

namespace {

    constexpr int DEFAULT_WIDTH{1024};
    constexpr int DEFAULT_HEIGHT{768};

    void init_loggers() {
        auto file_logger{spdlog::basic_logger_mt("root", "lastexecution.log", true)};
        spdlog::set_default_logger(file_logger);
        spdlog::flush_on(spdlog::level::critical);
    }

    class GlobalInit {
    public:
        GlobalInit() {
            init_loggers();

            spdlog::info("Startup");
            spdlog::info("Build {}", BUILD_VERSION);

            if (SDL_Init(SDL_INIT_VIDEO) != 0) {
                auto msg = fmt::format("Unable to initialize SDL: {}", SDL_GetError());
                spdlog::critical(msg);
                throw std::runtime_error(msg);
            }
            if (TTF_Init() != 0) {
                auto msg = fmt::format("Unable to initialize SDL_ttf: {}", SDL_GetError());
                spdlog::critical(msg);
                throw std::runtime_error(msg);
            }
        }

        ~GlobalInit() {
            // it's ok to call XXX_Quit even if XXX_Init has not been called
            TTF_Quit();
            SDL_Quit();
        }

        GlobalInit(const GlobalInit&) = delete;
        GlobalInit& operator=(const GlobalInit&) = delete;
        GlobalInit(GlobalInit&&) = delete;
        GlobalInit& operator=(GlobalInit&&) = delete;
    };

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
    GlobalInit init;

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

}

