#include "gui.h"

#include "primitives.h"

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
    } // anonymous namespace

    Gui::Gui(nk_context* pNuklearCtx, std::vector<Pattern> patterns, GuiBindings bindings) : pNuklearCtx{pNuklearCtx}, bindings{bindings}, patterns{std::move(patterns)} {
        std::array<nk_color, NK_COLOR_COUNT> table{};
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
    }

    static const int minSpeed = 0;
    static const int maxSpeed = 10;
    static const int minCellSize = 1;
    static const int maxCellSize = 32;

    void Gui::update(int viewPortWidth) {
        if (*bindings.selectedPattern != nullptr) {
            return;
        }
        constexpr Size margin{-7, 0};
        constexpr Size panelSize{200, 400};
        Rect panelRect{{viewPortWidth - panelSize.w - margin.w, margin.h}, panelSize};
        if (0 != nk_begin(pNuklearCtx, "", to_nk_rect(panelRect), 0)) {
            nk_layout_row_dynamic(pNuklearCtx, 8, 1);

            // Play/Pause button
            nk_layout_row_dynamic(pNuklearCtx, 0, 1);
            const char* text = *bindings.paused ? "Run" : "Pause";
            if (1 == nk_button_label(pNuklearCtx, text)) {
                *bindings.paused = !*bindings.paused;
            }

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
            nk_label(pNuklearCtx, "Cell size:", NK_TEXT_ALIGN_LEFT);
            nk_layout_row_dynamic(pNuklearCtx, 16, 1);
            nk_slider_int(pNuklearCtx, minCellSize, bindings.cellSize, maxCellSize, 1);
            nk_layout_row_dynamic(pNuklearCtx, 0, 1);
            nk_label(pNuklearCtx, fmt::format("{} px", *bindings.cellSize).c_str(), NK_TEXT_ALIGN_CENTERED | NK_TEXT_ALIGN_TOP);

            // patterns
            nk_layout_row_dynamic(pNuklearCtx, 0, 1);
            nk_label(pNuklearCtx, "Place pattern:", NK_TEXT_ALIGN_LEFT);
            for (const auto& pattern : patterns) {
                if (1 == nk_button_label(pNuklearCtx, pattern.name().c_str())) {
                    *bindings.selectedPattern = &pattern;
                }
            }
        }
        nk_end(pNuklearCtx);
    }

} // namespace app

