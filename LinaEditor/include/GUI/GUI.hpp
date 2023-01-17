/*
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

#pragma once

#ifndef LinaGUI_HPP
#define LinaGUI_HPP

#include "Core/SizeDefinitions.hpp"
#include "Utility/StringId.hpp"
#include "Math/Vector.hpp"
#include "Math/Rect.hpp"
#include "Data/String.hpp"
#include "Data/HashMap.hpp"
#include "Data/Deque.hpp"
#include "Math/Color.hpp"
#include "Widgets.hpp"
#include "Theme.hpp"

namespace Lina
{
    namespace Graphics
    {
        class Swapchain;
        class WindowManager;
        class Window;
    } // namespace Graphics

    namespace Event
    {
        struct EWindowFocused;
        struct EAdditionalSwapchainDestroyed;
    } // namespace Event
} // namespace Lina

namespace Lina::Editor
{
    class ImmediateGUI;
    class ImmediateWindow;
    class EditorRenderer;
    class DockSetup;

    enum ImmediateWindowMask
    {
        IMW_None                 = 1 << 0,
        IMW_UseAbsoluteDrawOrder = 1 << 1,
    };

    struct SwapchainInfo
    {
        void*                windowHandle = nullptr;
        Graphics::Swapchain* swapchain    = nullptr;
    };

    class ImmediateWidget
    {
    public:
        bool IsHovered();
        bool IsPressed();
        bool IsClicked();

        inline Vector2 GetAbsPos()
        {
            return m_absPos;
        }

    private:
        friend class ImmediateWindow;
        Vector2       m_size   = Vector2::Zero;
        Vector2       m_penPos = Vector2::Zero;
        Vector2       m_absPos = Vector2::Zero;
        ImmediateGUI* m_gui    = nullptr;
    };

    class ImmediateWindow
    {
    public:
        void Draw();
        bool IsHovered();

        // Widgets
        void             BeginWidget(const Vector2& size);
        void             EndWidget();
        ImmediateWidget& GetCurrentWidget();

        // Layout
        void BeginHorizontal();
        void EndHorizontal();

        inline void SetPenPos(const Vector2& pos)
        {
            m_penPos = pos;
        }

        inline void SetPenPosX(float x)
        {
            m_penPos.x = x;
        }

        inline void SetPenPosY(float y)
        {
            m_penPos.y = y;
        }

        inline Vector2 GetPenPos() const
        {
            return m_penPos;
        }

        inline Vector2 GetAbsPos() const
        {
            return _absPos;
        }

        inline Vector2 GetSize() const
        {
            return m_size;
        }

        inline int GetDrawOrder() const
        {
            return _drawOrder;
        }

        inline uint32 GetID()
        {
            return m_sid;
        }

        inline const Deque<int>& GetHorizontalRequests()
        {
            return m_horizontalRequests;
        }

    private:
        friend class ImmediateGUI;

        Bitmask16              m_mask     = 0;
        String                 m_name     = "";
        StringID               m_sid      = 0;
        Vector2                m_penPos   = Vector2::Zero;
        Vector2                m_localPos = Vector2::Zero;
        Vector2                m_size     = Vector2::Zero;
        Deque<ImmediateWidget> m_widgets;
        Deque<int>             m_horizontalRequests;
        Color                  m_color                = Color(0, 0, 0, 0);
        float                  m_maxPenPosX           = 0.0f;
        float                  m_maxYDuringHorizontal = 0.0f;
        bool                   m_docked               = false;

        Color         _finalColor = Color::White;
        int           _drawOrder  = 0;
        Vector2       _absPos     = Vector2::Zero;
        Rect          _rect       = Rect();
        StringID      _parent     = 0;
        Rect          _dragRect   = Rect();
        ImmediateGUI* m_gui       = nullptr;
    };

    class ImmediateGUI
    {
    public:
        // Frame
        void StartFrame();
        void EndFrame();

        // Window
        bool             BeginWindow(const char* name, Bitmask16 mask = 0, const Vector2i& pos = Vector2i::Zero);
        bool             BeginPopup(const char* name, const Vector2i& pos = Vector2i::Zero);
        void             EndWindow();
        void             EndPopup();
        void             SetWindowSize(const char* str, const Vector2& size);
        void             SetWindowColor(const char* str, const Color& col);
        ImmediateWindow& GetCurrentWindow();

        // Utility
        bool     IsPointInRect(const Vector2i& p, const Recti& rect);
        bool     IsMouseHoveringRect(const Rect& rect);
        Vector2i GetMousePosition();
        Vector2i GetMouseDelta();
        bool     GetMouseButtonDown(int button);
        bool     GetMouseButtonUp(int button);
        bool     GetMouseButton(int button);
        bool     GetMouseButtonClicked(int button);
        bool     GetMouseButtonDoubleClicked(int button);
        bool     GetKeyDown(int key);

        inline void SetAbsoluteDrawOrder(int drawOrder)
        {
            m_absoluteDrawOrder = drawOrder;
        }

        // Theme
        inline Theme& GetTheme()
        {
            return m_theme;
        }

        inline StringID GetIconTexture()
        {
            return m_iconTexture;
        }

        inline int GetThreadNumber()
        {
            return m_threadNumber;
        }

    private:
        friend class EditorGUIManager;

    private:
        Graphics::Swapchain*               m_currentSwaphchain  = nullptr;
        bool                               m_isSwapchainHovered = false;
        HashMap<StringID, ImmediateWindow> m_windowData;
        StringID                           m_lastWindow = 0;
        Theme                              m_theme;
        StringID                           m_iconTexture       = 0;
        int                                m_absoluteDrawOrder = 0;
        int                                m_threadNumber      = 0;
    };

} // namespace Lina::Editor

#endif
