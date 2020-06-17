#include "simulation.h"
#include <algorithm>

namespace app {

#ifdef ENABLE_PMR
std::pmr::unsynchronized_pool_resource Simulation::pool{};
#endif

Simulation::Simulation(int size, const Pattern& pattern) : m_size{size} {
    matrix.resize(size);
    for (int i = 0; i < size; ++i) {
        matrix[i].resize(size);
    }
    matrixCopy = matrix;
    init(pattern);
}

void Simulation::set(int x, int y, CellState cellState) {
    incrementalSet(x, y, cellState);
}

void Simulation::incrementalSet(int x, int y, CellState cellState) {
    if (x <= 1 || y <= 1 || x >= m_size - 2 || y >= m_size - 2) {
        return;
    }
    matrix[y][x] = cellState == CellState::ALIVE;

    if (matrixCopy[y][x] != matrix[y][x]) {
        m_changeList.insert({x, y});
    }
}

void Simulation::nextStep() {
    std::copy(matrix.cbegin(), matrix.cend(), matrixCopy.begin());

    std::vector<Point> changeListCopy{m_changeList.cbegin(), m_changeList.cend()};

    m_changeList.clear();

    for (const auto & it : changeListCopy) {
        int x = it.x;
        int y = it.y;
        updateCell(x - 1, y );
        updateCell(x, y);
        updateCell(x + 1, y);
        updateCell(x - 1, y -1);
        updateCell(x, y - 1);
        updateCell(x + 1, y - 1);
        updateCell(x - 1, y + 1);
        updateCell(x, y + 1);
        updateCell(x + 1, y + 1);
    }
}

void Simulation::updateCell(int x, int y) {
    int nbAliveNeighbours = 0;

    nbAliveNeighbours += matrixCopy[y - 1][x - 1] ? 1 : 0;
    nbAliveNeighbours += matrixCopy[y - 1][x] ? 1 : 0;
    nbAliveNeighbours += matrixCopy[y - 1][x + 1] ? 1 : 0;
    nbAliveNeighbours += matrixCopy[y][x - 1] ? 1 : 0;
    bool alive = matrixCopy[y][x];
    nbAliveNeighbours += matrixCopy[y][x + 1] ? 1 : 0;
    nbAliveNeighbours += matrixCopy[y + 1][x - 1] ? 1 : 0;
    nbAliveNeighbours += matrixCopy[y + 1][x] ? 1 : 0;
    nbAliveNeighbours += matrixCopy[y + 1][x + 1] ? 1 : 0;
    if (nbAliveNeighbours == 3) {
        alive = true; // birth
    } else if (alive && nbAliveNeighbours != 2) {
        alive = false; // death;
    }

    incrementalSet(x, y, alive ? CellState::ALIVE : CellState::DEAD);
}

void Simulation::init(const Pattern& pattern) {
    const int yOffset = (m_size - pattern.size().h) / 2;
    const int xOffset = (m_size - pattern.size().w) / 2;
    for (const auto& cell : pattern.aliveCells()) {
        set(cell.x + xOffset, cell.y + yOffset, CellState::ALIVE);
    }
}

}  // namespace app
