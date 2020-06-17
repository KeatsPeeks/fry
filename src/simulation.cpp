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
    init(pattern);
}

void Simulation::set(int x, int y, CellState cellState) {
    incrementalSet({cellState, x, y});
    matrix[y][x] = cellState;
}

void Simulation::incrementalSet(const Cell& p) {
    if (p.x <= 1 || p.y <= 1 || p.x >= m_size - 2 || p.y >= m_size - 2) {
        return;
    }

    if (matrix[p.y][p.x] != p.alive) {
        m_changeList.insert(p);
    }
}

void Simulation::nextStep() {
    std::vector<Cell> changeListCopy{m_changeList.cbegin(), m_changeList.cend()};

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

    for (const Cell& p : m_changeList) {
        matrix[p.y][p.x] = p.alive;
    }
}

void Simulation::updateCell(int x, int y) {
    int nbAliveNeighbours = 0;

    nbAliveNeighbours += matrix[y - 1][x - 1];
    nbAliveNeighbours += matrix[y - 1][x];
    nbAliveNeighbours += matrix[y - 1][x + 1];
    nbAliveNeighbours += matrix[y][x - 1];
    CellState state = matrix[y][x];
    nbAliveNeighbours += matrix[y][x + 1];
    nbAliveNeighbours += matrix[y + 1][x - 1];
    nbAliveNeighbours += matrix[y + 1][x];
    nbAliveNeighbours += matrix[y + 1][x + 1];
    if (nbAliveNeighbours == 3) {
        state = ALIVE; // birth
    } else if (state == ALIVE && nbAliveNeighbours != 2) {
        state = DEAD; // death;
    }

    incrementalSet({state, x, y});
}

void Simulation::init(const Pattern& pattern) {
    const int yOffset = (m_size - pattern.size().h) / 2;
    const int xOffset = (m_size - pattern.size().w) / 2;
    for (const auto& cell : pattern.aliveCells()) {
        set(cell.x + xOffset, cell.y + yOffset, CellState::ALIVE);
    }
}

}  // namespace app
