#pragma once

#include "pattern.h" // TODO virer, normalement le game est capable d'appeler set directement ?
#include "primitives.h"

#include <array>
#include <cstdint>
#include <unordered_set>
#include <vector>
#ifdef ENABLE_PMR
#include <memory_resource>
#endif

namespace app {

enum CellState : uint8_t {
    DEAD,
    ALIVE
};

class Simulation
{
#ifdef ENABLE_PMR
public:
    using TChangeList = std::pmr::unordered_set<size_t>;
private:
    static std::pmr::unsynchronized_pool_resource pool;
    TChangeList m_changeList{&pool};
#else
public:
    using TChangeList = std::unordered_set<size_t>;
private:
    TChangeList m_changeList{};
#endif

public:
    using TPattern = std::vector<std::vector<uint8_t>>;

    explicit Simulation(int size, const Pattern& pattern = {});

    [[nodiscard]] CellState get(int x, int y) const { return matrix[y * m_size + x]; }
    void set(int x, int y, CellState cellState);
    [[nodiscard]] int size() const { return m_size; }

    void nextStep();

private:
    int m_size;

    std::vector<CellState> matrix;

    void init(const Pattern& pattern);

    void updateCell(size_t index);

    void incrementalSet(size_t index, CellState state);
};

}  // namespace app

