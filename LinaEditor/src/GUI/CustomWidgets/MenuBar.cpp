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

#include "GUI/CustomWidgets/MenuBar.hpp"
#include "GUI/GUI.hpp"
#include "Input/Core/InputEngine.hpp"
#include "GUI/CustomWidgets/MenuPopup.hpp"
#include "Graphics/Platform/LinaVGIncl.hpp"

namespace Lina::Editor
{
    MenuBar::~MenuBar()
    {
        for (auto& i : m_items)
            delete i;

        m_items.clear();
    }

    void MenuBar::Draw(ImmediateGUI* gui)
    {
        auto&   window = gui->GetCurrentWindow();
        auto&   theme  = gui->GetTheme();
        Vector2 pos    = m_startPosition;

        bool anyHovered = false;

        for (int32 i = 0; i < m_items.size(); i++)
        {
            auto* item = m_items[i];
            window.SetPenPos(pos);

            const bool itemActive = m_activeItem == i;

            if (itemActive)
            {
                theme.PushColor(ThemeColor::ButtonBackground, ThemeColor::Highlight);
                theme.PushColor(ThemeColor::ButtonHovered, ThemeColor::Highlight);
            }

            Vector2 totalSize = Vector2::Zero;
            if (Widgets::ButtonFlexible(gui, item->GetName().c_str(), Bitmask8(), &totalSize))
            {
                if (m_activeItem != -1)
                    Reset();
                else
                    m_activeItem = i;
            }

            const Rect itemRect  = Rect(pos, totalSize);
            const bool isHovered = window.IsHovered() && gui->IsMouseHoveringRect(itemRect);

            if (isHovered)
                anyHovered = true;

            // If we are already active, hovering over items will switch activation
            if (m_activeItem != -1)
            {
                if (isHovered)
                {
                    if (m_activeItem != i)
                        item->Reset();

                    m_activeItem = i;
                }
            }

            if (itemActive)
            {
                theme.PopColor();
                theme.PopColor();

                theme.PushFont(ThemeFont::PopupMenuText);

                // Draw the item, if it's clicked disable the popup
                item->Calculate(gui, pos + Vector2(0, totalSize.y));

                if (item->Draw(gui))
                    anyHovered = true;

                theme.PopFont();
            }

            pos.x += totalSize.x;
        }

        if (m_activeItem != -1 && gui->GetMouseButtonDown(LINA_MOUSE_0) && !anyHovered)
        {
            m_items[m_activeItem]->Reset();
            m_activeItem = -1;
        }

        // So that we can correctly calculate total size of the menu bar from pen pos.
        window.SetPenPos(pos);
    }

    void MenuBar::Reset()
    {
        m_activeItem = -1;

        for (auto i : m_items)
            i->Reset();
    }

    void MenuBar::SetOnItemClicked(Delegate<void(uint32)>&& cb)
    {
        for (auto p : m_items)
            p->SetOnItemClicked(cb);
    }

} // namespace Lina::Editor
