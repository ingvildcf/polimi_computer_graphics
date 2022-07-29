#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <iostream>
#include <stdexcept>
#include <cstdlib>
#include <vector>
#include <optional>
#include <set>

const uint32_t WIDTH = 640;
const uint32_t HEIGHT = 480;

class Example08 {
public:
    void run() {
    	// init GLFW
        glfwInit();    	
		uint32_t glfwExtensionCount = 0;
		const char** glfwExtensions;		glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
		
		// Create a window (only to check the graphic capabilities of queue)
	    GLFWwindow* window;
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        window = glfwCreateWindow(WIDTH, HEIGHT, "Assignment 12", nullptr, nullptr);

    	// Create the instance
	    VkInstance instance;

    	VkApplicationInfo appInfo{};
       	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    	appInfo.pApplicationName = "Assignment 12";
    	appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    	appInfo.pEngineName = "No Engine";
    	appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);		appInfo.apiVersion = VK_API_VERSION_1_0;
		
		VkInstanceCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		createInfo.pApplicationInfo = &appInfo;
		createInfo.enabledExtensionCount = glfwExtensionCount;
		createInfo.ppEnabledExtensionNames = glfwExtensions;
		createInfo.enabledLayerCount = 0;
		
		VkResult result = vkCreateInstance(&createInfo, nullptr, &instance);
		
		if(result != VK_SUCCESS) {
			throw std::runtime_error("failed to create instance!");
		}
		
		//
		////// THE NEW PART STARTS HERE!!
		//
		
		// create the Surface [requires the Vulkan Instance and
		//					   the window to be already created]
		VkSurfaceKHR surface;
		if (glfwCreateWindowSurface(instance, window, nullptr, &surface) != VK_SUCCESS) {
			throw std::runtime_error("failed to create window surface!");
		}

