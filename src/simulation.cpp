#include "simulation.h"
#include <algorithm>

namespace app {

    Simulation::Simulation(int size, const std::vector<std::vector<uint8_t>> &pattern) : size{size} {
        matrix.resize(size);
        for (int i = 0; i < size; ++i) {
            matrix[i].resize(size);
        }

        init(pattern);
        matrixCopy = matrix;
    }

    void Simulation::set(int x, int y, CellState cellState) {
        incrementalSet(x, y, cellState);
        if (cellState == CellState::DEAD) {
            aliveList.erase(std::make_pair(x, y));
        }
    }

    void Simulation::incrementalSet(int x, int y, CellState cellState) {
        if (x <= 1 || y <= 1 || x >= size - 2 || y >= size - 2) {
            return;
        }
        matrix[y][x] = cellState == CellState::ALIVE;
        if (cellState == CellState::ALIVE) {
            aliveList.insert(std::make_pair(x, y));
        }
    }

    void Simulation::nextStep() {
        std::copy(matrix.cbegin(), matrix.cend(), matrixCopy.begin());

        std::vector<std::pair<int, int>> changeListCopy{aliveList.cbegin(), aliveList.cend()};

        aliveList.clear();

        for (const auto & it : changeListCopy) {
            int x = it.first;
            int y = it.second;
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
        nbAliveNeighbours += matrixCopy[y][x + 1] ? 1 : 0;
        nbAliveNeighbours += matrixCopy[y + 1][x - 1] ? 1 : 0;
        nbAliveNeighbours += matrixCopy[y + 1][x] ? 1 : 0;
        nbAliveNeighbours += matrixCopy[y + 1][x + 1] ? 1 : 0;
        bool alive = matrixCopy[y][x];
        if (nbAliveNeighbours == 3) {
            alive = true; // birth
        } else if (alive && nbAliveNeighbours != 2) {
            alive = false; // death;
        }
        incrementalSet(x, y, alive ? CellState::ALIVE : CellState::DEAD);
    }

    void Simulation::init(std::vector<std::vector<uint8_t>> pattern) {
        int height = static_cast<int>(pattern.size());
        const int yOffset = (size - height) / 2;
        for (int y = 0; y < height; ++y) {
            const auto& row = pattern[y];
            int width = static_cast<int>(row.size());
            const int xOffset = (size - width) / 2;
            for (int x = 0; x < width; ++x) {
                set(x + xOffset, y + yOffset, row[x] == 1 ? CellState::ALIVE : CellState::DEAD);
            }
        }
    }

}  // namespace app
