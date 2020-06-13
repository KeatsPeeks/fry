#pragma once

#include "../deps/nuklear/nuklear.h"

namespace app {

    struct GuiBindings {
        int* displayGrid;
        int* speed;
    };

    class Gui {
    public:
        Gui(nk_context* pNuklearCtx, GuiBindings bindings);
        void update(int viewPortWidth);

    private:
        nk_context* pNuklearCtx{};
        GuiBindings bindings{};
    };
} // namespace app
