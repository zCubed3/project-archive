#include "actor.h"

#include <random>

#include <math/angle_math.h>

#include <gtc/type_ptr.hpp>

#if defined(IMGUI_SUPPORT)
#include <imgui.h>
#include <misc/cpp/imgui_stdlib.h>
#endif

// TODO: Better generate IDs
Actor::Actor() {
    std::random_device rd;
    auto engine = std::default_random_engine(rd());

    id = engine();
}

Actor::~Actor() {

}

void Actor::tick(World *p_world) {
    for (Actor *child: children) {
        if (child != nullptr) {
            child->tick(p_world);
        }
    }
}

void Actor::draw(World *p_world) {
    for (Actor *child: children) {
        if (child != nullptr) {
            child->draw(p_world);
        }
    }
}

void Actor::on_enter_world(World *p_world) {
    for (Actor *child: children) {
        if (child != nullptr) {
            child->on_enter_world(p_world);
        }
    }
}

void Actor::on_exit_world(World *p_world) {
    for (Actor *child: children) {
        if (child != nullptr) {
            child->on_exit_world(p_world);
        }
    }
}

#if defined(IMGUI_SUPPORT)
void Actor::draw_imgui(World *p_world) {
    if (ImGui::CollapsingHeader("Actor")) {
        ImGui::InputText("Name", &name);

        ImGui::Text("ID: %zu", id);
    }

    if (ImGui::CollapsingHeader("Transform")) {
        glm::vec3 euler = glm::degrees(glm::eulerAngles(transform.quaternion));

        ImGui::DragFloat3("Position", glm::value_ptr(transform.position), 0.02F);
        ImGui::DragFloat3("Euler", glm::value_ptr(euler), 0.02F);
        ImGui::DragFloat3("Scale", glm::value_ptr(transform.scale), 0.02F);

        euler = glm::radians(euler);

        transform.quaternion = glm::quat(euler);
    }
}
#endif