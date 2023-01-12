/*
This file is a part of: Lina Engine
https://github.com/inanevin/LinaEngine

Author: Inan Evin
http://www.inanevin.com

Copyright (c) [2018-] [Inan Evin]

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#include "Core/EditorGUIManager.hpp"
#include "Core/EditorCommon.hpp"
#include "GUI/GUI.hpp"
#include "EventSystem/EventSystem.hpp"
#include "EventSystem/GraphicsEvents.hpp"
#include "EventSystem/ResourceEvents.hpp"
#include "EventSystem/MainLoopEvents.hpp"
#include "EventSystem/LevelEvents.hpp"
#include "Resource/Core/ResourceManager.hpp"
#include "Graphics/Resource/Font.hpp"
#include "Graphics/Resource/Texture.hpp"
#include "Graphics/Core/GUIBackend.hpp"
#include "Core/EditorRenderer.hpp"
#include "Graphics/Platform/LinaVGIncl.hpp"
#include "GUI/Drawable.hpp"
#include "GUI/DockArea.hpp"
#include "Graphics/Core/WindowManager.hpp"
#include "Graphics/PipelineObjects/Swapchain.hpp"
#include "Panels/TopPanel.hpp"
#include "Graphics/Core/RenderEngine.hpp"
#include "Core/EditorRenderer.hpp"

// Debug
#include "Input/Core/InputEngine.hpp"
#include "World/Core/World.hpp"
#include "Graphics/Resource/Texture.hpp"
#include "Graphics/Resource/Model.hpp"
#include "Graphics/Components/CameraComponent.hpp"
#include "Math/Math.hpp"
#include "Core/Time.hpp"

namespace Lina::Editor
{
    void EditorGUIManager::Initialize(Graphics::GUIBackend* guiBackend, EditorRenderer* rend)
    {
        m_renderer   = rend;
        m_guiBackend = guiBackend;

        auto* nunitoFont = Resources::ResourceManager::Get()->GetResource<Graphics::Font>("Resources/Editor/Fonts/NunitoSans.ttf");
        auto* rubikFont  = Resources::ResourceManager::Get()->GetResource<Graphics::Font>("Resources/Editor/Fonts/Rubik-Regular.ttf");

        rubikFont->GenerateFont(false, 12, 13);
        nunitoFont->GenerateFont(false, 12);

        m_guiBackend->UploadAllFontTextures();

        auto  gui   = ImmediateGUI::Get();
        auto& theme = LGUI->GetTheme();

        theme.m_fonts[ThemeFont::Default]       = rubikFont->GetHandle(12);
        theme.m_fonts[ThemeFont::PopupMenuText] = rubikFont->GetHandle(13);

        // Scan Icons folder & buffer all icons into a texture atlas.
        Vector<String> icons = Utility::GetFolderContents("Resources/Editor/Icons");
        m_iconTexture        = TexturePacker::PackFilesOrdered(icons, 2000, m_packedIcons);
        const StringID sid   = TO_SIDC("LINA_ENGINE_EDITOR_ICONPACK");
        m_iconTexture->ChangeSID(sid);
        m_iconTexture->SetUserManaged(true);
        Resources::ResourceManager::Get()->GetCache<Graphics::Texture>()->AddResource(sid, m_iconTexture);

        for (auto& pi : m_packedIcons)
        {
            ThemeIconData id;
            id.topLeft                     = pi.uvTL;
            id.bottomRight                 = pi.uvBR;
            id.size                        = pi.size;
            theme.m_icons[TO_SID(pi.name)] = id;
        }

        // Setup style
        const Color dark0 = Color(14.0f, 14.0f, 14.0f, 255.0f, true);
        const Color dark1 = Color(20.0f, 20.0f, 20.0f, 255.0f, true);
        const Color dark2 = Color(25.0f, 25.0f, 25.0f, 255.0f, true);

        const Color light0  = Color(35.0f, 35.0f, 35.0f, 255.0f, true);
        const Color light1  = Color(40.0f, 40.0f, 40.0f, 255.0f, true);
        const Color light15 = Color(55, 55, 55.0f, 255.0f, true);
        const Color light2  = Color(60.0f, 60.0f, 60.0f, 255.0f, true);
        const Color light3  = Color(80.0f, 80.0f, 80.0f, 255.0f, true);
        const Color light5  = Color(190.0f, 190.0f, 190.0f, 255.0f, true);

        theme.m_colors[ThemeColor::Error]                   = Color(160.0f, 30.0f, 30.0f, 255.0f, true);
        theme.m_colors[ThemeColor::Warn]                    = Color(60.0f, 60.0f, 30.0f, 255.0f, true);
        theme.m_colors[ThemeColor::Check]                   = Color(30.0f, 60.0f, 30.0f, 255.0f, true);
        theme.m_colors[ThemeColor::TextColor]               = Color::White;
        theme.m_colors[ThemeColor::Dark0]                   = dark0;
        theme.m_colors[ThemeColor::Light3]                  = light3;
        theme.m_colors[ThemeColor::Light5]                  = light5;
        theme.m_colors[ThemeColor::TopPanelBackground]      = dark1;
        theme.m_colors[ThemeColor::Window]                  = dark2;
        theme.m_colors[ThemeColor::WindowBorderColor]       = light1;
        theme.m_colors[ThemeColor::PopupBG]                 = light15;
        theme.m_colors[ThemeColor::PopupBorderColor]        = Color::Black;
        theme.m_colors[ThemeColor::ButtonBackground]        = light1;
        theme.m_colors[ThemeColor::ButtonHovered]           = light2;
        theme.m_colors[ThemeColor::ButtonPressed]           = light0;
        theme.m_colors[ThemeColor::ButtonBorder]            = dark0;
        theme.m_colors[ThemeColor::ButtonIconTint]          = Color(1, 1, 1, 1);
        theme.m_colors[ThemeColor::Highlight]               = Color(142, 34, 34, 255, true);
        theme.m_colors[ThemeColor::MenuBarPopupBG]          = light0;
        theme.m_colors[ThemeColor::MenuBarPopupBorderColor] = light1;
        theme.m_colors[ThemeColor::DockArea]                = dark1;
        theme.m_colors[ThemeColor::DockAreaBorder]          = light1;
        theme.m_colors[ThemeColor::DockAreaTitleBar]        = dark2;
        theme.m_colors[ThemeColor::DockAreaTitleBarBorder]  = Color(5.0f, 5.0f, 5.0f, 200.0f, true);

        theme.m_properties[ThemeProperty::AAEnabled]             = LinaVG::Config.aaEnabled ? 1.0f : 0.0f;
        theme.m_properties[ThemeProperty::WindowItemPaddingX]    = 12;
        theme.m_properties[ThemeProperty::WindowItemPaddingY]    = 12;
        theme.m_properties[ThemeProperty::WindowItemSpacingX]    = 12;
        theme.m_properties[ThemeProperty::WindowItemSpacingY]    = 12;
        theme.m_properties[ThemeProperty::WindowRounding]        = 0.0f;
        theme.m_properties[ThemeProperty::WindowBorderThickness] = 1.0f;
        theme.m_properties[ThemeProperty::WindowBorderThickness] = 1.0f;

        theme.m_properties[ThemeProperty::ButtonRounding]        = 0.2f;
        theme.m_properties[ThemeProperty::ButtonBorderThickness] = 1.0f;
        theme.m_properties[ThemeProperty::ButtonIconFit]         = 0.75f;
        theme.m_properties[ThemeProperty::ButtonTextFit]         = 0.35f;

        theme.m_properties[ThemeProperty::PopupRounding]               = 0.0f;
        theme.m_properties[ThemeProperty::PopupBorderThickness]        = 1.0f;
        theme.m_properties[ThemeProperty::MenuBarPopupBorderThickness] = 1.0f;
        theme.m_properties[ThemeProperty::MenuBarItemsTooltipSpacing]  = 24.0f;

        // Events
        Event::EventSystem::Get()->Connect<Event::EDrawGUI, &EditorGUIManager::OnDrawGUI>(this);
        Event::EventSystem::Get()->Connect<Event::ETick, &EditorGUIManager::OnTick>(this);

        // m_topPanel.Initialize();

        // top panel is a fixed window drawable, create init.

        // then bottom dock area.
        m_mainDockArea                = new DockArea();
        m_mainDockArea->m_swapchainID = LINA_MAIN_SWAPCHAIN_ID;
        m_dockAreas.push_back(m_mainDockArea);

        m_topPanel = new TopPanel();
        m_topPanel->Initialize();

        for (auto d : m_dockAreas)
            d->Initialize();
    }

    void EditorGUIManager::Shutdown()
    {
        m_topPanel->Shutdown();
        delete m_topPanel;

        for (auto d : m_dockAreas)
        {
            d->Shutdown();
            delete d;
        }

        // m_topPanel.Shutdown();
        Event::EventSystem::Get()->Disconnect<Event::EDrawGUI>(this);
        Event::EventSystem::Get()->Disconnect<Event::ETick>(this);
        Resources::ResourceManager::Get()->UnloadUserManaged<Graphics::Texture>(m_iconTexture);
    }

    World::EntityWorld* testWorld = nullptr;

    void EditorGUIManager::OnDrawGUI(const Event::EDrawGUI& ev)
    {
        m_topPanel->Draw();

        const auto&   topPanelRect = m_topPanel->GetRect();
        const Vector2 screen       = Graphics::RenderEngine::Get()->GetScreen().Size();
        m_mainDockArea->m_rect     = Rect(Vector2(0, topPanelRect.size.y), Vector2(screen.x, screen.y - topPanelRect.size.y));

        for (auto d : m_dockAreas)
        {
            d->UpdateCurrentSwapchainID(m_currentSwapchain->swapchainID);
            d->Draw();
        }

        // m_topPanel.Draw();
        //
        // m_dockPanel.SetStartY(m_topPanel.GetCurrentSize().y);
        //
        // m_dockPanel.Draw();
        // constexpr const char* aq = "TestWindow";
        // LGUI->SetWindowSize(aq, Vector2(500, 500));
        //
        // if (LGUI->BeginWindow(aq))
        //{
        //     LinaVG::StyleOptions style;
        //     style.color = LV4(Color::DarkBlue);
        //     // LinaVG::DrawRect(LV2(Vector2(0, 0)), LV2(Vector2(15, 15)), style, 0, 100);
        //     LGUI->EndWindow();
        // }
        //
        // constexpr const char* aq2 = "TestWindow2";
        // LGUI->SetWindowSize(aq2, Vector2(500, 500));
        //
        // if (LGUI->BeginWindow(aq2))
        //{
        //     LinaVG::StyleOptions style;
        //     style.color = LV4(Color::DarkBlue);
        //     // LinaVG::DrawRect(LV2(Vector2(0, 0)), LV2(Vector2(15, 15)), style, 0, 100);
        //     LGUI->EndWindow();
        // }
        //
        // return;
        //  LinaVG::StyleOptions style;
        //  LinaVG::DrawRect(LV2(Vector2::Zero), LV2(Vector2(500,500)), style, 0, 100);

        //  const String aq2 = "TestWindow2";
        //  LGUI->SetWindowSize(aq2, Vector2(500, 500));
        //  LGUI->SetWindowPosition(aq2, Vector2(1400,600));
        //
        //  if (test)
        //      LGUI->UndockWindow(aq2);
        //
        //  if (LGUI->BeginWindow(aq2))
        //  {
        //      LGUI->EndWindow();
        //  }

        //
        //
        //
        //
        //
        //
        // LGUI->SetWindowSize("TestWindow2", Vector2(500, 500));
        //
        // if (LGUI->BeginWindow("TestWindow2"))
        // {
        //
        //     LGUI->EndWindow();
        // }
    }

    Drawable* EditorGUIManager::GetContentFromPanelRequest(EditorPanel panel)
    {
        if (panel == EditorPanel::Entities)
        {
        }
        return nullptr;
    }

    void EditorGUIManager::OnTick(const Event::ETick& ev)
    {
        if (Input::InputEngine::Get()->GetKeyDown(LINA_KEY_R))
        {
            LaunchPanel(EditorPanel::Level);
        }

        const auto& additionalWindows = m_renderer->GetAdditionalWindows();

        auto it = m_panelRequests.begin();

        for (; it < m_panelRequests.end(); ++it)
        {
            bool  found = false;
            auto& req   = *it;
            for (const auto& [windowSid, windowData] : additionalWindows)
            {
                if (windowSid == req.sid)
                {
                    DockArea* area      = new DockArea();
                    area->m_rect        = Rect(req.pos, req.size);
                    area->m_swapchainID = req.sid;
                    area->m_detached    = true;
                    area->m_content.push_back(GetContentFromPanelRequest(req.panelType));

                    found = true;
                    m_dockAreas.push_back(area);
                    m_panelRequests.erase(it);
                    break;
                }
            }

            if (found)
                break;
        }
    }

    void EditorGUIManager::LaunchPanel(EditorPanel panel)
    {
        // ? check if we should preserve some layout for the panel
        // receive last pos & size.
        const Vector2 lastPos   = Vector2(100, 100);
        const Vector2 lastSize  = Vector2(500, 500);
        const String  panelName = "TestPanel";

        m_renderer->CreateAdditionalWindow(panelName, lastPos, lastSize);
        m_panelRequests.push_back({panel, TO_SID(panelName), Vector2::Zero, lastSize});
    }
} // namespace Lina::Editor
