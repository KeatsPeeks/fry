#include "gui.h"
#include "primitives.h"
#include <array>

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

    Gui::Gui(nk_context* pNuklearCtx, GuiBindings bindings) : pNuklearCtx{pNuklearCtx}, bindings{bindings} {
        std::array<nk_color, NK_COLOR_COUNT> table{};
        defaultBlueTheme(&table);
        table[NK_COLOR_TOGGLE] = table[NK_COLOR_WINDOW];
        table[NK_COLOR_TOGGLE_HOVER] = table[NK_COLOR_WINDOW];
        table[NK_COLOR_TOGGLE_CURSOR] = nk_rgba(10, 110, 220, 255);
        nk_style_from_table(pNuklearCtx, &table[0]);

        pNuklearCtx->style.checkbox.padding = nk_vec2(2, 2);
        pNuklearCtx->style.checkbox.border = 2;
        pNuklearCtx->style.checkbox.cursor_hover.data.color = table[NK_COLOR_TOGGLE_CURSOR];
        pNuklearCtx->style.checkbox.border_color = table[NK_COLOR_TEXT];

        pNuklearCtx->style.window.padding = nk_vec2(14, 6);
        pNuklearCtx->style.checkbox.spacing = 10;
    }

    void Gui::update(int viewPortWidth) {
        constexpr int margin = 12;
        constexpr Size panelSize{230, 250};
        Rect panelRect{{viewPortWidth - panelSize.w - margin, margin}, panelSize};
        if (0 != nk_begin(pNuklearCtx, "Controls", to_nk_rect(panelRect), NK_WINDOW_MINIMIZABLE | NK_WINDOW_TITLE)) {
            nk_layout_row_dynamic(pNuklearCtx, 0, 1);
            nk_checkbox_label(pNuklearCtx, "show grid", bindings.displayGrid);
        }
        nk_end(pNuklearCtx);
    }

} // namespace app
