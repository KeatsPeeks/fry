#include <vector>

namespace app {
    struct Patterns {
        static std::vector<std::vector<uint8_t>> acorn() {
            return {
                { 0, 1, 0, 0, 0, 0, 0 },
                { 0, 0, 0, 1, 0, 0, 0 },
                { 1, 1, 0, 0, 1, 1, 1 },
            };
        }

        static std::vector<std::vector<uint8_t>> infinite() {
            return {
                {1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, },
            };
        }
    };

}  // namespace app