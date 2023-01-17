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

#include "Graphics/Core/RenderEngine.hpp"
#include "Log/Log.hpp"
#include "World/Core/World.hpp"
#include "Graphics/Core/Renderer.hpp"

#include "Graphics/Resource/Shader.hpp"
#include "EventSystem/EventSystem.hpp"
#include "EventSystem/WindowEvents.hpp"
#include "EventSystem/MainLoopEvents.hpp"
#include "EventSystem/GraphicsEvents.hpp"
#include "EventSystem/LevelEvents.hpp"
#include "EventSystem/ResourceEvents.hpp"
#include "Resource/Core/ResourceManager.hpp"

#include "Graphics/Resource/Model.hpp"
#include "Graphics/Resource/ModelNode.hpp"
#include "Graphics/Resource/Mesh.hpp"
#include "Graphics/Resource/Material.hpp"
#include "Graphics/Core/GUIBackend.hpp"

#include "Profiling/Profiler.hpp"
#include "Graphics/Utility/Vulkan/VulkanUtility.hpp"
#include "Graphics/Core/SurfaceRenderer.hpp"
#include "Graphics/Core/WorldRenderer.hpp"
#include "Graphics/Platform/LinaVGIncl.hpp"

#ifdef LINA_PLATFORM_WINDOWS
#include "Graphics/Platform/Win32/Win32Window.hpp"
#endif

namespace Lina::Graphics
{

#define DEF_VTXBUF_SIZE   sizeof(Vertex) * 2000
#define DEF_INDEXBUF_SIZE sizeof(uint32) * 2000

    Model* cube = nullptr;

    void RenderEngine::AddToActionSyncQueue(const SimpleAction& act)
    {
        LOCK_GUARD(m_syncQueueMtx);
        m_syncedActions.push_back(act);
    }

    void RenderEngine::CreateChildWindow(const String& name, const Vector2i& pos, const Vector2i& size, SurfaceRenderer* associatedRenderer)
    {
        const StringID sid = TO_SID(name);
        m_windowManager.CreateAppWindow(m_windowManager.GetMainWindow().GetHandle(), name.c_str(), pos, size, true);

        auto* createdWindow = m_windowManager.m_windows[sid];
        auto* windowPtr     = createdWindow->GetHandle();
        auto* swapchainPtr  = Backend::Get()->CreateAdditionalSwapchain(sid, windowPtr, pos, size);
        associatedRenderer->AssignSwapchain(swapchainPtr);
        associatedRenderer->Initialize(this);
        m_renderers.push_back(associatedRenderer);

        m_windowData[sid] = WindowData{
            .window          = createdWindow,
            .windowHandle    = windowPtr,
            .swapchain       = swapchainPtr,
            .surfaceRenderer = associatedRenderer,
            .sid             = sid,
        };
    }

    void RenderEngine::DestroyChildWindow(StringID sid)
    {
        Backend::Get()->WaitIdle();
        auto it                = m_windowData.find(sid);
        auto renderer          = it->second.surfaceRenderer;
        auto itInRenderersList = linatl::find_if(m_renderers.begin(), m_renderers.end(), [renderer](Renderer* r) { return r == renderer; });
        m_renderers.erase(itInRenderersList);
        m_windowData.erase(it);
        renderer->Shutdown();
        delete renderer;
        m_windowManager.DestroyAppWindow(sid);
        Backend::Get()->DestroyAdditionalSwapchain(sid);
    }

    void RenderEngine::AddRenderer(Renderer* renderer)
    {
        SimpleAction action;
        action.Action = [this, renderer]() {
            if (renderer->Initialize(this))
                m_renderers.push_back(renderer);
        };
        m_syncedActions.push_back(action);
    }

