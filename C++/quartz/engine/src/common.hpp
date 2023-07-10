#pragma once

#ifdef WINDOWS
#define WIN32_LEAN_AND_MEAN
#endif

// Just name some basic engine parameters here
#define GAME_NAME "Quartz"

#define LOG std::cout << "[Log]: "
#define LOG_WARN std::cout << "[Warning]: "
#define LOG_ERROR std::cout << "[Error]: "

#define LOG_RENDERER std::cout << "[Renderer]: "
#define LOG_COMMAND std::cout << "[Command]: "

#define DEFAULT_WIN_WIDTH 1280
#define DEFAULT_WIN_HEIGHT 720

#define RGB_TO_LINEAR(RGB) (RGB / 255.0f)
#define RGB_TO_LINEAR_VEC4(TYPE, R, G, B, A) TYPE(R / 255.0f, G / 255.0f, B / 255.0f, A / 255.0f)
