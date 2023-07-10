#include "color.h"

Color::Color(float r, float g, float b, float a) {
    backing[0] = r;
    backing[1] = g;
    backing[2] = b;
    backing[3] = a;
}

Color::Color(char r, char g, char b, char a) {
    backing[0] = (float) r / 255.0F;
    backing[1] = (float) g / 255.0F;
    backing[2] = (float) b / 255.0F;
    backing[3] = (float) a / 255.0F;
}