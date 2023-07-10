#ifndef SAPPHIRE_PLATFORM_H
#define SAPPHIRE_PLATFORM_H

#include <string>
#include <vector>

#include <core/data/file.h>

// Describes a supported OS (aka Platform)
// Also provides interfaces for certain OS features
class Platform {
protected:
    static Platform* singleton;

public:
    enum ConsoleColor {
        CONSOLE_COLOR_WHITE,
        CONSOLE_COLOR_YELLOW,
        // TODO: More
    };

    static const Platform *get_singleton();

    virtual std::string get_name() const = 0;

    virtual bool create_folder(const std::string &path, bool create_parents = true) const = 0;

    virtual bool file_exists(const std::string &path) const = 0;
    virtual bool folder_exists(const std::string &path) const = 0;

    virtual std::vector<File> get_files(const std::string &folder) const = 0;

    virtual bool set_console_color(ConsoleColor color) const = 0;
};


#endif
