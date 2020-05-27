#include <iostream>
#include <memory>

int mainImpl(int argc, char** argv);

int main(int argc, char** argv) {
    return mainImpl(argc, argv);
}

#include <SDL2/SDL.h>

#include "main.h"

static const int SCREEN_WIDTH = 640;
static const int SCREEN_HEIGHT = 480;


int mainImpl(int /*argc*/, char** /*argv*/) {
    std::cout << "Build " << BUILD_VERSION << '\n';

    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        return 2;
    }

    std::unique_ptr<SDL_Window, decltype(&SDL_DestroyWindow)> window{SDL_CreateWindow(
            "hello_sdl2",
            SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
            SCREEN_WIDTH, SCREEN_HEIGHT,
            SDL_WINDOW_SHOWN
    ), SDL_DestroyWindow};

    if (window == nullptr) {
        return 1;
    }
    SDL_Surface* screenSurface = SDL_GetWindowSurface(window.get());
    SDL_FillRect(screenSurface, nullptr, SDL_MapRGB(screenSurface->format, 0, 0, 0));
    SDL_UpdateWindowSurface(window.get());
    static SDL_Event event;
    while (true) {
        SDL_PollEvent(&event);
        if ((SDL_QUIT == event.type) || (SDL_KEYDOWN == event.type && SDL_SCANCODE_ESCAPE == event.key.keysym.scancode)) {
            break;
        }
    }
    SDL_Quit();

    return 0;
}