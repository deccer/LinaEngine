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

#include "Panels/TopPanel.hpp"
#include "GUI/GUI.hpp"
#include "Graphics/Core/Screen.hpp"
#include "Graphics/Core/RenderEngine.hpp"
#include "Math/Math.hpp"
#include "Graphics/Resource/Texture.hpp"
#include "Resource/Core/ResourceManager.hpp"
#include "GUI/CustomWidgets/MenuPopup.hpp"
#include "Graphics/Platform/LinaVGIncl.hpp"
#include "Core/Time.hpp"
#include "Input/Core/InputEngine.hpp"
#include "Core/EditorGUIManager.hpp"

namespace Lina::Editor
{
#define TITLE_ANIM_SID  "toppanel_titleanim"
#define TEXT_ANIM_SPEED 0.05f

    enum TopPanelPopupIDs
    {
        // File
        TP_File_OpenProject = 1,
        TP_File_SaveProject,
        TP_File_ExportProject,
        TP_File_Exit,

        // Edit
        // todo

        // Panels
        TP_Panels_Entities,
        TP_Panels_Level,
        TP_Panels_Properties,
        TP_Panels_Resources,
        TP_Panels_Global,

        // Level,
        TP_Level_CreateLevel,
        TP_Level_SaveLevel,
        TP_Level_LoadLevel,

        // Entity,
        TP_Entity_CreateEntity,
        TP_Entity_Cube,
        TP_Entity_Cylinder,
        TP_Entity_Capsule,
        TP_Entity_Sphere,
        TP_Entity_LinaLogo,
        TP_Entity_Quad,
        TP_Entity_Plane,

        // Debug
        // todo

        // Help
        TP_Help_Github,
        TP_Help_Website,
        TP_Help_About,
    };

