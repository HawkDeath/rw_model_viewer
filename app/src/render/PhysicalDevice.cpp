#include <render/PhysicalDevice.h>
#include <Log.h>
#include <cstdint>

namespace rw {
PhysicalDevice::PhysicalDevice(VkPhysicalDevice physicalDevice)
  : mPhysicalDevice{physicalDevice}
{
    vkGetPhysicalDeviceFeatures(physicalDevice, &mFeatures);
    vkGetPhysicalDeviceProperties(physicalDevice, &mProperties);
    vkGetPhysicalDeviceMemoryProperties(physicalDevice, &mMemoryProperties);
    LOG("GPU: {}", mProperties.deviceName);
    LOG("\t Vulkan version: {}.{}.{}", VK_VERSION_MAJOR(mProperties.apiVersion), VK_VERSION_MINOR(mProperties.apiVersion), VK_VERSION_PATCH(mProperties.apiVersion));
    std::uint32_t queueCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueCount, nullptr);
    mQueueFamilyProperties.resize(queueCount);
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueCount, mQueueFamilyProperties.data());
}
}
