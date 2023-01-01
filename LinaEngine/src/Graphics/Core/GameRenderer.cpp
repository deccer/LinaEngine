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

#include "Graphics/Core/GameRenderer.hpp"
#include "Graphics/Core/GUIBackend.hpp"
#include "EventSystem/EventSystem.hpp"
#include "EventSystem/LevelEvents.hpp"
#include "EventSystem/WorldEvents.hpp"
#include "EventSystem/WindowEvents.hpp"
#include "EventSystem/MainLoopEvents.hpp"
#include "Graphics/PipelineObjects/Swapchain.hpp"
#include "Graphics/PipelineObjects/CommandPool.hpp"
#include "Graphics/Utility/Vulkan/VulkanUtility.hpp"
#include "Graphics/Data/Vertex.hpp"
#include "Profiling/Profiler.hpp"
#include "Graphics/Core/RenderEngine.hpp"
#include "World/Core/Entity.hpp"
#include "Graphics/Components/RenderableComponent.hpp"
#include "Graphics/Resource/Material.hpp"
#include "Graphics/Core/Backend.hpp"
#include "Graphics/Resource/Model.hpp"
#include "Graphics/Resource/ModelNode.hpp"
#include "EventSystem/GraphicsEvents.hpp"

namespace Lina::Graphics
{

#define DEF_VTXBUF_SIZE   sizeof(Vertex) * 2000
#define DEF_INDEXBUF_SIZE sizeof(uint32) * 2000