    void TopPanel::Initialize()
    {

        auto* titleTexture = Resources::ResourceManager::Get()->GetResource<Graphics::Texture>("Resources/Editor/Textures/TitleText.png");
        m_titleTexture     = titleTexture->GetSID();
        m_titleAspect      = static_cast<float>(titleTexture->GetExtent().width) / static_cast<float>(titleTexture->GetExtent().height);

        auto animFiles = Utility::GetFolderContents("Resources/Editor/Textures/TitleTextAnim/");
        m_packedAnim   = TexturePacker::PackFilesOrdered(m_subsystems.renderEngine->GetGPUUploader(), animFiles, 2050, m_packedAnimTextures);
        m_packedAnim->ChangeSID(TO_SIDC(TITLE_ANIM_SID));
        m_packedAnim->SetUserManaged(true);
        Resources::ResourceManager::Get()->GetCache<Graphics::Texture>()->AddResource(TO_SIDC(TITLE_ANIM_SID), m_packedAnim);

        // Base menu bar items.
        MenuPopup* file   = new MenuPopup("File");
        MenuPopup* edit   = new MenuPopup("Edit");
        MenuPopup* level  = new MenuPopup("Level");
        MenuPopup* view   = new MenuPopup("View");
        MenuPopup* entity = new MenuPopup("Entity");
        MenuPopup* debug  = new MenuPopup("Debug");
        MenuPopup* help   = new MenuPopup("Help");
        m_menuBar.AddItem(file, edit, level, view, entity, debug, help);

        // Popup element for each item.
        file->AddElement(new MenuPopupElement(MenuPopupElement::ElementType::Action, "Open Project", TP_File_OpenProject));
        file->AddElement(new MenuPopupElement(MenuPopupElement::ElementType::Action, "Save Project", TP_File_SaveProject, "CTRL+S"));
        file->AddElement(new MenuPopupElement(MenuPopupElement::ElementType::Action, "Export Project", TP_File_ExportProject));
        file->AddElement(new MenuPopupElement(MenuPopupElement::ElementType::Divider, "App", 0));
        file->AddElement(new MenuPopupElement(MenuPopupElement::ElementType::Action, "Exit", TP_File_Exit));

        // todo edit
        auto* panels = new MenuPopupElement(MenuPopupElement::ElementType::Expendable, "Panels", 0);
        panels->CreateExpandedPopup();
        panels->GetExpandedPopup()->AddElement(new MenuPopupElement(MenuPopupElement::ElementType::Action, "Entities", TP_Panels_Entities));
        panels->GetExpandedPopup()->AddElement(new MenuPopupElement(MenuPopupElement::ElementType::Action, "Level", TP_Panels_Level));
        panels->GetExpandedPopup()->AddElement(new MenuPopupElement(MenuPopupElement::ElementType::Action, "Properties", TP_Panels_Properties));
        panels->GetExpandedPopup()->AddElement(new MenuPopupElement(MenuPopupElement::ElementType::Action, "Global", TP_Panels_Global));
        panels->GetExpandedPopup()->AddElement(new MenuPopupElement(MenuPopupElement::ElementType::Action, "Resources", TP_Panels_Resources));

        auto* panels2 = new MenuPopupElement(MenuPopupElement::ElementType::Expendable, "Panels2", 0);
        panels2->CreateExpandedPopup();
        panels2->GetExpandedPopup()->AddElement(new MenuPopupElement(MenuPopupElement::ElementType::Action, "Entities2", TP_Panels_Entities));
        panels2->GetExpandedPopup()->AddElement(new MenuPopupElement(MenuPopupElement::ElementType::Action, "Level2", TP_Panels_Level));
        panels2->GetExpandedPopup()->AddElement(new MenuPopupElement(MenuPopupElement::ElementType::Action, "Propertie2s", TP_Panels_Properties));
        panels2->GetExpandedPopup()->AddElement(new MenuPopupElement(MenuPopupElement::ElementType::Action, "Global2", TP_Panels_Global));
        panels2->GetExpandedPopup()->AddElement(new MenuPopupElement(MenuPopupElement::ElementType::Action, "Resources2", TP_Panels_Resources));

        view->AddElement(panels);
        view->AddElement(panels2);
        view->AddElement(new MenuPopupElement(MenuPopupElement::ElementType::Divider, "", 0));
        view->AddElement(new MenuPopupElement(MenuPopupElement::ElementType::Action, "Dummy1", 0));

        //
        level->AddElement(new MenuPopupElement(MenuPopupElement::ElementType::Action, "Create Level", TP_Level_CreateLevel, "CTRL+L"));
        level->AddElement(new MenuPopupElement(MenuPopupElement::ElementType::Action, "Save Level As...", TP_Level_SaveLevel, "CTRL+SHIFT+S"));
        level->AddElement(new MenuPopupElement(MenuPopupElement::ElementType::Action, "Load Level", TP_Level_LoadLevel));
        //
        entity->AddElement(new MenuPopupElement(MenuPopupElement::ElementType::Action, "Empty Entity", TP_Entity_CreateEntity, "CTRL+E"));
        entity->AddElement(new MenuPopupElement(MenuPopupElement::ElementType::Action, "Cube", TP_Entity_Cube));
        entity->AddElement(new MenuPopupElement(MenuPopupElement::ElementType::Action, "Cylinder", TP_Entity_Cylinder));
        entity->AddElement(new MenuPopupElement(MenuPopupElement::ElementType::Action, "Capsule", TP_Entity_Capsule));
        entity->AddElement(new MenuPopupElement(MenuPopupElement::ElementType::Action, "Sphere", TP_Entity_Sphere));
        entity->AddElement(new MenuPopupElement(MenuPopupElement::ElementType::Action, "LinaLogo", TP_Entity_LinaLogo));
        entity->AddElement(new MenuPopupElement(MenuPopupElement::ElementType::Action, "Quad", TP_Entity_Quad));
        entity->AddElement(new MenuPopupElement(MenuPopupElement::ElementType::Action, "Plane", TP_Entity_Plane));

        // todo debug

        //
        help->AddElement(new MenuPopupElement(MenuPopupElement::ElementType::Action, "Github", TP_Help_Github));
        help->AddElement(new MenuPopupElement(MenuPopupElement::ElementType::Action, "Website", TP_Help_Website));
        help->AddElement(new MenuPopupElement(MenuPopupElement::ElementType::Action, "About", TP_Help_About));

        m_menuBar.SetOnItemClicked(BIND(&TopPanel::OnMenuBarItemClicked, this, std::placeholders::_1));
    }

