#pragma once

#include <SDL2/SDL.h>
#include <memory>
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
        return std::unique_ptr<T, Destructor<T>>{p, fun};
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
        explicit Texture(SDL_Texture* pTexture) : SdlResource(make_unique(pTexture, SDL_DestroyTexture)) {}

        void setBlendMode(SDL_BlendMode blendMode) const {
            sdl::check(SDL_SetTextureBlendMode(getRaw(), blendMode));
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

        void drawLine(int x1, int y1, int x2, int y2) const {
            check(SDL_RenderDrawLine(getRaw(), x1, y1, x2, y2));
        }

        void copy(SDL_Texture* texture, const SDL_Rect * srcrect, const SDL_Rect * dstrect) const {
            SDL_RenderCopy(getRaw(), texture, srcrect, dstrect);
        }

        [[nodiscard]] SDL_Point getOutputSize() const {
            SDL_Point p;
            SDL_GetRendererOutputSize(getRaw(), &p.x, &p.y);
            return p;
        }

        void present() const {
            SDL_RenderPresent(getRaw());
        }

        void clear() const {
            check(SDL_RenderClear(getRaw()));
        }
    };

}