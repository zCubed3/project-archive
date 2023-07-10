#include "editor_config.h"

#include <engine/editor/editor.h>

void EditorConfig::load_config(Editor *p_editor) {
    file.set_string("sLastProject", "History", "");

    file.set_string("iOpenLastProject", "Preferences", "1");

    file.read_from_path("config/editor.secf");
    file.write_to_path("config/editor.secf");

    last_project_path = file.try_get_string("sLastProject", "History");

    open_last_project = file.try_get_int("iOpenLastProject", "Preferences");
}

void EditorConfig::save_config(Editor *p_editor) {
    file.set_string("sLastProject", "History", p_editor->project.get_project_path());

    file.set_string("iOpenLastProject", "Preferences", open_last_project ? "1" : "0");

    file.write_to_path("config/editor.secf");
}