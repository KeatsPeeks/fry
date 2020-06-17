#include "simulation.h"
#include <algorithm>

namespace app {

Simulation::Simulation(int size, const Pattern& pattern) : m_size{size} {
    matrix.resize(size * size);
    init(pattern);
}

void Simulation::set(int x, int y, CellState cellState) {
    incrementalSet(y * m_size + x, cellState);
    matrix[y * m_size + x] = cellState;
}

void Simulation::incrementalSet(size_t index, CellState cellState) {
    auto x = index % m_size;
    auto y = index / m_size;
    if (x <= 1 || y <= 1 || x >= m_size - 2 || y >= m_size - 2) {
        return;
    }

    if (matrix[index] != cellState) {
        m_changeList.insert(index);
    }
}

void Simulation::nextStep() {
    std::vector<size_t> changeListCopy{m_changeList.cbegin(), m_changeList.cend()};

    m_changeList.clear();

    for (const size_t index : changeListCopy) {
        for (int i = -1; i <= 1; i++) {
            updateCell(index - 1 + i * m_size);
            updateCell(index + i * m_size);
            updateCell(index + 1 + i * m_size);
        }
    }

    for (size_t p : m_changeList) {
        matrix[p] = matrix[p] == ALIVE ? DEAD : ALIVE;
    }
}

void Simulation::updateCell(const size_t index) {
    int nbAliveNeighbours = 0;

    size_t i = index - m_size;
    nbAliveNeighbours += matrix[i - 1];
    nbAliveNeighbours += matrix[i];
    nbAliveNeighbours += matrix[i + 1];
    i += m_size;
    nbAliveNeighbours += matrix[i - 1];
    CellState state = matrix[i];
    nbAliveNeighbours += matrix[i + 1];
    i += m_size;
    nbAliveNeighbours += matrix[i - 1];
    nbAliveNeighbours += matrix[i];
    nbAliveNeighbours += matrix[i + 1];
    if (nbAliveNeighbours == 3) {
        state = ALIVE; // birth
    } else if (state == ALIVE && nbAliveNeighbours != 2) {
        state = DEAD; // death;
    }

    incrementalSet(index, state);
}

void Simulation::init(const Pattern& pattern) {
    const int yOffset = (m_size - pattern.size().h) / 2;
    const int xOffset = (m_size - pattern.size().w) / 2;
    for (const auto& cell : pattern.aliveCells()) {
        set(cell.x + xOffset, cell.y + yOffset, CellState::ALIVE);
    }
}

}  // namespace app
