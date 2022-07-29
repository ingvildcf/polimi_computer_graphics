#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>


#include <iostream>
#include <stdexcept>
#include <cstdlib>
#include <vector>
#include <array>
#include <optional>
#include <set>
#include <fstream>

const uint32_t WIDTH = 640;
const uint32_t HEIGHT = 480;


//// New: the vertex attributes specification
struct Vertex {
	glm::vec3 pos;
	glm::vec3 normal;
	glm::vec2 texCoord;
};


class Example11 {
	// To select the most appropriate swap chain features
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

//////////// New functions start here
	
	// To load a shader and create a module [Lesson 18]
	static std::vector<char> readFile(const std::string& filename) {
		std::ifstream file(filename, std::ios::ate | std::ios::binary);
		if (!file.is_open()) {
			throw std::runtime_error("failed to open file!");
		}
		
		size_t fileSize = (size_t) file.tellg();
		std::vector<char> buffer(fileSize);
		 
		file.seekg(0);
		file.read(buffer.data(), fileSize);
		 
		file.close();
		 
		return buffer;
	}

	VkShaderModule createShaderModule(const std::vector<char>& code, VkDevice device) {
		VkShaderModuleCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		createInfo.codeSize = code.size();
		createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());
		
		VkShaderModule shaderModule;
	
		VkResult result = vkCreateShaderModule(device, &createInfo, nullptr,
						&shaderModule);
		if (result != VK_SUCCESS) {
			throw std::runtime_error("failed to create shader module!");
		}
		
		return shaderModule;
	}
	
	// To support Z-Buffer in render passes [Lesson 19]
	VkFormat findDepthFormat(const VkPhysicalDevice &physicalDevice) {
		return findSupportedFormat({VK_FORMAT_D32_SFLOAT,
									VK_FORMAT_D32_SFLOAT_S8_UINT,
									VK_FORMAT_D24_UNORM_S8_UINT},
									VK_IMAGE_TILING_OPTIMAL, 								VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT,
								physicalDevice);
	}
	
	VkFormat findSupportedFormat(const std::vector<VkFormat> candidates,
						VkImageTiling tiling, VkFormatFeatureFlags features,
						const VkPhysicalDevice physicalDevice) {
		for (VkFormat format : candidates) {			VkFormatProperties props;
			
			vkGetPhysicalDeviceFormatProperties(physicalDevice, format, &props);
			if (tiling == VK_IMAGE_TILING_LINEAR &&
						(props.linearTilingFeatures & features) == features) {				return format;
			} else if (tiling == VK_IMAGE_TILING_OPTIMAL &&
						(props.optimalTilingFeatures & features) == features) {
				return format;
			}
		}

		throw std::runtime_error("failed to find supported Z-buffer format!");
	}
//////////// New functions ends here
		

	

