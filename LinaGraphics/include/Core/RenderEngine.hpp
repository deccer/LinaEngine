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

#ifndef RenderEngine_HPP
#define RenderEngine_HPP

#include "Renderer.hpp"
#include "Data/Deque.hpp"
#include "Utility/StringId.hpp"
#include "Data/FixedVector.hpp"
#include "Core/CommonApplication.hpp"
#include "Utility/DeletionQueue.hpp"
#include "PipelineObjects/DescriptorSetLayout.hpp"
#include "PipelineObjects/UploadContext.hpp"
#include "Backend.hpp"
#include "Window.hpp"
#include <functional>

namespace Lina
{
    namespace Event
    {
        struct ESwapchainRecreated;
        struct EEngineResourcesLoaded;
    } // namespace Event
} // namespace Lina

namespace LinaVG
{
    namespace Backend
    {
        class GUIBackend;
    }
} // namespace LinaVG

namespace Lina::Graphics
{
    class Model;
    class Texture;
    class ModelNode;
    class Material;

    class RenderEngine
    {

    public:
        RenderEngine()  = default;
        ~RenderEngine() = default;

        static inline RenderEngine* Get()
        {
            return s_instance;
        }

        inline Model* GetPlaceholderModel()
        {
            return m_placeholderModel;
        }

        inline ModelNode* GetPlaceholderModelNode()
        {
            return m_placeholderModelNode;
        }

        inline Material* GetPlaceholderMaterial()
        {
            return m_placeholderMaterial;
        }

        inline Renderer& GetRenderer()
        {
            return m_renderer;
        }

        inline DeletionQueue& GetMainDeletionQueue()
        {
            return m_mainDeletionQueue;
        }

        inline bool IsInitialized()
        {
            return m_initedSuccessfully;
        }

        inline Recti& GetScissor()
        {
            return m_scissor;
        }

        inline Viewport& GetViewport()
        {
            return m_viewport;
        }

        inline DescriptorSetLayout* GetLayout(DescriptorSetType set)
        {
            return m_descriptorLayouts[set];
        }

        inline DescriptorPool& GetDescriptorPool()
        {
            return m_descriptorPool;
        }

        inline UploadContext& GetGPUUploader()
        {
            return m_gpuUploader;
        }

        inline Material* GetEngineMaterial(EngineShaderType shader)
        {
            return m_engineMaterials[shader];
        }

        inline Model* GetEngineModel(EnginePrimitiveType primitive)
        {
            return m_engineModels[primitive];
        }

        inline Texture* GetEngineTexture(EngineTextureType texture)
        {
            return m_engineTextures[texture];
        }

        inline Shader* GetEngineShader(EngineShaderType sh)
        {
            return m_engineShaders[sh];
        }

        inline void SyncData()
        {
            m_renderer.SyncData();
        }

        void           Join();
        Vector<String> GetEngineShaderPaths();
        Vector<String> GetEngineMaterialPaths();
        Vector<String> GetEnginePrimitivePaths();
        Vector<String> GetEngineTexturePaths();
        Mesh*          GetPlaceholderMesh();

    private:
        void Initialize(const InitInfo& initInfo);
        void Clear();
        void Tick();
        void Render();
        void Shutdown();
        void OnSwapchainRecreated(const Event::ESwapchainRecreated& ev);
        void OnEngineResourcesLoaded(const Event::EEngineResourcesLoaded& ev);

    private:
        friend class Engine;

        static RenderEngine* s_instance;

        Model*        m_placeholderModel     = nullptr;
        ModelNode*    m_placeholderModelNode = nullptr;
        Material*     m_placeholderMaterial  = nullptr;
        DeletionQueue m_mainDeletionQueue;
        InitInfo      m_appInfo;
        Window        m_window;
        Backend       m_backend;
        bool          m_initedSuccessfully = false;

        UploadContext                                    m_gpuUploader;
        DescriptorPool                                   m_descriptorPool;
        DescriptorSetLayout                              m_globalSetLayout;
        DescriptorSetLayout                              m_passLayout;
        DescriptorSetLayout                              m_materialLayout;
        DescriptorSetLayout                              m_objectLayout;
        HashMap<DescriptorSetType, DescriptorSetLayout*> m_descriptorLayouts;
        Viewport                                         m_viewport;
        Recti                                            m_scissor;
        Renderer                                         m_renderer;
        LinaVG::Backend::GUIBackend*                     m_guiBackend;

        // Resources
        HashMap<EngineShaderType, String>    m_engineShaderNames;
        HashMap<EnginePrimitiveType, String> m_enginePrimitiveNames;
        HashMap<EngineTextureType, String>   m_engineTextureNames;
        HashMap<EngineShaderType, Shader*>   m_engineShaders;
        HashMap<EngineShaderType, Material*> m_engineMaterials;
        HashMap<EnginePrimitiveType, Model*> m_engineModels;
        HashMap<EngineTextureType, Texture*> m_engineTextures;
    };
} // namespace Lina::Graphics

#endif
