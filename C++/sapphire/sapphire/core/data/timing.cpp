#include "timing.h"

Timing* Timing::singleton = nullptr;

Timing *Timing::get_singleton() {
    if (singleton != nullptr) {
        return singleton;
    } else {
        singleton = new Timing();
        singleton->last = std::chrono::high_resolution_clock::now();

        return singleton;
    }
}

void Timing::new_frame() {
    auto now = std::chrono::high_resolution_clock::now();

    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now - last);
    delta = ms.count() / 1000.0;

    last = now;
}

double Timing::get_delta() const {
    return delta;
}
