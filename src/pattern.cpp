
#include "pattern.h"
#include <algorithm>


namespace app {

namespace {
    Pattern::TCells toAliveCells(std::vector<std::vector<uint8_t>> table) {
        int height = static_cast<int>(table.size());
        Pattern::TCells cells;
        for (int y = 0; y < height; ++y) {
            const auto& row = table[y];
            int width = static_cast<int>(row.size());
            for (int x = 0; x < width; ++x) {
                if (row[x] == 1) {
                    cells.push_back({x, y});
                }
            }
        }
        return cells;
    }

    Size getSize(Pattern::TCells cells) {
        auto min_max_x = std::minmax_element(cells.begin(), cells.end(), [](auto p1, auto p2) { return p1.x < p2.x; });
        auto min_max_y = std::minmax_element(cells.begin(), cells.end(), [](auto p1, auto p2) { return p1.y < p2.y; });
        return Size{
            min_max_x.second->x - min_max_x.first->x,
            min_max_y.second->y - min_max_y.first->y
        };
    }
} // anonymous namespace

Pattern::Pattern(std::string name, TCells aliveCells) : m_name{std::move(name)}, m_aliveCells{std::move(aliveCells)},
    m_size{getSize(m_aliveCells)}
{
}


    Pattern getDefaultPattern() {
    return {
            "acorn",
            toAliveCells({
                    { 0, 1, 0, 0, 0, 0, 0 },
                    { 0, 0, 0, 1, 0, 0, 0 },
                    { 1, 1, 0, 0, 1, 1, 1 },
            })
    };
}

std::vector<Pattern> getDefaultPatterns() {
    return {
            getDefaultPattern(),
            {
                    "infinite",
                    toAliveCells({
                            { 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1 },
                    })
            }
    };
}

}  // namespace app