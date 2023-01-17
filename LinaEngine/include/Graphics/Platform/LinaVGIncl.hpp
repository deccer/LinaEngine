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

#ifndef LinaVGInc_HPP
#define LinaVGInc_HPP

#include "Data/Vector.hpp"
#include "Data/Map.hpp"
#include <iostream>

#define LINAVG_TEXT_SUPPORT

#include <LinaVG/LinaVG.hpp>
#include <LinaVG/Backends/BaseBackend.hpp>

#define LV2(V) LinaVG::Vec2(V.x, V.y)
#define LV4(V) LinaVG::Vec4(V.x, V.y, V.z, V.w)
#define FL2(V) Vector2(V.x, V.y)
#define FL4(V) Vector4(V.x, V.y, V.z, V.w)

#include "Math/Color.hpp"

namespace Lina
{
    inline extern void DrawPoint(const Vector2& p, int drawOrder, Color col = Color::Red)
    {
        LinaVG::StyleOptions style;
        style.color             = LV4(col);
        const Vector2      size = Vector2(2, 2) * LinaVG::Config.framebufferScale.x;
        const LinaVG::Vec2 min  = LV2((p - size));
        const LinaVG::Vec2 max  = LV2((p + size));
        LinaVG::DrawRect(min, max, style, 0, drawOrder);
    }

} // namespace Lina
#endif
