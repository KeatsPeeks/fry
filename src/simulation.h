#pragma once

#include "../deps/robin_hood.h"

#include "pattern.h"
#include "primitives.h"

#include <array>
#include <cstdint>
#include <vector>

namespace app {

enum CellState : uint8_t {
    DEAD,
    ALIVE
};

class Simulation
{
public:

    explicit Simulation(int size, const Pattern& pattern = {});

    [[nodiscard]] CellState get(int x, int y) const { return matrix[y * m_size + x]; }
    void set(int x, int y, CellState cellState);
    [[nodiscard]] int size() const { return m_size; }

    void nextStep();

private:
    using TChangeList = robin_hood::unordered_set<size_t>;
    TChangeList m_changeList{};

    int m_size;

    std::vector<CellState> matrix;

    void init(const Pattern& pattern);

    void updateCell(size_t index);

    void incrementalSet(size_t index, CellState state);
};

}  // namespace app

