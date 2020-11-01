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

struct Cell {
    int x;
    int y;
    CellState state;
};

class Simulation
{
public:
    using TChangeList = robin_hood::unordered_set<int>;

    explicit Simulation(Size size, const Pattern& pattern = {});

    [[nodiscard]] CellState get(int x, int y) const { return matrix[y * m_size.w + x]; }
    void set(int x, int y, CellState cellState);
    [[nodiscard]] Size size() const { return m_size; }
    [[nodiscard]] std::shared_ptr<std::vector<Cell>> updatedCells() const { return lastUpdatedCells; }

    void nextStep();

    Simulation(const Simulation& right) = delete;
    Simulation& operator=(const Simulation& right) = delete;
    Simulation(Simulation&& right) noexcept = delete;
    Simulation& operator=(Simulation&& right) noexcept = delete;
    ~Simulation() = default;

private:
    TChangeList changeList{};
    TChangeList changeList2{};
    TChangeList* writeChangeList = &changeList;
    TChangeList* readChangeList = &changeList2;
    std::shared_ptr<std::vector<Cell>> lastUpdatedCells;

    Size m_size;

    std::vector<CellState> matrix;

    void init(const Pattern& pattern);

    void updateCell(int index);

    void updateChangeList(int index, CellState state);
};

}  // namespace app

