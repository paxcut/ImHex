#pragma once

#include <hex/ui/popup.hpp>

#include <hex/api/localization_manager.hpp>

#include <string>

namespace hex::plugin::builtin {

    class PopupTasksWaiting : public Popup<PopupTasksWaiting> {
    public:
        PopupTasksWaiting(std::function<void()> onFinish)
            : hex::Popup<PopupTasksWaiting>("hex.builtin.popup.waiting_for_tasks.title", false),
              m_onFinish(std::move(onFinish)){ }

        void drawContent() override {
            ImHexApi::System::unlockFrameRate();

            ImGui::TextUnformatted("hex.builtin.popup.waiting_for_tasks.desc"_lang);
            ImGui::Separator();

            ImGui::SetCursorPosX((ImGui::GetWindowWidth() - ImGui::CalcTextSize("[-]").x) / 2);
            ImGuiExt::TextSpinner("");
            ImGui::NewLine();
            ImGui::SetCursorPosX((ImGui::GetWindowWidth() - 150_scaled) / 2);
            if (ImGui::ButtonEx("hex.ui.common.cancel"_lang, ImVec2(150, 0)) || ImGui::IsKeyDown(ImGuiKey_Escape))
                ImGui::CloseCurrentPopup();

            if (TaskManager::getRunningTaskCount() == 0 && TaskManager::getRunningBackgroundTaskCount() == 0) {
                ImGui::CloseCurrentPopup();
                m_onFinish();
            }
        }

        [[nodiscard]] ImGuiWindowFlags getFlags() const override {
            return ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove;
        }

    private:
        std::function<void()> m_onFinish;
    };

}