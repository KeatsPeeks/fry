#pragma once

#include <array>
#include <cstdint>
#include <unordered_set>
#include <vector>
#ifdef ENABLE_PMR
#include <memory_resource>
#endif

namespace app {
    struct pair_hash
    {
        template <class T1, class T2>
        int operator ()(std::pair<T1, T2> const &pair) const
        {
            return pair.first ^ pair.second;
        }
    };

    const static int BUF_SIZE = 1024;

    class Simulation
    {
    public:
        explicit Simulation(int size, const std::vector<std::vector<uint8_t>>& pattern = {});
        [[nodiscard]] bool get(int x, int y) const { return matrix[y][x]; }
        void set(int x, int y, bool alive);
        [[nodiscard]] int getSize() const { return size; }

        void nextStep();

    private:
        const int size;

        std::vector<std::vector<bool>> matrix;
        std::vector<std::vector<bool>> matrixCopy;

#ifdef ENABLE_PMR
        std::pmr::unsynchronized_pool_resource pool{};
        std::pmr::unordered_set<std::pair<int, int>, pair_hash> changeList{&pool};
#else
        std::unordered_set<std::pair<size_t, size_t>, pair_hash> changeList{};
#endif

        void init(std::vector<std::vector<uint8_t>> pattern);

        void updateCell(int x, int y);
    };

}  // namespace app

