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

struct Cell {
    CellState alive;
    int x;
    int y;

    bool operator==(const Cell& p) const { return p.x == x && p.y == y; };
};

struct point_hash
{
    int operator ()(const Cell& point) const
    {
        return point.x ^ point.y;
    }
};

class Simulation
{
#ifdef ENABLE_PMR
public:
    using TChangeList = std::pmr::unordered_set<Cell, point_hash>;
private:
    static std::pmr::unsynchronized_pool_resource pool;
    TChangeList m_changeList{&pool};
#else
public:
    using TChangeList = std::unordered_set<Cell, point_hash>;
private:
    TChangeList m_changeList{};
#endif

public:
    using TPattern = std::vector<std::vector<uint8_t>>;

    explicit Simulation(int size, const Pattern& pattern = {});

    [[nodiscard]] CellState get(int x, int y) const { return matrix[y][x]; }
    void set(int x, int y, CellState cellState);
    [[nodiscard]] int size() const { return m_size; }

    void nextStep();
    [[nodiscard]] const TChangeList& changelist() const { return m_changeList; }

private:
    int m_size;

    std::vector<std::vector<CellState>> matrix;

    void init(const Pattern& pattern);

    void updateCell(int x, int y);

    void incrementalSet(const Cell& p);
};

}  // namespace app

