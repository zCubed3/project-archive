#ifndef SAPPHIRE_VIRTUAL_FILE_SYSTEM_H
#define SAPPHIRE_VIRTUAL_FILE_SYSTEM_H

#include <data/file.h>

#include <vector>
#include <string>

// Defines a "Virtual File System" that can be used similar to a platform's raw file access
// But instead accesses various archive types
class VirtualFileSystem {
public:
    virtual void read(const std::string& path) = 0;
};


#endif
