#include "../deps/SDL2_gfx/SDL2_gfxPrimitives.h"

#include "gui.h"
#include "colors.h"

#include <array>
#include <utility>

#include <fmt/format.h>

namespace app {

namespace {
    void defaultBlueTheme(std::array<nk_color, NK_COLOR_COUNT>* table) {
        (*table)[NK_COLOR_TEXT] = nk_rgba(20, 20, 20, 255);
        (*table)[NK_COLOR_WINDOW] = nk_rgba(202, 212, 214, 215);
        (*table)[NK_COLOR_HEADER] = nk_rgba(137, 182, 224, 220);
        (*table)[NK_COLOR_BORDER] = nk_rgba(140, 159, 173, 255);
        (*table)[NK_COLOR_BUTTON] = nk_rgba(137, 182, 224, 255);
        (*table)[NK_COLOR_BUTTON_HOVER] = nk_rgba(142, 187, 229, 255);
        (*table)[NK_COLOR_BUTTON_ACTIVE] = nk_rgba(147, 192, 234, 255);
        (*table)[NK_COLOR_TOGGLE] = nk_rgba(177, 210, 210, 255);
        (*table)[NK_COLOR_TOGGLE_HOVER] = nk_rgba(182, 215, 215, 255);
        (*table)[NK_COLOR_TOGGLE_CURSOR] = nk_rgba(137, 182, 224, 255);
        (*table)[NK_COLOR_SELECT] = nk_rgba(177, 210, 210, 255);
        (*table)[NK_COLOR_SELECT_ACTIVE] = nk_rgba(137, 182, 224, 255);
        (*table)[NK_COLOR_SLIDER] = nk_rgba(177, 210, 210, 255);
        (*table)[NK_COLOR_SLIDER_CURSOR] = nk_rgba(137, 182, 224, 245);
        (*table)[NK_COLOR_SLIDER_CURSOR_HOVER] = nk_rgba(142, 188, 229, 255);
        (*table)[NK_COLOR_SLIDER_CURSOR_ACTIVE] = nk_rgba(147, 193, 234, 255);
        (*table)[NK_COLOR_PROPERTY] = nk_rgba(210, 210, 210, 255);
        (*table)[NK_COLOR_EDIT] = nk_rgba(210, 210, 210, 225);
        (*table)[NK_COLOR_EDIT_CURSOR] = nk_rgba(20, 20, 20, 255);
        (*table)[NK_COLOR_COMBO] = nk_rgba(210, 210, 210, 255);
        (*table)[NK_COLOR_CHART] = nk_rgba(210, 210, 210, 255);
        (*table)[NK_COLOR_CHART_COLOR] = nk_rgba(137, 182, 224, 255);
        (*table)[NK_COLOR_CHART_COLOR_HIGHLIGHT] = nk_rgba( 255, 0, 0, 255);
        (*table)[NK_COLOR_SCROLLBAR] = nk_rgba(190, 200, 200, 255);
        (*table)[NK_COLOR_SCROLLBAR_CURSOR] = nk_rgba(64, 84, 95, 255);
        (*table)[NK_COLOR_SCROLLBAR_CURSOR_HOVER] = nk_rgba(70, 90, 100, 255);
        (*table)[NK_COLOR_SCROLLBAR_CURSOR_ACTIVE] = nk_rgba(75, 95, 105, 255);
        (*table)[NK_COLOR_TAB_HEADER] = nk_rgba(156, 193, 220, 255);
    }