    void GameRenderer::Initialize(Swapchain* swp, GUIBackend* guiBackend)
    {
        Renderer::Initialize(swp, guiBackend);

        m_cmdPool = CommandPool{.familyIndex = Backend::Get()->GetGraphicsQueue()._family, .flags = GetCommandPoolCreateFlags(CommandPoolFlags::Reset)};
        m_cmdPool.Create();

        m_renderWorldData.finalColorTexture = VulkanUtility::CreateDefaultPassTextureColor();
        m_renderWorldData.finalDepthTexture = VulkanUtility::CreateDefaultPassTextureDepth();

        const uint32 imageSize = static_cast<uint32>(Backend::Get()->GetMainSwapchain()._images.size());

        for (uint32 i = 0; i < imageSize; i++)
        {
            CommandBuffer buf = CommandBuffer{.count = 1, .level = CommandBufferLevel::Primary};
            m_cmds.push_back(buf);
            m_cmds[i].Create(m_cmdPool._ptr);
        }

        m_descriptorPool = DescriptorPool{
            .maxSets = 10,
            .flags   = DescriptorPoolCreateFlags::None,
        };
        m_descriptorPool.AddPoolSize(DescriptorType::UniformBuffer, 10)
            .AddPoolSize(DescriptorType::UniformBufferDynamic, 10)
            .AddPoolSize(DescriptorType::StorageBuffer, 10)
            .AddPoolSize(DescriptorType::CombinedImageSampler, 10)
            .Create();

        for (int i = 0; i < FRAMES_IN_FLIGHT; i++)
        {
            Frame& f = m_frames[i];

            // Sync
            f.presentSemaphore.Create();
            f.submitSemaphore.Create();

            f.graphicsFence = Fence{.flags = GetFenceFlags(FenceFlags::Signaled)};
            f.graphicsFence.Create();

            f.indirectBuffer = Buffer{.size        = OBJ_BUFFER_MAX * sizeof(VkDrawIndexedIndirectCommand),
                                      .bufferUsage = GetBufferUsageFlags(BufferUsageFlags::TransferDst) | GetBufferUsageFlags(BufferUsageFlags::StorageBuffer) | GetBufferUsageFlags(BufferUsageFlags::IndirectBuffer),
                                      .memoryUsage = MemoryUsageFlags::CpuToGpu};

            f.indirectBuffer.Create();

            // Scene data - set 0 b 0
            f.sceneDataBuffer = Buffer{
                .size        = sizeof(GPUSceneData),
                .bufferUsage = GetBufferUsageFlags(BufferUsageFlags::UniformBuffer),
                .memoryUsage = MemoryUsageFlags::CpuToGpu,
            };
            f.sceneDataBuffer.Create();

            // Light data - set 0 b 1
            f.lightDataBuffer = Buffer{
                .size        = sizeof(GPULightData),
                .bufferUsage = GetBufferUsageFlags(BufferUsageFlags::UniformBuffer),
                .memoryUsage = MemoryUsageFlags::CpuToGpu,
            };
            f.lightDataBuffer.Create();

            f.globalDescriptor = DescriptorSet{
                .setCount = 1,
                .pool     = m_descriptorPool._ptr,
            };

            f.globalDescriptor.Create(RenderEngine::Get()->GetLayout(DescriptorSetType::GlobalSet));
            f.globalDescriptor.BeginUpdate();
            f.globalDescriptor.AddBufferUpdate(f.sceneDataBuffer, f.sceneDataBuffer.size, 0, DescriptorType::UniformBuffer);
            f.globalDescriptor.AddBufferUpdate(f.lightDataBuffer, f.lightDataBuffer.size, 1, DescriptorType::UniformBuffer);
            f.globalDescriptor.SendUpdate();

            // Pass descriptor

            f.viewDataBuffer = Buffer{
                .size        = sizeof(GPUViewData),
                .bufferUsage = GetBufferUsageFlags(BufferUsageFlags::UniformBuffer),
                .memoryUsage = MemoryUsageFlags::CpuToGpu,
            };
            f.viewDataBuffer.Create();

            f.objDataBuffer = Buffer{
                .size        = sizeof(GPUObjectData) * OBJ_BUFFER_MAX,
                .bufferUsage = GetBufferUsageFlags(BufferUsageFlags::StorageBuffer),
                .memoryUsage = MemoryUsageFlags::CpuToGpu,
            };
            f.objDataBuffer.Create();
            f.objDataBuffer.boundSet     = &f.passDescriptor;
            f.objDataBuffer.boundBinding = 1;
            f.objDataBuffer.boundType    = DescriptorType::StorageBuffer;

            f.passDescriptor = DescriptorSet{
                .setCount = 1,
                .pool     = m_descriptorPool._ptr,
            };

            f.passDescriptor.Create(RenderEngine::Get()->GetLayout(DescriptorSetType::PassSet));
            f.passDescriptor.BeginUpdate();
            f.passDescriptor.AddBufferUpdate(f.viewDataBuffer, f.viewDataBuffer.size, 0, DescriptorType::UniformBuffer);
            f.passDescriptor.AddBufferUpdate(f.objDataBuffer, f.objDataBuffer.size, 1, DescriptorType::StorageBuffer);
            f.passDescriptor.SendUpdate();
        }

        ConnectEvents();

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

        // Draw passes.
        m_renderWorldData.opaquePass.Initialize(DrawPassMask::Opaque, 1000.0f);

        // IDlists
        m_renderWorldData.allRenderables.Initialize(250, nullptr);
        m_renderWorldData.allRenderables.Initialize(250, nullptr);
    }

    void GameRenderer::Shutdown()
    {
        for (uint32 i = 0; i < FRAMES_IN_FLIGHT; i++)
        {
            m_frames[i].objDataBuffer.Destroy();
            m_frames[i].sceneDataBuffer.Destroy();
            m_frames[i].viewDataBuffer.Destroy();
            m_frames[i].lightDataBuffer.Destroy();
            m_frames[i].indirectBuffer.Destroy();
        }

        DisconnectEvents();

        delete m_renderWorldData.finalColorTexture;
        delete m_renderWorldData.finalDepthTexture;

        m_gpuIndexBuffer.Destroy();
        m_gpuVtxBuffer.Destroy();
        m_cpuIndexBuffer.Destroy();
        m_cpuVtxBuffer.Destroy();
    }

    void GameRenderer::Tick()
    {
        m_cameraSystem.Tick();
        m_renderWorldData.playerView.Tick(m_cameraSystem.GetPos(), m_cameraSystem.GetView(), m_cameraSystem.GetProj());
    }

