#ifndef _SWAPCHAIN_H
#define _SWAPCHAIN_H

#include <render/Device.h>

#include <vulkan/vulkan.h>
// source: https://github.com/blurrypiano/littleVulkanEngine/blob/main/src/lve_swap_chain.hpp

#include <memory>
#include <vector>

namespace rw
{
  class SwapChain
  {
    const uint32_t MAX_FRAMES_IN_FLIGHT = {2u};
  public:
    SwapChain(Device& dev, VkExtent2D swapchainResolution);
    SwapChain(Device& dev, VkExtent2D swapchainResolution, std::shared_ptr<rw::SwapChain> previous);
    ~SwapChain();

    SwapChain(const SwapChain&) = delete;
    SwapChain& operator=(const SwapChain&) = delete;

    VkFramebuffer getFrameBuffer(int32_t frameIdx) {
      return mSwapChainFramebuffers[frameIdx];
    }
    VkImageView getImageViews(int32_t frameIdx) {
      return mSwapChainImageViews[frameIdx];
    }
    VkRenderPass getRenderPass() { return mRenderPass;  }
    VkSwapchainKHR getHanlder() { return mSwapChain; }

    VkResult acquireNextImage(uint32_t* imageIdx);
    VkResult submitCommandBuffer(const VkCommandBuffer* commands, uint32_t* imageIdx);
    VkFormat findDepthFormat();

    float aspectRatio() {
      return static_cast<float>(mSwapChainExtent.width) / static_cast<float>(mSwapChainExtent.height);
    }

    VkExtent2D getSwapChainResolution() {
      return mSwapChainExtent;
    }

    bool compareSwapFormats(const rw::SwapChain& swapchain) const {
      return swapchain.mSwapChainDepthFormat == mSwapChainDepthFormat &&
             swapchain.mSwapChainImageFormat == mSwapChainImageFormat;
    }

  private:
    void init();
    void createSwapChain();
    void createImageViews();
    void createDepthResources();
    void createRenderPass();
    void createFramebuffers();
    void createSyncObjects();

    // Helper functions
    VkSurfaceFormatKHR chooseSwapSurfaceFormat(
      const std::vector<VkSurfaceFormatKHR>& availableFormats);
    VkPresentModeKHR chooseSwapPresentMode(
      const std::vector<VkPresentModeKHR>& availablePresentModes);
    VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);

  private:
    Device& device;
    VkExtent2D mSwapChainExtent;
    std::shared_ptr<rw::SwapChain> mOldSwapChain;

    VkSwapchainKHR mSwapChain = { VK_NULL_HANDLE };
    VkRenderPass mRenderPass;

    // depth images
    VkFormat mSwapChainDepthFormat;
    std::vector<VkImage> mSwapChainDepthImages;
    std::vector<VkImageView> mSwapChainDepthViews;
    std::vector<VkDeviceMemory> mSwapChainDepthImageMemorys;

    // color images
    VkFormat mSwapChainImageFormat;
    std::vector<VkImage> mSwapChainImages;
    std::vector<VkImageView> mSwapChainImageViews;

    std::vector<VkFramebuffer> mSwapChainFramebuffers;

    // sync objects
    std::vector<VkSemaphore> mImageAvailableSemaphores;
    std::vector<VkSemaphore> mRenderFinishedSemaphores;
    std::vector<VkFence> mInFlightFences;
    std::vector<VkFence> mImagesInFlights;

    size_t mCurrentFrame = { 0 };
  };

}

#endif // !_SWAPCHAIN_H
