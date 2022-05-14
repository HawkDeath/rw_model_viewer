#ifndef PHYSICALDEVICE_H
#define PHYSICALDEVICE_H

#include <vulkan/vulkan.h>
#include <vector>

namespace rw {
class PhysicalDevice {
public:
    PhysicalDevice() = default;
    PhysicalDevice(VkPhysicalDevice physicalDevice);

    VkPhysicalDevice getPhysicalDevice() const { return mPhysicalDevice; }

    const VkPhysicalDeviceFeatures& getFeatures() const {
        return mFeatures;
    }

    const VkPhysicalDeviceMemoryProperties& getMemoryProperties() const {
        return mMemoryProperties;
    }

    const VkPhysicalDeviceProperties& getProperties() const {
        return mProperties;
    }

    const std::vector<VkQueueFamilyProperties> getQueueFamilyProperties() const {
        return mQueueFamilyProperties;
    }

private:
    VkPhysicalDevice mPhysicalDevice = VK_NULL_HANDLE;
    VkPhysicalDeviceFeatures mFeatures;
    VkPhysicalDeviceProperties mProperties;
    VkPhysicalDeviceMemoryProperties mMemoryProperties;
    std::vector<VkQueueFamilyProperties> mQueueFamilyProperties;
};
}

#endif // PHYSICALDEVICE_H
