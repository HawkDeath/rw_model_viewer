#ifndef BUFFER_H
#define BUFFER_H

#include <render/Device.h>

#include <vector>

namespace rw
{
  class Buffer
  {
  public:
	Buffer(Device &dev, VkDeviceSize bufferSize, VkBufferUsageFlags usageFlags, VkMemoryPropertyFlags memoryPropertyFlags, VkDeviceSize minOffsetAlignment = 1);
	~Buffer();

	Buffer(const Buffer&) = delete;
	Buffer& operator=(const Buffer&) = delete;

	void update(const std::vector<uint8_t>& data);

	VkBuffer getHandler() const { return mBuffer; }
	VkDeviceMemory getMemoryDevice() const { return mMemoryDevice; }
	const void* getMappedMemory() const { return mapped; }

	VkDeviceSize getBufferSize() const { return mBufferSize; }
	VkDeviceSize getAlignmentSize() const { return mAlignmentSize; }

	VkBufferUsageFlags getUsageFlags() const { return mUsageFlags; }
	VkMemoryPropertyFlags getMemoryPropertyFlags() const { return mMemoryPropertyFlags; }

  private:
	Device& device;

	void* mapped = nullptr;
	VkBufferUsageFlags mUsageFlags;
	VkMemoryPropertyFlags mMemoryPropertyFlags;

	VkBuffer mBuffer = { VK_NULL_HANDLE };
	VkDeviceMemory mMemoryDevice = { VK_NULL_HANDLE };

	VkDeviceSize mBufferSize = { 0 };
	VkDeviceSize mAlignmentSize = { 0 };
  };

}


#endif // !BUFFER_H
