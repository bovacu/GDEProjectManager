// Created by borja on 23/2/22.

#include <thread>
#include "ProjectInstallator.h"
#include <imgui.h>
#include <core/Engine.h>
#include <core/systems/fileSystem/FilesSystem.h>
#include "ProjectManagerLayer.h"
#include <imgui_internal.h>
#include <fstream>
#include "Macros.h"

namespace engine {


    void ProjectInstallator::init(imgui_addons::ImGuiFileBrowser* _fileBrowser, GlobalConfig* _globalConfig, ProjectManagerLayer* _projectManagerLayer) {
        fileBrowser = _fileBrowser;
        globalConfig = _globalConfig;
        projectManagerLayer = _projectManagerLayer;
    }

    static void asyncInstaller(std::string* installStep, float* installPercentage, const std::string& _path) {
        auto _command = APPEND_S("./installer.sh ", _path);
        std::system(_command.c_str());
        *installPercentage = 0.3f;

        _command = APPEND_S("./installationSteps/install.sh ", _path);
        *installStep = "Installing dependencies...";
        std::system(_command.c_str());
        *installPercentage = 0.6f;

        _command = APPEND_S("./installationSteps/setupAndroid.sh ", _path);
        *installStep = "Installing submodules...";
        std::system(_command.c_str());
        *installPercentage = 0.8f;

        _command = APPEND_S("./installationSteps/6.sh ", _path);
        *installStep = "Installing and building freetype...";
        std::system(_command.c_str());
        *installPercentage = 1.f;

//        _command = APPEND_S("./installationSteps/2.sh ", _path);
//        *installStep = "Installing and building SDL...";
//        std::system(_command.c_str());
//        *installPercentage = 0.6f;
//
//        _command = APPEND_S("./installationSteps/3.sh ", _path);
//        *installStep = "Installing and building SDL_image...";
//        std::system(_command.c_str());
//        *installPercentage = 0.7f;
//
//        _command = APPEND_S("./installationSteps/4.sh ", _path);
//        *installStep = "Installing and building SDL_mixer...";
//        std::system(_command.c_str());
//        *installPercentage = 0.8f;
//
//        _command = APPEND_S("./installationSteps/5.sh ", _path);
//        *installStep = "Installing and building SDL_net...";
//        std::system(_command.c_str());
//        *installPercentage = 0.9f;
//
    }

    void ProjectInstallator::render() {
        if(!show) return;
        static float _installWidth = 0, _cancelWidth = 0;

        INIT_CENTERED_WINDOW_WITH_SIZE(ImGui::GetIO().DisplaySize.x * 0.5f, ImGui::GetIO().DisplaySize.y * 0.5f)

        if(!showLoading) {
            INIT_MODAL("Installation")
            if(ImGui::BeginPopupModal("Installation", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoMove)) {
                ImGui::Text("Path to install"); ImGui::SameLine();
                ImGui::InputText("###path", pathToInstallGDE, IM_ARRAYSIZE(pathToInstallGDE)); ImGui::SameLine();

                if(ImGui::Button("Open...")) {
                    ImGui::OpenPopup("FileBrowser");
                }

                showError();

                ImGui::SetCursorPosX(ImGui::GetWindowWidth() / 2.f - (_installWidth + _cancelWidth));
                if(ImGui::Button("Install")) {
                    checkError();
                    if(error == ProjectError::NONE) {
                        auto* _handler = FilesSystem::createFile("assets/data.json");
                        auto _gdePath = APPEND_S(pathToInstallGDE, "/GDE");
                        auto _line = APPEND_S("GDE_path=", _gdePath);
                        FilesSystem::appendChunkToFileAtEnd(_handler, _line);
                        std::thread _asyncInstallation(asyncInstaller, &installationStep, &installationPercentage, pathToInstallGDE);
                        _asyncInstallation.detach();
                        showLoading = true;
                        FilesSystem::close(_handler);
                    }
                }
                _installWidth = ImGui::GetItemRectSize().x / 2.f;

                ImGui::SameLine();

                if(ImGui::Button("Cancel")) {
                    Engine::get().setRunning(false);
                }

                _cancelWidth = ImGui::GetItemRectSize().x / 2.f;

                Vec2F _size = {(float)Engine::get().getWindowSize().x, (float)Engine::get().getWindowSize().y};
                BROWSER_SELECT(fileBrowser, "FileBrowser", {
                    strcpy(pathToInstallGDE, fileBrowser->selected_path.c_str());
                })

                ImGui::EndPopup();
            }
        }

        END_CENTERED_WINDOW

        if(showLoading) {
            INIT_CENTERED_WINDOW_WITH_SIZE(ImGui::GetIO().DisplaySize.x * 0.5f, ImGui::GetIO().DisplaySize.y * 0.5f)
            INIT_MODAL("ProgressInstallation")
            if(ImGui::BeginPopupModal("ProgressInstallation")) {
                ImGui::Text("%s", installationStep.c_str());
                const ImU32 col = ImGui::GetColorU32(ImGuiCol_ButtonHovered);
                const ImU32 bg = ImGui::GetColorU32(ImGuiCol_Button);

                BufferingBar("##buffer_bar", installationPercentage, ImVec2(400, 6), bg, col);
                ImGui::EndPopup();
            }

            END_CENTERED_WINDOW

            if(installationPercentage == 1.f) {
                showLoading = false;
                show = false;
                projectManagerLayer->onInit();
            }
        }
    }

