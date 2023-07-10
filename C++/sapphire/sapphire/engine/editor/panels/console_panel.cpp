#include "console_panel.h"

#if defined(IMGUI_SUPPORT)

#include <engine/console/console.h>

#include <imgui.h>
#include <misc/cpp/imgui_stdlib.h>

#include <engine/rendering/render_server.h>
#include <core/data/timing.h>

const char *ConsolePanel::get_title() {
    return "Console";
}

void ConsolePanel::draw_contents(Engine* p_engine) {
    if (ImGui::Button("Ignores")) {
        ImGui::OpenPopup("Severities");
    }

    ImGui::SameLine();

    if (ImGui::Button("Clear")) {
        Console::console_cout.clear();
    }

    ImGui::SameLine();

    ImGui::PushItemWidth(-1);
    if (ImGui::InputText("##buffer", &buffer, ImGuiInputTextFlags_EnterReturnsTrue)) {
        // Split the arguments by space
        std::vector<std::string> arguments;

        std::string arg_buffer;
        for (char c: buffer) {
            if (c == ' ') {
                if (!arg_buffer.empty()) {
                    arguments.push_back(arg_buffer);
                    arg_buffer.clear();
                }

                continue;
            }

            arg_buffer += c;
        }

        if (!arg_buffer.empty()) {
            arguments.push_back(arg_buffer);
        }

        Console::log("> " + buffer);

        buffer.clear();
        Console::execute(arguments);
    }

    ImGui::PopItemWidth();

    ImGui::BeginChild("ConsoleLog", {0, 0}, true);
    for (ConsoleStream::ConsoleMessage& msg: Console::console_cout.message_queue) {
        if (ignore_severities[msg.severity]) {
            continue;
        }

        switch (msg.severity) {
            case ConsoleStream::MESSAGE_SEVERITY_NONE:
                break;

            case ConsoleStream::MESSAGE_SEVERITY_WARNING:
                ImGui::PushStyleColor(ImGuiCol_Text, {1.0F, 0.921F, 0.231F, 1.0F});
                break;

            case ConsoleStream::MESSAGE_SEVERITY_ERROR:
                ImGui::PushStyleColor(ImGuiCol_Text, {0.776F, 0.156F, 0.156F, 1.0F});
                break;
        }

        ImGui::Text("%s", msg.message.c_str());

        switch (msg.severity) {
            case ConsoleStream::MESSAGE_SEVERITY_NONE:
                break;

            case ConsoleStream::MESSAGE_SEVERITY_WARNING:
            case ConsoleStream::MESSAGE_SEVERITY_ERROR:
                ImGui::PopStyleColor();
                break;
        }
    }
    ImGui::EndChild();

    if (ImGui::BeginPopup("Severities")) {
        for (int i = 0; i < 3; i++) {
            const char* name;
            switch (i) {
                case 0:
                    name = "Messages";
                    break;

                case 1:
                    name = "Warnings";
                    break;

                case 2:
                    name = "Errors";
                    break;
            }

            ImGui::Checkbox(name, &ignore_severities[i]);
        }

        ImGui::EndPopup();
    }
}
#endif