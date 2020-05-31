#pragma once

#include <SDL2/SDL.h>
#include <memory>

namespace app {

    template<class T>
    using SDLDestructor = void (*)(T *);

    template<class T>
    using sdl_unique_ptr = std::unique_ptr<T, SDLDestructor<T>>;

    template<class T>
    sdl_unique_ptr<T> make_unique(T *p, SDLDestructor<T> fun) {
        if (p == nullptr) {
            throw std::runtime_error(SDL_GetError());
        }
        return std::unique_ptr<T, SDLDestructor<T>>{p, fun};
    }

    inline void sdl_check(int result) {
        if (result != 0) {
            throw std::runtime_error(SDL_GetError());
        }
    }

}