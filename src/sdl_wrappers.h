#pragma once

#include "primitives.h"
#include <SDL2/SDL.h>
#include <memory>
#include <span>
#include <stdexcept>

namespace app::sdl {

    template<class T>
    using Destructor = void (*)(T *);

    template<class T>
    using unique_ptr = std::unique_ptr<T, Destructor<T>>;

    template<class T>
    [[nodiscard]] unique_ptr<T> make_unique(T *p, Destructor<T> fun) {
        if (p == nullptr) {
            throw std::runtime_error(SDL_GetError());
        }
        return {p, fun};
    }

    inline void check(int result) {
        if (result != 0) {
            throw std::runtime_error(SDL_GetError());
        }
    }

    template <class T>
    class SdlResource {
    public:
        [[nodiscard]] T* getRaw() const {
            return wrapper.get();
        }

        SdlResource() : wrapper{nullptr, nullptr} {}
        SdlResource(const SdlResource& right) = delete;
        SdlResource<T>& operator=(const SdlResource& right) = delete;

    protected:
        explicit SdlResource(unique_ptr<T> wrapper) : wrapper(std::move(wrapper)) {}

        SdlResource(SdlResource&& right) noexcept = default;
        SdlResource<T>& operator=(SdlResource&& right) noexcept = default;
        ~SdlResource() = default;

    private:
        unique_ptr<T> wrapper;
    };

    class Texture : public SdlResource<SDL_Texture> {
    public:
        explicit Texture() = default;
        explicit Texture(SDL_Texture* pTexture) : SdlResource(make_unique(pTexture, SDL_DestroyTexture)) {}

        void setBlendMode(SDL_BlendMode blendMode) const {
            check(SDL_SetTextureBlendMode(getRaw(), blendMode));
        }

        void update(const SDL_Rect* rect, const void* pixels, int pitch) {
            check(SDL_UpdateTexture(getRaw(), rect, pixels, pitch));
        }

    };

    class Surface : public SdlResource<SDL_Surface> {
    public:
        explicit Surface(SDL_Surface* pSurface) : SdlResource(make_unique(pSurface, SDL_FreeSurface)) {}
    };

    class Cursor : public SdlResource<SDL_Cursor> {
    public:
        explicit Cursor(SDL_Cursor* pCursor) : SdlResource(make_unique(pCursor, SDL_FreeCursor)) {}

        void set() { SDL_SetCursor(getRaw()); }
    };

    class Window : public SdlResource<SDL_Window> {
    public:
        explicit Window(SDL_Window* pWindow) : SdlResource(make_unique(pWindow, SDL_DestroyWindow)) {}

        [[nodiscard]] Size getSize() const {
            Size p;
            SDL_GetWindowSize(getRaw(), &p.w, &p.h);
            return p;
        }

        void setSize(int w, int h) {
            SDL_SetWindowSize(getRaw(), w, h);
        }

        void showSimpleMessageBox(Uint32 flags, const char* title, const char* message) {
            check(SDL_ShowSimpleMessageBox(flags, title, message, getRaw()));
        }
    };

    class Renderer : public SdlResource<SDL_Renderer> {
    public:
        explicit Renderer(SDL_Renderer* pRenderer) : SdlResource(make_unique(pRenderer, SDL_DestroyRenderer)) {}

        void setTarget(SDL_Texture* texture) const {
            check(SDL_SetRenderTarget(getRaw(), texture));
        }

        void setDrawColor(const SDL_Color& color) const {
            check(SDL_SetRenderDrawColor(getRaw(), color.r, color.g, color.b, color.a));
        }

        void drawLine(Point p1, Point p2) const {
            check(SDL_RenderDrawLine(getRaw(), p1.x, p1.y, p2.x, p2.y));
        }

        void copy(SDL_Texture* texture, const SDL_Rect* srcrect, const SDL_Rect* dstrect) const {
            check(SDL_RenderCopy(getRaw(), texture, srcrect, dstrect));
        }

        void fillRect(const SDL_Rect * rect) const {
            check(SDL_RenderFillRect(getRaw(), rect));
        }

        void drawPoints(SDL_Point point) const {
            check(SDL_RenderDrawPoint(getRaw(), point.x, point.y));
        }
        void drawPoints(std::span<SDL_Point> points) const {
#ifdef __EMSCRIPTEN__
            // Weird bugs with SDL_RenderDrawPoints on the browser
            for (auto p : points) {
                check(SDL_RenderDrawPoint(getRaw(), p.x, p.y));
            }
#else
            check(SDL_RenderDrawPoints(getRaw(), &points[0], static_cast<int>(points.size())));
#endif
        }

        [[nodiscard]] Size getOutputSize() const {
            Size p;
            check(SDL_GetRendererOutputSize(getRaw(), &p.w, &p.h));
            return p;
        }

        void present() const {
            SDL_RenderPresent(getRaw());
        }

        void clear() const {
            check(SDL_RenderClear(getRaw()));
        }

    };

} // namespace app::sdl