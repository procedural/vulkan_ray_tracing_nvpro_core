/* Copyright (c) 2014-2018, NVIDIA CORPORATION. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *  * Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *  * Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *  * Neither the name of NVIDIA CORPORATION nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#pragma once

#include <platform.h>
#include <assert.h>
#include <algorithm>
#include <vulkan/vulkan.h>

namespace nvvk {

  class GraphicsPipelineState
  {
  public:

    void resetPointers()
    {
      createInfo.pNext = nullptr;
      createInfo.pStages = stages;
      createInfo.pVertexInputState = &viState;
      createInfo.pInputAssemblyState = &iaState;
      createInfo.pViewportState = &vpState;
      createInfo.pRasterizationState = &rsState;
      createInfo.pMultisampleState = &msState;
      createInfo.pDepthStencilState = &dsState;
      createInfo.pColorBlendState = &cbState;

      viState.pVertexAttributeDescriptions = viState.vertexAttributeDescriptionCount ? inputAttributes : nullptr;
      viState.pVertexBindingDescriptions = viState.vertexBindingDescriptionCount ? inputBindings : nullptr;

      vpState.pScissors = scissors;
      vpState.pViewports = viewports;

      dyState.pDynamicStates = dyState.dynamicStateCount ? dynamicStates : nullptr;

      msState.pSampleMask = &sampleMask;

      for (uint32_t i = 0; i < dyState.dynamicStateCount; i++) {
        switch (dynamicStates[i]) {
        case VK_DYNAMIC_STATE_VIEWPORT:
          vpState.pViewports = nullptr;
          break;
        case VK_DYNAMIC_STATE_SCISSOR:
          vpState.pScissors = nullptr;
          break;
        }
      }

      cbState.pAttachments = attachments;
    }

    GraphicsPipelineState(VkPipelineLayout layout=VK_NULL_HANDLE, VkPipelineCreateFlags flags = 0)
    {
      createInfo = { VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO };
      createInfo.flags = flags;
      createInfo.layout = layout;

      viState = { VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO };

      iaState = { VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO };
      iaState.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
      iaState.primitiveRestartEnable = VK_FALSE;

      vpState = { VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO };
      vpState.viewportCount = 1;
      vpState.scissorCount = 1;

      rsState = { VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO };
      rsState.rasterizerDiscardEnable = VK_FALSE;
      rsState.polygonMode = VK_POLYGON_MODE_FILL;
      rsState.cullMode = VK_CULL_MODE_BACK_BIT;
      rsState.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
      rsState.depthClampEnable = VK_FALSE;
      rsState.depthBiasEnable = VK_FALSE;
      rsState.depthBiasConstantFactor = 0.0;
      rsState.depthBiasSlopeFactor = 0.0f;
      rsState.depthBiasClamp = 0.0f;
      rsState.lineWidth = 1.0f;

      sampleMask = ~0;
      msState = { VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO };
      msState.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
      msState.alphaToCoverageEnable = VK_FALSE;
      msState.alphaToOneEnable = VK_FALSE;
      msState.sampleShadingEnable = VK_FALSE;
      msState.minSampleShading = 1.0f;

      dsState = { VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO };
      dsState.depthTestEnable = VK_FALSE;
      dsState.depthBoundsTestEnable = VK_FALSE;
      dsState.depthWriteEnable = VK_FALSE;
      dsState.depthCompareOp = VK_COMPARE_OP_ALWAYS;
      dsState.minDepthBounds = 0.0f;
      dsState.maxDepthBounds = 0.0f;
      dsState.stencilTestEnable = VK_FALSE;

      cbState = { VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO };
      cbState.attachmentCount = 1;
      

      dyState = { VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO };

      for (size_t i = 0; i < NV_ARRAY_SIZE(stages); ++i) {
        stages[i] = { VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO };
      }

      for (size_t i = 0; i < NV_ARRAY_SIZE(attachments); ++i) {
        attachments[i] = { VK_FALSE,
        VK_BLEND_FACTOR_ZERO, VK_BLEND_FACTOR_ZERO, VK_BLEND_OP_ADD,
        VK_BLEND_FACTOR_ZERO, VK_BLEND_FACTOR_ZERO, VK_BLEND_OP_ADD,
        VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT };
      }

      for (size_t i = 0; i < NV_ARRAY_SIZE(scissors); ++i) {
        scissors[i] = { { 0, 0 }, { 0, 0 } };
      }

      for (size_t i = 0; i < NV_ARRAY_SIZE(viewports); ++i) {
        viewports[i] = { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f };
      }

      for (size_t i = 0; i < NV_ARRAY_SIZE(dynamicStates); i++) {
        dynamicStates[i] = VK_DYNAMIC_STATE_MAX_ENUM;
      }
      resetPointers();
    }

    GraphicsPipelineState(const GraphicsPipelineState& other){
      memcpy(this, &other, sizeof(GraphicsPipelineState));
      resetPointers();
    }

    void operator=(const GraphicsPipelineState &other) {
      memcpy(this, &other, sizeof(GraphicsPipelineState));
      resetPointers();
    }

    operator const VkGraphicsPipelineCreateInfo* () const {
      return &createInfo;
    }

    void setRenderPass(VkRenderPass pass) { createInfo.renderPass = pass; }
    void setPipelineLayout(VkPipelineLayout layout) { createInfo.layout = layout; }

    void clearShaderStages() { 
      createInfo.stageCount = 0;
    }
    void clearDynamicStates() {
      dyState.dynamicStateCount = 0;
      dyState.pDynamicStates = nullptr;
    }
    void clearVertexInputBindings() {
      viState.vertexBindingDescriptionCount = 0;
      viState.pVertexBindingDescriptions = nullptr;
    }
    void clearVertexInputAttributes() {
      viState.vertexAttributeDescriptionCount = 0;
      viState.pVertexAttributeDescriptions = nullptr;
    }

    void addShaderStage(VkShaderStageFlagBits stage, VkShaderModule module, const char *entrypoint = "main")
    {
      assert(createInfo.stageCount < NV_ARRAY_SIZE(stages));
      stages[createInfo.stageCount].stage = stage;
      stages[createInfo.stageCount].module = module;
      stages[createInfo.stageCount].pName = entrypoint;
      createInfo.stageCount++;
    }
    void addDynamicState(VkDynamicState state)
    {
      dynamicStates[dyState.dynamicStateCount] = state;
      dyState.dynamicStateCount++;
      dyState.pDynamicStates = dynamicStates;
      createInfo.pDynamicState = &dyState;

      switch (state)
      {
      case VK_DYNAMIC_STATE_VIEWPORT:
        vpState.pViewports = nullptr;
        break;
      case VK_DYNAMIC_STATE_SCISSOR:
        vpState.pScissors = nullptr;
        break;
      }
    }

    void addVertexInputBinding(uint32_t binding, uint32_t stride, VkVertexInputRate rate=VK_VERTEX_INPUT_RATE_VERTEX)
    {
      assert(viState.vertexBindingDescriptionCount < NV_ARRAY_SIZE(inputBindings));
      inputBindings[viState.vertexBindingDescriptionCount].binding = binding;
      inputBindings[viState.vertexBindingDescriptionCount].stride = stride;
      inputBindings[viState.vertexBindingDescriptionCount].inputRate = rate;
      viState.vertexBindingDescriptionCount++;
      viState.pVertexBindingDescriptions = inputBindings;
    }
    void addVertexInputAttribute(uint32_t location, uint32_t binding, VkFormat format, uint32_t offset)
    {
      assert(viState.vertexAttributeDescriptionCount < NV_ARRAY_SIZE(inputAttributes));
      inputAttributes[viState.vertexAttributeDescriptionCount].location = location;
      inputAttributes[viState.vertexAttributeDescriptionCount].binding = binding;
      inputAttributes[viState.vertexAttributeDescriptionCount].format = format;
      inputAttributes[viState.vertexAttributeDescriptionCount].offset = offset;
      viState.vertexAttributeDescriptionCount++;
      viState.pVertexAttributeDescriptions = inputAttributes;
    }
    void setVertexInputBindings(uint32_t num_bindings, const VkVertexInputBindingDescription *bindings)
    {
      assert(num_bindings <= NV_ARRAY_SIZE(inputBindings));
      memcpy(inputBindings, bindings, sizeof(VkVertexInputBindingDescription) * num_bindings);
      viState.vertexBindingDescriptionCount = num_bindings;
      viState.pVertexBindingDescriptions = inputBindings;
    }
    void setVertexInputAttributes(uint32_t num_attributes, const VkVertexInputAttributeDescription *attributes)
    {
      assert(num_attributes <= NV_ARRAY_SIZE(inputAttributes));
      memcpy(inputAttributes, attributes, sizeof(VkVertexInputAttributeDescription) * num_attributes);
      viState.vertexAttributeDescriptionCount = num_attributes;
      viState.pVertexAttributeDescriptions = inputAttributes;
    }
    void setPrimitiveTopology(VkPrimitiveTopology topology) { iaState.topology = topology; }
    void setCullMode(VkCullModeFlags mode, VkFrontFace front)
    {
      rsState.cullMode = mode;
      rsState.frontFace = front;
    }
    void setPolygonMode(VkPolygonMode mode, float lineWidth = 0.0f) { rsState.polygonMode = mode; rsState.lineWidth = lineWidth; }
    void setAttachmentColorMask(uint32_t attachment, VkColorComponentFlags mask)
    {
      assert(attachment < NV_ARRAY_SIZE(attachments));
      attachments[attachment].colorWriteMask = mask;
      cbState.attachmentCount = std::max(cbState.attachmentCount, attachment + 1);
    }
    void setDepthBias(bool enable, float factor, float slope_factor)
    {
      rsState.depthBiasEnable = enable;
      rsState.depthBiasConstantFactor = factor;
      rsState.depthBiasSlopeFactor = slope_factor;
    }
    void setDepthTest(bool enable, bool write, VkCompareOp op = VK_COMPARE_OP_ALWAYS)
    {
      dsState.depthTestEnable = enable;
      dsState.depthWriteEnable = write;
      dsState.depthCompareOp = op;
    }
    void setDepthBounds(bool enable, float min = 0.0f, float max = 1.0f)
    {
      dsState.depthBoundsTestEnable = enable;
      dsState.minDepthBounds = min;
      dsState.maxDepthBounds = max;
    }
    void setStencilTest(bool enable, VkCompareOp op = VK_COMPARE_OP_ALWAYS)
    {
      dsState.stencilTestEnable = enable;
      dsState.back.compareOp = op;
      dsState.front.compareOp = op;
    }

    void setScissorRect(uint32_t index, const VkRect2D &rect)
    {
      assert(index < NV_ARRAY_SIZE(scissors));
      scissors[index] = rect;
      vpState.scissorCount = std::max(vpState.scissorCount, index + 1);
    }
    void setViewportRect(uint32_t index, const VkViewport &viewport)
    {
      assert(index < NV_ARRAY_SIZE(viewports));
      viewports[index] = viewport;
      vpState.viewportCount = std::max(vpState.viewportCount, index + 1);
    }

    void setRasterizationSamples(VkSampleCountFlagBits samples)
    {
      msState.rasterizationSamples = samples;
    }

    VkGraphicsPipelineCreateInfo createInfo;

    VkPipelineVertexInputStateCreateInfo viState;
    VkPipelineInputAssemblyStateCreateInfo iaState;
    VkPipelineViewportStateCreateInfo vpState;
    VkPipelineRasterizationStateCreateInfo rsState;
    VkPipelineMultisampleStateCreateInfo msState;
    VkPipelineDepthStencilStateCreateInfo dsState;
    VkPipelineColorBlendStateCreateInfo cbState;
    VkPipelineDynamicStateCreateInfo dyState;

    uint32_t  sampleMask;
    VkPipelineShaderStageCreateInfo stages[5];
    VkPipelineColorBlendAttachmentState attachments[16];
    VkRect2D scissors[16];
    VkViewport viewports[16];
    VkDynamicState dynamicStates[16];
    VkVertexInputBindingDescription inputBindings[16];
    VkVertexInputAttributeDescription inputAttributes[16];
  };

}