    void RenderEngine::DeleteRenderer(Renderer* renderer)
    {
        SimpleAction action;
        action.Action = [this, renderer]() {
            auto it = linatl::find_if(m_renderers.begin(), m_renderers.end(), [renderer](Renderer* rend) { return renderer == rend; });

            if (it != m_renderers.end())
            {
                delete *it;
                m_renderers.erase(it);
            }
            else
                LINA_ERR("[Render Engine] -> Renderer not found!");
        };

        m_syncedActions.push_back(action);
    }

    void RenderEngine::Initialize(const InitInfo& initInfo)
    {
        LINA_TRACE("[Initialization] -> Render Engine ({0})", typeid(*this).name());

        Event::EventSystem::Get()->Connect<Event::EEngineResourcesLoaded, &RenderEngine::OnEngineResourcesLoaded>(this);
        Event::EventSystem::Get()->Connect<Event::EPreMainLoop, &RenderEngine::OnPreMainLoop>(this);
        Event::EventSystem::Get()->Connect<Event::EWindowPositioned, &RenderEngine::OnWindowPositioned>(this);
        Event::EventSystem::Get()->Connect<Event::EWindowResized, &RenderEngine::OnWindowResized>(this);
        Event::EventSystem::Get()->Connect<Event::ELevelInstalled, &RenderEngine::OnLevelInstalled>(this);
        Event::EventSystem::Get()->Connect<Event::ELevelUninstalled, &RenderEngine::OnLevelUninstalled>(this);
        Event::EventSystem::Get()->Connect<Event::EResourceLoaded, &RenderEngine::OnResourceLoaded>(this);

        Backend::s_instance = &m_backend;

        m_windowManager.Initialize(initInfo.windowProperties, &m_screen);
        m_backend.Initialize(initInfo, &m_windowManager);
        m_backend.SetSwapchainPosition(m_windowManager.GetMainWindow().GetHandle(), m_windowManager.GetMainWindow().GetPos());
        m_screen.Initialize(m_backend.m_swapchains[LINA_MAIN_SWAPCHAIN_ID]);

        m_windowData[LINA_MAIN_SWAPCHAIN_ID] = WindowData{
            .window       = m_windowManager.m_windows[LINA_MAIN_SWAPCHAIN_ID],
            .windowHandle = m_windowManager.m_windows[LINA_MAIN_SWAPCHAIN_ID]->GetHandle(),
            .swapchain    = m_backend.m_swapchains[LINA_MAIN_SWAPCHAIN_ID],
            .sid          = LINA_MAIN_SWAPCHAIN_ID,
        };

        DescriptorSetLayoutBinding globalBinding = DescriptorSetLayoutBinding{
            .binding         = 0,
            .descriptorCount = 1,
            .stageFlags      = GetShaderStage(ShaderStage::Fragment) | GetShaderStage(ShaderStage::Vertex),
            .type            = DescriptorType::UniformBuffer,
        };

        m_descriptorLayouts[DescriptorSetType::GlobalSet].AddBinding(globalBinding).Create(m_mainDeletionQueue);

        DescriptorSetLayoutBinding sceneBinding = DescriptorSetLayoutBinding{
            .binding         = 0,
            .descriptorCount = 1,
            .stageFlags      = GetShaderStage(ShaderStage::Fragment) | GetShaderStage(ShaderStage::Vertex),
            .type            = DescriptorType::UniformBuffer,
        };

        DescriptorSetLayoutBinding viewDataBinding = DescriptorSetLayoutBinding{
            .binding         = 1,
            .descriptorCount = 1,
            .stageFlags      = GetShaderStage(ShaderStage::Vertex),
            .type            = DescriptorType::UniformBuffer,
        };

        DescriptorSetLayoutBinding lightDataBinding = DescriptorSetLayoutBinding{
            .binding         = 2,
            .descriptorCount = 1,
            .stageFlags      = GetShaderStage(ShaderStage::Vertex),
            .type            = DescriptorType::UniformBuffer,
        };

        DescriptorSetLayoutBinding objDataBinding = DescriptorSetLayoutBinding{
            .binding         = 3,
            .descriptorCount = 1,
            .stageFlags      = GetShaderStage(ShaderStage::Vertex),
            .type            = DescriptorType::StorageBuffer,
        };

        m_descriptorLayouts[DescriptorSetType::PassSet].AddBinding(sceneBinding).AddBinding(viewDataBinding).AddBinding(lightDataBinding).AddBinding(objDataBinding).Create(m_mainDeletionQueue);

        m_globalAndPassLayout.AddDescriptorSetLayout(m_descriptorLayouts[DescriptorSetType::GlobalSet]).AddDescriptorSetLayout(m_descriptorLayouts[DescriptorSetType::PassSet]).Create(m_mainDeletionQueue);
        m_gpuUploader.Create(m_mainDeletionQueue);

        // Init GUI Backend, LinaVG
        LinaVG::Config.displayPosX           = 0;
        LinaVG::Config.displayPosY           = 0;
        LinaVG::Config.displayWidth          = 0;
        LinaVG::Config.displayHeight         = 0;
        LinaVG::Config.flipTextureUVs        = false;
        LinaVG::Config.framebufferScale.x    = m_screen.GetContentScale().x;
        LinaVG::Config.framebufferScale.y    = m_screen.GetContentScale().y;
        LinaVG::Config.aaMultiplier          = LinaVG::Config.framebufferScale.x * 0.4f;
        LinaVG::Config.aaEnabled             = true;
        LinaVG::Config.textCachingEnabled    = true;
        LinaVG::Config.textCachingSDFEnabled = true;
        LinaVG::Config.textCacheReserve      = 10000;

        LinaVG::Config.errorCallback = [](const std::string& err) { LINA_ERR(err.c_str()); };
        LinaVG::Config.logCallback   = [](const std::string& log) { LINA_TRACE(log.c_str()); };

        m_guiBackend = new GUIBackend();
        m_guiBackend->SetRenderEngine(this);
        LinaVG::Backend::BaseBackend::SetBackend(m_guiBackend);
        LinaVG::Initialize();

        // Engine materials
        m_engineShaderNames[EngineShaderType::LitStandard]   = "LitStandard";
        m_engineShaderNames[EngineShaderType::GUIStandard]   = "GUIStandard";
        m_engineShaderNames[EngineShaderType::GUIText]       = "GUIText";
        m_engineShaderNames[EngineShaderType::SQFinal]       = "ScreenQuads/SQFinal";
        m_engineShaderNames[EngineShaderType::SQPostProcess] = "ScreenQuads/SQPostProcess";

        // Engine models
        m_enginePrimitiveNames[EnginePrimitiveType::Capsule]  = "Capsule";
        m_enginePrimitiveNames[EnginePrimitiveType::Cube]     = "Cube";
        m_enginePrimitiveNames[EnginePrimitiveType::Cylinder] = "Cylinder";
        m_enginePrimitiveNames[EnginePrimitiveType::LinaLogo] = "LinaLogo";
        m_enginePrimitiveNames[EnginePrimitiveType::Plane]    = "Plane";
        m_enginePrimitiveNames[EnginePrimitiveType::Quad]     = "Quad";
        m_enginePrimitiveNames[EnginePrimitiveType::Sphere]   = "Sphere";
        m_enginePrimitiveNames[EnginePrimitiveType::Suzanne]  = "Suzanne";

        // Engine textures
        m_engineTextureNames[EngineTextureType::LogoWithText]    = "LogoWithText";
        m_engineTextureNames[EngineTextureType::LogoColored1024] = "Logo_Colored_1024";
        m_engineTextureNames[EngineTextureType::LogoWhite512]    = "Logo_White_512";
        m_engineTextureNames[EngineTextureType::Grid512]         = "Grid512";
        m_engineTextureNames[EngineTextureType::DummyBlack32]    = "DummyBlack_32";

        // Merged buffers.
        m_cpuVtxBuffer = Buffer{
            .size        = DEF_VTXBUF_SIZE,
            .bufferUsage = GetBufferUsageFlags(BufferUsageFlags::TransferSrc),
            .memoryUsage = MemoryUsageFlags::CpuOnly,
        };

        m_cpuIndexBuffer = Buffer{
            .size        = DEF_INDEXBUF_SIZE,
            .bufferUsage = GetBufferUsageFlags(BufferUsageFlags::TransferSrc),
            .memoryUsage = MemoryUsageFlags::CpuOnly,
        };

        m_gpuVtxBuffer = Buffer{
            .size        = DEF_VTXBUF_SIZE,
            .bufferUsage = GetBufferUsageFlags(BufferUsageFlags::VertexBuffer) | GetBufferUsageFlags(BufferUsageFlags::TransferDst),
            .memoryUsage = MemoryUsageFlags::GpuOnly,
        };

        m_gpuIndexBuffer = Buffer{
            .size        = DEF_INDEXBUF_SIZE,
            .bufferUsage = GetBufferUsageFlags(BufferUsageFlags::IndexBuffer) | GetBufferUsageFlags(BufferUsageFlags::TransferDst),
            .memoryUsage = MemoryUsageFlags::GpuOnly,
        };

        m_cpuVtxBuffer.Create();
        m_cpuIndexBuffer.Create();
        m_gpuVtxBuffer.Create();
        m_gpuIndexBuffer.Create();

        for (int i = 0; i < FRAMES_IN_FLIGHT; i++)
        {
            Frame& f = m_frames[i];

            f.graphicsFence = Fence{.flags = GetFenceFlags(FenceFlags::Signaled)};
            f.graphicsFence.Create(m_mainDeletionQueue);
        }
    }

