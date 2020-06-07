#include "nuklear_sdl.h"
#include <algorithm>
#include <cmath>
#include <cstring>
#include <span>
#include <string>
#include <vector>

namespace app {

    namespace {

        void sdl_draw_text(SDL_Renderer *renderer, TTF_Font *font, const char *str, int x, int y, struct nk_color c) {
            sdl::Surface surface{TTF_RenderText_Blended(font, str, SDL_Color{c.r, c.g, c.b, c.a})};
            sdl::Texture texture{SDL_CreateTextureFromSurface(renderer, surface.getRaw())};
            int texW = 0;
            int texH = 0;
            SDL_QueryTexture(texture.getRaw(), nullptr, nullptr, &texW, &texH);
            SDL_Rect dstrect = {x, y, texW, texH };
            SDL_RenderCopy(renderer, texture.getRaw(), nullptr, &dstrect);
        }

        void sdl_draw_rect(SDL_Renderer *renderer, int16_t x, int16_t y, int16_t w, int16_t h, int16_t rounding, struct nk_color color) {
            roundedRectangleRGBA(renderer, x, y, (x + w), (y + h),
                                 rounding, color.r, color.g, color.b, color.a);
        }

        void sdl_draw_rect_fill(SDL_Renderer *renderer, int16_t x, int16_t y, int16_t w, int16_t h, int16_t rounding, struct nk_color color) {
            roundedBoxRGBA(renderer, x, y, (x + w), (y + h), rounding, color.r, color.g, color.b, color.a);
        }

        void sdl_draw_line(SDL_Renderer *renderer, int16_t x, int16_t y, int16_t x2, int16_t y2, struct nk_color color) {
            lineRGBA(renderer, x, y, x2, y2, color.r, color.g, color.b, color.a);
        }

        void sdl_draw_ellipse(SDL_Renderer *renderer, int16_t x, int16_t y, int16_t rx, int16_t ry, struct nk_color color) {
            ellipseRGBA(renderer, x, y, rx, ry, color.r, color.g, color.b, color.a);
        }

        void sdl_draw_ellipse_filled(SDL_Renderer *renderer, int16_t x, int16_t y, int16_t rx, int16_t ry, struct nk_color color) {
            filledEllipseRGBA(renderer, x, y, rx, ry, color.r, color.g, color.b, color.a);
        }

        [[maybe_unused]] void sdl_draw_circle_filled(SDL_Renderer *renderer, int16_t x, int16_t y, int16_t rad, struct nk_color color) {
            filledCircleRGBA(renderer, x, y, rad, color.r, color.g, color.b, color.a);
        }

        void sdl_draw_triangle(SDL_Renderer *renderer, int16_t x1,int16_t y1, int16_t x2, int16_t y2, int16_t x3, int16_t y3, struct nk_color color) {
            trigonRGBA(renderer, x1,y1, x2,y2, x3,y3, color.r, color.g, color.b, color.a);
        }

        void sdl_draw_filled_triangle(SDL_Renderer *renderer, int16_t x1,int16_t y1, int16_t x2, int16_t y2, int16_t x3, int16_t y3, struct nk_color color) {
            filledTrigonRGBA(renderer, x1,y1, x2,y2, x3,y3, color.r, color.g, color.b, color.a);
        }

        void sdl_draw_polyline(SDL_Renderer *renderer, const Sint16 *vx, const Sint16 *vy, int n, struct nk_color color) {
            polygonRGBA(renderer, vx, vy, n, color.r, color.g, color.b, color.a);
        }

        void sdl_draw_arc(SDL_Renderer *renderer, int16_t x, int16_t y, int16_t rad, int16_t start, int16_t end, struct nk_color color) {
            arcRGBA(renderer, x,y, rad, start,end, color.r, color.g, color.b, color.a);
        }

        void sdl_draw_filled_polygon(SDL_Renderer *renderer, const Sint16 *vx, const Sint16 *vy, int n, struct nk_color color) {
            filledPolygonRGBA(renderer, vx, vy, n, color.r, color.g, color.b, color.a);
        }

        void sdl_draw_image(SDL_Renderer *renderer, const struct nk_image *img, int x, int y, int w, int h) {
            auto *texture = static_cast<SDL_Texture *>(img->handle.ptr);
            SDL_Rect rect{x, y, w, h};
            SDL_RenderCopy(renderer, texture, nullptr, &rect);
        }