		// Enumerate devices
		uint32_t deviceCount = 0;
		result = vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);
		if(result != VK_SUCCESS || deviceCount <= 0) {
			throw std::runtime_error("failed to find GPUs with Vulkan Support!");
		}
		
		std::cout << deviceCount << " Physical Device(s) found \n\n";
		
		std::vector<VkPhysicalDevice> devices(deviceCount);
		result = vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());
		if(result != VK_SUCCESS) {
			throw std::runtime_error("Could not enumerate devices");
		}

		// check device properties and features
		int i = 0;		
		for (const auto& physicalDevice : devices) { 
			std::cout << "Device: " << i << "\n";

			// Show device properties			
			VkPhysicalDeviceProperties deviceProperties;
			vkGetPhysicalDeviceProperties(physicalDevice, &deviceProperties);

			std::cout <<  "\tAPI version: 0x" << std::hex <<
								deviceProperties.apiVersion << "\n";
			std::cout << "\tDriver version: 0x" << std::hex <<
								deviceProperties.driverVersion << "\n";
			std::cout << "\tVendor ID: 0x" << std::hex << deviceProperties.vendorID << "\n";
			std::cout << "\tDevice ID: 0x" << std::hex << deviceProperties.deviceID << "\n";
			std::cout << "\tPhysical Device Type: " << deviceProperties.deviceType << "\t";
			if(deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU )
				std::cout << " (Discrete GPU)\n";
			if(deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU )
				std::cout << " (Integrated GPU)\n";
			if(deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU )
				std::cout << " (Virtual GPU)\n";
			if(deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_CPU )
				std::cout <<  " (CPU)\n";
			std::cout << "\tDevice Name: " << deviceProperties.deviceName << "\n";

			// Show device features
			VkPhysicalDeviceFeatures deviceFeatures;
			vkGetPhysicalDeviceFeatures(physicalDevice, &deviceFeatures);

			std::cout << "\n\tPhysical Device Features:\n";
			std::cout << "\t\tgeometryShader = " << deviceFeatures.geometryShader << "\n";
			std::cout << "\t\ttessellationShader = " <<
								deviceFeatures.tessellationShader << "\n";
			std::cout << "\t\tmultiDrawIndirect = " << deviceFeatures.multiDrawIndirect << "\n";
			std::cout << "\t\twideLines = " << deviceFeatures.wideLines << "\n";
			std::cout << "\t\tlargePoints = " << deviceFeatures.largePoints << "\n";
			std::cout << "\t\tmultiViewport = " << deviceFeatures.multiViewport << "\n";
			std::cout << "\t\tocclusionQueryPrecise = " << 
								deviceFeatures.occlusionQueryPrecise << "\n";
			std::cout << "\t\tpipelineStatisticsQuery = " <<
								deviceFeatures.pipelineStatisticsQuery << "\n";
			std::cout << "\t\tshaderFloat64 = " << deviceFeatures.shaderFloat64 << "\n";
			std::cout << "\t\tshaderInt64 = " << deviceFeatures.shaderInt64 << "\n";
			std::cout << "\t\tshaderInt16 = " << deviceFeatures.shaderInt16 << "\n";
			
			// Show device memory properties
			VkPhysicalDeviceMemoryProperties vpdmp;
			vkGetPhysicalDeviceMemoryProperties(physicalDevice, &vpdmp);
			
			std::cout << "\n\tMemory Types: " << vpdmp.memoryTypeCount << "\n";
			for(unsigned int i = 0; i < vpdmp.memoryTypeCount; i++) {
				VkMemoryType vmt = vpdmp.memoryTypes[i];
					std::cout << "\t\tMemory: " << i << ":";
				if((vmt.propertyFlags & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT) != 0 )
					std::cout << " DeviceLocal";
				if((vmt.propertyFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) != 0 )
					std::cout << " HostVisible";
				if((vmt.propertyFlags & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT) != 0 )
					std::cout << " HostCoherent"; 
				if((vmt.propertyFlags & VK_MEMORY_PROPERTY_HOST_CACHED_BIT) != 0 )
					std::cout << " HostCached";
				if((vmt.propertyFlags & VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT) != 0 )
					std::cout << " LazilyAllocated";
				std::cout << "\n";
			}


			// Memory heaps
			std::cout << "\n\tMemory Heaps: " << vpdmp.memoryHeapCount << "\n";
			for(unsigned int i = 0; i < vpdmp.memoryHeapCount; i++ ) {
				std::cout << "\t\tHeap : " << i;
				VkMemoryHeap vmh = vpdmp.memoryHeaps[i];
				std::cout << " size = " << std::hex << (unsigned long int)vmh.size;
				if((vmh.flags & VK_MEMORY_HEAP_DEVICE_LOCAL_BIT) != 0)
					std::cout << " DeviceLocal";
				std::cout << "\n";	
			}
			
			// Comment on the previous commands:
			// Users need to list the capabilities of devices to rank them and pick
			// the most suitable if more the one is present, or take a drastical
			// fallback action (i.e. switch to OpenGL) if no suitable device is present.
			// In simple applications, such as the one we are doing in this course,
			// the user can always pick the first device. Since on some rare systems,
			// there could be more than one device, and only the second might have
			// Vulkan support (i.e. a very old PC with a poor integraded GPU that does
			// not support Vulkan, expanded with a newer graphics card which has
			// Vulkan support), it can be safer to loop among devices, and select
			// the one which has graphics capabilities. However, such capabilities
			// are generally seen examining the queues, and it will be done below	

			// Extensions
			//
			// Since Device Layers have been deprecated, it is ok to have zero
			// extensions here, or to have the same as the Instance Extensions
			// as seen in Example E07
			//
			uint32_t layerCount;
			result = vkEnumerateDeviceLayerProperties(physicalDevice, &layerCount, nullptr);
			if(result != VK_SUCCESS) {
				throw std::runtime_error("failed to enumerate layers!");
			}
			std::cout << "\n\tLayers found: " << layerCount << "\n";
			std::vector<VkLayerProperties> deviceLayers(layerCount);
			vkEnumerateDeviceLayerProperties(physicalDevice, &layerCount, deviceLayers.data());
			
			for(auto& layer : deviceLayers) {
				std::cout << "\t\t" << std::hex << layer.specVersion;
				std::cout << " " << std::hex << layer.implementationVersion;
				std::cout << " '" << layer.layerName;
				std::cout << "' '" << layer.description << "'\n";
				
				uint32_t extensionCount;
				vkEnumerateDeviceExtensionProperties(physicalDevice, layer.layerName,
													 &extensionCount, nullptr);
				std::vector<VkExtensionProperties> deviceExtensions(extensionCount);
				vkEnumerateDeviceExtensionProperties(physicalDevice, layer.layerName,
													 &extensionCount, deviceExtensions.data());
				for(auto& extension : deviceExtensions) {
					std::cout << "\t\t\t" << std::hex << extension.specVersion;
					std::cout << "' '" << std::hex << extension.extensionName << "'\n";
				}
			}

			// Queues
			uint32_t queueFamCount = -1;
			vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamCount, nullptr);
			std::cout << "\n\tQueue Families found: " << queueFamCount << "\n";
			std::vector<VkQueueFamilyProperties> queues(queueFamCount);
			vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice,
												     &queueFamCount, queues.data());
			// to check if it has graphic and presentation support
			std::optional<uint32_t> aQueueWithGraphicsCapability;	
			std::optional<uint32_t> aQueueWithPresentationCapability;	
			for(unsigned int i = 0; i < queueFamCount; i++ ) {
				std::cout << "\t\tFam. " << i << ": queueCount = " <<
							 queues[i].queueCount << "; ";
				if((queues[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) != 0) {
					std::cout << " Graphics";
					aQueueWithGraphicsCapability = i;
				}
				if((queues[i].queueFlags & VK_QUEUE_COMPUTE_BIT) != 0)
					std::cout << " Compute";
				if((queues[i].queueFlags & VK_QUEUE_TRANSFER_BIT) != 0)
					std::cout << " Transfer";
				// check presentation support
				VkBool32 presentSupport = false;
				vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, i, surface, &presentSupport);
				if(presentSupport) {
					std::cout << " Presentation";
					aQueueWithPresentationCapability = i;
				}
				std::cout << "\n";
			}
			
			// This is the check of graphic capabilities of the device, as seen looking
			// at its queues. Also in this case, most of the systems will have identical
			// queue families, each one with all capabilities. However, it might happen
			// on some adapters, that presentation, graphic and transfer queues are
			// different. For this reason, we have to add some logic to support cases
			// in which all features come from the same queue, or when different
			// queues must be used.
					
			if(aQueueWithGraphicsCapability.has_value() &&
			   aQueueWithPresentationCapability.has_value()) {
				std::cout << "\tGraphic queue family selected: " <<
							 aQueueWithGraphicsCapability.value() << "\n";
				std::cout << "\tPresentation queue family selected: " <<
							 aQueueWithPresentationCapability.value() << "\n";

				std::cout << "\n\tTRYING TO CREATE A LOGICAL DEVICE, WITH QUEUEs AND A COMMAND BUFFER\n";
				// data structure ofr the logical device and the required queues
			    VkDevice device;
			    VkQueue graphicsQueue;
			    VkQueue presentQueue;
				
				// Prepare the queue definitions
				std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
				std::set<uint32_t> uniqueQueueFamilies =
						{aQueueWithGraphicsCapability.value(),
						 aQueueWithPresentationCapability.value()};				
				float queuePriority = 1.0f;
				for (uint32_t queueFamily : uniqueQueueFamilies) {
					VkDeviceQueueCreateInfo queueCreateInfo{};
					queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
					queueCreateInfo.queueFamilyIndex = queueFamily;
					queueCreateInfo.queueCount = 1;
					queueCreateInfo.pQueuePriorities = &queuePriority;
					queueCreateInfos.push_back(queueCreateInfo);
				}
				
				VkPhysicalDeviceFeatures deviceFeatures{};
//				deviceFeatures.samplerAnisotropy = VK_TRUE;		// Examples of properties
//				deviceFeatures.sampleRateShading = VK_TRUE;
				
				// Prepare the main device creation structure
				VkDeviceCreateInfo createInfo{};
				createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
				createInfo.pQueueCreateInfos = queueCreateInfos.data();				createInfo.queueCreateInfoCount = 
					static_cast<uint32_t>(queueCreateInfos.size());
				createInfo.pEnabledFeatures = &deviceFeatures;	// properties specified above
				createInfo.enabledExtensionCount = 0;			// No extensions right now
				createInfo.ppEnabledExtensionNames = nullptr;
				createInfo.enabledLayerCount = 0;				// No debug now
				
				result = vkCreateDevice(physicalDevice, &createInfo, nullptr, &device);
				
				if (result != VK_SUCCESS) {
					throw std::runtime_error("failed to create logical device!");
				}
				
				// get queues
				vkGetDeviceQueue(device, aQueueWithGraphicsCapability.value(), 0,
								 &graphicsQueue);
				vkGetDeviceQueue(device, aQueueWithPresentationCapability.value(), 0,
								 &presentQueue);

				std::cout << "\tDevice created\n";

				// Creates the command pools
				VkCommandPool commandPool;
				
				VkCommandPoolCreateInfo poolInfo{};
				poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;				poolInfo.queueFamilyIndex = aQueueWithGraphicsCapability.value();
				poolInfo.flags = 0; // Optional
				
				result = vkCreateCommandPool(device, &poolInfo, nullptr, &commandPool);
				if (result != VK_SUCCESS) {					throw std::runtime_error("failed to create command pool!");
				}				
				// Create the command buffer
				VkCommandBuffer commandBuffer;
				
		    	VkCommandBufferAllocateInfo allocInfo{};
				allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
				allocInfo.commandPool = commandPool;
				allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
				allocInfo.commandBufferCount = 1;
				
				result = vkAllocateCommandBuffers(device, &allocInfo,						&commandBuffer);
				if (result != VK_SUCCESS) {
					throw std::runtime_error("failed to allocate command buffer!");
				}
		
				std::cout << "\tCommand Pool and Command Buffers created\n";


				// releases device and pools
		    	vkDestroyCommandPool(device, commandPool, nullptr);
		 		vkDestroyDevice(device, nullptr);		
				std::cout << "\tResources released correctly\n\n";
					 
			} else {
				std::cout << "This device cannot be used\n";
			}			

			i++;		}


		// Cleanup
		vkDestroySurfaceKHR(instance, surface, nullptr);
    	vkDestroyInstance(instance, nullptr);
        glfwDestroyWindow(window);
        glfwTerminate();
    }
};

int main() {
    Example08 app;

    try {
        app.run();
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}