    std::vector<sdl::Texture> loadTextures(const sdl::Renderer &renderer, const std::vector<Pattern>& patterns) {
        std::vector<sdl::Texture> textures;
        for (const Pattern& pattern : patterns) {
            int texSize = std::max(16, std::max(pattern.size().w + 2, pattern.size().h + 2));
            Vector offset = Vector{texSize - pattern.size().w, texSize - pattern.size().h} / 2;
            sdl::Texture texture{SDL_CreateTexture(renderer.getRaw(), SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_TARGET, texSize, texSize)};
            texture.setBlendMode(SDL_BLENDMODE_BLEND);
            renderer.setTarget(texture.getRaw());
            renderer.setDrawColor(Color::DeadCell);
            renderer.fillRect(nullptr);

            std::vector<SDL_Point> alives;
            for (const Point& p : pattern.aliveCells()) {
                alives.push_back({p.x + offset.x, p.y + offset.y});
            }
            if (!alives.empty()) {
                renderer.setDrawColor(Color::AliveCell);
                renderer.drawPoints(alives);
            }
            textures.push_back(std::move(texture));
        }
        renderer.setTarget(nullptr);
        return textures;
    }

    sdl::Texture pause(const sdl::Renderer &renderer) {
        int texSize = 128;
        sdl::Texture texture{SDL_CreateTexture(renderer.getRaw(), SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_TARGET, texSize, texSize)};
        texture.setBlendMode(SDL_BLENDMODE_BLEND);
        renderer.setTarget(texture.getRaw());

        thickLineRGBA(renderer.getRaw(), 42, 12, 42, 116, 24, 52, 119, 235, 255);
        thickLineRGBA(renderer.getRaw(), 86, 12, 86, 116, 24, 52, 119, 235, 255);

        renderer.setTarget(nullptr);
        return texture;
    }

    sdl::Texture play(const sdl::Renderer &renderer) {
        int texSize = 128;
        sdl::Texture texture{SDL_CreateTexture(renderer.getRaw(), SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_TARGET, texSize, texSize)};
        texture.setBlendMode(SDL_BLENDMODE_BLEND);
        renderer.setTarget(texture.getRaw());
        filledTrigonRGBA(renderer.getRaw(), 30, 12, 98, 64, 30,116, 52, 119, 235, 255);

        renderer.setTarget(nullptr);
        return texture;
    }

    sdl::Texture next(const sdl::Renderer &renderer) {
        int texSize = 128;
        sdl::Texture texture{SDL_CreateTexture(renderer.getRaw(), SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_TARGET, texSize, texSize)};
        texture.setBlendMode(SDL_BLENDMODE_BLEND);
        renderer.setTarget(texture.getRaw());
        filledTrigonRGBA(renderer.getRaw(), 30, 12, 98, 64, 30,116, 52, 119, 235, 255);
        thickLineRGBA(renderer.getRaw(), 98, 12, 98, 116, 16, 52, 119, 235, 255);

        renderer.setTarget(nullptr);
        return texture;
    }

