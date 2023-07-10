#include "platform.h"

Platform *Platform::singleton = nullptr;

const Platform *Platform::get_singleton() {
    return singleton;
}