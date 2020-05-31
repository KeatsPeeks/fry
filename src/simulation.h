#pragma once

#include <vector>
#include <unordered_set>

namespace app {
    struct pair_hash
    {
        template <class T1, class T2>
        std::size_t operator ()(std::pair<T1, T2> const &pair) const
        {
            return pair.first ^ pair.second;
        }
    };

    class Simulation
    {
    public:
        explicit Simulation(size_t size, const std::vector<std::vector<uint8_t>>& pattern = {});
        [[nodiscard]] bool get(size_t x, size_t y) const { return matrix[y][x]; }
        void set(size_t x, size_t y, bool alive);
        [[nodiscard]] size_t getSize() const { return size; }

        void nextStep();

    private:
        std::vector<std::vector<bool>> matrix;
        std::vector<std::vector<bool>> matrixCopy;
        std::unordered_set<std::pair<size_t, size_t>, pair_hash> changeList;
        const size_t size;

        void init(std::vector<std::vector<uint8_t>> pattern);
    };

}  // namespace app