    void RenderEngine::Tick()
    {
        for (auto r : m_renderers)
            r->Tick();

        if (ApplicationInfo::GetAppMode() == ApplicationMode::Standalone)
        {
            m_defaultWorldRenderer->SetAspectRatio(0.0f);
            m_defaultWorldRenderer->SetRenderResolution(m_defaultSurfaceRenderer->m_swapchain->size);
        }
    }

    void RenderEngine::Render()
    {
        PROFILER_FUNC(PROFILER_THREAD_RENDER);

        auto& window = m_windowManager.GetMainWindow();
        if (window.IsMinimized())
            return;

        if (ApplicationInfo::GetAppMode() == ApplicationMode::Standalone)
        {
            if (!window.GetIsAppActive())
                return;
        }

        m_gpuUploader.Poll();

        if (m_renderers.empty())
            return;

        const uint32 frameIndex = GetFrameIndex();
        Frame&       frame      = m_frames[frameIndex];

        Vector<Renderer*> surfaceRenderers;
        Vector<Renderer*> worldRenderers;

        for (auto r : m_renderers)
        {
            if (r->GetType() == RendererType::SurfaceRenderer)
                surfaceRenderers.push_back(r);

            if (r->GetType() == RendererType::WorldRenderer)
                worldRenderers.push_back(r);
        }

        frame.graphicsFence.Wait(true, 1.0f);

        Vector<Renderer*> acquiredSurfaceRenderers;
        for (auto r : surfaceRenderers)
        {
            auto& submitSemaphore = r->GetSwapchain()->_submitSemaphores[frameIndex];
            if (submitSemaphore._markSignaled)
            {
                submitSemaphore.Destroy();
                submitSemaphore.Create();
            }

            const bool imageOK            = r->AcquireImage(frameIndex);
            submitSemaphore._markSignaled = true;

            if (imageOK)
                acquiredSurfaceRenderers.push_back(r);
        }

        // Nothing to present.
        if (acquiredSurfaceRenderers.empty())
            return;

        frame.graphicsFence.Reset();

        // Render all worlds.
        Vector<uint32>         imageIndices;
        Vector<Semaphore*>     submitSemaphores;
        Vector<Semaphore*>     presentSemaphores;
        Vector<Swapchain*>     swapchains;
        Vector<CommandBuffer*> commandBuffers;

        const size_t worldRenderersSize   = static_cast<size_t>(worldRenderers.size());
        const size_t surfaceRenderersSize = static_cast<size_t>(acquiredSurfaceRenderers.size());
        imageIndices.resize(surfaceRenderersSize);
        submitSemaphores.resize(surfaceRenderersSize);
        presentSemaphores.resize(surfaceRenderersSize);
        swapchains.resize(surfaceRenderersSize);
        commandBuffers.resize(worldRenderersSize + surfaceRenderersSize);

        // Render worlds in parallel
        Taskflow tf;
        tf.for_each_index(0, static_cast<int>(worldRenderers.size()), 1, [&](int i) { commandBuffers[i] = worldRenderers[i]->Render(frameIndex, frame.graphicsFence); });
        JobSystem::Get()->GetMainExecutor().RunAndWait(tf);

        const int threadSize = static_cast<int>(acquiredSurfaceRenderers.size());

        if (threadSize == 2)
        {
            int a = 5;
        }
        LINA_TRACE("Starting frame");

        LinaVG::StartFrame(threadSize);
        m_guiBackend->Prepare(frameIndex);

        for (int i = 0; i < threadSize; i++)
        {
            auto sz = LinaVG::Internal::g_rendererData[i].m_defaultBuffers.m_size;
            LINA_TRACE("Size {0}", sz);
        }

        Taskflow tf2;
        tf2.for_each_index(0, static_cast<int>(acquiredSurfaceRenderers.size()), 1, [&](int i) {
            auto r          = acquiredSurfaceRenderers[i];
            auto swp        = r->GetSwapchain();
            imageIndices[i] = r->GetAcquiredImage();
            r->SetUserData(i);
            commandBuffers[worldRenderersSize + i] = r->Render(frameIndex, frame.graphicsFence);
            submitSemaphores[i]                    = &swp->_submitSemaphores[frameIndex];
            presentSemaphores[i]                   = &swp->_presentSemaphores[frameIndex];
            swapchains[i]                          = swp;
        });

        JobSystem::Get()->GetMainExecutor().RunAndWait(tf2);

        LinaVG::EndFrame();

        // Submit command waits on the present semaphore, e.g. it waits for the acquired image to be ready.
        // Then submits command, and signals render semaphore when its submitted.
        PROFILER_SCOPE_START("Queue Submit & Present", PROFILER_THREAD_RENDER);
        Backend::Get()->GetGraphicsQueue().Submit(submitSemaphores, presentSemaphores, frame.graphicsFence, commandBuffers, 1);
        VulkanResult res;
        Backend::Get()->GetGraphicsQueue().Present(presentSemaphores, swapchains, imageIndices, res);

        for (auto r : acquiredSurfaceRenderers)
        {
            // Not unsignalled yet on the GPU, but we don't care, by the time fence is over it will be.
            r->GetSwapchain()->_submitSemaphores[frameIndex]._markSignaled = false;
            r->OnPostPresent(res);
        }

        PROFILER_SCOPE_END("Queue Submit & Present", PROFILER_THREAD_RENDER);

        m_frameNumber++;
    }

