#include "Util.h"

void sleep_millis(int duration) {
    std::this_thread::sleep_for(std::chrono::milliseconds(duration));
}