public:
    void run() {
    	// init GLFW
        glfwInit();    	
		uint32_t glfwExtensionCount = 0;
		const char** glfwExtensions;		glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
		
		// Create a window (only to check the graphic capabilities of queue)
	    GLFWwindow* window;
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        window = glfwCreateWindow(WIDTH, HEIGHT, "Example 11", nullptr, nullptr);

    	// Create the instance
	    VkInstance instance;

    	VkApplicationInfo appInfo{};
       	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    	appInfo.pApplicationName = "Example 11";
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

				const std::vector<const char*> deviceExtensions = {
					VK_KHR_SWAPCHAIN_EXTENSION_NAME
				};
				createInfo.enabledExtensionCount =
							static_cast<uint32_t>(deviceExtensions.size());			
				createInfo.ppEnabledExtensionNames = deviceExtensions.data();

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

				// Query Swap Chain Capabilities, supported Formats and Present modes
				// One: capabilities
				VkSurfaceCapabilitiesKHR SCcapabilities;
				vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface,
				 		&SCcapabilities);
				
				// Two: formats
				uint32_t formatCount;
				vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatCount,
						nullptr);
										std::vector<VkSurfaceFormatKHR> SCformats;				if (formatCount != 0) {
					SCformats.resize(formatCount);
					vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface,
							&formatCount, SCformats.data());
				}
				// Three: present modes
				uint32_t presentModeCount;
				vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface,
					&presentModeCount, nullptr);
				
				std::vector<VkPresentModeKHR> SCpresentModes;
				if (presentModeCount != 0) {
					SCpresentModes.resize(presentModeCount);
					vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface,
							&presentModeCount, SCpresentModes.data());
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
					SCcreateInfo.queueFamilyIndexCount = 0; // Optional
					SCcreateInfo.pQueueFamilyIndices = nullptr; // Optional				}
				
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
				
				
//////////// New part starts here
				// [Lesson 17]
				
				// Create the Vertex format data
				//// Step one: bindings
				VkVertexInputBindingDescription bindingDescription{};
				bindingDescription.binding = 0;
				bindingDescription.stride = sizeof(Vertex);
				bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
				
				//// Step two: Vertex Input Descriptors
				std::array<VkVertexInputAttributeDescription, 3> attributeDescriptions{};

				attributeDescriptions[0].binding = 0;
				attributeDescriptions[0].location = 0;
				attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
				attributeDescriptions[0].offset = offsetof(Vertex, pos);
								
				attributeDescriptions[1].binding = 0;
				attributeDescriptions[1].location = 1;
				attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
				attributeDescriptions[1].offset = offsetof(Vertex, normal);
				
				attributeDescriptions[2].binding = 0;
				attributeDescriptions[2].location = 2;
				attributeDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
				attributeDescriptions[2].offset = offsetof(Vertex, texCoord);
				
				// Setp Three: Vertex Input State
				VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
				vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
						
				vertexInputInfo.vertexBindingDescriptionCount = 1;
				vertexInputInfo.vertexAttributeDescriptionCount =3;
				vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
				vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();		
				
				// Step four: Input assembly
				VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
				inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
				inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
				inputAssembly.primitiveRestartEnable = VK_FALSE;


				// [Lesson 18]

				// Load the shaders
				auto vertShaderCode = readFile("vert.spv");
				auto fragShaderCode = readFile("frag.spv");
				
				VkShaderModule vertShaderModule = createShaderModule(vertShaderCode, device);
				VkShaderModule fragShaderModule = createShaderModule(fragShaderCode, device);
				
				VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
				vertShaderStageInfo.sType =	VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
				vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
				vertShaderStageInfo.module = vertShaderModule;
				vertShaderStageInfo.pName = "main";
				
				VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
				fragShaderStageInfo.sType =	VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
				fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
				fragShaderStageInfo.module = fragShaderModule;
				fragShaderStageInfo.pName = "main";
				
				VkPipelineShaderStageCreateInfo shaderStages[] =
					{vertShaderStageInfo, fragShaderStageInfo};


				// [Lesson 19]
				
				VkPipeline Pipeline;
				
				// ViewPort
				
				VkViewport viewport{};
				viewport.x = 0.0f;
				viewport.y = 0.0f;				viewport.width = (float) extent.width;
				viewport.height = (float) extent.height;
				viewport.minDepth = 0.0f;
				viewport.maxDepth = 1.0f;

				// Scissor

				VkRect2D scissor{};
				scissor.offset = {0, 0};				scissor.extent = extent;

				// Viewport state

				VkPipelineViewportStateCreateInfo viewportState{};				viewportState.sType =
						VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
				viewportState.viewportCount = 1;
				viewportState.pViewports = &viewport;				viewportState.scissorCount = 1;
				viewportState.pScissors = &scissor;
				
				// Rasterizer state (Backface culling control)

				VkPipelineRasterizationStateCreateInfo rasterizer{};				rasterizer.sType =
						VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
				rasterizer.depthClampEnable = VK_FALSE;
				rasterizer.rasterizerDiscardEnable = VK_FALSE;
				rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
				rasterizer.lineWidth = 1.0f;
				rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
				rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
				rasterizer.depthBiasEnable = VK_FALSE;
				rasterizer.depthBiasConstantFactor = 0.0f; // Optional
				rasterizer.depthBiasClamp = 0.0f; // Optional
				rasterizer.depthBiasSlopeFactor = 0.0f; // Optional
				

				
				// Z-buffer and stencil
				VkPipelineDepthStencilStateCreateInfo depthStencil{};
				depthStencil.sType = 
						VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
				depthStencil.depthTestEnable = VK_TRUE;
				depthStencil.depthWriteEnable = VK_TRUE;
				depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
				depthStencil.depthBoundsTestEnable = VK_FALSE;				depthStencil.minDepthBounds = 0.0f; // Optional
				depthStencil.maxDepthBounds = 1.0f; // Optional
				depthStencil.stencilTestEnable = VK_FALSE;				depthStencil.front = {}; // Optional
				depthStencil.back = {}; // Optional

				
				// Pipeline Layout (Uniform variables) - currently empty
				VkPipelineLayout pipelineLayout;
				
				VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
				pipelineLayoutInfo.sType =
					VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
				pipelineLayoutInfo.setLayoutCount = 0; // Optional				pipelineLayoutInfo.pSetLayouts = nullptr; // Optional
				pipelineLayoutInfo.pushConstantRangeCount = 0; // Optional
				pipelineLayoutInfo.pPushConstantRanges = nullptr; // Optional
				
				if (vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr,
							&pipelineLayout) != VK_SUCCESS) {
					throw std::runtime_error("failed to create pipeline layout!");
				}
						

				// Color Blending (in other things for the pipeline) 

				VkPipelineColorBlendAttachmentState colorBlendAttachment{};
				colorBlendAttachment.colorWriteMask =
						VK_COLOR_COMPONENT_R_BIT |
						VK_COLOR_COMPONENT_G_BIT |
						VK_COLOR_COMPONENT_B_BIT |
						VK_COLOR_COMPONENT_A_BIT;
				colorBlendAttachment.blendEnable = VK_FALSE;				colorBlendAttachment.srcColorBlendFactor =
						VK_BLEND_FACTOR_ONE; // Optional
				colorBlendAttachment.dstColorBlendFactor =
						VK_BLEND_FACTOR_ZERO; // Optional
				colorBlendAttachment.colorBlendOp =
						VK_BLEND_OP_ADD; // Optional
				colorBlendAttachment.srcAlphaBlendFactor =
						VK_BLEND_FACTOR_ONE; // Optional
				colorBlendAttachment.dstAlphaBlendFactor =
						VK_BLEND_FACTOR_ZERO; // Optional
				colorBlendAttachment.alphaBlendOp =
						VK_BLEND_OP_ADD; // Optional
		
				VkPipelineColorBlendStateCreateInfo colorBlending{};
				colorBlending.sType =
						VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;				colorBlending.logicOpEnable = VK_FALSE;
				colorBlending.logicOp = VK_LOGIC_OP_COPY; // Optional
				colorBlending.attachmentCount = 1;
				colorBlending.pAttachments = &colorBlendAttachment;
				colorBlending.blendConstants[0] = 0.0f; // Optional
				colorBlending.blendConstants[1] = 0.0f; // Optional				colorBlending.blendConstants[2] = 0.0f; // Optional
				colorBlending.blendConstants[3] = 0.0f; // Optional

				// Multisampling (in other things)		
				VkPipelineMultisampleStateCreateInfo multisampling{};
				multisampling.sType =
						VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
				multisampling.sampleShadingEnable = VK_FALSE;
				multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
				multisampling.minSampleShading = 1.0f; // Optional				multisampling.pSampleMask = nullptr; // Optional
				multisampling.alphaToCoverageEnable = VK_FALSE; // Optional
				multisampling.alphaToOneEnable = VK_FALSE; // Optional



				// Render Pass configuration
				VkRenderPass renderPass;
				
				VkAttachmentDescription depthAttachment{};				depthAttachment.format = findDepthFormat(physicalDevice);
				depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
				depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
				depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
				depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
				depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;				depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
				depthAttachment.finalLayout =
								VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
		
				VkAttachmentReference depthAttachmentRef{};
				depthAttachmentRef.attachment = 1;				depthAttachmentRef.layout = 
								VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
		
		    	VkAttachmentDescription colorAttachment{};				colorAttachment.format = surfaceFormat.format;
				colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
				colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
				colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
				colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
				colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
				colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
				colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
				
				VkAttachmentReference colorAttachmentRef{};
				colorAttachmentRef.attachment = 0;
				colorAttachmentRef.layout =
						VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
				
				VkSubpassDescription subpass{};
				subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
				subpass.colorAttachmentCount = 1;
				subpass.pColorAttachments = &colorAttachmentRef;
				subpass.pDepthStencilAttachment = &depthAttachmentRef;
								VkSubpassDependency dependency{};				dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
				dependency.dstSubpass = 0;
				dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;				dependency.srcAccessMask = 0;
				dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
				dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		
				std::array<VkAttachmentDescription, 2> attachments =
										{colorAttachment, depthAttachment};
		
				VkRenderPassCreateInfo renderPassInfo{};				renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
				renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());;
				renderPassInfo.pAttachments = attachments.data();
				renderPassInfo.subpassCount = 1;
				renderPassInfo.pSubpasses = &subpass;				renderPassInfo.dependencyCount = 1;
				renderPassInfo.pDependencies = &dependency;
		
				result = vkCreateRenderPass(device, &renderPassInfo, nullptr,
							&renderPass);
				if (result != VK_SUCCESS) {
					throw std::runtime_error("failed to create render pass!");
				}		
				






				// Pipeline creation
				VkGraphicsPipelineCreateInfo pipelineInfo{};
				pipelineInfo.sType =
						VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
				pipelineInfo.stageCount = 2;
				pipelineInfo.pStages = shaderStages;
				pipelineInfo.pVertexInputState = &vertexInputInfo;				pipelineInfo.pInputAssemblyState = &inputAssembly;
				pipelineInfo.pViewportState = &viewportState;
				pipelineInfo.pRasterizationState = &rasterizer;
				pipelineInfo.pMultisampleState = &multisampling;
				pipelineInfo.pDepthStencilState = &depthStencil;
				pipelineInfo.pColorBlendState = &colorBlending;				pipelineInfo.pDynamicState = nullptr; // Optional
				pipelineInfo.layout = pipelineLayout;
				pipelineInfo.renderPass = renderPass;
				pipelineInfo.subpass = 0;
				pipelineInfo.basePipelineHandle = VK_NULL_HANDLE; // Optional				pipelineInfo.basePipelineIndex = -1; // Optional
				
				result = vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1,
						&pipelineInfo, nullptr, &Pipeline);
				if (result != VK_SUCCESS) {					throw std::runtime_error("failed to create graphics pipeline!");
				}
				
				std::cout << "\tPipeline succesfully created\n";
				
				
				// resources release (new part)
				vkDestroyPipeline(device, Pipeline, nullptr);
    			vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
				vkDestroyRenderPass(device, renderPass, nullptr);
				// Afer pipeline has been created, shader modules can be destroyed
				vkDestroyShaderModule(device, fragShaderModule, nullptr);
				vkDestroyShaderModule(device, vertShaderModule, nullptr);


//////////// New part ends here

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
    Example11 app;

    try {
        app.run();
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}