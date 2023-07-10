#ifndef SAPPHIRE_FILE_H
#define SAPPHIRE_FILE_H

#include <string>
#include <vector>

class File {
public:
    enum FileType {
        FILE_TYPE_UNKNOWN,
        FILE_TYPE_FILE,
        FILE_TYPE_FOLDER
    };

    enum FileFlags {
        FILE_FLAG_HIDDEN,
        FILE_FLAG_READONLY
    };

    FileType type = FILE_TYPE_UNKNOWN;
    uint32_t flags = 0;

    std::string path;
    std::string name;

    // Used by folders
    std::vector<File> child_files;
};


#endif
