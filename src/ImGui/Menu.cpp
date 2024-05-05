#include "Menu.h"

#include <imgui.h>
#include <imgui_internal.h>

#include "../InputManager.h"
#include "Renderer.h"

void Menu::Open()
{
    //auto uiMQ = RE::UIMessageQueue::GetSingleton();
    //uiMQ->AddMessage(RE::CursorMenu::MENU_NAME, RE::UI_MESSAGE_TYPE::kShow, nullptr);
    isOpen = true;
    ImGui::Renderer::shouldRender = true;
    InputManager::SetBlocked();
}

void Menu::Close()
{
    //auto uiMQ = RE::UIMessageQueue::GetSingleton();
    //uiMQ->AddMessage(RE::CursorMenu::MENU_NAME, RE::UI_MESSAGE_TYPE::kHide, nullptr);
    isOpen = false;
    ImGui::Renderer::shouldRender = false;
    InputManager::SetWantUnblock();
}

void Menu::Draw()
{
    auto viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->GetCenter(), ImGuiCond_Always, ImVec2{ 0.5f, 0.5f });
    ImGui::SetNextWindowSize(ImVec2{ viewport->Size.x * 0.3f, viewport->Size.y * 0.5f });

    ImGuiWindowFlags window_flags = 0;
    window_flags |= ImGuiWindowFlags_NoCollapse;

    ImGui::Begin("Mod Function Menu", nullptr, window_flags);
    {
        auto tree = CurrentSection();
        ImGui::Text("%s", tree->CurrentPathStr().c_str());
        ImGui::Spacing();

        if (ImGui::BeginTable("Explorer", 1)) {
            ImGui::PushStyleVar(ImGuiStyleVar_ButtonTextAlign, ImVec2{ 0.0f, 0.0f });

            ImVec2 sz = ImVec2(-FLT_MIN, 0.0f);

            ImGui::TableNextColumn();
            if (ImGui::Button("..", sz)) {
                OnClickParentEntry(tree);
            }

            auto node = tree->CurrentPath();
            for (auto& entry : node->children) {
                ImGui::TableNextColumn();
                if (ImGui::Button(entry.name.c_str(), sz)) {
                    OnClickEntry(tree, std::addressof(entry));
                }
            }

            ImVec2 center = ImGui::GetMainViewport()->GetCenter();
            ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

            if (ImGui::BeginPopupModal("MessageBox", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
                ImGui::Text("%s", _msg.data());

                if (ImGui::Button("OK", ImVec2(120, 0))) {
                    ImGui::CloseCurrentPopup();
                }
                ImGui::SetItemDefaultFocus();
                ImGui::SameLine();
                if (ImGui::Button("Cancel", ImVec2(120, 0))) {
                    ImGui::CloseCurrentPopup();
                }
                ImGui::EndPopup();
            }

            ImGui::PopStyleVar();
            ImGui::EndTable();
        }
    }
    ImGui::End();

    ImGui::ShowDemoWindow();
}

void Menu::OnClickParentEntry(MFM_Tree* a_tree) { a_tree->ResetCurrentPathToParent(); }

void Menu::OnClickEntry(MFM_Tree* a_tree, const MFM_Node* a_node)
{
    switch (a_node->type) {
    case MFM_Node::Type::kRegular:
        {
            auto func = MFM_Function::Get(a_node->path);

            switch (func.preAction) {
            case MFMAPI_PreAction::kNone:
                break;
            case MFMAPI_PreAction::kCloseMenu:
                Close();
                break;
            case MFMAPI_PreAction::kCloseMenuAndResetPath:
                Close();
                a_tree->ResetCurrentPath();
                break;
            }

            InvokeFunction(func);

            switch (func.postAction) {
            case MFMAPI_PostAction::kNone:
                break;
            case MFMAPI_PostAction::kCloseMenu:
                Close();
                break;
            case MFMAPI_PostAction::kCloseMenuAndResetPath:
                Close();
                a_tree->ResetCurrentPath();
                break;
            }
        }
        break;
    case MFM_Node::Type::kDirectory:
        {
            a_tree->CurrentPath(a_node);
        }
        break;
    }
}

void Menu::InvokeFunction(const MFM_Function& a_func)
{
    switch (a_func.type) {
    case MFMAPI_Type::kVoid:
        SKSE::log::debug("Invoke Void function.");
        a_func();
        break;
    case MFMAPI_Type::kMessage:
        SKSE::log::debug("Invoke Message function.");
        // TODO
    case MFMAPI_Type::kMessageBox:
        SKSE::log::debug("Invoke MessageBox function.");
        {
            _msg.resize(0x4000);
            a_func(_msg.data(), _msg.size());
            _msg.back() = '\0';

            ImGui::OpenPopup("MessageBox");
        }
        break;
    }
}
