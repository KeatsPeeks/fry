#pragma once

#include "../deps/nuklear/nuklear.h"
#include "pattern.h"
#include "sdl_wrappers.h"

namespace app {

struct GuiBindings {
    int* displayGrid;
    int* speed;
    bool* paused;
    int* cellSize;
    const Pattern** selectedPattern;
    bool* patternModalOpened;
};

class Gui {
public:
    Gui(nk_context* pNuklearCtx, std::vector<Pattern> patterns, GuiBindings bindings);
    void update(const sdl::Renderer& renderer);

private:
    nk_context* pNuklearCtx = nullptr;
    GuiBindings bindings = {};
    std::vector<Pattern> patterns;
    std::vector<sdl::Texture> patternTextures;
    std::vector<struct nk_image> patternImages;

    void mainMenu(const Size &viewPort);
    void patternMenu(const sdl::Renderer& renderer);
};

} // namespace app
