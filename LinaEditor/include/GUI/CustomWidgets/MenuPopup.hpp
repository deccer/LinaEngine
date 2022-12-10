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

#ifndef MenuPopup_HPP
#define MenuPopup_HPP

#include "Data/String.hpp"
#include "Data/Vector.hpp"
#include "Math/Vector.hpp"
#include "Math/Rect.hpp"

namespace Lina::Editor
{
    class MenuPopup;

    class MenuPopupElement
    {

    public:
        enum class ElementType
        {
            Divider,
            Action,
            Expendable,
            Toggle,
        };

        MenuPopupElement() = default;
        MenuPopupElement(ElementType type, const String& name, uint32 id, const String& tooltip = "")
            : m_type(type), m_name(name), m_id(id), m_tooltip(tooltip){

                                                    };
        virtual ~MenuPopupElement() = default;

        bool    Draw();
        Vector2 GetTextSize();
        Vector2 GetTooltipSize();

        inline const String& GetName() const
        {
            return m_name;
        }

        inline bool HasExtra()
        {
            return m_type == ElementType::Toggle || m_type == ElementType::Expendable;
        }

    private:
        friend class MenuPopup;

        ElementType m_type         = ElementType::Action;
        String      m_name         = "";
        uint32      m_id           = 0;
        String      m_tooltip      = "";
        Rect        m_rect         = Rect(0, 0);
        Vector2     m_textSize     = Vector2::Zero;
        Vector2     m_maxTextSize  = Vector2::Zero;
        Vector2     m_maxExtraSize = Vector2::Zero;
        bool        m_toggleValue  = false;
    };

    class MenuPopup
    {
    public:
        MenuPopup(const String& name) : m_name(name){};
        ~MenuPopup();

        void AddElement(MenuPopupElement* element);
        bool Draw(const Vector2& startPosition);

        inline const String& GetName() const
        {
            return m_name;
        }

    private:
        Vector<MenuPopupElement*> m_elements;
        String                    m_name = "";
    };
} // namespace Lina::Editor

#endif
