#include <render/Buffer.h>
#include <Log.h>

namespace rw
{
  VkDeviceSize getAlignment(VkDeviceSize instanceSize, VkDeviceSize minOffsetAlignment) {
    if (minOffsetAlignment > 0) {
      return (instanceSize + minOffsetAlignment - 1) & ~(minOffsetAlignment - 1);
    }
    return instanceSize;
  }

  Buffer::Buffer(Device& dev, VkDeviceSize bufferSize, VkBufferUsageFlags usageFlags, VkMemoryPropertyFlags memoryPropertyFlags, VkDeviceSize minOffsetAlignment) : device{ dev }, mBufferSize{ bufferSize }, mUsageFlags { usageFlags }, mMemoryPropertyFlags{ memoryPropertyFlags }
  {
    mAlignmentSize = getAlignment(bufferSize, minOffsetAlignment);
    device.createBuffer(bufferSize, usageFlags, memoryPropertyFlags, mBuffer, mMemoryDevice);
  }

  Buffer::~Buffer()
  {

    vkDestroyBuffer(device.getDevice(), mBuffer, nullptr);
    vkFreeMemory(device.getDevice(), mMemoryDevice, nullptr);
  }

  void Buffer::update(const std::vector<uint8_t>& data)
  {
    // map memory

    // copy memory

    // flush memory

    // unmap memory
  }


}