#include "angle_math.h"

#include <cmath>

const float AngleMath::PI = 3.141592654F;

float AngleMath::wrap(float x, float min, float max) {
    return x - (max - min) * floor(x / (max - min));
}