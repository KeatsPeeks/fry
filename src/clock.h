#pragma once

#include <chrono>
#include <spdlog/spdlog.h>

namespace app {

    struct GameTime {
        const std::chrono::duration<double> totalTime{};
        const std::chrono::duration<double> elapsedTime{};
    };

    using std::chrono::time_point;
    using std::chrono::steady_clock;

    class GameClock {

    public:
        GameTime update() {
            auto now = steady_clock::now();
            GameTime gameTime{now - epoch, now - last};
            last = now;
            return gameTime;
        }

    private:
        const time_point<steady_clock> epoch{steady_clock::now()};
        time_point<steady_clock> last{steady_clock::now()};
    };

}  // namespace app