    void GameRenderer::SyncData()
    {
        PROFILER_FUNC(PROFILER_THREAD_MAIN);

        m_renderWorldData.extractedRenderables.clear();
        const auto& renderables = m_renderWorldData.allRenderables.GetItems();

        uint32 i = 0;
        for (auto r : renderables)
        {
            if (r == nullptr)
                continue;

            if (!r->GetEntity()->GetEntityMask().IsSet(World::EntityMask::Visible))
                continue;

            RenderableData data;
            data.entityID          = r->GetEntity()->GetID();
            data.modelMatrix       = r->GetEntity()->ToMatrix();
            data.entityMask        = r->GetEntity()->GetEntityMask();
            data.position          = r->GetEntity()->GetPosition();
            data.aabb              = r->GetAABB();
            data.passMask          = r->GetDrawPasses();
            data.type              = r->GetType();
            data.meshMaterialPairs = r->GetMeshMaterialPairs();
            data.objDataIndex      = i++;
            m_renderWorldData.extractedRenderables.push_back(data);
        }

        m_renderWorldData.opaquePass.PrepareRenderData(m_renderWorldData.extractedRenderables, m_renderWorldData.playerView);
    }

    void GameRenderer::Render()
    {
        PROFILER_FUNC(PROFILER_THREAD_RENDER);

        const uint32 frameIndex = GetFrameIndex();
        auto&        frame      = m_frames[frameIndex];

        frame.graphicsFence.Wait(true, 1.0f);

        VulkanResult res;
        const uint32 imageIndex         = Backend::Get()->GetMainSwapchain().AcquireNextImage(1.0, frame.submitSemaphore, res);
        auto         swapchainImage     = Backend::Get()->GetMainSwapchain()._images[imageIndex];
        auto         swapchainImageView = Backend::Get()->GetMainSwapchain()._imageViews[imageIndex];

        if (HandleOutOfDateImage(res))
            return;

        frame.graphicsFence.Reset();

        auto& cmd = m_cmds[imageIndex];
        cmd.Reset(true);
        cmd.Begin(GetCommandBufferFlags(CommandBufferFlags::OneTimeSubmit));

        const Recti defaultRenderArea = Recti(Vector2(m_viewport.x, m_viewport.y), Vector2(m_viewport.width, m_viewport.height));

        cmd.CMD_SetViewport(m_viewport);
        cmd.CMD_SetScissors(m_scissors);

        // Merged object buffer.
        uint64 offset = 0;
        cmd.CMD_BindVertexBuffers(0, 1, m_gpuVtxBuffer._ptr, &offset);
        cmd.CMD_BindIndexBuffers(m_gpuIndexBuffer._ptr, 0, IndexType::Uint32);

        // Global set.
        cmd.CMD_BindDescriptorSets(PipelineBindPoint::Graphics, RenderEngine::Get()->GetGlobalAndPassLayouts()._ptr, 0, 1, &frame.globalDescriptor, 0, nullptr);

        // Pass set.
        cmd.CMD_BindDescriptorSets(PipelineBindPoint::Graphics, RenderEngine::Get()->GetGlobalAndPassLayouts()._ptr, 1, 1, &frame.passDescriptor, 0, nullptr);

        // Global - scene data.
        frame.sceneDataBuffer.CopyInto(&m_renderWorldData.sceneData, sizeof(GPUSceneData));

        // Global - light data.
        frame.lightDataBuffer.CopyInto(&m_renderWorldData.lightData, sizeof(GPULightData));

        // Per render pass - obj data.
        Vector<GPUObjectData> gpuObjectData;

        for (auto& r : m_renderWorldData.extractedRenderables)
        {
            // Object data.
            GPUObjectData objData;
            objData.modelMatrix = r.modelMatrix;
            gpuObjectData.push_back(objData);
        }

        frame.objDataBuffer.CopyInto(gpuObjectData.data(), sizeof(GPUObjectData) * gpuObjectData.size());

        // Put necessary images to correct layouts.
        auto mainPassImage      = m_renderWorldData.finalColorTexture->GetImage()._allocatedImg.image;
        auto mainPassImageView  = m_renderWorldData.finalColorTexture->GetImage()._ptrImgView;
        auto mainPassDepthImage = m_renderWorldData.finalDepthTexture->GetImage()._allocatedImg.image;
        auto mainPassDepthView  = m_renderWorldData.finalDepthTexture->GetImage()._ptrImgView;
        cmd.CMD_ImageTransition_ToColorOptimal(mainPassImage);
        cmd.CMD_ImageTransition_ToDepthOptimal(mainPassDepthImage);
        cmd.CMD_ImageTransition_ToColorOptimal(swapchainImage);

        // ********* MAIN PASS *********
        {
            PROFILER_SCOPE_START("Main Pass", PROFILER_THREAD_RENDER);
            cmd.CMD_BeginRenderingDefault(mainPassImageView, mainPassDepthView, defaultRenderArea);
            m_renderWorldData.opaquePass.UpdateViewData(frame.viewDataBuffer, m_renderWorldData.playerView);
            m_renderWorldData.opaquePass.RecordDrawCommands(cmd, m_meshEntries, frame.indirectBuffer);
            cmd.CMD_EndRendering();

            // Final texture is gonna be read from the next pass.
            cmd.CMD_ImageTransition_ToColorShaderRead(mainPassImage);
            PROFILER_SCOPE_END("Main Pass", PROFILER_THREAD_RENDER);
        }

        // ********* FINAL & PP PASS *********
        {
            PROFILER_SCOPE_START("Final Pass", PROFILER_THREAD_RENDER);

            // Issue GUI draw commands.
            m_guiBackend->SetIndex(imageIndex);
            m_guiBackend->SetCmd(&cmd);
            Event::EventSystem::Get()->Trigger<Event::EDrawGUI>();

            cmd.CMD_BeginRenderingFinal(swapchainImageView, defaultRenderArea);
            auto* ppMat = RenderEngine::Get()->GetEngineMaterial(EngineShaderType::SQPostProcess);
            ppMat->Bind(cmd, MaterialBindFlag::BindPipeline | MaterialBindFlag::BindDescriptor);
            cmd.CMD_Draw(3, 1, 0, 0);

            // Render GUI on top
            m_guiBackend->RecordDrawCommands();

            cmd.CMD_EndRendering();

            // Make sure presentable
            cmd.CMD_ImageTransition_ToPresent(swapchainImage);
            PROFILER_SCOPE_END("Final Pass", PROFILER_THREAD_RENDER);
        }

        cmd.End();

        // Submit command waits on the present semaphore, e.g. it waits for the acquired image to be ready.
        // Then submits command, and signals render semaphore when its submitted.
        PROFILER_SCOPE_START("Queue Submit & Present", PROFILER_THREAD_RENDER);
        Backend::Get()->GetGraphicsQueue().Submit(frame.submitSemaphore, frame.presentSemaphore, frame.graphicsFence, cmd, 1);
        Backend::Get()->GetGraphicsQueue().Present(frame.presentSemaphore, imageIndex, res);
        HandleOutOfDateImage(res);
        // Backend::Get()->GetGraphicsQueue().WaitIdle();
        PROFILER_SCOPE_END("Queue Submit & Present", PROFILER_THREAD_RENDER);

        m_frameNumber++;
    }