    void RenderEngine::Stop()
    {
    }

    void RenderEngine::Join()
    {
        Backend::Get()->WaitIdle();
        Backend::Get()->GetGraphicsQueue().WaitIdle();
        for (int i = 0; i < FRAMES_IN_FLIGHT; i++)
            m_frames[i].graphicsFence.Wait();
    }

    void RenderEngine::MergeMeshes()
    {
        // Get all the meshes currently loaded in the resource manager
        // Meaning all used meshes for this level.
        // Merge them into big vtx & indx buffers.
        auto  rm    = Resources::ResourceManager::Get();
        auto  cache = rm->GetCache<Graphics::Model>();
        auto& res   = cache->GetResources();
        m_meshEntries.clear();
        m_mergedModelIDs.clear();

        Vector<Vertex> mergedVertices;
        Vector<uint32> mergedIndices;

        uint32 vertexOffset = 0;
        uint32 firstIndex   = 0;

        for (auto& pair : res)
        {
            m_mergedModelIDs.push_back(pair.second->GetSID());

            for (auto& node : pair.second->GetNodes())
            {
                for (auto& m : node->GetMeshes())
                {
                    const auto& vertices = m->GetVertices();
                    const auto& indices  = m->GetIndices();

                    MergedBufferMeshEntry entry;
                    entry.vertexOffset = vertexOffset;
                    entry.firstIndex   = firstIndex;
                    entry.indexSize    = static_cast<uint32>(indices.size());

                    const uint32 vtxSize   = static_cast<uint32>(vertices.size());
                    const uint32 indexSize = static_cast<uint32>(indices.size());
                    for (auto& v : vertices)
                        mergedVertices.push_back(v);

                    for (auto& i : indices)
                        mergedIndices.push_back(i);

                    vertexOffset += vtxSize;
                    firstIndex += indexSize;

                    m_meshEntries[m] = entry;
                }
            }
        }

        const uint32 vtxSize   = static_cast<uint32>(mergedVertices.size() * sizeof(Vertex));
        const uint32 indexSize = static_cast<uint32>(mergedIndices.size() * sizeof(uint32));

        m_cpuVtxBuffer.CopyInto(mergedVertices.data(), vtxSize);
        m_cpuIndexBuffer.CopyInto(mergedIndices.data(), indexSize);

        // Realloc if necessary.
        if (m_gpuVtxBuffer.size < m_cpuVtxBuffer.size)
        {
            m_gpuVtxBuffer.Destroy();
            m_gpuVtxBuffer.size = m_cpuVtxBuffer.size;
            m_gpuVtxBuffer.Create();
        }

        if (m_gpuIndexBuffer.size < m_cpuIndexBuffer.size)
        {
            m_gpuIndexBuffer.Destroy();
            m_gpuIndexBuffer.size = m_cpuIndexBuffer.size;
            m_gpuIndexBuffer.Create();
        }

        Command vtxCmd;
        vtxCmd.Record = [this, vtxSize](CommandBuffer& cmd) {
            BufferCopy copy = BufferCopy{
                .destinationOffset = 0,
                .sourceOffset      = 0,
                .size              = vtxSize,
            };

            Vector<BufferCopy> regions;
            regions.push_back(copy);

            cmd.CMD_CopyBuffer(m_cpuVtxBuffer._ptr, m_gpuVtxBuffer._ptr, regions);
        };

        vtxCmd.OnSubmitted = [this]() {
            m_gpuVtxBuffer._ready = true;
            // f.cpuVtxBuffer.Destroy();
        };

        m_gpuUploader.SubmitImmediate(vtxCmd);

        Command indexCmd;
        indexCmd.Record = [this, indexSize](CommandBuffer& cmd) {
            BufferCopy copy = BufferCopy{
                .destinationOffset = 0,
                .sourceOffset      = 0,
                .size              = indexSize,
            };

            Vector<BufferCopy> regions;
            regions.push_back(copy);

            cmd.CMD_CopyBuffer(m_cpuIndexBuffer._ptr, m_gpuIndexBuffer._ptr, regions);
        };

        indexCmd.OnSubmitted = [this] {
            m_gpuIndexBuffer._ready = true;
            // f.cpuIndexBuffer.Destroy();
        };

        m_gpuUploader.SubmitImmediate(indexCmd);
    }

