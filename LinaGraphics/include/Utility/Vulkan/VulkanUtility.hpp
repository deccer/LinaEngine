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

#ifndef VulkanUtility_HPP
#define VulkanUtility_HPP

#include "Core/GraphicsCommon.hpp"
#include "Data/Vector.hpp"
#include <vulkan/vulkan.h>

namespace Lina::Graphics
{
    struct Attachment;
    class RenderPass;

    class VulkanUtility
    {
    public:
        static VkAttachmentDescription CreateAttachmentDescription(const Attachment& att);

        // Pipeline
        static VkPipelineShaderStageCreateInfo        CreatePipelineShaderStageCreateInfo(ShaderStage stage, VkShaderModule shaderModule);
        static VkPipelineVertexInputStateCreateInfo   CreatePipelineVertexInputStateCreateInfo(const Vector<VkVertexInputBindingDescription>&   bindingDescs,
                                                                                               const Vector<VkVertexInputAttributeDescription>& attDescs);
        static VkPipelineInputAssemblyStateCreateInfo CreatePipelineInputAssemblyCreateInfo(Topology top, bool primitiveRestart = false);
        static VkPipelineRasterizationStateCreateInfo CreatePipelineRasterStateCreateInfo(PolygonMode pm, CullMode mode, FrontFace frontFace);
        static VkPipelineMultisampleStateCreateInfo   CreatePipelineMSAACreateInfo();
        static VkPipelineColorBlendAttachmentState    CreatePipelineBlendAttachmentState();
        static VkPipelineDepthStencilStateCreateInfo  CreatePipelineDepthStencilStateCreateInfo(bool depthTest, bool depthWrite, CompareOp op);

        // Render Pass
        static void SetupMainRenderPass(RenderPass& pass);
        static void SetupFinalRenderPass(RenderPass& pass);

        // InputDesc
        static VertexInputDescription GetVertexDescription();
        static VertexInputDescription GetEmptyVertexDescription();
        static void GetDescriptionStructs(const VertexInputDescription& desc, Vector<VkVertexInputBindingDescription>& bindingDescs, Vector<VkVertexInputAttributeDescription>& attDescs);

        // Image
        static VkImageCreateInfo     GetImageCreateInfo(Format format, uint32 usageFlags, ImageTiling tiling, Extent3D extent);
        static VkImageViewCreateInfo GetImageViewCreateInfo(VkImage img, Format format, ImageSubresourceRange subres);

        // Pass
        static VkSubpassDependency GetSubpassDependency(SubPassDependency& dependency);

        // Descriptor
        static VkDescriptorSetLayoutBinding GetDescriptorSetLayoutBinding(const DescriptorSetLayoutBinding& binding);

        // Buffers
        static VkBufferCopy             GetBufferCopy(const BufferCopy& copy);
        static VkBufferImageCopy        GetBufferImageCopy(const BufferImageCopy& copy);
        static VkImageSubresourceLayers GetImageSubresourceLayers(const ImageSubresourceRange& r);

        // Barriers
        static VkMemoryBarrier       GetMemoryBarrier(const DefaultMemoryBarrier& bar);
        static VkBufferMemoryBarrier GetBufferMemoryBarrier(const BufferMemoryBarrier& bar);
        static VkImageMemoryBarrier  GetImageMemoryBarrier(const ImageMemoryBarrier& bar);

        // Others
        static VkExtent3D GetExtent3D(Extent3D e);
        static VkOffset3D GetOffset3D(Offset3D e);
        static size_t     PadUniformBufferSize(size_t originalSize);
    };

} // namespace Lina::Graphics

#endif
