#ifndef MANTA_FMATH_HPP
#define MANTA_FMATH_HPP

namespace Manta::FMath {
    const float RAD_2_DEG = 57.2957795131f;
    const float DEG_2_RAD = 0.01745329251f;

    inline float Max(float a, float b) {
        return (a > b ? a : b);
    }

    inline float Min(float a, float b) {
        return (a > b ? b : a);
    }

    inline float Clamp(float a, float min, float max) {
        return Max(min, Min(max, a));
    }
}

#endif
