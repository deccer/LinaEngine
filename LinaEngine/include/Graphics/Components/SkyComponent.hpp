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

#ifndef SkyComponent_HPP
#define SkyComponent_HPP

// Headers here.
#include "RenderableComponent.hpp"
#include "Reflection/ReflectionCommon.hpp"

namespace Lina::Graphics
{
    LINA_COMPONENT("Sky Component", "Graphics")
    class SkyComponent : public RenderableComponent
    {
    public:

        virtual AABB& GetAABB()
        {
            return m_aabb;
        }

        virtual void SaveToArchive(Serialization::Archive<OStream>& oarchive) override
        {
            RenderableComponent::SaveToArchive(oarchive);
        };

        virtual void LoadFromArchive(Serialization::Archive<IStream>& iarchive) override
        {
            RenderableComponent::LoadFromArchive(iarchive);
        };

        virtual TypeID GetTID() override
        {
            return GetTypeID<SkyComponent>();
        }

        virtual Bitmask16 GetDrawPasses() override
        {
            return 0;
        }

        virtual Vector<MeshMaterialPair> GetMeshMaterialPairs() override
        {
            return Vector<MeshMaterialPair>();
        }

        virtual RenderableType GetType() override
        {
            return RenderableType::RenderableSky;
        }


    private:
        AABB m_aabb;
    };
} // namespace Lina::Graphics

#endif