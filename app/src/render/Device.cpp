#include <render/Device.h>
#include <Log.h>

#include <algorithm>

namespace rw {
Device::Device(Window &window) : mWindow {window} {
    createInstance();
    createSurface();
    getPhysicalDevices();
    pickPhysicalDevice();
    createLogicalDevice();
    createCommandPool();
}

Device::~Device() {
    // vkDestroyCommandPool(mDevice, mCommandPool, VK_CUSTOM_ALLOCATOR);
    // vkDestroyDevice(mDevice, VK_CUSTOM_ALLOCATOR);
    vkDestroySurfaceKHR(mInstance, mSurface, VK_CUSTOM_ALLOCATOR);
    vkDestroyInstance(mInstance, VK_CUSTOM_ALLOCATOR);
}

void Device::createInstance() {
    VkApplicationInfo appInfo = {};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "rw_model_viewer";
    appInfo.applicationVersion = VK_MAKE_VERSION(0, 1, 0);
    appInfo.pEngineName = "rw_model_viewer_engine";
    appInfo.engineVersion = VK_MAKE_VERSION(0, 1, 0);
    appInfo.apiVersion = VK_VERSION_1_1;

    VkInstanceCreateInfo instanceInfo = {};
    instanceInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instanceInfo.pApplicationInfo = &appInfo;
    auto extensions = requiredExtensions();
    instanceInfo.enabledExtensionCount = static_cast<std::uint32_t>(extensions.size());
    instanceInfo.ppEnabledExtensionNames = extensions.data();
    instanceInfo.enabledLayerCount = 0;
    instanceInfo.ppEnabledLayerNames = nullptr;
    instanceInfo.pNext = nullptr;

    VK_CHECK(vkCreateInstance(&instanceInfo, VK_CUSTOM_ALLOCATOR, &mInstance), "Failed to create instance");
}

void Device::getPhysicalDevices() {
    std::uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(mInstance, &deviceCount, nullptr);
    LOG("Available GPU(s): {}", deviceCount);

    std::vector<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(mInstance, &deviceCount, devices.data());
    int i = 0;
    for (auto &gpu : devices) {
        PhysicalDevice phyDev(gpu);
        gpus.insert(std::make_pair(phyDev.getProperties().deviceType, phyDev));
    }
}

void Device::pickPhysicalDevice() {
    auto dedicatedGPU = gpus.find(VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU);
    if (dedicatedGPU != gpus.end()) {
        mPhysicalDevice = dedicatedGPU->second;
        LOG("Choosen {} GPU", mPhysicalDevice.getProperties().deviceName);
        return;
    }
    dedicatedGPU = gpus.find(VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU);
    if (dedicatedGPU != gpus.end()) {
        mPhysicalDevice = dedicatedGPU->second;
        LOG("Choosen {} GPU", mPhysicalDevice.getProperties().deviceName);
        return;
    }
    RT_THROW("Cannot find suitable GPU");
}

void Device::createSurface() { mWindow.createSurface(mInstance, &mSurface); }

void Device::createCommandPool() {}

void Device::createLogicalDevice() {}


VkCommandBuffer Device::beginSingleTimeCommand() {
    VkCommandBufferAllocateInfo cmdAllocInfo = {};
    cmdAllocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    cmdAllocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    cmdAllocInfo.commandBufferCount = 1;
    cmdAllocInfo.commandPool = mCommandPool;

    VkCommandBuffer command;
    vkAllocateCommandBuffers(mDevice, &cmdAllocInfo, &command);

    VkCommandBufferBeginInfo begInfo = {};
    begInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    begInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    vkBeginCommandBuffer(command, &begInfo);
    return command;
}

void Device::endSingleTimeCommand(VkCommandBuffer command) {
    vkEndCommandBuffer(command);

    VkSubmitInfo submitInfo = {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &command;

    vkQueueSubmit(mGraphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(mGraphicsQueue);
    vkFreeCommandBuffers(mDevice, mCommandPool, 1, &command);
}

std::vector<const char*> Device::requiredExtensions() {
    std::uint32_t count = 0;
    const char **glfwExtensions = glfwGetRequiredInstanceExtensions(&count);

    std::vector<const char*> extensions(glfwExtensions, glfwExtensions + count);
    return extensions;
}

}
