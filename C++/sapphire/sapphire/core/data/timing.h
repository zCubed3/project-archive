#ifndef SAPPHIRE_TIME_H
#define SAPPHIRE_TIME_H

#include <chrono>

class Timing {
protected:
    static Timing *singleton;

    std::chrono::time_point<std::chrono::high_resolution_clock> last;
    double delta;

public:
    static Timing *get_singleton();

    void new_frame();
    double get_delta() const;
};

#endif
