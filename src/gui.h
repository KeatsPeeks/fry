#pragma once

#include "../deps/nuklear/nuklear.h"
#include "pattern.h"
#include "sdl_wrappers.h"

#include <string>

namespace app {

struct GuiBindings {
    int* displayGrid;
    int* speed;
    bool* paused;
    int* cellSize;
    const Pattern** selectedPattern;
    bool* patternModalOpened;
    bool* step;
    bool* clear;

    int* iteration;
};

struct NkIcon {
    sdl::Texture texture;
    struct nk_image image;
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
    NkIcon playIcon;
    NkIcon pauseIcon;
    NkIcon nextIcon;
    std::string iteration = "0";

    void mainMenu(const Size &viewPort);
    void patternMenu(const sdl::Renderer& renderer);
};

} // namespace app
