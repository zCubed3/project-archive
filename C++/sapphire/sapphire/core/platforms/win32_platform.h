#ifndef SAPPHIRE_WIN32_PLATFORM_H
#define SAPPHIRE_WIN32_PLATFORM_H

#include <core/platforms/platform.h>

class Win32Platform : public Platform {
private:
    Win32Platform() = default;

public:
    static const Win32Platform* create_win32_platform();

    std::string get_name() const override;

    bool create_folder(const std::string &path, bool create_parents = true) const override;

    bool file_exists(const std::string &path) const override;
    bool folder_exists(const std::string &path) const override;

    std::vector<File> get_files(const std::string &folder) const override;

    bool set_console_color(ConsoleColor color) const override;
};


#endif
