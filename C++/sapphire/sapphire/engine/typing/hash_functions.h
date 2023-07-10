#ifndef SAPPHIRE_HASH_FUNCTIONS_H
#define SAPPHIRE_HASH_FUNCTIONS_H

#include <cstdlib>

// http://www.cse.yorku.ca/~oz/hash.html
constexpr size_t hash_cstr_djb2(const char* string) {
    if (string == nullptr) {
        return -1;
    }

    size_t hash = 0;
    char c = 0;
    while ((c = *string++)) {
        hash = ((hash << 5) + hash) + c;
    }

    return hash;
}

#endif
