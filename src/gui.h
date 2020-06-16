#pragma once

#include "../deps/nuklear/nuklear.h"
#include "pattern.h"

namespace app {

    struct GuiBindings {
        int* displayGrid;
        int* speed;
        bool* paused;
        int* cellSize;
        const Pattern** selectedPattern;
    };

    class Gui {
    public:
        Gui(nk_context* pNuklearCtx, std::vector<Pattern> patterns, GuiBindings bindings);
        void update(Size viewPort);

    private:
        nk_context* pNuklearCtx{};
        GuiBindings bindings{};
        std::vector<Pattern> patterns;
    };
} // namespace app
