#ifndef DEVICE_H
#define DEVICE_H

#include <Window.h>
#include <render/PhysicalDevice.h>

#include <cstdint>
#include <vector>
#include <unordered_map>

namespace rw {

class Device {
public:
    Device(Window &window);
    ~Device();

    VkDevice getDevice() const { return mDevice; }
    VkCommandPool getCommandPool() const { return mCommandPool; }
    VkQueue getGraphicsQueue() const { return mGraphicsQueue; }
    VkQueue getPresentQueue() const { return mPresentQueue; }
    VkSurfaceKHR getSurface() const { return mSurface; }
    PhysicalDevice getCurrentPhysicalDevice() { return mPhysicalDevice; }

    VkCommandBuffer beginSingleTimeCommand();
    void endSingleTimeCommand(VkCommandBuffer command);

private:
    void createInstance();
    void getPhysicalDevices();
    void pickPhysicalDevice();
    void createSurface();
    void createCommandPool();
    void createLogicalDevice();

    std::vector<const char*> requiredExtensions();
private:
    Window &mWindow;
    VkInstance mInstance;
    PhysicalDevice mPhysicalDevice;
    std::unordered_map<VkPhysicalDeviceType ,PhysicalDevice> gpus;
    VkDevice mDevice;
    VkSurfaceKHR mSurface;
    VkCommandPool mCommandPool;
    VkQueue mGraphicsQueue;
    VkQueue mPresentQueue;
};
}

#endif // DEVICE_H