    void TopPanel::Shutdown()
    {
        Resources::ResourceManager::Get()->UnloadUserManaged<Graphics::Texture>(m_packedAnim);
    }

    void TopPanel::Draw()
    {
        const Vector2 screenSize     = m_subsystems.renderEngine->GetScreen().Size();
        const float   borderInMargin = 2.0f;

        auto& theme = m_gui->GetTheme();

        // App border
        LinaVG::StyleOptions appBorderStyle;
        appBorderStyle.isFilled  = false;
        appBorderStyle.thickness = 3.0f;
        appBorderStyle.color     = LV4(theme.GetColor(ThemeColor::AppBorder));
        LinaVG::DrawRect(m_gui->GetThreadNumber(), LV2(Vector2(borderInMargin)), LV2((screenSize - borderInMargin)), appBorderStyle, 0.0f, 100);

        constexpr const char* name = "TopPanel";
        m_gui->SetWindowSize(name, m_rect.size);
        m_gui->SetWindowColor(name, theme.GetColor(ThemeColor::TopPanelBackground));

        if (m_gui->BeginWindow(name))
        {
            DrawFileMenu();
            m_fileMenuMaxX = m_gui->GetCurrentWindow().GetPenPos().x;
            DrawLinaLogo();
            DrawButtons();
            DrawControls();
            m_gui->EndWindow();
        }

        const Recti dragRect = Recti(static_cast<int>(m_fileMenuMaxX), 0, static_cast<int>(m_minimizeStart - m_fileMenuMaxX), static_cast<int>(m_rect.size.y * 0.4f));
         m_subsystems.renderEngine->GetWindowManager()->GetMainWindow().SetDragRect(dragRect);
    }

    void TopPanel::DrawFileMenu()
    {
        const Vector2 display = m_subsystems.renderEngine->GetScreen().DisplayResolution();
        auto&         theme   = m_gui->GetTheme();
        auto&         w       = m_gui->GetCurrentWindow();

        const float    offset    = display.x * 0.005f;
        const Vector2  logoStart = Vector2(offset, offset * 0.5f);
        const Vector2  logoSize  = Vector2(display.x * 0.01f);
        const StringID txtSid    = m_subsystems.renderEngine->GetEngineTexture(Graphics::EngineTextureType::LogoColored1024)->GetSID();
        LinaVG::DrawImage(m_gui->GetThreadNumber(), txtSid, LV2((logoStart + logoSize * 0.5f)), LV2(logoSize), LinaVG::Vec4(1, 1, 1, 1), 0.0f, 2);

        theme.PushColor(ThemeColor::ButtonBackground, ThemeColor::TopPanelBackground);
        const float   buttonSizeX = display.x * 0.027f;
        const float   buttonSizeY = buttonSizeX * 0.5f;
        const Vector2 buttonSize  = Vector2(buttonSizeX, buttonSizeY);
        m_menuBar.SetStartPosition(Vector2(logoStart.x + logoSize.x + offset * 0.5f, offset * 0.5f));
        m_menuBar.Draw(m_gui);
        theme.PopColor();
    }

