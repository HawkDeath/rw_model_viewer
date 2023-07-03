#ifndef DEVICE_H
#define DEVICE_H

#include <Window.h>
#include <render/PhysicalDevice.h>

#include <cstdint>
#include <optional>
#include <vector>
#include <unordered_map>

namespace rw {

  struct QueueFamilyIndices
  {
    std::optional<uint32_t> graphicsFamily;
    std::optional<uint32_t> presentFamily;

    bool isComplete() {
      return graphicsFamily.has_value() && presentFamily.has_value();
    }
  };

  struct SwapChainSupportDetails
  {
    VkSurfaceCapabilitiesKHR capabilities;

    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> presentModes;
  };

  class Device {
  public:
    Device(Window& window);
    ~Device();

    VkDevice getDevice() const { return mDevice; }
    VkCommandPool getCommandPool() const { return mCommandPool; }
    VkQueue getGraphicsQueue() const { return mGraphicsQueue; }
    VkQueue getPresentQueue() const { return mPresentQueue; }
    VkSurfaceKHR getSurface() const { return mSurface; }
    PhysicalDevice getCurrentPhysicalDevice() { return mPhysicalDevice; }

    VkCommandBuffer beginSingleTimeCommand();
    void endSingleTimeCommand(VkCommandBuffer command);

    VkFormat findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features);
    uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);

    void createImageWithInfo(const VkImageCreateInfo& imageInfo, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory);
    void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);

    QueueFamilyIndices findQueueFamilies();
    SwapChainSupportDetails getSwapChainSupport();

  private:
    void createInstance();
    void getPhysicalDevices();
    void pickPhysicalDevice();
    void createSurface();
    void createCommandPool();
    void createLogicalDevice();

    std::vector<const char*> requiredExtensions();

  private:
    Window& mWindow;
    VkInstance mInstance;
    PhysicalDevice mPhysicalDevice;
    std::unordered_map<VkPhysicalDeviceType, PhysicalDevice> gpus;
    VkDevice mDevice;
    VkSurfaceKHR mSurface;
    VkCommandPool mCommandPool;

    // queues
    VkQueue mGraphicsQueue;
    VkQueue mPresentQueue;
  };
}

#endif // DEVICE_H
