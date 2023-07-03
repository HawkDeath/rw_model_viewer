#ifndef _INSTANCE_H
#define _INSTANCE_H

#include <vulkan/vulkan.h>
#include <vector>

namespace rw {
	class Instance
	{
	public:
		Instance();
		~Instance();

		std::vector<VkPhysicalDevice> gpus() const {
			return mGpus;
		}

	private:
		VkInstance mInstance;
		std::vector<VkPhysicalDevice> mGpus;
	};

	Instance::Instance()
	{
	}

	Instance::~Instance()
	{
	}
}


#endif