    void RenderEngine::SyncData()
    {
        for (auto r : m_renderers)
            r->SyncData();

        for (auto& c : m_syncedActions)
        {
            c.Action();

            if (c.OnExecuted)
                c.OnExecuted();
        }

        m_syncedActions.clear();
    }

    void RenderEngine::Shutdown()
    {
        Event::EventSystem::Get()->Disconnect<Event::EEngineResourcesLoaded>(this);
        Event::EventSystem::Get()->Disconnect<Event::EPreMainLoop>(this);
        Event::EventSystem::Get()->Disconnect<Event::EWindowPositioned>(this);
        Event::EventSystem::Get()->Disconnect<Event::EWindowResized>(this);
        Event::EventSystem::Get()->Disconnect<Event::ELevelInstalled>(this);
        Event::EventSystem::Get()->Disconnect<Event::ELevelUninstalled>(this);
        Event::EventSystem::Get()->Disconnect<Event::EResourceLoaded>(this);

        for (auto pair : m_engineMaterials)
            delete pair.second;

        m_gpuIndexBuffer.Destroy();
        m_gpuVtxBuffer.Destroy();
        m_cpuIndexBuffer.Destroy();
        m_cpuVtxBuffer.Destroy();

        LinaVG::Terminate();

        LINA_TRACE("[Shutdown] -> Render Engine ({0})", typeid(*this).name());

        m_gpuUploader.Destroy();
        m_mainDeletionQueue.Flush();

        Vector<StringID> childs;
        for (auto [sid, wd] : m_windowData)
        {
            if (sid != LINA_MAIN_SWAPCHAIN_ID)
                childs.push_back(sid);
        }

        for (auto c : childs)
            DestroyChildWindow(c);

        for (auto r : m_renderers)
        {
            r->Shutdown();
            delete r;
        }

        m_renderers.clear();
        m_windowManager.Shutdown();
        m_backend.Shutdown();
    }

