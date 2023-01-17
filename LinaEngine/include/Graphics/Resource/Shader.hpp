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

#ifndef Shader_HPP
#define Shader_HPP

#include "Resource/Core/Resource.hpp"
#include "Data/String.hpp"
#include "Data/Vector.hpp"
#include "Graphics/Core/GraphicsCommon.hpp"
#include "Data/HashMap.hpp"
#include "Data/Bitmask.hpp"

#include "Graphics/PipelineObjects/Pipeline.hpp"
#include "Graphics/PipelineObjects/PipelineLayout.hpp"
#include "Graphics/PipelineObjects/DescriptorSetLayout.hpp"

struct VkShaderModule_T;

namespace Lina::Graphics
{
    struct ShaderModule
    {
        String               moduleName = "";
        VkShaderModule_T*    ptr        = nullptr;
        String               moduleText = "";
        Vector<unsigned int> byteCode;
    };

    class Shader : public Resources::Resource
    {
    public:
        Shader() = default;
        virtual ~Shader();

        inline const HashMap<ShaderStage, ShaderModule>& GetModules()
        {
            return m_modules;
        }

        inline SurfaceType GetSurfaceType()
        {
            return m_surface;
        }

        inline Pipeline& GetPipeline()
        {
            return m_pipeline;
        }

        inline Bitmask16 GetDrawPassMask()
        {
            return m_drawPassMask;
        }

        inline const Vector<ShaderReflectedProperty>& GetReflectedProperties() const
        {
            return m_reflectedProperties;
        }

        inline DescriptorSetLayout& GetMaterialSetLayout()
        {
            return m_materialLayout;
        }

    protected:
        virtual Resource* LoadFromMemory(Serialization::Archive<IStream>& archive) override;
        virtual Resource* LoadFromFile(const char* path) override;
        virtual void      WriteToPackage(Serialization::Archive<OStream>& archive) override;
        virtual void      SaveToArchive(Serialization::Archive<OStream>& archive) override;
        virtual void      LoadFromArchive(Serialization::Archive<IStream>& archive) override;

    private:
        void CheckShaderProperties();
        void CheckIfModuleExists(const String& name, ShaderStage stage, const String& define);
        void GenerateByteCode();
        bool CreateShaderModules();
        void GeneratePipeline();

    private:
        // HashMap<RenderPassType, Pipeline> m_pipelines;
        Pipeline                        m_pipeline;
        PipelineLayout                  m_pipelineLayout;
        DescriptorSetLayout             m_materialLayout;
        SurfaceType                     m_surface = SurfaceType::Opaque;
        String                          m_text    = "";
        Vector<ShaderReflectedProperty> m_reflectedProperties;
        Vector<uint8>                   m_renderPasses;
        PipelineType                    m_pipelineType = PipelineType::Default;

    private:
        HashMap<ShaderStage, ShaderModule> m_modules;
        Bitmask16                          m_drawPassMask;
    };

} // namespace Lina::Graphics

#endif