    void GameRenderer::Stop()
    {
    }

    void GameRenderer::Join()
    {
        for (int i = 0; i < FRAMES_IN_FLIGHT; i++)
            m_frames[i].graphicsFence.Wait();
    }

    void GameRenderer::SetMaterialTextures()
    {
        auto* ppMat = RenderEngine::Get()->GetEngineMaterial(EngineShaderType::SQPostProcess);
        ppMat->SetTexture(0, m_renderWorldData.finalColorTexture);
        ppMat->CheckUpdatePropertyBuffers();
        m_guiBackend->UpdateProjection();
    }

    void GameRenderer::OnLevelUninstalled(const Event::ELevelUninstalled& ev)
    {
        Join();
        m_renderWorldData.allRenderables.Reset();
        m_meshEntries.clear();
        m_mergedModelIDs.clear();
        m_hasLevelLoaded = false;
    }

    void GameRenderer::OnLevelInstalled(const Event::ELevelInstalled& ev)
    {
        m_hasLevelLoaded = true;
        MergeMeshes();
    }

    void GameRenderer::OnResourceLoaded(const Event::EResourceLoaded& res)
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

    void GameRenderer::OnComponentCreated(const Event::EComponentCreated& ev)
    {
        if (ev.ptr->GetComponentMask().IsSet(World::ComponentMask::Renderable))
        {
            auto renderable            = static_cast<RenderableComponent*>(ev.ptr);
            renderable->m_renderableID = m_renderWorldData.allRenderables.AddItem(renderable);
        }
    }

