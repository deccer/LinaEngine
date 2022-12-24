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

#ifndef Window_HPP
#define Window_HPP

#include "Math/Vector.hpp"
#include "Core/CommonWindow.hpp"

struct GLFWwindow;

namespace Lina::Graphics
{
    class Window
    {
    public:
        virtual void  SetSize(const Vector2i& newSize)                                                     = 0;
        virtual void  SetPos(const Vector2i& newPos)                                                       = 0;
        virtual void  SetPosCentered(const Vector2i& newPos)                                               = 0;
        virtual void  SetVsync(VsyncMode mode)                                                             = 0;
        virtual void  SetTitle(const char*)                                                                = 0;
        virtual void  Minimize()                                                                           = 0;
        virtual void  Maximize()                                                                           = 0;
        virtual void* CreateAdditionalWindow(const char* title, const Vector2i& pos, const Vector2i& size) = 0;
        virtual void  UpdateAdditionalWindow(StringID sid, const Vector2i& pos, const Vector2i& size)      = 0;
        virtual void  DestroyAdditionalWindow(StringID sid)                                                = 0;
        virtual bool  AdditionalWindowExists(StringID sid)                                                 = 0;

        /// <summary>
        /// NOTE: This is not the surface size, it's the full window size including any decorations and title bars.
        /// </summary>
        /// <returns></returns>
        inline const Vector2i& GetSize()
        {
            return m_size;
        }

        inline bool IsMinimized()
        {
            return m_minimized;
        }

        inline bool IsMaximized()
        {
            return m_maximized;
        }

        inline const Vector2i& GetPos()
        {
            return m_pos;
        }

        static Window* Get()
        {
            return s_instance;
        }

        inline float GetAspect()
        {
            return m_aspectRatio;
        }

        inline const char* GetTitle()
        {
            return m_title;
        }

        void SetIsActiveWindow(bool act)
        {
            m_isActive = act;
        }

        inline bool IsActiveWindow()
        {
            return true;
            return m_isActive;
        }

        virtual void Close() = 0;

    protected:
        friend class RenderEngine;
        friend class Engine;

        Window()                                               = default;
        ~Window()                                              = default;
        virtual bool Initialize(const WindowProperties& props) = 0;
        virtual void Shutdown()                                = 0;

        VsyncMode   m_vsync       = VsyncMode::None;
        Vector2i    m_size        = Vector2i(0, 0);
        Vector2i    m_pos         = Vector2i(0, 0);
        float       m_aspectRatio = 0.0f;
        const char* m_title       = "";
        bool        m_minimized   = false;
        bool        m_maximized   = false;
        bool        m_isActive    = false;
        bool        m_hasFocus    = false;

    private:
        static Window* s_instance;
    };
} // namespace Lina::Graphics

#endif
