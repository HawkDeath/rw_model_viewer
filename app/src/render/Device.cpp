#include <render/Device.h>
#include <Log.h>

#include <algorithm>
#include <set>

const std::vector<const char*> deviceExtensions = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

namespace rw {
  Device::Device(Window& window)
    : mWindow{ window }
  {
    createInstance();
    createSurface();
    getPhysicalDevices();
    pickPhysicalDevice();
    createLogicalDevice();
    createCommandPool();
  }

  Device::~Device()
  {
    vkDestroyCommandPool(mDevice, mCommandPool, nullptr);
    vkDestroyDevice(mDevice, nullptr);
    vkDestroySurfaceKHR(mInstance, mSurface, nullptr);
    vkDestroyInstance(mInstance, nullptr);
  }

  void Device::createInstance()
  {
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

    VK_CHECK(vkCreateInstance(&instanceInfo, nullptr, &mInstance), "Failed to create instance");

    LOG("Extensions number {}", extensions.size());
    for (auto& ext : extensions)
    {
      LOG("\t{}", ext);
    }
  }

  void Device::getPhysicalDevices()
  {
    std::uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(mInstance, &deviceCount, nullptr);
    LOG("Available GPU(s): {}", deviceCount);

    std::vector<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(mInstance, &deviceCount, devices.data());

    int i = 0;
    for (auto& gpu : devices) {
      PhysicalDevice phyDev(gpu);
      gpus.insert(std::make_pair(phyDev.getProperties().deviceType, phyDev));
    }
  }