    void GameRenderer::OnComponentDestroyed(const Event::EComponentDestroyed& ev)
    {
        if (ev.ptr->GetComponentMask().IsSet(World::ComponentMask::Renderable))
            m_renderWorldData.allRenderables.RemoveItem(static_cast<RenderableComponent*>(ev.ptr)->GetRenderableID());
    }

    void GameRenderer::OnWindowResized(const Event::EWindowResized& ev)
    {
        m_recreateSwapchain = true;
    }

    void GameRenderer::OnWindowPositioned(const Event::EWindowPositioned& newPos)
    {
        m_recreateSwapchain = true;
    }

    bool GameRenderer::HandleOutOfDateImage(VulkanResult res)
    {
        bool shouldRecreate = false;

        if (m_recreateSwapchain || res == VulkanResult::OutOfDateKHR || res == VulkanResult::SuboptimalKHR)
        {
            m_recreateSwapchain = false;
            shouldRecreate      = true;
        }
        else if (res != VulkanResult::Success)
            LINA_ASSERT(false, "Could not acquire next image!");

        if (shouldRecreate)
        {
            Backend::Get()->WaitIdle();
            Vector2i size = Window::Get()->GetSize();

            if (size.x == 0 || size.y == 0)
                return true;

            // Swapchain
            m_swapchain->Destroy();
            m_swapchain->size = size;
            m_swapchain->Create();

            // Make sure we always match swapchain
            size = m_swapchain->size;

            delete m_renderWorldData.finalColorTexture;
            delete m_renderWorldData.finalDepthTexture;
            m_renderWorldData.finalColorTexture = VulkanUtility::CreateDefaultPassTextureColor();
            m_renderWorldData.finalDepthTexture = VulkanUtility::CreateDefaultPassTextureDepth();

            // Re-assign target textures to render passes
            SetMaterialTextures();

            UpdateViewport(size);
            m_guiBackend->UpdateProjection();

            // Make sure the semaphore is unsignalled after resize operation.
            Backend::Get()->GetGraphicsQueue().Submit(m_frames[GetFrameIndex()].submitSemaphore);

            return true;
        }

        return false;
    }

    void GameRenderer::MergeMeshes()
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

        RenderEngine::Get()->GetGPUUploader().SubmitImmediate(vtxCmd);

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

        RenderEngine::Get()->GetGPUUploader().SubmitImmediate(indexCmd);
    }

    void GameRenderer::ConnectEvents()
    {
        Event::EventSystem::Get()->Connect<Event::ELevelUninstalled, &GameRenderer::OnLevelUninstalled>(this);
        Event::EventSystem::Get()->Connect<Event::ELevelInstalled, &GameRenderer::OnLevelInstalled>(this);
        Event::EventSystem::Get()->Connect<Event::EComponentCreated, &GameRenderer::OnComponentCreated>(this);
        Event::EventSystem::Get()->Connect<Event::EComponentDestroyed, &GameRenderer::OnComponentDestroyed>(this);
        Event::EventSystem::Get()->Connect<Event::EWindowResized, &GameRenderer::OnWindowResized>(this);
        Event::EventSystem::Get()->Connect<Event::EWindowPositioned, &GameRenderer::OnWindowPositioned>(this);
        Event::EventSystem::Get()->Connect<Event::EResourceLoaded, &GameRenderer::OnResourceLoaded>(this);
    }

    void GameRenderer::DisconnectEvents()
    {
        Event::EventSystem::Get()->Disconnect<Event::ELevelInstalled>(this);
        Event::EventSystem::Get()->Disconnect<Event::ELevelUninstalled>(this);
        Event::EventSystem::Get()->Disconnect<Event::EComponentCreated>(this);
        Event::EventSystem::Get()->Disconnect<Event::EComponentDestroyed>(this);
        Event::EventSystem::Get()->Disconnect<Event::EWindowResized>(this);
        Event::EventSystem::Get()->Disconnect<Event::EWindowPositioned>(this);
        Event::EventSystem::Get()->Disconnect<Event::EResourceLoaded>(this);
    }

} // namespace Lina::Graphics