        nk_color interpolate_color(nk_color c1, nk_color c2, float fraction)
        {
            float r = std::lerp(c1.r, c2.r, fraction);
            float g = std::lerp(c1.g, c2.g, fraction);
            float b = std::lerp(c1.b, c2.b, fraction);
            float a = std::lerp(c1.a, c2.a, fraction);

            return {
                    static_cast<nk_byte>(std::clamp(r, 0.F, 255.F)),
                    static_cast<nk_byte>(std::clamp(0.F, g, 255.F)),
                    static_cast<nk_byte>(std::clamp(0.F, b, 255.F)),
                    static_cast<nk_byte>(std::clamp(0.F, a, 255.F))
            };
        }

        void nk_sdl_fill_rect_multi_color(SDL_Renderer *renderer, int16_t x, int16_t y, uint16_t w, uint16_t h, struct nk_color left, struct nk_color top,  struct nk_color right, struct nk_color bottom) {
            float fw = w;
            float fh = h;
            for (int j = 0; j < h; j++) {
                float fraction_y = (static_cast<float>(j)) / fh;
                for (int i = 0; i < w; i++) {
                    float fraction_x = (static_cast<float>(i)) / fw;
                    nk_color X1 = interpolate_color(left, top, fraction_x);
                    nk_color X2 = interpolate_color(right, bottom, fraction_x);
                    nk_color Y = interpolate_color(X1, X2, fraction_y);

                    pixelRGBA(renderer, static_cast<Sint16>(x + i), static_cast<Sint16>(y + j), Y.r, Y.g, Y.b, Y.a);
                }
            }
        }

        void nk_sdl_stroke_curve(SDL_Renderer *renderer, struct nk_vec2i p1, struct nk_vec2i p2, struct nk_vec2i p3, struct nk_vec2i p4, unsigned int num_segments, uint16_t /*line_thickness*/, struct nk_color col) {
            struct nk_vec2i last = p1;
            struct nk_vec2 p1f{static_cast<float>(p1.x), static_cast<float>(p1.y)};
            struct nk_vec2 p2f{static_cast<float>(p2.x), static_cast<float>(p2.y)};
            struct nk_vec2 p3f{static_cast<float>(p3.x), static_cast<float>(p3.y)};
            struct nk_vec2 p4f{static_cast<float>(p4.x), static_cast<float>(p4.y)};

            num_segments = std::max(num_segments, 1U);
            for (unsigned int i_step = 1; i_step <= num_segments; ++i_step) {
                float t = static_cast<float>(i_step)/static_cast<float>(num_segments);
                float u = 1.0F - t;
                float w1 = u * u * u;
                float w2 = 3 * u * u * t;
                float w3 = 3 * u * t * t;
                float w4 = t * t * t;
                auto x = static_cast<Sint16>(lround(w1 * p1f.x + w2 * p2f.x + w3 * p3f.x + w4 * p4f.x));
                auto y = static_cast<Sint16>(lround(w1 * p1f.y + w2 * p2f.y + w3 * p3f.y + w4 * p4f.y));
                lineRGBA(renderer, last.x, last.y, x, y, col.r, col.g, col.b, col.a);
                last.x = x; last.y = y;
            }
        }


        void nk_sdl_clipboard_paste(nk_handle /*usr*/, struct nk_text_edit *edit)
        {
            const char *text = SDL_GetClipboardText();
            if (text != nullptr) {
                nk_textedit_paste(edit, text, nk_strlen(text));
            }
        }

        void nk_sdl_clipboard_copy(nk_handle /*usr*/, const char *text, int len)
        {
            if (text == nullptr) {
                return;
            }
            std::string tmp(text, len);
            SDL_SetClipboardText(tmp.c_str());
        }

        float nk_sdl_font_get_text_width(nk_handle handle, float /*height*/, const char *text, int len)
        {
            auto *font = static_cast<TTF_Font*>(handle.ptr);
            if (font == nullptr || text == nullptr) {
                return 0;
            }
            /* We must copy into a new buffer with exact length null-terminated
               as nuklear uses variable size buffers and TTF_SizeText doesn't
               accept a length, it infers length from null-termination */
            std::string tmp(text, len);

            int w{};
            int h{};
            TTF_SizeText(font, tmp.c_str(), &w, &h);
            return static_cast<float>(w);
        }

    } // anonymous namespace

