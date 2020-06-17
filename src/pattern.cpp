
#include "pattern.h"
#include <algorithm>
#include <fstream>
#include <string>


namespace app {

namespace {
    Size getSize(Pattern::TCells cells) {
        auto min_max_x = std::minmax_element(cells.begin(), cells.end(), [](auto p1, auto p2) { return p1.x < p2.x; });
        auto min_max_y = std::minmax_element(cells.begin(), cells.end(), [](auto p1, auto p2) { return p1.y < p2.y; });
        return Size{
            min_max_x.second->x - min_max_x.first->x + 1,
            min_max_y.second->y - min_max_y.first->y + 1
        };
    }
} // anonymous namespace

Pattern::Pattern(std::string_view name, TCells aliveCells) : m_name{name}, m_aliveCells{std::move(aliveCells)},
    m_size{getSize(m_aliveCells)} {
}

Pattern loadFromStrings(std::string_view name, const std::vector<std::string>& strings) {
    Pattern::TCells cells;
    for (int y = 0; const auto& line : strings) {
        for (int x = 0; auto c : line) {
            if (c == 'O') {
                cells.push_back({x, y});
            }
            x++;
        }
        y++;
    }
    return Pattern{name, cells};
}

std::optional<Pattern> loadFromFile(std::string_view name, std::string_view filePath) {
    std::ifstream file{filePath.data()};
    std::string line;
    std::vector<std::string> strings;
    while (std::getline(file, line)) {
        if (!line.starts_with('!')) {
            strings.push_back(line);
        }
    }
    return strings.empty() ? std::nullopt : std::optional(loadFromStrings(name, strings));
}

}  // namespace app