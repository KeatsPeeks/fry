
#include <iostream>

#include "main.h"

int main() {
    std::cout << "Build " << BUILD_VERSION;
    for (int i = 0; i < 1000000; i++) {
        // no-op
    }
    return 0;
}
