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

#include "GUI/CustomWidgets/MenuPopup.hpp"
#include "Math/Math.hpp"

namespace Lina::Editor
{
    void MenuPopupExpandableElement::OnClicked()
    {
    }

    void MenuPopupExpandableElement::Draw()
    {
    }

    float MenuPopupExpandableElement::GetWidth()
    {
        return 0.0f;
    }

    void MenuPopupActionElement::OnClicked()
    {
    }

    void MenuPopupActionElement::Draw()
    {
    }

    float MenuPopupActionElement::GetWidth()
    {
        return 0.0f;
    }

    void MenuPopupToggleElement::OnClicked()
    {
    }

    void MenuPopupToggleElement::Draw()
    {
    }

    float MenuPopupToggleElement::GetWidth()
    {
        return 0.0f;
    }

    MenuPopup::~MenuPopup()
    {
        for (auto e : m_elements)
            delete e;

        m_elements.clear();
    }

    void MenuPopup::AddElement(MenuPopupElement* element)
    {
        m_elements.push_back(element);
    }

    void MenuPopup::Draw(const Vector2& startPosition)
    {
        m_maxWidth = 0.0f;
        for (auto e : m_elements)
            m_maxWidth = Math::Max(m_maxWidth, e->GetWidth());

        // Draw a rect for popup background.

        // Draw each element.
        for (auto e : m_elements)
        {
        }
    }
    void MenuPopupDividerElement::OnClicked()
    {
    }
    void MenuPopupDividerElement::Draw()
    {
    }
    float MenuPopupDividerElement::GetWidth()
    {
        return 0.0f;
    }
} // namespace Lina::Editor