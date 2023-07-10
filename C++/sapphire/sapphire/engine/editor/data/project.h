#ifndef SAPPHIRE_PROJECT_H
#define SAPPHIRE_PROJECT_H

#include <core/config/config_file.h>

class Project {
protected:
    std::string project_path;
    ConfigFile config;

public:
    std::string name;

    void open_project(const std::string& path);

    std::string get_project_path() const;
};


#endif
