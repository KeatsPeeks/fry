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

std::optional<Pattern> loadFromFile(std::string_view fileName, std::string_view filePath);
Pattern loadPlaintext(std::string_view name, const std::vector<std::string>& strings);
Pattern loadRle(std::string_view name, const std::vector<std::string>& strings);

namespace Patterns {

    inline Pattern acorn() {
        return loadPlaintext("Acorn", {
            ".O.....",
            "...O...",
            "OO..OOO",
        });
    }

    inline Pattern r_pentomino() {
        return loadPlaintext("R-pentomino", {
            ".OO",
            "OO.",
            ".O.",
        });
    }

    inline Pattern diehard() {
        return loadPlaintext("Diehard", {
            "......O.",
            "OO......",
            ".O...OOO",
        });
    }

    inline Pattern glider() {
        return loadPlaintext("Glider", {
            "OOO",
            "O..",
            ".O.",
        });
    }
    
    inline Pattern spaceship() {
        return loadPlaintext("Glider", {
            "..O...",
            "O...O.",
            ".....O",
            "O....O",
            ".OOOOO",
        });
    }
    
    inline Pattern infinite() {
        return loadPlaintext("Infinite", {
            "OOOOOOOO.OOOOO...OOO......OOOOOOO.OOOOO",
        });
    }

    inline std::vector<Pattern> defaultPatterns() {
        return { acorn(), r_pentomino(), diehard(), glider(), spaceship(), infinite() };
    }
    
} // namespace Patterns

} // namespace app
