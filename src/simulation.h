#pragma once

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

    struct pair_hash
    {
        template <class T1, class T2>
        int operator ()(std::pair<T1, T2> const &pair) const
        {
            return pair.first ^ pair.second;
        }
    };

    class Simulation
    {
#ifdef ENABLE_PMR
    public:
        using TAliveList = std::pmr::unordered_set<std::pair<int, int>, pair_hash>;
    private:
        static std::pmr::unsynchronized_pool_resource pool;
        TAliveList aliveList{&pool};
#else
    public:
        using TAliveList = std::unordered_set<std::pair<size_t, size_t>, pair_hash>;
    private:
        TAliveList aliveList{};
#endif

    public:
        explicit Simulation(int size, const std::vector<std::vector<uint8_t>>& pattern = {});

        [[nodiscard]] bool get(int x, int y) const { return matrix[y][x]; }
        void set(int x, int y, CellState cellState);
        [[nodiscard]] int getSize() const { return size; }

        void nextStep();
        [[nodiscard]] const TAliveList& getAliveCells() const { return aliveList; }

    private:
        int size;

        std::vector<std::vector<bool>> matrix;
        std::vector<std::vector<bool>> matrixCopy;

        void init(std::vector<std::vector<uint8_t>> pattern);

        void updateCell(int x, int y);

        void incrementalSet(int x, int y, CellState cellState);
    };

}  // namespace app

