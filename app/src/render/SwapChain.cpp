#include <render/SwapChain.h>
#include <Log.h>

#include <array>

namespace rw
{

  SwapChain::SwapChain(Device& dev, VkExtent2D swapchainResolution) : device{ dev }, mSwapChainExtent{ swapchainResolution }, mOldSwapChain{ nullptr }
  {
    init();
  }
  SwapChain::SwapChain(Device& dev, VkExtent2D swapchainResolution, std::shared_ptr<rw::SwapChain> previous) : device{ dev }, mSwapChainExtent{ swapchainResolution }, mOldSwapChain{ previous }
  {
    init();
    mOldSwapChain = nullptr;
  }

  SwapChain::~SwapChain()
  {
    for (auto imageView : mSwapChainImageViews) {
      vkDestroyImageView(device.getDevice(), imageView, nullptr);
    }
    mSwapChainImageViews.clear();

    if (mSwapChain != VK_NULL_HANDLE) {
      vkDestroySwapchainKHR(device.getDevice(), mSwapChain, nullptr);
      mSwapChain = VK_NULL_HANDLE;
    }

    for (int i = 0; i < mSwapChainDepthImages.size(); i++) {
      vkDestroyImageView(device.getDevice(), mSwapChainDepthViews[i], nullptr);
      vkDestroyImage(device.getDevice(), mSwapChainDepthImages[i], nullptr);
      vkFreeMemory(device.getDevice(), mSwapChainDepthImageMemorys[i], nullptr);
    }

    for (auto framebuffer : mSwapChainFramebuffers) {
      vkDestroyFramebuffer(device.getDevice(), framebuffer, nullptr);
    }

    vkDestroyRenderPass(device.getDevice(), mRenderPass, nullptr);

    // cleanup synchronization objects
    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
      vkDestroySemaphore(device.getDevice(), mRenderFinishedSemaphores[i], nullptr);
      vkDestroySemaphore(device.getDevice(), mImageAvailableSemaphores[i], nullptr);
      vkDestroyFence(device.getDevice(), mInFlightFences[i], nullptr);
    }
  }

  void SwapChain::init()
  {
    createSwapChain();
    createImageViews();
    createRenderPass();
    createDepthResources();
    createFramebuffers();
    createSyncObjects();
  }

  void SwapChain::createSwapChain()
  {
    SwapChainSupportDetails swapChainSupport = device.getSwapChainSupport();

    VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
    VkPresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
    VkExtent2D extent = chooseSwapExtent(swapChainSupport.capabilities);

    uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
    if (swapChainSupport.capabilities.maxImageCount > 0 &&
      imageCount > swapChainSupport.capabilities.maxImageCount) {
      imageCount = swapChainSupport.capabilities.maxImageCount;
    }

    VkSwapchainCreateInfoKHR createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = device.getSurface();

    createInfo.minImageCount = imageCount;
    createInfo.imageFormat = surfaceFormat.format;
    createInfo.imageColorSpace = surfaceFormat.colorSpace;
    createInfo.imageExtent = extent;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    QueueFamilyIndices indices = device.findQueueFamilies();
    uint32_t queueFamilyIndices[] = { indices.graphicsFamily.value(), indices.presentFamily.value() };

    if (indices.graphicsFamily != indices.presentFamily) {
      createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
      createInfo.queueFamilyIndexCount = 2;
      createInfo.pQueueFamilyIndices = queueFamilyIndices;
    }
    else {
      createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
      createInfo.queueFamilyIndexCount = 0;      // Optional
      createInfo.pQueueFamilyIndices = nullptr;  // Optional
    }

    createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;

    createInfo.presentMode = presentMode;
    createInfo.clipped = VK_TRUE;

    createInfo.oldSwapchain = mOldSwapChain == nullptr ? VK_NULL_HANDLE : mOldSwapChain->mSwapChain;

    if (vkCreateSwapchainKHR(device.getDevice(), &createInfo, nullptr, &mSwapChain) != VK_SUCCESS) {
      throw std::runtime_error("failed to create swap chain!");
    }

    vkGetSwapchainImagesKHR(device.getDevice(), mSwapChain, &imageCount, nullptr);
    mSwapChainImages.resize(imageCount);
    vkGetSwapchainImagesKHR(device.getDevice(), mSwapChain, &imageCount, mSwapChainImages.data());

    mSwapChainImageFormat = surfaceFormat.format;
    mSwapChainExtent = extent;
  }

  void SwapChain::createImageViews()
  {
    mSwapChainImageViews.resize(mSwapChainImages.size());
    for (size_t i = 0; i < mSwapChainImages.size(); i++) {
      VkImageViewCreateInfo viewInfo{};
      viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
      viewInfo.image = mSwapChainImages[i];
      viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
      viewInfo.format = mSwapChainImageFormat;
      viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
      viewInfo.subresourceRange.baseMipLevel = 0;
      viewInfo.subresourceRange.levelCount = 1;
      viewInfo.subresourceRange.baseArrayLayer = 0;
      viewInfo.subresourceRange.layerCount = 1;

      VK_CHECK(vkCreateImageView(device.getDevice(), &viewInfo, nullptr, &mSwapChainImageViews[i]), "Failed to create image views");
    }
  }

  void SwapChain::createDepthResources()
  {
    uint32_t imageCount = static_cast<uint32_t>(mSwapChainImages.size());
    VkFormat depthFormat = findDepthFormat();
    mSwapChainDepthFormat = depthFormat;
    VkExtent2D swapChainExtent = mSwapChainExtent;

    mSwapChainDepthImages.resize(imageCount);
    mSwapChainDepthViews.resize(imageCount);
    mSwapChainDepthImageMemorys.resize(imageCount);

    for (int i = 0; i < mSwapChainDepthImages.size(); i++)
    {
      VkImageCreateInfo imageInfo{};
      imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
      imageInfo.imageType = VK_IMAGE_TYPE_2D;
      imageInfo.extent.width = swapChainExtent.width;
      imageInfo.extent.height = swapChainExtent.height;
      imageInfo.extent.depth = 1;
      imageInfo.mipLevels = 1;
      imageInfo.arrayLayers = 1;
      imageInfo.format = depthFormat;
      imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
      imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
      imageInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
      imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
      imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
      imageInfo.flags = 0;

      device.createImageWithInfo(
        imageInfo,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        mSwapChainDepthImages[i],
        mSwapChainDepthImageMemorys[i]);

      VkImageViewCreateInfo viewInfo{};
      viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
      viewInfo.image = mSwapChainDepthImages[i];
      viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
      viewInfo.format = depthFormat;
      viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
      viewInfo.subresourceRange.baseMipLevel = 0;
      viewInfo.subresourceRange.levelCount = 1;
      viewInfo.subresourceRange.baseArrayLayer = 0;
      viewInfo.subresourceRange.layerCount = 1;

      VK_CHECK(vkCreateImageView(device.getDevice(), &viewInfo, nullptr, &mSwapChainDepthViews[i]), "Failed to create depth image view");
    }
  }

  void SwapChain::createRenderPass()
  {
    VkAttachmentDescription depthAttachment{};
    depthAttachment.format = findDepthFormat();
    depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkAttachmentReference depthAttachmentRef{};
    depthAttachmentRef.attachment = 1;
    depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkAttachmentDescription colorAttachment = {};
    colorAttachment.format = mSwapChainImageFormat;
    colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentReference colorAttachmentRef = {};
    colorAttachmentRef.attachment = 0;
    colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass = {};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmentRef;
    subpass.pDepthStencilAttachment = &depthAttachmentRef;

    VkSubpassDependency dependency = {};
    dependency.dstSubpass = 0;
    dependency.dstAccessMask =
      VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
    dependency.dstStageMask =
      VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.srcAccessMask = 0;
    dependency.srcStageMask =
      VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;

    std::array<VkAttachmentDescription, 2> attachments = { colorAttachment, depthAttachment };
    VkRenderPassCreateInfo renderPassInfo = {};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
    renderPassInfo.pAttachments = attachments.data();
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;
    renderPassInfo.dependencyCount = 1;
    renderPassInfo.pDependencies = &dependency;

    VK_CHECK(vkCreateRenderPass(device.getDevice(), &renderPassInfo, nullptr, &mRenderPass), "Failed to create render pass");
  }

  void SwapChain::createFramebuffers()
  {
    uint32_t imageCount = static_cast<uint32_t>(mSwapChainImages.size());
    mSwapChainFramebuffers.resize(imageCount);
    for (size_t i = 0; i < imageCount; i++) {
      std::array<VkImageView, 2> attachments = { mSwapChainImageViews[i], mSwapChainDepthViews[i] };

      VkExtent2D swapChainExtent = mSwapChainExtent;
      VkFramebufferCreateInfo framebufferInfo = {};
      framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
      framebufferInfo.renderPass = mRenderPass;
      framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
      framebufferInfo.pAttachments = attachments.data();
      framebufferInfo.width = swapChainExtent.width;
      framebufferInfo.height = swapChainExtent.height;
      framebufferInfo.layers = 1;

      VK_CHECK(vkCreateFramebuffer(device.getDevice(), &framebufferInfo, nullptr, &mSwapChainFramebuffers[i]), "Failed to create framebuffer");
    }
  }

  void SwapChain::createSyncObjects()
  {
    mImageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
    mRenderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
    mInFlightFences.resize(MAX_FRAMES_IN_FLIGHT);
    mImagesInFlights.resize(mSwapChainImages.size(), VK_NULL_HANDLE);


    VkSemaphoreCreateInfo semaphoreInfo = {};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fenceInfo = {};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
    {
      VK_CHECK(vkCreateSemaphore(device.getDevice(), &semaphoreInfo, nullptr, &mImageAvailableSemaphores[i]), "Failed to create image semaphore");
      VK_CHECK(vkCreateSemaphore(device.getDevice(), &semaphoreInfo, nullptr, &mRenderFinishedSemaphores[i]), "Failed to create render semaphore");
      VK_CHECK(vkCreateFence(device.getDevice(), &fenceInfo, nullptr, &mInFlightFences[i]), "Failed to create frame in flight fence");
    }

  }

  // Helper functions
  VkSurfaceFormatKHR SwapChain::chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats)
  {
    if (availableFormats.empty()) RT_THROW("Not available swapchain format canditats");

    for (const auto& format : availableFormats)
    {
      if (format.format == VK_FORMAT_B8G8R8A8_SRGB && format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
        LOG("Choosen foramt VK_FORMAT_B8G8R8A8_SRGB and color space VK_COLOR_SPACE_SRGB_NONLINEAR_KHR");
        return format;
      }
    }

    return availableFormats[0];
  }
  
  VkPresentModeKHR SwapChain::chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes)
  {
    for (const auto& mode : availablePresentModes)
    {
      if (mode == VK_PRESENT_MODE_MAILBOX_KHR)
      {
        LOG("Present mode: VK_PRESENT_MODE_MAILBOX_KHR");
        return mode;
      }
    }

    LOG("Present mode: VK_PRESENT_MODE_FIFO_KHR");
    return VK_PRESENT_MODE_FIFO_KHR;
  }
 
  VkExtent2D SwapChain::chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities)
  {
    if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
    {
      return capabilities.currentExtent;
    }

    VkExtent2D actualExtent = mSwapChainExtent;

    actualExtent.width = std::max(capabilities.minImageExtent.width, std::min(capabilities.maxImageExtent.width, actualExtent.width));
    actualExtent.height = std::max(capabilities.minImageExtent.height, std::min(capabilities.maxImageExtent.height, actualExtent.height));

    return actualExtent;
  }

  VkFormat SwapChain::findDepthFormat()
  {
    return device.findSupportedFormat({ VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT }, // vector of cantidats - each gpu vendor has different supported depth formats
                                      VK_IMAGE_TILING_OPTIMAL, 
                                      VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
  }

  VkResult SwapChain::acquireNextImage(uint32_t* imageIdx) 
  {
    vkWaitForFences(device.getDevice(), 1u, &mInFlightFences[mCurrentFrame], VK_TRUE, std::numeric_limits<uint64_t>::max());

    return vkAcquireNextImageKHR(device.getDevice(), mSwapChain, std::numeric_limits<uint64_t>::max(), mImageAvailableSemaphores[mCurrentFrame], VK_NULL_HANDLE, imageIdx);
  }

  VkResult SwapChain::submitCommandBuffer(const VkCommandBuffer* commands, uint32_t* imageIdx)
  {
    if (mImagesInFlights[*imageIdx] != VK_NULL_HANDLE)
    {
      vkWaitForFences(device.getDevice(), 1u, &mImagesInFlights[*imageIdx], VK_TRUE, std::numeric_limits<uint64_t>::max());
    }

    mImagesInFlights[*imageIdx] = mInFlightFences[mCurrentFrame];

    VkSubmitInfo submitInfo = {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    VkSemaphore waitSemaphores[] = { mImageAvailableSemaphores[mCurrentFrame] };
    VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };

    submitInfo.waitSemaphoreCount = 1u;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;

    submitInfo.commandBufferCount = 1u;
    submitInfo.pCommandBuffers = commands;

    VkSemaphore signalSemaphores[] = { mRenderFinishedSemaphores[mCurrentFrame] };
    submitInfo.signalSemaphoreCount = 1u;
    submitInfo.pSignalSemaphores = signalSemaphores;

    vkResetFences(device.getDevice(), 1u, &mInFlightFences[mCurrentFrame]);
    VK_CHECK(vkQueueSubmit(device.getGraphicsQueue(), 1u, &submitInfo, mInFlightFences[mCurrentFrame]), "Failed to submit draw command buffer");

    // output to window

    VkPresentInfoKHR presentInfo = {};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

    presentInfo.waitSemaphoreCount = 1u;
    presentInfo.pWaitSemaphores = signalSemaphores;

    VkSwapchainKHR swapchains[] = { mSwapChain };
    presentInfo.swapchainCount = 1u;
    presentInfo.pSwapchains = swapchains;

    presentInfo.pImageIndices = imageIdx;

    mCurrentFrame = (mCurrentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
    return vkQueuePresentKHR(device.getPresentQueue(), &presentInfo);

  }

}