#ifndef SAPPHIRE_COLOR_H
#define SAPPHIRE_COLOR_H

#include <cstdlib>

// Wrapper around color types
// Avoids including GLM in whenever color is needed
class Color {
public:
    float backing[4] = {0, 0, 0, 0};

    Color(float r, float g, float b, float a = 1);
    Color(char r, char g, char b, char a);

    // TODO: Can this be made safer?
    float operator[](size_t index) {
        return backing[index];
    }
};

#endif