  void Device::pickPhysicalDevice()
  {
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

  void Device::createCommandPool()
  {
    QueueFamilyIndices indices = findQueueFamilies();
    VkCommandPoolCreateInfo commandPoolInfo = {};
    commandPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    commandPoolInfo.queueFamilyIndex = indices.graphicsFamily.value();
    commandPoolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    VK_CHECK(vkCreateCommandPool(mDevice, &commandPoolInfo, nullptr, &mCommandPool), "Failed to create command pool");
  }

  void Device::createLogicalDevice()
  {
    QueueFamilyIndices indices = findQueueFamilies();

    std::vector<VkDeviceQueueCreateInfo> queues;
    std::set<uint32_t> uniqueQueueFamilies = { indices.graphicsFamily.value(), indices.presentFamily.value() };
    float prio = 1.0f;

    for (uint32_t queueFamily : uniqueQueueFamilies)
    {
      VkDeviceQueueCreateInfo queueinfo;
      queueinfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
      queueinfo.pNext = VK_NULL_HANDLE;
      queueinfo.queueFamilyIndex = queueFamily;
      queueinfo.queueCount = 1;
      queueinfo.pQueuePriorities = &prio;
      queueinfo.flags = 0U;
      queues.push_back(queueinfo);
    }

    VkPhysicalDeviceFeatures requestedFeatures = {};
    requestedFeatures.samplerAnisotropy = VK_TRUE;

    VkDeviceCreateInfo deviceInfo = {};
    deviceInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    deviceInfo.pNext = VK_NULL_HANDLE;

    deviceInfo.queueCreateInfoCount = static_cast<uint32_t>(queues.size());
    deviceInfo.pQueueCreateInfos = queues.data();

    deviceInfo.pEnabledFeatures = &requestedFeatures;

    deviceInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
    deviceInfo.ppEnabledExtensionNames = deviceExtensions.data();

    deviceInfo.enabledLayerCount = 0;

    VK_CHECK(vkCreateDevice(mPhysicalDevice.getPhysicalDevice(), &deviceInfo, nullptr, &mDevice), "Failed to create logical device");

    vkGetDeviceQueue(mDevice, indices.graphicsFamily.value(), 0, &mGraphicsQueue);
    vkGetDeviceQueue(mDevice, indices.presentFamily.value(), 0, &mPresentQueue);
  }

  void Device::createImageWithInfo(const VkImageCreateInfo& imageInfo, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory)
  {
    VK_CHECK(vkCreateImage(mDevice, &imageInfo, nullptr, &image), "Failed to create image");

    VkMemoryRequirements memReq;
    vkGetImageMemoryRequirements(mDevice, image, &memReq);

    VkMemoryAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memReq.size;
    allocInfo.memoryTypeIndex = findMemoryType(memReq.memoryTypeBits, properties);

    VK_CHECK(vkAllocateMemory(mDevice, &allocInfo, nullptr, &imageMemory), "Failed to allocate image memory");
    VK_CHECK(vkBindImageMemory(mDevice, image, imageMemory, VkDeviceSize(0)), "Failed to bind image with image memory");
  }

  VkCommandBuffer Device::beginSingleTimeCommand()
  {
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

  void Device::endSingleTimeCommand(VkCommandBuffer command)
  {
    vkEndCommandBuffer(command);

    VkSubmitInfo submitInfo = {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &command;

    vkQueueSubmit(mGraphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(mGraphicsQueue);
    vkFreeCommandBuffers(mDevice, mCommandPool, 1, &command);
  }

  void Device::createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory)
  {
    VkBufferCreateInfo bufferInfo = {};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = size;
    bufferInfo.usage = usage;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VK_CHECK(vkCreateBuffer(mDevice, &bufferInfo, nullptr, &buffer), "failed to create buffer!");

    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(mDevice, buffer, &memRequirements);

    VkMemoryAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties);

    VK_CHECK(vkAllocateMemory(mDevice, &allocInfo, nullptr, &bufferMemory), "failed to allocate vertex buffer memory!");

    vkBindBufferMemory(mDevice, buffer, bufferMemory, 0);
  }

  std::vector<const char*> Device::requiredExtensions()
  {
    std::uint32_t count = 0;
    const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&count);

    std::vector<const char*> extensions(glfwExtensions, glfwExtensions + count);
    return extensions;
  }

  QueueFamilyIndices Device::findQueueFamilies()
  {
    // get graphics and present queue index
    QueueFamilyIndices indices;
    int i = 0;
    for (const auto& queue : mPhysicalDevice.getQueueFamilyProperties())
    {
      if (queue.queueFlags & VK_QUEUE_GRAPHICS_BIT)
      {
        indices.graphicsFamily = i;
      }

      VkBool32 isPresentSupport = VK_FALSE;
      vkGetPhysicalDeviceSurfaceSupportKHR(mPhysicalDevice.getPhysicalDevice(), i, mSurface, &isPresentSupport);
      if (isPresentSupport)
      {
        indices.presentFamily = i;
      }

      if (indices.isComplete())
        break;

      i++;
    }
    return indices;
  }

  SwapChainSupportDetails Device::getSwapChainSupport()
  {
    SwapChainSupportDetails result;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(mPhysicalDevice.getPhysicalDevice(), mSurface, &result.capabilities);

    uint32_t formatCount = 0u;
    vkGetPhysicalDeviceSurfaceFormatsKHR(mPhysicalDevice.getPhysicalDevice(), mSurface, &formatCount, nullptr);

    if (formatCount == 0) RT_THROW("Failed to find any supported surface format");

    result.formats.resize(formatCount);
    vkGetPhysicalDeviceSurfaceFormatsKHR(mPhysicalDevice.getPhysicalDevice(), mSurface, &formatCount, result.formats.data());

    uint32_t presentModeCount = 0u;
    vkGetPhysicalDeviceSurfacePresentModesKHR(mPhysicalDevice.getPhysicalDevice(), mSurface, &presentModeCount, nullptr);

    if (presentModeCount == 0) RT_THROW("Failed to find any present mode");

    result.presentModes.resize(presentModeCount);
    vkGetPhysicalDeviceSurfacePresentModesKHR(mPhysicalDevice.getPhysicalDevice(), mSurface, &presentModeCount, result.presentModes.data());

    return result;
  }

  uint32_t Device::findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties)
  {
    VkPhysicalDeviceMemoryProperties memProp;
    vkGetPhysicalDeviceMemoryProperties(mPhysicalDevice.getPhysicalDevice(), &memProp);

    for (uint32_t i = 0u; i < memProp.memoryTypeCount; ++i)
    {
      if ((typeFilter & (1 << i)) && (memProp.memoryTypes[i].propertyFlags & properties) == properties)
      {
        return i;
      }
    }
    RT_THROW("Failed to find suitable memory type");
  }

  VkFormat Device::findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features)
  {
    for (auto format : candidates)
    {
      VkFormatProperties props;
      vkGetPhysicalDeviceFormatProperties(mPhysicalDevice.getPhysicalDevice(), format, &props);

      if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features)
      {
        LOG("Choosen image format {}", format); // TODO: add string formmat converter
        return format;
      }
      else if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features)
      {
        LOG("Choosen image format {}", format);
        return format;
      }
    }
    RT_THROW("Failed to find a supported format!");
  }
}