    NuklearSdl::NuklearSdl(SDL_Window *win, SDL_Renderer *renderer, std::string_view font, int fontSize) :
            win(win),
            renderer(renderer),
            ttf_font{sdl::make_unique(TTF_OpenFont(font.data(), fontSize), TTF_CloseFont)},
            nk_font{
                    nk_handle_ptr(ttf_font.get()),
                    static_cast<float>(TTF_FontHeight(ttf_font.get())),
                    nk_sdl_font_get_text_width
            } {
        nk_init_default(&context, &nk_font);

        context.clip.copy = nk_sdl_clipboard_copy;
        context.clip.paste = nk_sdl_clipboard_paste;
        context.clip.userdata = nk_handle_ptr(nullptr);
    }

    void NuklearSdl::handleEvent(SDL_Event *evt) {
        auto& ctx = this->context;
        /* optional grabbing behavior */
        if (ctx.input.mouse.grab != 0) {
            SDL_SetRelativeMouseMode(SDL_TRUE);
            ctx.input.mouse.grab = 0;
        } else if (ctx.input.mouse.ungrab != 0) {
            int x = static_cast<int>(lround(ctx.input.mouse.prev.x));
            int y = static_cast<int>(lround(ctx.input.mouse.prev.y));
            SDL_SetRelativeMouseMode(SDL_FALSE);
            SDL_WarpMouseInWindow(win, x, y);
            ctx.input.mouse.ungrab = 0;
        }
        if (evt->type == SDL_KEYUP || evt->type == SDL_KEYDOWN) {
            /* key events */
            int down = evt->type == SDL_KEYDOWN ? 1 : 0;
            int numkeys{};
            const Uint8* state = SDL_GetKeyboardState(&numkeys);
            std::span keyStates(state, numkeys);
            bool ctrl = keyStates[SDL_SCANCODE_LCTRL] == 1 || keyStates[SDL_SCANCODE_RCTRL] == 1;
            bool ctrlDown = ctrl && down == 1;
            SDL_Keycode sym = evt->key.keysym.sym;
            if (sym == SDLK_RSHIFT || sym == SDLK_LSHIFT) {
                nk_input_key(&ctx, NK_KEY_SHIFT, down);
            } else if (sym == SDLK_DELETE) {
                nk_input_key(&ctx, NK_KEY_DEL, down);
            } else if (sym == SDLK_RETURN) {
                nk_input_key(&ctx, NK_KEY_ENTER, down);
            } else if (sym == SDLK_TAB) {
                nk_input_key(&ctx, NK_KEY_TAB, down);
            } else if (sym == SDLK_BACKSPACE) {
                nk_input_key(&ctx, NK_KEY_BACKSPACE, down);
            } else if (sym == SDLK_HOME) {
                nk_input_key(&ctx, NK_KEY_TEXT_START, down);
                nk_input_key(&ctx, NK_KEY_SCROLL_START, down);
            } else if (sym == SDLK_END) {
                nk_input_key(&ctx, NK_KEY_TEXT_END, down);
                nk_input_key(&ctx, NK_KEY_SCROLL_END, down);
            } else if (sym == SDLK_PAGEDOWN) {
                nk_input_key(&ctx, NK_KEY_SCROLL_DOWN, down);
            } else if (sym == SDLK_PAGEUP) {
                nk_input_key(&ctx, NK_KEY_SCROLL_UP, down);
            } else if (sym == SDLK_z) {
                nk_input_key(&ctx, NK_KEY_TEXT_UNDO, ctrlDown ? 1 : 0);
            } else if (sym == SDLK_r) {
                nk_input_key(&ctx, NK_KEY_TEXT_REDO, ctrlDown ? 1 : 0);
            } else if (sym == SDLK_c) {
                nk_input_key(&ctx, NK_KEY_COPY, ctrlDown ? 1 : 0);
            } else if (sym == SDLK_v) {
                nk_input_key(&ctx, NK_KEY_PASTE, ctrlDown ? 1 : 0);
            } else if (sym == SDLK_x) {
                nk_input_key(&ctx, NK_KEY_CUT, ctrlDown ? 1 : 0);
            } else if (sym == SDLK_b) {
                nk_input_key(&ctx, NK_KEY_TEXT_LINE_START, ctrlDown ? 1 : 0);
            } else if (sym == SDLK_e) {
                nk_input_key(&ctx, NK_KEY_TEXT_LINE_END, ctrlDown ? 1 : 0);
            } else if (sym == SDLK_UP) {
                nk_input_key(&ctx, NK_KEY_UP, down);
            } else if (sym == SDLK_DOWN) {
                nk_input_key(&ctx, NK_KEY_DOWN, down);
            } else if (sym == SDLK_LEFT) {
                if (ctrlDown) {
                    nk_input_key(&ctx, NK_KEY_TEXT_WORD_LEFT, down);
                } else {
                    nk_input_key(&ctx, NK_KEY_LEFT, down);
                }
            } else if (sym == SDLK_RIGHT) {
                if (ctrlDown) {
                    nk_input_key(&ctx, NK_KEY_TEXT_WORD_RIGHT, down);
                } else {
                    nk_input_key(&ctx, NK_KEY_RIGHT, down);
                }
            }
        } else if (evt->type == SDL_MOUSEBUTTONDOWN || evt->type == SDL_MOUSEBUTTONUP) {
            /* mouse button */
            int down = evt->type == SDL_MOUSEBUTTONDOWN ? 1 : 0;
            const int x = evt->button.x;
            const int y = evt->button.y;
            if (evt->button.button == SDL_BUTTON_LEFT) {
                if (evt->button.clicks > 1) {
                    nk_input_button(&ctx, NK_BUTTON_DOUBLE, x, y, down);
                }
                nk_input_button(&ctx, NK_BUTTON_LEFT, x, y, down);
            } else if (evt->button.button == SDL_BUTTON_MIDDLE) {
                nk_input_button(&ctx, NK_BUTTON_MIDDLE, x, y, down);
            } else if (evt->button.button == SDL_BUTTON_RIGHT) {
                nk_input_button(&ctx, NK_BUTTON_RIGHT, x, y, down);
            }
        } else if (evt->type == SDL_MOUSEMOTION) {
            /* mouse motion */
            if (ctx.input.mouse.grabbed != 0) {
                int x = static_cast<int>(ctx.input.mouse.prev.x);
                int y = static_cast<int>(ctx.input.mouse.prev.y);
                nk_input_motion(&ctx, x + evt->motion.xrel, y + evt->motion.yrel);
            } else {
                nk_input_motion(&ctx, evt->motion.x, evt->motion.y);
            }
        } else if (evt->type == SDL_TEXTINPUT) {
            /* text input */
            nk_glyph glyph;
            memcpy(&glyph[0], &evt->text.text[0], NK_UTF_SIZE);
            nk_input_glyph(&ctx, &glyph[0]);
        } else if (evt->type == SDL_MOUSEWHEEL) {
            /* mouse wheel */
            nk_input_scroll(&ctx, nk_vec2(static_cast<float>(evt->wheel.x), static_cast<float>(evt->wheel.y)));
        }
    }


