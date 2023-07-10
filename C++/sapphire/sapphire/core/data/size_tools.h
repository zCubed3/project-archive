#ifndef SAPPHIRE_SIZE_TOOLS_H
#define SAPPHIRE_SIZE_TOOLS_H


class SizeTools {
public:
    SizeTools() = delete;

    static size_t bytes_to_kib(size_t bytes);
    static size_t bytes_to_mib(size_t bytes);
    static size_t bytes_to_gib(size_t bytes);

    static size_t kib_to_bytes(size_t kib);
    static size_t mib_to_bytes(size_t mib);
    static size_t gib_to_bytes(size_t gib);
};


#endif
