#pragma once

#include "primitives.h"

#include <optional>
#include <string>
#include <string_view>
#include <utility>
#include <vector>


namespace app {

class Pattern {
public:
    using TCells = std::vector<Point>;

    Pattern() = default;
    Pattern(std::string_view name, TCells aliveCells);

    [[nodiscard]] const std::string& name() const { return m_name; }
    [[nodiscard]] const TCells& aliveCells() const { return m_aliveCells; }
    [[nodiscard]] const Size& size() const { return m_size; }

private:
    std::string m_name;
    std::vector<Point> m_aliveCells;
    Size m_size;
};

std::vector<Pattern> getDefaultPatterns();
Pattern getDefaultPattern();
std::optional<Pattern> loadFromFile(std::string_view name, std::string_view filePath);

} // namespace app