    void TopPanel::DrawLinaLogo()
    {
        const Vector2 screenSize = m_subsystems.renderEngine->GetScreen().Size();
        auto&         w          = m_gui->GetCurrentWindow();

        // BG
        Vector<LinaVG::Vec2> points;
        const float          bgHeight     = m_rect.size.y * 0.35f;
        const float          desiredTextY = bgHeight * 0.6f;
        const Vector2        textureSize  = Vector2(m_titleAspect * desiredTextY, desiredTextY);
        const float          bgWidth      = textureSize.x + textureSize.x * 0.3f;
        const float          bgFactor     = bgWidth * 0.04f;
        float                centerX      = m_rect.size.x * 0.5f;
        centerX                           = Math::Max(centerX, m_fileMenuMaxX + bgWidth * 0.5f);
        const LinaVG::Vec2 bg1            = LinaVG::Vec2(centerX - bgWidth * 0.5f, 0.0f);
        const LinaVG::Vec2 bg2            = LinaVG::Vec2(centerX + bgWidth * 0.5f, 0.0f);
        const LinaVG::Vec2 bg3            = LinaVG::Vec2(centerX + bgWidth * 0.5f - bgFactor, bgHeight);
        const LinaVG::Vec2 bg4            = LinaVG::Vec2(centerX - bgWidth * 0.5f + bgFactor, bgHeight);
        m_titleMaxX                       = bg2.x;

        LinaVG::StyleOptions bgStyle;
        bgStyle.color = LV4(m_gui->GetTheme().GetColor(ThemeColor::Dark0));
        points.push_back(bg1);
        points.push_back(bg2);
        points.push_back(bg3);
        points.push_back(bg4);
        LinaVG::DrawConvex(m_gui->GetThreadNumber(), points.data(), static_cast<int>(points.size()), bgStyle, 0.0f, w.GetDrawOrder() + 1);

        // Title
        const Color   tint       = Color(0.4f, 0.4f, 0.4f, 1.0f);
        const Vector2 texturePos = Vector2(centerX, bgHeight * 0.5f);
        LinaVG::DrawImage(m_gui->GetThreadNumber(), m_titleTexture, LV2(texturePos), LV2(textureSize), LV4(tint), 0.0f, w.GetDrawOrder() + 2);

        if (m_gui->IsMouseHoveringRect(Rect(texturePos - textureSize * 0.5f, textureSize)))
        {
            auto&       pa      = m_packedAnimTextures[m_textAnimationIndex % m_packedAnimTextures.size()];
            const float elapsed = Time::GetElapsedTimeF();

            if (elapsed > m_lastTextAnimTime + TEXT_ANIM_SPEED)
            {
                m_textAnimationIndex++;
                m_lastTextAnimTime = elapsed;
            }

            const Color animTint = Color(0.8f, 0.8f, 0.8f, 1.0f);
            LinaVG::DrawImage(m_gui->GetThreadNumber(), m_packedAnim->GetSID(), LV2(Vector2(texturePos)), LV2(textureSize), LV4(animTint), 0.0f, w.GetDrawOrder() + 2, LinaVG::Vec2(1, 1), LinaVG::Vec2(0, 0), LV2(pa.uvTL), LV2(pa.uvBR));

            // Popup
            const Vector2 mouse         = m_gui->GetMousePosition();
            const Vector2 popupPosition = Vector2(mouse.x + 15, mouse.y + 15);

            const char* popupName = "TitlePopup";

            if (Widgets::BeginPopup(m_gui, popupName, popupPosition, Vector2::Zero))
            {
                const String versionText = "Lina Engine: " + TO_STRING(LINA_MAJOR) + "." + TO_STRING(LINA_MINOR) + "." + TO_STRING(LINA_PATCH) + " b." + TO_STRING(LINA_BUILD);
                const String configText  = "Configuration: " + String(LINA_CONFIGURATION);
                Widgets::Text(m_gui, versionText.c_str(), 400);
                Widgets::Text(m_gui, configText.c_str(), 400);
                Widgets::EndPopup(m_gui);
            }
        }
    }