    void ProjectInstallator::setShow(bool _show) {
        show = _show;
    }

    bool ProjectInstallator::BufferingBar(const char* label, float value,  const ImVec2& size_arg, const ImU32& bg_col, const ImU32& fg_col) {
        ImGuiWindow* window = ImGui::GetCurrentWindow();
        if (window->SkipItems)
            return false;

        ImGuiContext& g = *GImGui;
        const ImGuiStyle& style = g.Style;
        const ImGuiID id = window->GetID(label);

        ImVec2 pos = window->DC.CursorPos;
        ImVec2 size = size_arg;
        size.x -= style.FramePadding.x * 2;

        const ImRect bb(pos, ImVec2(pos.x + size.x, pos.y + size.y));
        ImGui::ItemSize(bb, style.FramePadding.y);
        if (!ImGui::ItemAdd(bb, id))
            return false;

        // Render
        const float circleStart = size.x * 0.7f;
        const float circleEnd = size.x;
        const float circleWidth = circleEnd - circleStart;

        window->DrawList->AddRectFilled(bb.Min, ImVec2(pos.x + circleStart, bb.Max.y), bg_col);
        window->DrawList->AddRectFilled(bb.Min, ImVec2(pos.x + circleStart*value, bb.Max.y), fg_col);

        const float t = g.Time;
        const float r = size.y / 2;
        const float speed = 1.5f;

        const float a = speed*0;
        const float b = speed*0.333f;
        const float c = speed*0.666f;

        const float o1 = (circleWidth+r) * (t+a - speed * (int)((t+a) / speed)) / speed;
        const float o2 = (circleWidth+r) * (t+b - speed * (int)((t+b) / speed)) / speed;
        const float o3 = (circleWidth+r) * (t+c - speed * (int)((t+c) / speed)) / speed;

        window->DrawList->AddCircleFilled(ImVec2(pos.x + circleEnd - o1, bb.Min.y + r), r, bg_col);
        window->DrawList->AddCircleFilled(ImVec2(pos.x + circleEnd - o2, bb.Min.y + r), r, bg_col);
        window->DrawList->AddCircleFilled(ImVec2(pos.x + circleEnd - o3, bb.Min.y + r), r, bg_col);

        return true;
    }

    bool ProjectInstallator::Spinner(const char* label, float radius, int thickness, const ImU32& color) {
        ImGuiWindow* window = ImGui::GetCurrentWindow();
        if (window->SkipItems)
            return false;

        ImGuiContext& g = *GImGui;
        const ImGuiStyle& style = g.Style;
        const ImGuiID id = window->GetID(label);

        ImVec2 pos = window->DC.CursorPos;
        ImVec2 size((radius )*2, (radius + style.FramePadding.y)*2);

        const ImRect bb(pos, ImVec2(pos.x + size.x, pos.y + size.y));
        ImGui::ItemSize(bb, style.FramePadding.y);
        if (!ImGui::ItemAdd(bb, id))
            return false;

        // Render
        window->DrawList->PathClear();

        int num_segments = 30;
        int start = abs(ImSin(g.Time*1.8f)*(num_segments-5));

        const float a_min = IM_PI*2.0f * ((float)start) / (float)num_segments;
        const float a_max = IM_PI*2.0f * ((float)num_segments-3) / (float)num_segments;

        const ImVec2 centre = ImVec2(pos.x+radius, pos.y+radius+style.FramePadding.y);

        for (int i = 0; i < num_segments; i++) {
            const float a = a_min + ((float)i / (float)num_segments) * (a_max - a_min);
            window->DrawList->PathLineTo(ImVec2(centre.x + ImCos(a+g.Time*8) * radius,
                                                centre.y + ImSin(a+g.Time*8) * radius));
        }

        window->DrawList->PathStroke(color, false, thickness);

        return true;
    }

    bool ProjectInstallator::isShown() const {
        return show;
    }

    void ProjectInstallator::checkError() {
        error = ProjectError::NONE;

        std::ifstream _stream(pathToInstallGDE);
        if(!_stream.good()) error |= ProjectError::WRONG_PATH;
        _stream.close();
        if(strlen(pathToInstallGDE) == 0) error |= ProjectError::PATH_NOT_SET;
    }

    void ProjectInstallator::showError() {
        if((error & ProjectError::PATH_NOT_SET) == ProjectError::PATH_NOT_SET)
            ImGui::TextColored({255, 0, 0, 255}, "Path cannot be empty");

        if((error & ProjectError::WRONG_PATH) == ProjectError::WRONG_PATH)
            ImGui::TextColored({255, 0, 0, 255}, "Selected path is not valid");
    }
}