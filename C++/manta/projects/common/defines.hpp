#ifndef MANTA_COMMON_DEFINES_HPP
#define MANTA_COMMON_DEFINES_HPP

#ifdef __linux__
#define EXPORT extern "C"
#define OS_PLATFORM "Linux"
#endif

#ifdef WIN32
#define EXPORT extern "C" __declspec(dllexport)
#define OS_PLATFORM "Windows"
#endif

#endif