    void RenderEngine::OnEngineResourcesLoaded(const Event::EEngineResourcesLoaded& ev)
    {
        auto* rm = Resources::ResourceManager::Get();

        for (auto& p : m_engineTextureNames)
        {
            const String txtPath      = "Resources/Engine/Textures/" + p.second + ".png";
            Texture*     texture      = rm->GetResource<Texture>(txtPath);
            m_engineTextures[p.first] = texture;
        }

        for (auto& p : m_engineShaderNames)
        {
            const String shaderPath  = "Resources/Engine/Shaders/" + p.second + ".linashader";
            Shader*      shader      = rm->GetResource<Shader>(shaderPath);
            m_engineShaders[p.first] = shader;

            const String materialPath = "Resources/Engine/Materials/" + p.second + ".linamat";
            Material*    material     = new Material();
            material->m_path          = materialPath;
            material->m_sid           = TO_SID(materialPath);
            material->CreateBuffer();
            material->SetShader(shader);
            m_engineMaterials[p.first] = material;
        }

        for (auto& p : m_enginePrimitiveNames)
        {
            const String modelPath  = "Resources/Engine/Models/" + p.second + ".fbx";
            Model*       model      = rm->GetResource<Model>(modelPath);
            m_engineModels[p.first] = model;
        }

        m_placeholderMaterial  = m_engineMaterials[EngineShaderType::LitStandard];
        m_placeholderModel     = m_engineModels[EnginePrimitiveType::Cube];
        m_placeholderModelNode = m_placeholderModel->GetRootNode();

        // world renderer.
        m_defaultWorldRenderer = new WorldRenderer();
        m_defaultWorldRenderer->SetRenderMask(0);

        // surface renderer.
        m_defaultSurfaceRenderer = new SurfaceRenderer();
        m_defaultSurfaceRenderer->AssignSwapchain(m_backend.m_swapchains[LINA_MAIN_SWAPCHAIN_ID]);

        if (m_appInfo.appMode == ApplicationMode::Editor)
        {
            m_defaultWorldRenderer->Initialize(this);
            m_defaultSurfaceRenderer->Initialize(this);
            m_defaultSurfaceRenderer->SetRenderMask(RM_RenderGUI);
        }
        else
        {
            m_defaultSurfaceRenderer->SetRenderMask(RM_RenderGUI | RM_DrawOffscreenTexture);
            m_defaultWorldRenderer->Initialize(this);
            m_defaultSurfaceRenderer->Initialize(this);
            m_defaultSurfaceRenderer->SetOffscreenTexture(m_defaultWorldRenderer->GetFinalTexture());
            m_defaultWorldRenderer->AddFinalTextureListener(m_defaultSurfaceRenderer);
        }

        m_renderers.push_back(m_defaultWorldRenderer);
        m_renderers.push_back(m_defaultSurfaceRenderer);
    }

