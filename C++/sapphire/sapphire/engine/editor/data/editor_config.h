#ifndef SAPPHIRE_EDITOR_CONFIG_H
#define SAPPHIRE_EDITOR_CONFIG_H

#include <core/config/config_file.h>

#include <string>

class Editor;

class EditorConfig {
protected:
    ConfigFile file;

public:
    std::string last_project_path;

    bool open_last_project;

    void load_config(Editor *p_editor);
    void save_config(Editor *p_editor);
};


#endif
