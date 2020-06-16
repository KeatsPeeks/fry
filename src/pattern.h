#pragma once

#include "primitives.h"

#include <string>
#include <utility>
#include <vector>


namespace app {

class Pattern {
public:
    using TCells = std::vector<Point>;

    Pattern() = default;
    Pattern(std::string name, TCells aliveCells);

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

} // namespace app