    void RenderEngine::OnPreMainLoop(const Event::EPreMainLoop& ev)
    {
        auto mainSwapchain = m_backend.m_swapchains[LINA_MAIN_SWAPCHAIN_ID];
        // m_guiBackend->UpdateProjection(mainSwapchain->size);
    }

    void RenderEngine::OnWindowPositioned(const Event::EWindowPositioned& ev)
    {
        m_backend.SetSwapchainPosition(ev.window, ev.newPos);
    }

    void RenderEngine::OnWindowResized(const Event::EWindowResized& ev)
    {
        // during early init
        if (m_backend.m_swapchains.empty())
            return;

        if (ev.window != m_backend.GetMainSwapchain()._windowHandle)
            return;

        LinaVG::Config.displayWidth  = static_cast<unsigned int>(ev.newSize.x);
        LinaVG::Config.displayHeight = static_cast<unsigned int>(ev.newSize.y);
    }

    void RenderEngine::OnLevelUninstalled(const Event::ELevelUninstalled& ev)
    {
        m_meshEntries.clear();
        m_mergedModelIDs.clear();
        m_hasLevelLoaded = false;
        m_defaultWorldRenderer->RemoveWorld();
    }

    void RenderEngine::OnLevelInstalled(const Event::ELevelInstalled& ev)
    {
        m_hasLevelLoaded = true;
        MergeMeshes();
        m_defaultWorldRenderer->SetWorld(ev.world);
    }

