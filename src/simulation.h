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

enum class CellState {
    DEAD,
    ALIVE
};

struct point_hash
{
    int operator ()(const Point& point) const
    {
        return point.x ^ point.y;
    }
};

class Simulation
{
#ifdef ENABLE_PMR
public:
    using TAliveList = std::pmr::unordered_set<Point, point_hash>;
private:
    static std::pmr::unsynchronized_pool_resource pool;
    TAliveList aliveList{&pool};
#else
public:
    using TAliveList = std::unordered_set<Point, point_hash>;
private:
    TAliveList aliveList{};
#endif

public:
    using TPattern = std::vector<std::vector<uint8_t>>;

    explicit Simulation(int size, const Pattern& pattern = {});

    [[nodiscard]] bool get(int x, int y) const { return matrix[y][x]; }
    void set(int x, int y, CellState cellState);
    [[nodiscard]] int getSize() const { return size; }

    void nextStep();
    [[nodiscard]] const TAliveList& getAliveCells() const { return aliveList; }

private:
    int size;

    std::vector<std::vector<bool>> matrix;
    std::vector<std::vector<bool>> matrixCopy;

    void init(const Pattern& pattern);

    void updateCell(int x, int y);

    void incrementalSet(int x, int y, CellState cellState);
};

}  // namespace app