    void NuklearSdl::render()
    {
        const nk_command* command{};
        nk_foreach(command, &context)
        {
            const void* cmd = command;
            //    Uint32 color; temporalmente no tiene uso.
            switch (command->type) {
                case NK_COMMAND_NOP: {}break;
                case NK_COMMAND_SCISSOR: {
                    const auto *s = static_cast<const struct nk_command_scissor*>(cmd);
                    SDL_Rect rect{static_cast<int>(s->x), static_cast<int>(s->y), static_cast<int>(s->w), static_cast<int>(s->h)};
                    SDL_RenderSetClipRect(renderer, &rect );
                }break;
                case NK_COMMAND_LINE: {
                    const  auto *l = static_cast<const struct nk_command_line *>(cmd);
                    sdl_draw_line(renderer, l->begin.x, l->begin.y, l->end.x, l->end.y, l->color);
                }break;
                case NK_COMMAND_RECT: {
                    const  auto *r = static_cast<const struct nk_command_rect *>(cmd);
                    sdl_draw_rect(renderer, r->x, r->y, r->w, r->h, r->rounding, r->color);
                }break;
                case NK_COMMAND_RECT_FILLED: {
                    const auto *r = static_cast<const struct nk_command_rect_filled *>(cmd);
                    sdl_draw_rect_fill(renderer, r->x, r->y, r->w, r->h, r->rounding, r->color);
                }break;
                case NK_COMMAND_CIRCLE: {
                    const auto *c = static_cast<const struct nk_command_circle *>(cmd);
                    int16_t xr = c->w/2;
                    int16_t yr = c->h/2;
                    sdl_draw_ellipse(renderer, c->x + xr, c->y + yr, xr, yr, c->color);
                }break;
                case NK_COMMAND_CIRCLE_FILLED: {
                    const auto *c = static_cast<const struct nk_command_circle_filled *>(cmd);
                    int16_t xr = c->w/2;
                    int16_t yr = c->h/2;
                    sdl_draw_ellipse_filled(renderer, c->x + xr, c->y + yr, xr, yr, c->color);
                }break;
                case NK_COMMAND_TRIANGLE: {
                    const auto*t = static_cast<const struct nk_command_triangle*>(cmd);
                    sdl_draw_triangle(renderer, t->a.x, t->a.y, t->b.x, t->b.y,
                                      t->c.x, t->c.y, t->color);
                }break;
                case NK_COMMAND_TRIANGLE_FILLED: {
                    const auto *t = static_cast<const struct nk_command_triangle_filled *>(cmd);
                    sdl_draw_filled_triangle(renderer, t->a.x, t->a.y, t->b.x,
                                             t->b.y, t->c.x, t->c.y, t->color);
                }break;
                case NK_COMMAND_POLYGON: {
                    const auto *p = static_cast<const struct nk_command_polygon*>(cmd);
                    std::vector<Sint16> vertices(p->point_count * 2);
                    std::span points{&p->points[0], p->point_count};
                    for (uint16_t i = 0; i < p->point_count; i++) {
                        vertices[i] = points[i].x;
                        vertices[i + p->point_count] = points[i].y;
                    }
                    sdl_draw_polyline(renderer, &vertices[0], &vertices[p->point_count], p->point_count, p->color);
                }break;
                case NK_COMMAND_POLYGON_FILLED: {
                    const auto *p = static_cast<const struct nk_command_polygon_filled *>(cmd);
                    std::vector<Sint16> vertices(p->point_count * 2);
                    std::span points{&p->points[0], p->point_count};
                    for (uint16_t i = 0; i < p->point_count; i++) {
                        vertices[i] = points[i].x;
                        vertices[i + p->point_count] = points[i].y;
                    }
                    sdl_draw_filled_polygon(renderer, &vertices[0], &vertices[p->point_count], p->point_count, p->color);
                }break;
                case NK_COMMAND_POLYLINE: {}break;
                case NK_COMMAND_TEXT: {
                    const auto *t = static_cast<const struct nk_command_text*>(cmd);
                    auto *font = static_cast<TTF_Font*>(t->font->userdata.ptr);
                    sdl_draw_text(renderer, font, &t->string[0], t->x, t->y, t->foreground);
                }break;
                case NK_COMMAND_CURVE: {
                    const auto *q = static_cast<const struct nk_command_curve *>(cmd);
                    nk_sdl_stroke_curve(renderer, q->begin, q->ctrl[0], q->ctrl[1],
                                        q->end, 22, q->line_thickness, q->color);
                } break;
                case NK_COMMAND_ARC: {
                    const auto *a = static_cast<const struct nk_command_arc *>(cmd);
                    sdl_draw_arc(renderer, a->cx, a->cy, a->r, static_cast<uint16_t>(a->a[0]),static_cast<uint16_t>(a->a[1]), a->color);
                }break;
                case NK_COMMAND_RECT_MULTI_COLOR: {
                    const auto *r = static_cast<const struct nk_command_rect_multi_color *>(cmd);
                    nk_sdl_fill_rect_multi_color(renderer, r->x, r->y, r->w, r->h, r->left, r->top, r->right, r->bottom);
                }break;
                case NK_COMMAND_IMAGE: {
                    const auto * i = static_cast<const struct nk_command_image *>(cmd);
                    sdl_draw_image(renderer, &i->img, i->x, i->y, i->w, i->h);
                }break;
                case NK_COMMAND_ARC_FILLED:
                default: break;
            }
        }
        nk_clear(&context);
    }

    NuklearSdl::~NuklearSdl() {
        nk_free(&context);
    }

}  // namespace app
