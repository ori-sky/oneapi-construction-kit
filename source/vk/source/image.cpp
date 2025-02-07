// Copyright (C) Codeplay Software Limited
//
// Licensed under the Apache License, Version 2.0 (the "License") with LLVM
// Exceptions; you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     https://github.com/codeplaysoftware/oneapi-construction-kit/blob/main/LICENSE.txt
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
// WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the
// License for the specific language governing permissions and limitations
// under the License.
//
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#include <vk/device.h>
#include <vk/image.h>

namespace vk {
VkResult CreateImage(vk::device device, const VkImageCreateInfo* pCreateInfo,
                     vk::allocator allocator, vk::image* pImage) {
  (void)device;
  (void)pCreateInfo;
  (void)allocator;
  (void)pImage;

  return VK_ERROR_FEATURE_NOT_PRESENT;
}

void DestroyImage(vk::device device, vk::image image, vk::allocator allocator) {
  (void)device;
  (void)image;
  (void)allocator;
}

void GetImageMemoryRequirements(vk::device device, vk::image image,
                                VkMemoryRequirements* pMemoryRequirements) {
  (void)device;
  (void)image;
  (void)pMemoryRequirements;
}

void GetImageSparseMemoryRequirements(
    vk::device device, vk::image image, uint32_t* pSparseMemoryAllocationCount,
    VkSparseImageMemoryRequirements* pSparseMemoryRequirements) {
  (void)device;
  (void)image;
  (void)pSparseMemoryAllocationCount;
  (void)pSparseMemoryRequirements;
}

void GetImageSubresourceLayout(vk::device device, vk::image image,
                               const VkImageSubresource* pSubresource,
                               VkSubresourceLayout* pLayout) {
  (void)device;
  (void)image;
  (void)pSubresource;
  (void)pLayout;
}
}  // namespace vk