    void TopPanel::DrawButtons()
    {
        int   closeState = 0, minimizeState = 0, maximizeState = 0;
        auto& w     = m_gui->GetCurrentWindow();
        auto& theme = m_gui->GetTheme();

        Vector2       buttonSize   = Vector2::Zero;
        const Vector2 appTitleSize = Widgets::GetTextSize(m_gui, ApplicationInfo::GetAppName());

        m_minimizeStart = Widgets::WindowButtons(m_gui, m_subsystems.renderEngine->GetScreen().DisplayResolution(), &closeState, &minimizeState, &maximizeState, m_titleMaxX, &buttonSize, appTitleSize.x);

        if (closeState == 1)
             m_subsystems.renderEngine->GetWindowManager()->GetMainWindow().Close();

        if (minimizeState == 1)
            m_subsystems.renderEngine->AddToActionSyncQueue(SimpleAction([this]() {  m_subsystems.renderEngine->GetWindowManager()->GetMainWindow().Minimize(); }));

        if (maximizeState == 1)
            m_subsystems.renderEngine->AddToActionSyncQueue(SimpleAction([this]() {  m_subsystems.renderEngine->GetWindowManager()->GetMainWindow().Maximize(); }));

        w.SetPenPos(Vector2(m_minimizeStart - theme.GetProperty(ThemeProperty::WindowItemSpacingX), buttonSize.y * 0.5f));
        const String str = TO_STRING(Time::GetFPS()) + ApplicationInfo::GetAppName();
        Widgets::Text(m_gui, str.c_str(), 0.0f, TextAlignment::Right, true);
    }

    void TopPanel::DrawControls()
    {
        auto& w     = m_gui->GetCurrentWindow();
        auto& theme = m_gui->GetTheme();

        const Vector2         pos  = Vector2(0, m_rect.size.y * 0.55f);
        constexpr const char* name = "TopPanelControls";
        m_gui->SetWindowSize(name, Vector2(m_rect.size.x, m_rect.size.y * 0.45f));

        if (m_gui->BeginWindow(name, 0, pos))
        {
            m_gui->EndWindow();
        }
    }

    void TopPanel::OnMenuBarItemClicked(uint32 id)
    {
        m_menuBar.Reset();

        if (id == TP_File_OpenProject)
        {
        }
        else if (id == TP_File_SaveProject)
        {
        }
        else if (id == TP_File_ExportProject)
        {
        }
        else if (id == TP_File_Exit)
        {
        }
        else if (id == TP_Panels_Entities)
        {
            m_guiManager->LaunchPanel(EditorPanel::Entities);
        }
        else if (id == TP_Panels_Global)
        {
            m_guiManager->LaunchPanel(EditorPanel::Global);
        }
        else if (id == TP_Panels_Level)
        {
            m_guiManager->LaunchPanel(EditorPanel::Level);
        }
        else if (id == TP_Panels_Resources)
        {
            m_guiManager->LaunchPanel(EditorPanel::Resources);
        }
        else if (id == TP_Panels_Properties)
        {
            m_guiManager->LaunchPanel(EditorPanel::Properties);
        }
        else if (id == TP_Level_CreateLevel)
        {
        }
        else if (id == TP_Level_SaveLevel)
        {
        }
        else if (id == TP_Level_LoadLevel)
        {
        }
        else if (id == TP_Entity_CreateEntity)
        {
        }
        else if (id == TP_Entity_Capsule)
        {
        }
        else if (id == TP_Entity_Cube)
        {
        }
        else if (id == TP_Entity_Cylinder)
        {
        }
        else if (id == TP_Entity_LinaLogo)
        {
        }
        else if (id == TP_Entity_Plane)
        {
        }
        else if (id == TP_Entity_Quad)
        {
        }
        else if (id == TP_Entity_Sphere)
        {
        }
        else if (id == TP_Help_About)
        {
        }
        else if (id == TP_Help_Github)
        {
        }
        else if (id == TP_Help_Website)
        {
        }
    }
} // namespace Lina::Editor