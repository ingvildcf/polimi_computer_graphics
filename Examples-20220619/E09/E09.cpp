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

class Example09 {
	VkSurfaceFormatKHR chooseSwapSurfaceFormat(
				const std::vector<VkSurfaceFormatKHR>& availableFormats)
	{
		for (const auto& availableFormat : availableFormats) {
			if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB &&
				availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
				return availableFormat;
			}
		}
		
		return availableFormats[0];
	}

	VkPresentModeKHR chooseSwapPresentMode(
			const std::vector<VkPresentModeKHR>& availablePresentModes) {
		for (const auto& availablePresentMode : availablePresentModes) {
			if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
				return availablePresentMode;
			}
		}		return VK_PRESENT_MODE_FIFO_KHR;
	}
	
	VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities,
								GLFWwindow* window) {
		if (capabilities.currentExtent.width != UINT32_MAX) {
			return capabilities.currentExtent;
		} else {
			int width, height;
			glfwGetFramebufferSize(window, &width, &height);
			
			VkExtent2D actualExtent = {
				static_cast<uint32_t>(width),
				static_cast<uint32_t>(height)
			};			actualExtent.width = std::max(capabilities.minImageExtent.width,
					std::min(capabilities.maxImageExtent.width, actualExtent.width));
			actualExtent.height = std::max(capabilities.minImageExtent.height,
					std::min(capabilities.maxImageExtent.height, actualExtent.height));
			return actualExtent;
		}	}

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
			

			// Queues
			uint32_t queueFamCount = -1;
			vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamCount, nullptr);
			std::vector<VkQueueFamilyProperties> queues(queueFamCount);
			vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice,
												     &queueFamCount, queues.data());
			// to check if it has graphic and presentation support
			std::optional<uint32_t> aQueueWithGraphicsCapability;	
			std::optional<uint32_t> aQueueWithPresentationCapability;	
			for(unsigned int i = 0; i < queueFamCount; i++ ) {
				if((queues[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) != 0) {
					aQueueWithGraphicsCapability = i;
				}
				VkBool32 presentSupport = false;
				vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, i, surface, &presentSupport);
				if(presentSupport) {
					aQueueWithPresentationCapability = i;
				}
			}
								
			if(aQueueWithGraphicsCapability.has_value() &&
			   aQueueWithPresentationCapability.has_value()) {
				std::cout << "\tGraphic queue family selected: " <<
							 aQueueWithGraphicsCapability.value() << "\n";
				std::cout << "\tPresentation queue family selected: " <<
							 aQueueWithPresentationCapability.value() << "\n";

				std::cout << "\n\tTRYING TO CREATE A LOGICAL DEVICE, WITH QUEUEs\n";
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
				
				// Prepare the main device creation structure
				VkDeviceCreateInfo createInfo{};
				createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
				createInfo.pQueueCreateInfos = queueCreateInfos.data();				createInfo.queueCreateInfoCount = 
					static_cast<uint32_t>(queueCreateInfos.size());
				createInfo.pEnabledFeatures = &deviceFeatures;	// properties specified above
/** NEW! **/				
				// Here we assume that this device, since it has a presentation queue, it supports
				// the Swap Chain extension. This however is not guarantee, and a check if the
				// required extension is supported before picking the device should be done.
				// See the Vulkan Tutorial for an example on how this can be done.
				const std::vector<const char*> deviceExtensions = {
					VK_KHR_SWAPCHAIN_EXTENSION_NAME
				};
				createInfo.enabledExtensionCount =
							static_cast<uint32_t>(deviceExtensions.size());			
				createInfo.ppEnabledExtensionNames = deviceExtensions.data();
/** New part ends here **/
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

/** NEW! **/
				// Query Swap Chain Capabilities, supported Formats and Present modes
				// One: capabilities
				VkSurfaceCapabilitiesKHR SCcapabilities;
				vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface,
				 		&SCcapabilities);
				std::cout << "\t Min. Image Count: " << SCcapabilities.minImageCount << "\n";
				std::cout << "\t Max. Image Count: " << SCcapabilities.maxImageCount << "\n";
				std::cout << "\t Min. Image Exent: " << SCcapabilities.minImageExtent.width << "x" <<
														SCcapabilities.minImageExtent.height << "\n";
				std::cout << "\t Cur. Image Exent: " << SCcapabilities.currentExtent.width << "x" <<
														SCcapabilities.currentExtent.height << "\n";
				std::cout << "\t Max. Image Exent: " << SCcapabilities.maxImageExtent.width << "x" <<
														SCcapabilities.maxImageExtent.height << "\n";
				std::cout << "\n";
				
				// Two: formats
				uint32_t formatCount;
				vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatCount,
						nullptr);
										std::vector<VkSurfaceFormatKHR> SCformats;				if (formatCount != 0) {
					SCformats.resize(formatCount);
					vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface,
							&formatCount, SCformats.data());
					std::cout << "\t Supported Formats: " << formatCount << "\n";
					for(int i = 0; i < formatCount; i++) {
						  std::cout << "\t\tFormat: " << SCformats[i].format <<
						  			   ", Color Space:" << SCformats[i].colorSpace <<"\n";
					}
				}
				std::cout << "[see: <https://www.khronos.org/registry/vulkan/specs/1.3-extensions/man/html/VkFormat.html> for Formats]\n";
				std::cout << "[see: <https://www.khronos.org/registry/vulkan/specs/1.3-extensions/man/html/VkColorSpaceKHR.html> for Color Spaces]\n";
				std::cout << "\n";
				
				// Three: present modes
				uint32_t presentModeCount;
				vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface,
					&presentModeCount, nullptr);
				
				std::vector<VkPresentModeKHR> SCpresentModes;
				if (presentModeCount != 0) {
					SCpresentModes.resize(presentModeCount);
					vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface,
							&presentModeCount, SCpresentModes.data());
					std::cout << "\t Supported Modes: " << presentModeCount << "\n";
					for(int i = 0; i < presentModeCount; i++) {
						switch(SCpresentModes[i]) {
						  case VK_PRESENT_MODE_IMMEDIATE_KHR:
						  	std::cout << "\t\tVK_PRESENT_MODE_IMMEDIATE_KHR\n";
						  	break;
						  case VK_PRESENT_MODE_MAILBOX_KHR:
						  	std::cout << "\t\tVK_PRESENT_MODE_MAILBOX_KHR\n";
						  	break;
						  case VK_PRESENT_MODE_FIFO_KHR:
						  	std::cout << "\t\tVK_PRESENT_MODE_FIFO_KHR\n";
						  	break;
						  case VK_PRESENT_MODE_FIFO_RELAXED_KHR:
						  	std::cout << "\t\tVK_PRESENT_MODE_FIFO_RELAXED_KHR\n";
						  	break;
						  default :
						  	std::cout << "\t\tMode: " << SCpresentModes[i] << "\n";
						  	break;
						}
					}
				}
				std::cout << "\n";

				// Creates the swap chain
				std::cout << "\tTrying to create the Swap Chain\n";
				
				// one: selection of format, mode and number of images
				VkSurfaceFormatKHR surfaceFormat =
						chooseSwapSurfaceFormat(SCformats);
				VkPresentModeKHR presentMode =
						chooseSwapPresentMode(SCpresentModes);
				VkExtent2D extent = chooseSwapExtent(SCcapabilities, window);
				
				uint32_t imageCount = SCcapabilities.minImageCount + 1;
				
				if (SCcapabilities.maxImageCount > 0 &&
						imageCount > SCcapabilities.maxImageCount) {					imageCount = SCcapabilities.maxImageCount;
				}

				std::cout << "\tFormat chosen: " << surfaceFormat.format <<
						  			   ", Color Space:" << surfaceFormat.colorSpace << "\n";
				switch(presentMode) {
				  case VK_PRESENT_MODE_IMMEDIATE_KHR:
				  	std::cout << "\tVK_PRESENT_MODE_IMMEDIATE_KHR\n";
				  	break;
				  case VK_PRESENT_MODE_MAILBOX_KHR:
				  	std::cout << "\tVK_PRESENT_MODE_MAILBOX_KHR\n";
				  	break;
				  case VK_PRESENT_MODE_FIFO_KHR:
				  	std::cout << "\tVK_PRESENT_MODE_FIFO_KHR\n";
				  	break;
				  case VK_PRESENT_MODE_FIFO_RELAXED_KHR:
				  	std::cout << "\tVK_PRESENT_MODE_FIFO_RELAXED_KHR\n";
				  	break;
				  default :
				  	std::cout << "\t\tMode: " << presentMode << "\n";
				  	break;
				}
				std::cout << "\tChosen Image Exent: " << extent.width << "x" <<
														extent.height << "\n";
				std::cout << "\tSwap chain Image Count: " << imageCount << "\n";
				
				// two: actual swap chain creation
				VkSwapchainKHR swapChain;
				
				VkSwapchainCreateInfoKHR SCcreateInfo{};
				SCcreateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
				SCcreateInfo.surface = surface;
				SCcreateInfo.minImageCount = imageCount;
				SCcreateInfo.imageFormat = surfaceFormat.format;				SCcreateInfo.imageColorSpace = surfaceFormat.colorSpace;
				SCcreateInfo.imageExtent = extent;
				SCcreateInfo.imageArrayLayers = 1;
				SCcreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
				
				uint32_t queueFamilyIndices[] = {aQueueWithGraphicsCapability.value(),
												 aQueueWithPresentationCapability.value()};
				if (aQueueWithPresentationCapability != aQueueWithPresentationCapability) {
					SCcreateInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;					SCcreateInfo.queueFamilyIndexCount = 2;
					SCcreateInfo.pQueueFamilyIndices = queueFamilyIndices;
				} else {
					SCcreateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
					SCcreateInfo.queueFamilyIndexCount = 0; 
					SCcreateInfo.pQueueFamilyIndices = nullptr; 				}
				
				SCcreateInfo.preTransform = SCcapabilities.currentTransform;
				SCcreateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
				SCcreateInfo.presentMode = presentMode;
				SCcreateInfo.clipped = VK_TRUE;
				SCcreateInfo.oldSwapchain = VK_NULL_HANDLE;
				 
				VkResult result = vkCreateSwapchainKHR(device, &SCcreateInfo,
													   nullptr, &swapChain);
				if (result != VK_SUCCESS) {
					throw std::runtime_error("failed to create swap chain!");
				}
				std::cout << "\tSwap chain created\n";
				
				//swap chain image view acquisition
				// one: VkImage
			    std::vector<VkImage> swapChainImages;

				vkGetSwapchainImagesKHR(device, swapChain, &imageCount, nullptr);
				swapChainImages.resize(imageCount);				vkGetSwapchainImagesKHR(device, swapChain, &imageCount,
						swapChainImages.data());

				// two: VkImageView
				std::vector<VkImageView> swapChainImageViews;

				swapChainImageViews.resize(swapChainImages.size());
				
				for (size_t i = 0; i < swapChainImages.size(); i++) {
					VkImageViewCreateInfo viewInfo{};
					viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
					viewInfo.image = swapChainImages[i];					viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
					viewInfo.format = surfaceFormat.format;
					viewInfo.subresourceRange.aspectMask =
												VK_IMAGE_ASPECT_COLOR_BIT;
					viewInfo.subresourceRange.baseMipLevel = 0;
					viewInfo.subresourceRange.levelCount = 1;
					viewInfo.subresourceRange.baseArrayLayer = 0;					viewInfo.subresourceRange.layerCount = 1;

					VkImageView imageView;
			
					VkResult result = vkCreateImageView(device, &viewInfo, nullptr,
							&imageView);
					if (result != VK_SUCCESS) {						throw std::runtime_error("failed to create image view!");
					}
					
					swapChainImageViews[i] = imageView;
				}
				std::cout << "\tImages and Views retrieved\n";

				// device resource release	
				for (size_t i = 0; i < swapChainImageViews.size(); i++){
					vkDestroyImageView(device, swapChainImageViews[i], nullptr);
				}		
				vkDestroySwapchainKHR(device, swapChain, nullptr);
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
    Example09 app;

    try {
        app.run();
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}