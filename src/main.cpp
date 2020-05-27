#include <iostream>

int mainImpl(int argc, char** argv);

int main(int argc, char** argv) {
    return mainImpl(argc, argv);
}

#include <SDL2/SDL.h>

#include "main.h"

static const int SCREEN_WIDTH = 640;
static const int SCREEN_HEIGHT = 480;
static const int DELAY = 1000;

int mainImpl(int /*argc*/, char** /*argv*/) {
    std::cout << "Build " << BUILD_VERSION << '\n';


    SDL_Window* window = nullptr;
    SDL_Surface* screenSurface = nullptr;
    if (SDL_Init(SDL_INIT_VIDEO ) < 0) {
        return 2;
    }
    window = SDL_CreateWindow(
            "hello_sdl2",
            SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, // NOLINT
            SCREEN_WIDTH, SCREEN_HEIGHT,
            SDL_WINDOW_SHOWN
    );
    if (window == nullptr) {
        return 1;
    }
    screenSurface = SDL_GetWindowSurface(window);
    SDL_FillRect(screenSurface, nullptr, SDL_MapRGB(screenSurface->format, 0, 0, 0));
    SDL_UpdateWindowSurface(window);
    static SDL_Event event;
    while (true) {
        SDL_PollEvent(&event);
        if ((SDL_QUIT == event.type) || (SDL_KEYDOWN == event.type && SDL_SCANCODE_ESCAPE == event.key.keysym.scancode)) {
            break;
        }
    }
    return 0;
}