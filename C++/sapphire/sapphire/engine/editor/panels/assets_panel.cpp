#include "assets_panel.h"

#include <imgui.h>

#include <engine/assets/asset_loader.h>

const char *AssetsPanel::get_title() {
    return "Assets";
}

void AssetsPanel::draw_file_recursive(File &file) {
    if (file.type == File::FILE_TYPE_FOLDER) {
        int flags = ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_OpenOnArrow;
        bool open = ImGui::TreeNodeEx(file.name.c_str(), flags);

        if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen()) {
            file.child_files = Platform::get_singleton()->get_files(file.path);
        }

        if (open) {
            for (File& child_file: file.child_files) {
                draw_file_recursive(child_file);
            }

            ImGui::TreePop();
        }
    }

    if (file.type == File::FILE_TYPE_FILE) {
        ImGui::Selectable(file.name.c_str());
    }
}

void AssetsPanel::draw_contents(Engine *p_engine) {
    if (ImGui::Button("Refresh")) {
        // TODO: Make this relative to a project
        files = Platform::get_singleton()->get_files("./");
    }

    // TODO: Also move this to its own panel
    for (auto pair: ClassRegistry::class_map) {
        if (ImGui::TreeNode(pair.second.name)) {
            ImGui::BulletText("Hash = %zu", pair.second.hash);

            if (pair.second.parent_hash != CLASS_REGISTRY_INVALID_HASH) {
                if (ImGui::TreeNode("Parent")) {
                    ClassRegistry::ClassEntry entry = ClassRegistry::get_type_parent(pair.second.parent_hash);

                    if (entry.hash != -1) {
                        ImGui::BulletText("Parent Hash = %zu", entry.hash);
                        ImGui::BulletText("%s", entry.name);
                    }

                    ImGui::TreePop();
                }
            } else {
                ImGui::BulletText("Is base class!");
            }

            ImGui::TreePop();
        }
    }

    ImGui::Spacing();

    // TODO: Move this to its own panel
    for (AssetLoader* loader: AssetLoader::loaders) {
        if (ImGui::TreeNode(loader->get_class_name())) {
            for (const auto &pair: loader->asset_cache) {
                ImGui::Text("%s", pair.first.c_str());
            }

            ImGui::TreePop();
        }
    }

    for (File& file: files) {
        if (file.type == File::FILE_TYPE_FOLDER) {
            draw_file_recursive(file);
        }
    }

    for (File& file: files) {
        if (file.type == File::FILE_TYPE_FILE) {
            draw_file_recursive(file);
        }
    }
}