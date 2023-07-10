#include "pak_file_system.h"

#include <fstream>

// PACK as hex (backwards)
const uint32_t PAK_IDENT = 0x4B434150;

PakFileSystem *PakFileSystem::open_pak(const std::string &path) {
    // TODO: Allow writing back a new Pak?
    FILE* file = fopen(path.c_str(), "rb");

    if (file == nullptr) {
        return nullptr;
    }

    pak_header_t header;
    fread(&header, sizeof(pak_header_t), 1, file);

    if (header.ident != PAK_IDENT) {
        return nullptr;
    }

    size_t entry_count = header.size / sizeof(pak_entry_t);

    fseek(file, header.offset, SEEK_SET);

    std::vector<pak_entry_t> entries;
    entries.resize(entry_count);

    fread(entries.data(), header.size, 1, file);

    PakFileSystem *fs = new PakFileSystem();
    fs->entries = entries;
    fs->file = file;

    return fs;
}

void PakFileSystem::read(const std::string &path) {

}
