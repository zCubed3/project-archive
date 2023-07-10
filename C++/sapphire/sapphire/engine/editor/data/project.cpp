#include "project.h"

#include <core/data/string_tools.h>

void Project::open_project(const std::string &path) {
    config.read_from_path(path);
    project_path = path;

    name = config.try_get_string("sName", "Project", "Unknown Project");
}

std::string Project::get_project_path() const{
    return project_path;
}
