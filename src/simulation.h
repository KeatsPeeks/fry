#pragma once

#include <vector>
#include <array>
#include <unordered_set>
#include <memory_resource>

namespace app {
    struct pair_hash
    {
        template <class T1, class T2>
        std::size_t operator ()(std::pair<T1, T2> const &pair) const
        {
            return pair.first ^ pair.second;
        }
    };

    const static int BUF_SIZE = 1024;

    class Simulation
    {
    public:
        explicit Simulation(size_t size, const std::vector<std::vector<uint8_t>>& pattern = {});
        [[nodiscard]] bool get(size_t x, size_t y) const { return matrix[y][x]; }
        void set(size_t x, size_t y, bool alive);
        [[nodiscard]] size_t getSize() const { return size; }

        void nextStep();

    private:
        const size_t size;

        std::vector<std::vector<bool>> matrix;
        std::vector<std::vector<bool>> matrixCopy;

        std::pmr::unsynchronized_pool_resource mbr{};
        std::pmr::unordered_set<std::pair<size_t, size_t>, pair_hash> changeList{&mbr};

        void init(std::vector<std::vector<uint8_t>> pattern);
    };

}  // namespace app