    constexpr int minSpeed = 0;
    constexpr int maxSpeed = 10;
    constexpr int minCellSize = 1;
    constexpr int maxCellSize = 32;
    constexpr int widgetSize = 180;

} // anonymous namespace

Gui::Gui(nk_context* pNuklearCtx, std::vector<Pattern> patterns, GuiBindings bindings) : pNuklearCtx{pNuklearCtx}, bindings{bindings}, patterns{std::move(patterns)} {
    std::array<nk_color, NK_COLOR_COUNT> table = {};
    defaultBlueTheme(&table);
    const nk_color customBlue = nk_rgba(52, 119, 235, 255);
    table[NK_COLOR_TOGGLE] = table[NK_COLOR_WINDOW];
    table[NK_COLOR_TOGGLE_HOVER] = table[NK_COLOR_WINDOW];
    table[NK_COLOR_TOGGLE_CURSOR] = customBlue;
    table[NK_COLOR_SLIDER] = table[NK_COLOR_HEADER];
    table[NK_COLOR_SLIDER_CURSOR] = customBlue;
    table[NK_COLOR_SLIDER_CURSOR_HOVER] = customBlue;
    table[NK_COLOR_SLIDER_CURSOR_ACTIVE] = customBlue;
    table[NK_COLOR_WINDOW] = nk_rgba(0, 0, 0, 0);
    nk_style_from_table(pNuklearCtx, &table[0]);

    pNuklearCtx->style.checkbox.padding = nk_vec2(2, 2);
    pNuklearCtx->style.checkbox.border = 2;
    pNuklearCtx->style.checkbox.cursor_hover.data.color = customBlue;
    pNuklearCtx->style.checkbox.border_color = table[NK_COLOR_TEXT];
    pNuklearCtx->style.checkbox.spacing = 10;

    pNuklearCtx->style.button.rounding = 0;
    pNuklearCtx->style.button.border_color = customBlue;

    pNuklearCtx->style.slider.padding = nk_vec2(-10, 2);

    pNuklearCtx->style.window.padding = nk_vec2(14, 6);
    pNuklearCtx->style.window.border = 3.;
    pNuklearCtx->style.window.header.minimize_button.active.data.color = table[NK_COLOR_BUTTON];
    pNuklearCtx->style.window.header.minimize_button.normal.data.color = table[NK_COLOR_BUTTON_ACTIVE];
    pNuklearCtx->style.window.header.minimize_button.hover.data.color = table[NK_COLOR_BUTTON_HOVER];

    *bindings.patternModalOpened = false;
    *bindings.selectedPattern = nullptr;
}

void Gui::update(const sdl::Renderer& renderer) {
    // no menu when placing a pattern
    if (*bindings.selectedPattern != nullptr) {
        return;
    }

    // on-demand texture loading
    if (playIcon.texture.getRaw() == nullptr) {
        playIcon.texture = play(renderer);
        playIcon.image = nk_image_ptr(playIcon.texture.getRaw());
    }
    if (pauseIcon.texture.getRaw() == nullptr) {
        pauseIcon.texture = pause(renderer);
        pauseIcon.image = nk_image_ptr(pauseIcon.texture.getRaw());
    }
    if (nextIcon.texture.getRaw() == nullptr) {
        nextIcon.texture = next(renderer);
        nextIcon.image = nk_image_ptr(nextIcon.texture.getRaw());
    }

    patternMenu(renderer);
    mainMenu(renderer.getOutputSize());
}

void Gui::mainMenu(const Size &viewPort) {
    if (*bindings.patternModalOpened) {
        return;
    }

    const Size margin = {0, -2};
    const Size panelSize = {200, viewPort.h};
    Rect panelRect = {{viewPort.w - panelSize.w - margin.w, margin.h}, panelSize};
    if (0 != nk_begin(pNuklearCtx, "main", to_nk_rect(panelRect), NK_WINDOW_NO_SCROLLBAR)) {
        nk_layout_row_dynamic(pNuklearCtx, 8, 1);

        // Buttons
        nk_layout_row_begin(pNuklearCtx, NK_DYNAMIC, 0, 3);
        struct nk_style_button style = pNuklearCtx->style.button;
        nk_layout_row_push(pNuklearCtx, 0.25F);
        if (1 == nk_button_image_styled(pNuklearCtx, &style, *bindings.paused ? playIcon.image : pauseIcon.image)) {
            *bindings.paused = !*bindings.paused;
        }
        nk_layout_row_push(pNuklearCtx, 0.25F);
        if (1 == nk_button_image_styled(pNuklearCtx, &style, nextIcon.image)) {
            *bindings.step = true;
        }
        nk_layout_row_push(pNuklearCtx, 0.5F);
        iteration = std::to_string(*bindings.iteration);
        nk_text(pNuklearCtx, iteration.c_str(), 12, NK_TEXT_ALIGN_RIGHT | NK_TEXT_ALIGN_MIDDLE);
        nk_layout_row_end(pNuklearCtx);

        // Grid checkbox
        nk_layout_row_dynamic(pNuklearCtx, 50, 1);
        nk_checkbox_label(pNuklearCtx, "show grid", bindings.displayGrid);

        // Speed slider
        nk_layout_row_dynamic(pNuklearCtx, 20, 1);
        nk_label(pNuklearCtx, "Speed:", NK_TEXT_ALIGN_LEFT);
        nk_layout_row_dynamic(pNuklearCtx, 16, 1);
        nk_slider_int(pNuklearCtx, minSpeed, bindings.speed, maxSpeed, 1);
        nk_layout_row_dynamic(pNuklearCtx, 24, 1);
        nk_label(pNuklearCtx, fmt::format("{} ups", 1 << *bindings.speed).c_str(), NK_TEXT_ALIGN_CENTERED | NK_TEXT_ALIGN_TOP);

        // Size slider
        nk_layout_row_dynamic(pNuklearCtx, 20, 1);
        nk_label(pNuklearCtx, "Zoom level:", NK_TEXT_ALIGN_LEFT);
        nk_layout_row_dynamic(pNuklearCtx, 16, 1);
        nk_slider_int(pNuklearCtx, minCellSize, bindings.cellSize, maxCellSize, 1);
        nk_layout_row_dynamic(pNuklearCtx, 0, 1);
        nk_label(pNuklearCtx, fmt::format("{}", *bindings.cellSize).c_str(), NK_TEXT_ALIGN_CENTERED | NK_TEXT_ALIGN_TOP);

        // patterns
        nk_layout_row_dynamic(pNuklearCtx, 0, 1);
        if (1 == nk_button_label(pNuklearCtx, "Load Pattern...")) {
            *bindings.patternModalOpened = true;
        }

        // clear
        nk_layout_row_dynamic(pNuklearCtx, 0, 1);
        if (1 == nk_button_label(pNuklearCtx, "Clear")) {
            *bindings.clear = true;
        }
    }
    nk_end(pNuklearCtx);
}

void Gui::patternMenu(const sdl::Renderer& renderer) {
    if (!*bindings.patternModalOpened) {
        return;
    }

    // Render textures on first use
    if (patternImages.empty() && !patterns.empty()) {
        patternTextures = loadTextures(renderer, patterns);
        for (const sdl::Texture& tex : patternTextures) {
            patternImages.push_back(nk_image_ptr(tex.getRaw()));
        }
    }

    comprends pas.
    on a :
    - rajouté des boutons
    - rajouté le clear
    Et quand on redimenssionne/pattern/clear plein de fois ça finit par perdre les textures dans nuklear. en debug elles existent encore (pointeur non nul en tout cas).
    quelle est la baise ?

    auto spacingPush = pNuklearCtx->style.window.spacing;

    const int spacing = 20;
    pNuklearCtx->style.window.spacing = nk_vec2(spacing, 0);
    Rect menuRect = {{}, renderer.getOutputSize()};
    struct nk_style_button style = pNuklearCtx->style.button;
    style.hover.data.color = nk_rgba(48, 89, 254, 255);
    if (0 != nk_begin(pNuklearCtx, "patterns", to_nk_rect(menuRect), 0)) {
        nk_layout_row_dynamic(pNuklearCtx, 0, 1);
        const int nbCols = (menuRect.size.w - spacing) / (widgetSize + spacing);
        for (size_t row = 0; row * nbCols < patterns.size(); ++row) {
            // pictures
            nk_layout_row_static(pNuklearCtx, static_cast<float>(widgetSize), widgetSize, nbCols);
            std::vector<const char*> legends;
            for (int col = 0; col < nbCols && row * nbCols + col < patterns.size(); ++col) {
                const Pattern& pattern = patterns[row * nbCols + col];
                if (1 == nk_button_image_styled(pNuklearCtx, &style, patternImages[row * nbCols + col])) {
                    *bindings.selectedPattern = &pattern;
                    *bindings.patternModalOpened = false;
                }
                legends.push_back(pattern.name().c_str());
            }

            // legends
            nk_layout_row_static(pNuklearCtx, 32, widgetSize, nbCols);
            for (const char* legend : legends) {
                nk_label(pNuklearCtx, legend, NK_TEXT_ALIGN_TOP | NK_TEXT_ALIGN_CENTERED);
            }
        }
    }
    nk_end(pNuklearCtx);

    pNuklearCtx->style.window.spacing = spacingPush;
}

} // namespace app

