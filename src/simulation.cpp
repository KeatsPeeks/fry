#include "simulation.h"
#include <algorithm>

namespace app {

Simulation::Simulation(Size size, const Pattern& pattern) : m_size{size} {
    matrix.resize(size.w * size.h);
    init(pattern);
}

void Simulation::set(int x, int y, CellState cellState) {
    updateChangeList(y * m_size.w + x, cellState);
    matrix[y * m_size.w + x] = cellState;
}

void Simulation::updateChangeList(int index, CellState cellState) {
    auto x = index % m_size.w;
    auto y = index / m_size.w;
    if (x <= 1 || y <= 1 || x >= m_size.w - 2 || y >= m_size.h - 2) {
        return;
    }

    if (matrix[index] != cellState) {
        writeChangeList->insert(index);
    }
}

void Simulation::nextStep() {
    std::swap(readChangeList, writeChangeList);
    writeChangeList->clear();
    for (const int index : *readChangeList) {
        // (this often does the calculations more than once on the same cell, but it's still faster than preventing it with a set)
        for (int i = -1; i <= 1; i++) {
            updateCell(index - 1 + i * m_size.w);
            updateCell(index + i * m_size.w);
            updateCell(index + 1 + i * m_size.w);
        }
    }

    for (int p : *writeChangeList) {
        matrix[p] = matrix[p] == ALIVE ? DEAD : ALIVE;
    }
}

void Simulation::updateCell(const int index) {
    int nbAliveNeighbours = 0;

    size_t i = index - m_size.w;
    nbAliveNeighbours += matrix[i - 1];
    nbAliveNeighbours += matrix[i];
    nbAliveNeighbours += matrix[i + 1];
    i += m_size.w;
    nbAliveNeighbours += matrix[i - 1];
    CellState state = matrix[i];
    nbAliveNeighbours += matrix[i + 1];
    i += m_size.w;
    nbAliveNeighbours += matrix[i - 1];
    nbAliveNeighbours += matrix[i];
    nbAliveNeighbours += matrix[i + 1];
    if (nbAliveNeighbours == 3) {
        state = ALIVE; // birth
    } else if (state == ALIVE && nbAliveNeighbours != 2) {
        state = DEAD; // death;
    }

    updateChangeList(index, state);
}

void Simulation::init(const Pattern& pattern) {
    const int yOffset = (m_size.h - pattern.size().h) / 2;
    const int xOffset = (m_size.w - pattern.size().w) / 2;
    for (const auto& cell : pattern.aliveCells()) {
        set(cell.x + xOffset, cell.y + yOffset, CellState::ALIVE);
    }
}

}  // namespace app
