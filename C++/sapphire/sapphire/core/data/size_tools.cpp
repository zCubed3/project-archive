#include "size_tools.h"

size_t SizeTools::bytes_to_kib(size_t bytes) {
    return bytes / 1024;
}

size_t SizeTools::bytes_to_mib(size_t bytes) {
    return bytes / 1048576;
}

size_t SizeTools::bytes_to_gib(size_t bytes) {
    return bytes / 1073741824;
}

size_t SizeTools::kib_to_bytes(size_t kib) {
    return kib * 1024;
}

size_t SizeTools::mib_to_bytes(size_t mib) {
    return mib * 1048576;
}

size_t SizeTools::gib_to_bytes(size_t gib) {
    return gib * 1073741824;
}

