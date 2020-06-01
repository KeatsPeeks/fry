#include "simulation.h"
#include <algorithm>
#include <execution>

namespace app {

    Simulation::Simulation(size_t size, const std::vector<std::vector<uint8_t>> &pattern) : size{size} {
        matrix.resize(size);
        for (size_t i = 0; i < size; ++i) {
            matrix[i].resize(size);
        }

        init(pattern);
        matrixCopy = matrix;
    }

    void Simulation::set(size_t x, size_t y, bool alive) {
        matrix[y][x] = alive;
        if (alive && x > 1 && x < size - 2 && y > 1 && y < size - 2) {
            changeList.insert(std::make_pair(x - 1, y - 1));
            changeList.insert(std::make_pair(x - 1, y));
            changeList.insert(std::make_pair(x - 1, y + 1));
            changeList.insert(std::make_pair(x, y - 1));
            changeList.insert(std::make_pair(x, y));
            changeList.insert(std::make_pair(x, y + 1));
            changeList.insert(std::make_pair(x + 1, y - 1));
            changeList.insert(std::make_pair(x + 1, y));
            changeList.insert(std::make_pair(x + 1, y + 1));
        }
    }

    void Simulation::nextStep() {
        std::copy(matrix.cbegin(), matrix.cend(), matrixCopy.begin());

        std::vector<std::pair<size_t, size_t>> changeListCopy(changeList.cbegin(), changeList.cend());

        changeList.clear();

        for (const auto & it : changeListCopy) {

            size_t x = it.first;
            size_t y = it.second;
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
            if (!alive && nbAliveNeighbours == 3) {
                alive = true; // birth
            }
            else if (alive && nbAliveNeighbours != 2 && nbAliveNeighbours != 3) {
                alive = false; // death;
            }
            set(x, y, alive);
        }
    }

    void Simulation::init(std::vector<std::vector<uint8_t>> pattern) {
        const size_t yOffset = (size - pattern.size()) / 2;
        for (size_t y = 0; y < pattern.size(); ++y) {
            const auto& row = pattern[y];
            const size_t xOffset = (size - row.size()) / 2;
            for (size_t x = 0; x < row.size(); ++x) {
                set(x+xOffset, y + yOffset, row[x] == 1);
            }
        }
    }

}  // namespace app