    void RenderEngine::OnResourceLoaded(const Event::EResourceLoaded& res)
    {
        // When a new model is loaded re-merge meshes
        // Do this only after a level is fully loaded, not during loading level resources.
        if (!m_hasLevelLoaded)
            return;

        if (res.tid == GetTypeID<Model>())
        {
            auto* found = linatl::find_if(m_mergedModelIDs.begin(), m_mergedModelIDs.end(), [&res](StringID sid) { return sid == res.sid; });
            if (found == m_mergedModelIDs.end())
            {
                Join();
                MergeMeshes();
            }
        }
    }

    Vector<String> RenderEngine::GetEngineShaderPaths()
    {
        Vector<String> paths;

        for (auto& p : m_engineShaderNames)
        {
            const String path = "Resources/Engine/Shaders/" + p.second + ".linashader";
            paths.push_back(path);
        }

        return paths;
    }

    Vector<String> RenderEngine::GetEnginePrimitivePaths()
    {
        Vector<String> paths;

        for (auto& p : m_enginePrimitiveNames)
        {
            const String path = "Resources/Engine/Models/" + p.second + ".fbx";
            paths.push_back(path);
        }

        return paths;
    }

    Vector<String> RenderEngine::GetEngineTexturePaths()
    {
        Vector<String> paths;

        for (auto& p : m_engineTextureNames)
        {
            const String path = "Resources/Engine/Textures/" + p.second + ".png";
            paths.push_back(path);
        }

        return paths;
    }

    Mesh* RenderEngine::GetPlaceholderMesh()
    {
        return GetPlaceholderModelNode()->GetMeshes()[0];
    }

} // namespace Lina::Graphics
