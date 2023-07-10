#ifndef SAPPHIRE_PAK_FILE_SYSTEM_H
#define SAPPHIRE_PAK_FILE_SYSTEM_H

#include "virtual_file_system.h"

#include <cstdio>

typedef struct pak_header {
    uint32_t ident;
    uint32_t offset;
    uint32_t size;
} pak_header_t;

typedef struct pak_entry {
    char path[56];
    uint32_t offset;
    uint32_t size;
} pak_entry_t;

// Support for a Quake PAK as a file system
class PakFileSystem : public VirtualFileSystem {
protected:
    std::vector<pak_entry_t> entries;
    FILE* file;

public:
    static PakFileSystem* open_pak(const std::string& path);

    void read(const std::string &path) override;
};


#endif
