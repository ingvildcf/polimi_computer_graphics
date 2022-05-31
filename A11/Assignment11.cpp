#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <iostream>
#include <stdexcept>
#include <cstdlib>
#include <vector>
#include <cstring>
#include <optional>
#include <set>
#include <cstdint>
#include <algorithm>
#include <fstream>
#include <array>
#include <unordered_map>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>#include <chrono>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

const uint32_t WIDTH = 800;
const uint32_t HEIGHT = 600;

const std::string MODEL_PATH = "models/";
const std::string TEXTURE_PATH = "textures/";
const std::string SHADER_PATH = "shaders/";

const int MAX_FRAMES_IN_FLIGHT = 2;

#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>

const std::vector<const char*> validationLayers = {	"VK_LAYER_KHRONOS_validation"
};

const std::vector<const char*> deviceExtensions = {
	VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

#ifdef NDEBUG	const bool enableValidationLayers = false;
	const bool Verbose = false;
#else
	const bool enableValidationLayers = true;
	const bool Verbose = true;
#endif

enum PipelineType {Flat, Wire};

struct Model {
	const char *ObjFile;
	const char *TextureFile;
	const glm::vec3 pos;
	const float scale;
	const PipelineType pt;
};

const std::vector<Model> SceneToLoad = {	{"X-WING-baker.obj", "A.png", {0,0,0}, 0.1, Wire}
};


namespace std {
	template<> struct hash<std::vector<float>> {
		size_t operator()(std::vector<float> const& vertex) const {
			size_t h = 0;
			for(int i = 0; i < vertex.size(); i++) {
				h ^= static_cast<size_t>(vertex[i]);
			}			return h;
		}
	};
}


struct VertexDescriptor {
	bool hasPos;
	bool hasNormal;
	bool hasTexCoord;
	bool hasColor;
	bool hasTangent;
	
	int deltaPos;
	int deltaNormal;
	int deltaTexCoord;
	int deltaColor;
	int deltaTangent;
	
	int locPos;
	int locNormal;
	int locTexCoord;
	int locColor;
	int locTangent;
	
	int size;
	int loc;
	
	VertexDescriptor(bool hPos, bool hNormal, bool hTexCoord, bool hColor, bool hTangent) {
		size = 0;
		loc = 0;
		
		hasPos = hPos;
		hasNormal = hNormal;
		hasTexCoord = hTexCoord;
		hasColor = hColor;
		hasTangent = hTangent;
		
		if(hasPos) {deltaPos = size; size += 3; locPos = loc; loc++;} else {deltaPos = -1; locPos = -1;}
		if(hasNormal) {deltaNormal = size; size += 3; locNormal = loc; loc++;} else {deltaNormal = -1; locNormal = -1;}
		if(hasTexCoord) {deltaTexCoord = size; size += 2; locTexCoord = loc; loc++;} else {deltaTexCoord = -1; locTexCoord = -1;}
		if(hasColor) {deltaColor = size; size += 4; locColor = loc; loc++;} else {deltaColor = -1; locColor = -1;}
		if(hasTangent) {deltaTangent = size; size += 4; locTangent = loc; loc++;} else {deltaTangent = -1; locTangent = -1;}
	}

	
	VkVertexInputBindingDescription getBindingDescription() {
		VkVertexInputBindingDescription bindingDescription{};
		bindingDescription.binding = 0;
		bindingDescription.stride = size * sizeof(float);		bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
		
		return bindingDescription;	}
	
	std::vector<VkVertexInputAttributeDescription>
						getAttributeDescriptions(int binding = 0) {
		std::vector<VkVertexInputAttributeDescription>
						attributeDescriptions{};
		attributeDescriptions.resize(loc);
		if(hasPos) {
			attributeDescriptions[locPos].binding = binding;
			attributeDescriptions[locPos].location = locPos;
			attributeDescriptions[locPos].format = VK_FORMAT_R32G32B32_SFLOAT;			attributeDescriptions[locPos].offset = deltaPos * sizeof(float);
		}
						
		if(hasNormal) {
			attributeDescriptions[locNormal].binding = binding;
			attributeDescriptions[locNormal].location = locNormal;
			attributeDescriptions[locNormal].format = VK_FORMAT_R32G32B32_SFLOAT;			attributeDescriptions[locNormal].offset = deltaNormal * sizeof(float);
		}
		
		if(hasTexCoord) {
			attributeDescriptions[locTexCoord].binding = binding;
			attributeDescriptions[locTexCoord].location = locTexCoord;
			attributeDescriptions[locTexCoord].format = VK_FORMAT_R32G32_SFLOAT;			attributeDescriptions[locTexCoord].offset = deltaTexCoord * sizeof(float);
		}
								
		if(hasColor) {
			attributeDescriptions[locColor].binding = binding;
			attributeDescriptions[locColor].location = locColor;
			attributeDescriptions[locColor].format = VK_FORMAT_R32G32B32A32_SFLOAT;			attributeDescriptions[locColor].offset = deltaColor * sizeof(float);
		}
						
		if(hasTangent) {
			attributeDescriptions[locTangent].binding = binding;
			attributeDescriptions[locTangent].location = locTangent;
			attributeDescriptions[locTangent].format = VK_FORMAT_R32G32B32A32_SFLOAT;			attributeDescriptions[locTangent].offset = deltaTangent * sizeof(float);
		}
					
		return attributeDescriptions;	}

	glm::vec3 getPos(float *data, int i) {
		if(hasPos) {
			return glm::vec3(data[i * size + deltaPos], data[i * size + deltaPos + 1], data[i * size + deltaPos + 2]);
		} else {
			return glm::vec3(0.0f);
			std::cerr << "Vertex has no position \n";
		}
	}
	
	void setPos(float *data, int i, glm::vec3 pos) {
		if(hasPos) {
			data[i * size + deltaPos] = pos.x;
			data[i * size + deltaPos + 1] = pos.y;
			data[i * size + deltaPos + 2] = pos.z;
		} else {
			std::cerr << "Vertex has no position \n";
		}
	}

	glm::vec3 getNormal(float *data, int i) {
		if(hasPos) {
			return glm::vec3(data[i * size + deltaNormal], data[i * size + deltaNormal + 1], data[i * size + deltaNormal + 2]);
		} else {
			return glm::vec3(0.0f);
			std::cerr << "Vertex has no normal \n";
		}
	}
	
	void setNormal(float *data, int i, glm::vec3 norm) {
		if(hasNormal) {
			data[i * size + deltaNormal] = norm.x;
			data[i * size + deltaNormal + 1] = norm.y;
			data[i * size + deltaNormal + 2] = norm.z;
		} else {
			std::cerr << "Vertex has no normal \n";
		}
	}

	glm::vec2 getTexCoord(float *data, int i) {
		if(hasPos) {
			return glm::vec2(data[i * size + deltaTexCoord], data[i * size + deltaTexCoord + 1]);
		} else {
			return glm::vec2(0.0f);
			std::cerr << "Vertex has no UV \n";
		}
	}
	
	void setTexCoord(float *data, int i, glm::vec3 uv) {
		if(hasNormal) {
			data[i * size + deltaTexCoord] = uv.x;
			data[i * size + deltaTexCoord + 1] = uv.y;
		} else {
			std::cerr << "Vertex has no UV \n";
		}
	}
};



// Questi dovrebbero essere definibili a piacimento e nel modo piu' semplice possibile,
// traferirsi nello shader
struct UniformBufferObject {
	alignas(16) glm::mat4 worldMat;
	alignas(16) glm::mat4 viewMat;	alignas(16) glm::mat4 prjMat;
};

struct GlobalUniformBufferObject {
	alignas(16) float time;
};

VkResult CreateDebugUtilsMessengerEXT(VkInstance instance,
			const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
			const VkAllocationCallbacks* pAllocator,
			VkDebugUtilsMessengerEXT* pDebugMessenger) {
	auto func = (PFN_vkCreateDebugUtilsMessengerEXT)
				vkGetInstanceProcAddr(instance,
					"vkCreateDebugUtilsMessengerEXT");
	if (func != nullptr) {		return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
	} else {
		return VK_ERROR_EXTENSION_NOT_PRESENT;
	}
}

void DestroyDebugUtilsMessengerEXT(VkInstance instance,
			VkDebugUtilsMessengerEXT debugMessenger,
			const VkAllocationCallbacks* pAllocator) {	auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)
				vkGetInstanceProcAddr(instance,
					"vkDestroyDebugUtilsMessengerEXT");
	if (func != nullptr) {
		func(instance, debugMessenger, pAllocator);
	}
}

struct QueueFamilyIndices {
	std::optional<uint32_t> graphicsFamily;
	std::optional<uint32_t> presentFamily;

	bool isComplete() {
		return graphicsFamily.has_value() &&
			   presentFamily.has_value();	}
};
    
struct SwapChainSupportDetails {
	VkSurfaceCapabilitiesKHR capabilities;
	std::vector<VkSurfaceFormatKHR> formats;	std::vector<VkPresentModeKHR> presentModes;
};


struct errorcode {
	VkResult resultCode;
	std::string meaning;
}
ErrorCodes[ ] = {
	{ VK_NOT_READY, "Not Ready" },
	{ VK_TIMEOUT, "Timeout" },
	{ VK_EVENT_SET, "Event Set" },
	{ VK_EVENT_RESET, "Event Reset" },
	{ VK_INCOMPLETE, "Incomplete" },
	{ VK_ERROR_OUT_OF_HOST_MEMORY, "Out of Host Memory" },
	{ VK_ERROR_OUT_OF_DEVICE_MEMORY, "Out of Device Memory" },
	{ VK_ERROR_INITIALIZATION_FAILED, "Initialization Failed" },
	{ VK_ERROR_DEVICE_LOST, "Device Lost" },
	{ VK_ERROR_MEMORY_MAP_FAILED, "Memory Map Failed" },
	{ VK_ERROR_LAYER_NOT_PRESENT, "Layer Not Present" },
	{ VK_ERROR_EXTENSION_NOT_PRESENT, "Extension Not Present" },
	{ VK_ERROR_FEATURE_NOT_PRESENT, "Feature Not Present" },
	{ VK_ERROR_INCOMPATIBLE_DRIVER, "Incompatible Driver" },
	{ VK_ERROR_TOO_MANY_OBJECTS, "Too Many Objects" },
	{ VK_ERROR_FORMAT_NOT_SUPPORTED, "Format Not Supported" },
	{ VK_ERROR_FRAGMENTED_POOL, "Fragmented Pool" },
	{ VK_ERROR_SURFACE_LOST_KHR, "Surface Lost" },
	{ VK_ERROR_NATIVE_WINDOW_IN_USE_KHR, "Native Window in Use" },
	{ VK_SUBOPTIMAL_KHR, "Suboptimal" },
	{ VK_ERROR_OUT_OF_DATE_KHR, "Error Out of Date" },
	{ VK_ERROR_INCOMPATIBLE_DISPLAY_KHR, "Incompatible Display" },
	{ VK_ERROR_VALIDATION_FAILED_EXT, "Valuidation Failed" },
	{ VK_ERROR_INVALID_SHADER_NV, "Invalid Shader" },
	{ VK_ERROR_OUT_OF_POOL_MEMORY_KHR, "Out of Pool Memory" },
	{ VK_ERROR_INVALID_EXTERNAL_HANDLE, "Invalid External Handle" },

};

void PrintVkError( VkResult result ) {
	const int numErrorCodes = sizeof( ErrorCodes ) / sizeof( struct errorcode );
	std::string meaning = "";
	for( int i = 0; i < numErrorCodes; i++ ) {
		if( result == ErrorCodes[i].resultCode ) {
			meaning = ErrorCodes[i].meaning;
			break;
		}
	}
	std::cout << "Error: " << result << ", " << meaning << "\n";
}

struct ModelData {
	VertexDescriptor *vertDesc;
	std::vector<float> vertices;
	std::vector<uint32_t> indices;
	VkBuffer vertexBuffer;
	VkDeviceMemory vertexBufferMemory;
	VkBuffer indexBuffer;
	VkDeviceMemory indexBufferMemory;	
};

struct TextureData {
	VkImage textureImage;
	VkDeviceMemory textureImageMemory;
	VkImageView textureImageView;
	VkSampler textureSampler;
	uint32_t mipLevels;
};	

struct SceneModel {
	// Model data
	ModelData MD;
	
	// Texture data
	TextureData TD;
};


class Assignment11 {
public:
    void run() {
        initWindow();
        initVulkan();
        mainLoop();
        cleanup();
    }

private:
    GLFWwindow* window;
    VkInstance instance;
	VkDebugUtilsMessengerEXT debugMessenger;
	VkSurfaceKHR surface;
    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
    VkDevice device;
    VkQueue graphicsQueue;
    VkQueue presentQueue;
    VkSwapchainKHR swapChain;
    std::vector<VkImage> swapChainImages;
	VkFormat swapChainImageFormat;	VkExtent2D swapChainExtent;
	std::vector<VkImageView> swapChainImageViews;
	VkRenderPass renderPass;
	
	VkDescriptorPool descriptorPool;

	VertexDescriptor 	phongAndSkyBoxVertices = VertexDescriptor(true, false, false, false, false);
	VertexDescriptor 	textVertices = VertexDescriptor(true, false, true, false, false);

	// Phong pipeline
 	VkDescriptorSetLayout PhongDescriptorSetLayout;
  	VkPipelineLayout PhongPipelineLayout;
  	VkPipelineLayout PhongWirePipelineLayout;
	VkPipeline PhongPipeline;
	VkPipeline PhongWirePipeline;
	//// For the first uniform (per object)
	std::vector<VkBuffer> uniformBuffers;	std::vector<VkDeviceMemory> uniformBuffersMemory;
	//// For the second uniform (per scene)
	std::vector<VkBuffer> globalUniformBuffers;	std::vector<VkDeviceMemory> globalUniformBuffersMemory;	
	// to access uniforms in the pipeline
	std::vector<VkDescriptorSet> PhongDescriptorSets;
	// Scene graph using the Phong pipeline
	std::vector<SceneModel> Scene;
	

	std::vector<VkFramebuffer> swapChainFramebuffers;
	VkCommandPool commandPool;
	std::vector<VkCommandBuffer> commandBuffers;
	std::vector<VkSemaphore> imageAvailableSemaphores;
	std::vector<VkSemaphore> renderFinishedSemaphores;
	std::vector<VkFence> inFlightFences;
	std::vector<VkFence> imagesInFlight;
	size_t currentFrame = 0;
	bool framebufferResized = false;

	// Questi dovrebbero essere i render target
	VkImage depthImage;	VkDeviceMemory depthImageMemory;
	VkImageView depthImageView;

	VkSampleCountFlagBits msaaSamples = VK_SAMPLE_COUNT_1_BIT;
	VkImage colorImage;
	VkDeviceMemory colorImageMemory;	VkImageView colorImageView;
	
	
	// Camera
	glm::mat3 CamDir = glm::mat3(1.0f);
	glm::vec3 CamPos = glm::vec3(0.0f, 0.5f, 2.5f);
	
	// Other global variables
	int curText = 0;
	
    void initWindow() {
        glfwInit();

        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

        window = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan", nullptr, nullptr);
        glfwSetWindowUserPointer(window, this);
        glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);
    }

	static void framebufferResizeCallback(GLFWwindow* window, int width, int height) {
		auto app = reinterpret_cast<Assignment11*>
						(glfwGetWindowUserPointer(window));
		app->framebufferResized = true;
	}    

    void initVulkan() {
		createInstance();
		setupDebugMessenger();
		createSurface();
		pickPhysicalDevice();
		createLogicalDevice();
		createSwapChain();
		createImageViews();
		createRenderPass();
		createDescriptorSetLayouts();
		createPipelines();
		createCommandPool();
		createColorResources();
		createDepthResources();
		createFramebuffers();
		loadModels();
		createUniformBuffers();
		createDescriptorPool();
		createDescriptorSets();
		createCommandBuffers();
		createSyncObjects();
    }

    void createInstance() {
    	VkApplicationInfo appInfo{};
       	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    	appInfo.pApplicationName = "Hello Triangle";
    	appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    	appInfo.pEngineName = "No Engine";
    	appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);		appInfo.apiVersion = VK_API_VERSION_1_0;
		
		VkInstanceCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		createInfo.pApplicationInfo = &appInfo;

		uint32_t glfwExtensionCount = 0;
		const char** glfwExtensions;		glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
				
		createInfo.enabledExtensionCount = glfwExtensionCount;
		createInfo.ppEnabledExtensionNames = glfwExtensions;

		createInfo.enabledLayerCount = 0;
		
		uint32_t extensionCount = 0;
		vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
		 
		std::vector<VkExtensionProperties> extensionsPr(extensionCount);
		vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, 
		 										extensionsPr.data());
		 
		auto extensions = getRequiredExtensions();
		createInfo.enabledExtensionCount =
			static_cast<uint32_t>(extensions.size());
		createInfo.ppEnabledExtensionNames = extensions.data();		
		
		if (enableValidationLayers && !checkValidationLayerSupport()) {
			throw std::runtime_error("validation layers requested, but not available!");		}

		VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo;
		if (enableValidationLayers) {
			createInfo.enabledLayerCount =
				static_cast<uint32_t>(validationLayers.size());			createInfo.ppEnabledLayerNames = validationLayers.data();
			
			populateDebugMessengerCreateInfo(debugCreateInfo);
			createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)
									&debugCreateInfo;
		} else {			createInfo.enabledLayerCount = 0;
			createInfo.pNext = nullptr;
		}
		
		VkResult result = vkCreateInstance(&createInfo, nullptr, &instance);
		
		if(result != VK_SUCCESS) {
		 	PrintVkError(result);
			throw std::runtime_error("failed to create instance!");
		}
    }
    
    std::vector<const char*> getRequiredExtensions() {
		uint32_t glfwExtensionCount = 0;
		const char** glfwExtensions;		glfwExtensions =
			glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

		std::vector<const char*> extensions(glfwExtensions,
			glfwExtensions + glfwExtensionCount);		if (enableValidationLayers) {
			extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
		}
		
		return extensions;	}
	
	bool checkValidationLayerSupport() {
		uint32_t layerCount;
		vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

		std::vector<VkLayerProperties> availableLayers(layerCount);		vkEnumerateInstanceLayerProperties(&layerCount,
					availableLayers.data());

		for (const char* layerName : validationLayers) {
			bool layerFound = false;
			
			for (const auto& layerProperties : availableLayers) {
				if (strcmp(layerName, layerProperties.layerName) == 0) {
					layerFound = true;					break;
				}
			}
		
			if (!layerFound) {
				return false;			}
		}
		
		return true;    
	}

    void populateDebugMessengerCreateInfo(
    		VkDebugUtilsMessengerCreateInfoEXT& createInfo) {
    	createInfo = {};
		createInfo.sType =
			VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
		createInfo.messageSeverity =
//			VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT | // disabled in the tutorial
			VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
			VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |			VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
		createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
			VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
			VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
		createInfo.pfnUserCallback = debugCallback;
		createInfo.pUserData = nullptr;
    }

	static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
		VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
		VkDebugUtilsMessageTypeFlagsEXT messageType,		const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData) {

		std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;				return VK_FALSE;
	}

	void setupDebugMessenger() {
		if (!enableValidationLayers) return;

		VkDebugUtilsMessengerCreateInfoEXT createInfo{};
		populateDebugMessengerCreateInfo(createInfo);
		
		if (CreateDebugUtilsMessengerEXT(instance, &createInfo, nullptr,
				&debugMessenger) != VK_SUCCESS) {
			throw std::runtime_error("failed to set up debug messenger!");
		}
	}

    void createSurface() {
    	if (glfwCreateWindowSurface(instance, window, nullptr, &surface)
    			!= VK_SUCCESS) {
			throw std::runtime_error("failed to create window surface!");
		}
    }

    void pickPhysicalDevice() {
    	uint32_t deviceCount = 0;
    	vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);
    	 
    	if (deviceCount == 0) {
			throw std::runtime_error("failed to find GPUs with Vulkan support!");
		}
		
		std::vector<VkPhysicalDevice> devices(deviceCount);
		vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());
		
		std::cout << "Physical devices found: " << deviceCount << "\n";
		
		for (const auto& device : devices) {
			if (isDeviceSuitable(device)) {
				physicalDevice = device;
				msaaSamples = getMaxUsableSampleCount();
				std::cout << "\n\nMaximum samples for anti-aliasing: " << msaaSamples << "\n\n\n";
				break;			}
		}
		
		if (physicalDevice == VK_NULL_HANDLE) {
			throw std::runtime_error("failed to find a suitable GPU!");
		}
    }

    bool isDeviceSuitable(VkPhysicalDevice device) {
 		QueueFamilyIndices indices = findQueueFamilies(device);

		bool extensionsSupported = checkDeviceExtensionSupport(device);

		bool swapChainAdequate = false;
		if (extensionsSupported) {
			SwapChainSupportDetails swapChainSupport = querySwapChainSupport(device);			swapChainAdequate = !swapChainSupport.formats.empty() &&
								!swapChainSupport.presentModes.empty();
		}
		
		VkPhysicalDeviceFeatures supportedFeatures;
		vkGetPhysicalDeviceFeatures(device, &supportedFeatures);
		
		return indices.isComplete() && extensionsSupported && swapChainAdequate &&
						supportedFeatures.samplerAnisotropy;
	}
    
    QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device) {		QueueFamilyIndices indices;
		
		uint32_t queueFamilyCount = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount,
						nullptr);

		std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);		vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount,
								queueFamilies.data());
								
		int i=0;
		for (const auto& queueFamily : queueFamilies) {
			if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
				indices.graphicsFamily = i;
			}
				
			VkBool32 presentSupport = false;
			vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface,
												 &presentSupport);			if (presentSupport) {
			 	indices.presentFamily = i;
			}

			if (indices.isComplete()) {
				break;
			}			
			i++;
		}

		return indices;
	}

	bool checkDeviceExtensionSupport(VkPhysicalDevice device) {
		uint32_t extensionCount;
		vkEnumerateDeviceExtensionProperties(device, nullptr,					&extensionCount, nullptr);
					
		std::vector<VkExtensionProperties> availableExtensions(extensionCount);
		vkEnumerateDeviceExtensionProperties(device, nullptr,					&extensionCount, availableExtensions.data());
					
		std::set<std::string> requiredExtensions(deviceExtensions.begin(),
					deviceExtensions.end());
							for (const auto& extension : availableExtensions){
			requiredExtensions.erase(extension.extensionName);
		}

		return requiredExtensions.empty();
	}

	SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device) {
		SwapChainSupportDetails details;
		
		 vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface,
		 		&details.capabilities);

		uint32_t formatCount;
		vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount,
				nullptr);
						if (formatCount != 0) {
			details.formats.resize(formatCount);
			vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface,
					&formatCount, details.formats.data());
		}
		
		uint32_t presentModeCount;
		vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface,
			&presentModeCount, nullptr);
		
		if (presentModeCount != 0) {
			details.presentModes.resize(presentModeCount);
			vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface,
					&presentModeCount, details.presentModes.data());
		}
		 		return details;
	}

	VkSampleCountFlagBits getMaxUsableSampleCount() {
		VkPhysicalDeviceProperties physicalDeviceProperties;
		vkGetPhysicalDeviceProperties(physicalDevice, &physicalDeviceProperties);
		
		VkSampleCountFlags counts =
				physicalDeviceProperties.limits.framebufferColorSampleCounts &
				physicalDeviceProperties.limits.framebufferDepthSampleCounts;
		
		if (counts & VK_SAMPLE_COUNT_64_BIT) { return VK_SAMPLE_COUNT_64_BIT; }
		if (counts & VK_SAMPLE_COUNT_32_BIT) { return VK_SAMPLE_COUNT_32_BIT; }
		if (counts & VK_SAMPLE_COUNT_16_BIT) { return VK_SAMPLE_COUNT_16_BIT; }
		if (counts & VK_SAMPLE_COUNT_8_BIT) { return VK_SAMPLE_COUNT_8_BIT; }
		if (counts & VK_SAMPLE_COUNT_4_BIT) { return VK_SAMPLE_COUNT_4_BIT; }
		if (counts & VK_SAMPLE_COUNT_2_BIT) { return VK_SAMPLE_COUNT_2_BIT; }

		return VK_SAMPLE_COUNT_1_BIT;
	}	
	
	void createLogicalDevice() {
		QueueFamilyIndices indices = findQueueFamilies(physicalDevice);
		
		std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
		std::set<uint32_t> uniqueQueueFamilies =
				{indices.graphicsFamily.value(), indices.presentFamily.value()};		
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
		deviceFeatures.samplerAnisotropy = VK_TRUE;
		deviceFeatures.sampleRateShading = VK_TRUE;
		deviceFeatures.fillModeNonSolid = VK_TRUE;
		
		VkDeviceCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		
		createInfo.pQueueCreateInfos = queueCreateInfos.data();		createInfo.queueCreateInfoCount = 
			static_cast<uint32_t>(queueCreateInfos.size());
		
		createInfo.pEnabledFeatures = &deviceFeatures;
		createInfo.enabledExtensionCount =				static_cast<uint32_t>(deviceExtensions.size());
		createInfo.ppEnabledExtensionNames = deviceExtensions.data();

		if (enableValidationLayers) {			createInfo.enabledLayerCount = 
					static_cast<uint32_t>(validationLayers.size());
			createInfo.ppEnabledLayerNames = validationLayers.data();
		} else {
			createInfo.enabledLayerCount = 0;
		}
		
		VkResult result = vkCreateDevice(physicalDevice, &createInfo, nullptr, &device);
		
		if (result != VK_SUCCESS) {
		 	PrintVkError(result);
			throw std::runtime_error("failed to create logical device!");
		}
		
		vkGetDeviceQueue(device, indices.graphicsFamily.value(), 0, &graphicsQueue);
		vkGetDeviceQueue(device, indices.presentFamily.value(), 0, &presentQueue);
	}
	
	void createSwapChain() {
		SwapChainSupportDetails swapChainSupport =
				querySwapChainSupport(physicalDevice);		VkSurfaceFormatKHR surfaceFormat =
				chooseSwapSurfaceFormat(swapChainSupport.formats);
		VkPresentModeKHR presentMode =
				chooseSwapPresentMode(swapChainSupport.presentModes);
		VkExtent2D extent = chooseSwapExtent(swapChainSupport.capabilities);
		
		uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
		
		if (swapChainSupport.capabilities.maxImageCount > 0 &&
				imageCount > swapChainSupport.capabilities.maxImageCount) {			imageCount = swapChainSupport.capabilities.maxImageCount;
		}
		
		VkSwapchainCreateInfoKHR createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		createInfo.surface = surface;
		createInfo.minImageCount = imageCount;
		createInfo.imageFormat = surfaceFormat.format;		createInfo.imageColorSpace = surfaceFormat.colorSpace;
		createInfo.imageExtent = extent;
		createInfo.imageArrayLayers = 1;
		createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
		
		QueueFamilyIndices indices = findQueueFamilies(physicalDevice);		uint32_t queueFamilyIndices[] = {indices.graphicsFamily.value(),
										 indices.presentFamily.value()};
		if (indices.graphicsFamily != indices.presentFamily) {
			createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;			createInfo.queueFamilyIndexCount = 2;
			createInfo.pQueueFamilyIndices = queueFamilyIndices;
		} else {
			createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
			createInfo.queueFamilyIndexCount = 0; // Optional
			createInfo.pQueueFamilyIndices = nullptr; // Optional		}
		
		 createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
		 createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
		 createInfo.presentMode = presentMode;
		 createInfo.clipped = VK_TRUE;
		 createInfo.oldSwapchain = VK_NULL_HANDLE;
		 
		 VkResult result = vkCreateSwapchainKHR(device, &createInfo, nullptr, &swapChain);
		 if (result != VK_SUCCESS) {
		 	PrintVkError(result);
			throw std::runtime_error("failed to create swap chain!");
		}
		
		vkGetSwapchainImagesKHR(device, swapChain, &imageCount, nullptr);
		swapChainImages.resize(imageCount);		vkGetSwapchainImagesKHR(device, swapChain, &imageCount,
				swapChainImages.data());
				
		swapChainImageFormat = surfaceFormat.format;
		swapChainExtent = extent;	}

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
	
	VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities) {
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

	void createImageViews() {
		swapChainImageViews.resize(swapChainImages.size());
		
		for (size_t i = 0; i < swapChainImages.size(); i++) {
			swapChainImageViews[i] = createImageView(swapChainImages[i],
													 swapChainImageFormat,
													 VK_IMAGE_ASPECT_COLOR_BIT, 1,
													 VK_IMAGE_VIEW_TYPE_2D, 1);		}
	}
	
	VkImageView createImageView(VkImage image, VkFormat format,
								VkImageAspectFlags aspectFlags,
								uint32_t mipLevels, VkImageViewType type, int layerCount) {
		VkImageViewCreateInfo viewInfo{};
		viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		viewInfo.image = image;		viewInfo.viewType = type;
		viewInfo.format = format;
		viewInfo.subresourceRange.aspectMask = aspectFlags;
		viewInfo.subresourceRange.baseMipLevel = 0;
		viewInfo.subresourceRange.levelCount = mipLevels;
		viewInfo.subresourceRange.baseArrayLayer = 0;		viewInfo.subresourceRange.layerCount = layerCount;
		VkImageView imageView;

		VkResult result = vkCreateImageView(device, &viewInfo, nullptr,
				&imageView);
		if (result != VK_SUCCESS) {		 	PrintVkError(result);
			throw std::runtime_error("failed to create image view!");
		}
		return imageView;
	}

    void createRenderPass() {
		VkAttachmentDescription colorAttachmentResolve{};
		colorAttachmentResolve.format = swapChainImageFormat;
		colorAttachmentResolve.samples = VK_SAMPLE_COUNT_1_BIT;
		colorAttachmentResolve.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;		colorAttachmentResolve.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		colorAttachmentResolve.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		colorAttachmentResolve.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		colorAttachmentResolve.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;		colorAttachmentResolve.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

		VkAttachmentReference colorAttachmentResolveRef{};
		colorAttachmentResolveRef.attachment = 2;
		colorAttachmentResolveRef.layout =
						VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		
		VkAttachmentDescription depthAttachment{};		depthAttachment.format = findDepthFormat();
		depthAttachment.samples = msaaSamples;
		depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;		depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		depthAttachment.finalLayout =
						VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		VkAttachmentReference depthAttachmentRef{};
		depthAttachmentRef.attachment = 1;		depthAttachmentRef.layout = 
						VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    	VkAttachmentDescription colorAttachment{};		colorAttachment.format = swapChainImageFormat;
		colorAttachment.samples = msaaSamples;
		colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		colorAttachment.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkAttachmentReference colorAttachmentRef{};
		colorAttachmentRef.attachment = 0;
		colorAttachmentRef.layout =
				VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		
		VkSubpassDescription subpass{};
		subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass.colorAttachmentCount = 1;
		subpass.pColorAttachments = &colorAttachmentRef;
		subpass.pDepthStencilAttachment = &depthAttachmentRef;
		subpass.pResolveAttachments = &colorAttachmentResolveRef;
				VkSubpassDependency dependency{};		dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
		dependency.dstSubpass = 0;
		dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;		dependency.srcAccessMask = 0;
		dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

		std::array<VkAttachmentDescription, 3> attachments =
								{colorAttachment, depthAttachment,
								 colorAttachmentResolve};

		VkRenderPassCreateInfo renderPassInfo{};		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());;
		renderPassInfo.pAttachments = attachments.data();
		renderPassInfo.subpassCount = 1;
		renderPassInfo.pSubpasses = &subpass;		renderPassInfo.dependencyCount = 1;
		renderPassInfo.pDependencies = &dependency;

		VkResult result = vkCreateRenderPass(device, &renderPassInfo, nullptr,
					&renderPass);
		if (result != VK_SUCCESS) {
		 	PrintVkError(result);
			throw std::runtime_error("failed to create render pass!");
		}		
	}

	void createDescriptorSetLayouts() {
		createPhongDescriptorSetLayout();
	}
	
	void createPhongDescriptorSetLayout() {
		VkDescriptorSetLayoutBinding uboLayoutBinding{};		uboLayoutBinding.binding = 0;
		uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		uboLayoutBinding.descriptorCount = 1;
		uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
		uboLayoutBinding.pImmutableSamplers = nullptr;
		
		VkDescriptorSetLayoutBinding samplerLayoutBinding{};		samplerLayoutBinding.binding = 2;
		samplerLayoutBinding.descriptorType =
						VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		samplerLayoutBinding.descriptorCount = 1;
		samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;		samplerLayoutBinding.pImmutableSamplers = nullptr;
		
		VkDescriptorSetLayoutBinding globalUboLayoutBinding{};		globalUboLayoutBinding.binding = 1;
		globalUboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		globalUboLayoutBinding.descriptorCount = 1;
		globalUboLayoutBinding.stageFlags = VK_SHADER_STAGE_ALL_GRAPHICS;
		globalUboLayoutBinding.pImmutableSamplers = nullptr;

		std::array<VkDescriptorSetLayoutBinding, 3> bindings =
							{uboLayoutBinding, globalUboLayoutBinding, samplerLayoutBinding};
		
		VkDescriptorSetLayoutCreateInfo layoutInfo{};
		layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
		layoutInfo.pBindings = bindings.data();
		
		VkResult result = vkCreateDescriptorSetLayout(device, &layoutInfo,
									nullptr, &PhongDescriptorSetLayout);		if (result != VK_SUCCESS) {
			PrintVkError(result);
			throw std::runtime_error("failed to create descriptor set layout!");
		}
	}


 	void createPipelines() {
 		createPhongPipeline();
 	}
 	
 	void createPhongPipeline() {
 		createPipeline("Vert.spv", "Frag.spv",
 					    PhongPipelineLayout, PhongPipeline,
 					    PhongDescriptorSetLayout, VK_COMPARE_OP_LESS, VK_POLYGON_MODE_FILL,
 					    VK_CULL_MODE_NONE, false, phongAndSkyBoxVertices);
 		createPipeline("Vert.spv", "Frag.spv",
 					    PhongWirePipelineLayout, PhongWirePipeline,
 					    PhongDescriptorSetLayout, VK_COMPARE_OP_LESS, VK_POLYGON_MODE_LINE,
 					    VK_CULL_MODE_BACK_BIT, false, phongAndSkyBoxVertices);				    
 	}

 	void createPipeline(const char* VertexShaderName, const char* FragShaderName,
 						VkPipelineLayout& PipelineLayout,
 						VkPipeline& Pipeline,
 						VkDescriptorSetLayout &descriptorSetLayout,
 						VkCompareOp compareOp, VkPolygonMode polyModel,
 						VkCullModeFlagBits CM, bool transp,
 						VertexDescriptor &VD) {
		auto vertShaderCode = readFile((SHADER_PATH + VertexShaderName).c_str());
		auto fragShaderCode = readFile((SHADER_PATH + FragShaderName).c_str());
		
		VkShaderModule vertShaderModule =
				createShaderModule(vertShaderCode);
		VkShaderModule fragShaderModule =
				createShaderModule(fragShaderCode);

		VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
        vertShaderStageInfo.sType =
        		VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
        vertShaderStageInfo.module = vertShaderModule;
        vertShaderStageInfo.pName = "main";

        VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
        fragShaderStageInfo.sType =
        		VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
        fragShaderStageInfo.module = fragShaderModule;
        fragShaderStageInfo.pName = "main";

        VkPipelineShaderStageCreateInfo shaderStages[] =
        		{vertShaderStageInfo, fragShaderStageInfo};

		VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
		vertexInputInfo.sType =
				VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		auto bindingDescription = VD.getBindingDescription();		auto attributeDescriptions = VD.getAttributeDescriptions();
				
		vertexInputInfo.vertexBindingDescriptionCount = 1;
		vertexInputInfo.vertexAttributeDescriptionCount =
				static_cast<uint32_t>(attributeDescriptions.size());		vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
		vertexInputInfo.pVertexAttributeDescriptions =
				attributeDescriptions.data();		

		VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
		inputAssembly.sType =			VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		inputAssembly.primitiveRestartEnable = VK_FALSE;

		VkViewport viewport{};
		viewport.x = 0.0f;
		viewport.y = 0.0f;		viewport.width = (float) swapChainExtent.width;
		viewport.height = (float) swapChainExtent.height;
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;
		
		VkRect2D scissor{};
		scissor.offset = {0, 0};		scissor.extent = swapChainExtent;
		
		VkPipelineViewportStateCreateInfo viewportState{};		viewportState.sType =
				VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		viewportState.viewportCount = 1;
		viewportState.pViewports = &viewport;		viewportState.scissorCount = 1;
		viewportState.pScissors = &scissor;
		
		VkPipelineRasterizationStateCreateInfo rasterizer{};		rasterizer.sType =
				VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		rasterizer.depthClampEnable = VK_FALSE;
		rasterizer.rasterizerDiscardEnable = VK_FALSE;
		rasterizer.polygonMode = polyModel;
		rasterizer.lineWidth = 1.0f;
		rasterizer.cullMode = CM;
		rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
		rasterizer.depthBiasEnable = VK_FALSE;
		rasterizer.depthBiasConstantFactor = 0.0f; // Optional
		rasterizer.depthBiasClamp = 0.0f; // Optional
		rasterizer.depthBiasSlopeFactor = 0.0f; // Optional
		
		VkPipelineMultisampleStateCreateInfo multisampling{};
		multisampling.sType =
				VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		multisampling.sampleShadingEnable = VK_TRUE;
		multisampling.minSampleShading = .2f;
		multisampling.rasterizationSamples = msaaSamples;
		multisampling.minSampleShading = 1.0f; // Optional		multisampling.pSampleMask = nullptr; // Optional
		multisampling.alphaToCoverageEnable = VK_FALSE; // Optional
		multisampling.alphaToOneEnable = VK_FALSE; // Optional
		
		VkPipelineColorBlendAttachmentState colorBlendAttachment{};
		colorBlendAttachment.colorWriteMask =
				VK_COLOR_COMPONENT_R_BIT |
				VK_COLOR_COMPONENT_G_BIT |
				VK_COLOR_COMPONENT_B_BIT |
				VK_COLOR_COMPONENT_A_BIT;
		colorBlendAttachment.blendEnable = transp ? VK_TRUE : VK_FALSE;		colorBlendAttachment.srcColorBlendFactor =
				transp ? VK_BLEND_FACTOR_SRC_ALPHA : VK_BLEND_FACTOR_ONE;
		colorBlendAttachment.dstColorBlendFactor =
				transp ? VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA : VK_BLEND_FACTOR_ZERO;
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
				VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;		colorBlending.logicOpEnable = VK_FALSE;
		colorBlending.logicOp = VK_LOGIC_OP_COPY; // Optional
		colorBlending.attachmentCount = 1;
		colorBlending.pAttachments = &colorBlendAttachment;
		colorBlending.blendConstants[0] = 0.0f; // Optional
		colorBlending.blendConstants[1] = 0.0f; // Optional		colorBlending.blendConstants[2] = 0.0f; // Optional
		colorBlending.blendConstants[3] = 0.0f; // Optional
		
		VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
		pipelineLayoutInfo.sType =
			VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.setLayoutCount = 1;		pipelineLayoutInfo.pSetLayouts = &descriptorSetLayout;
		pipelineLayoutInfo.pushConstantRangeCount = 0; // Optional
		pipelineLayoutInfo.pPushConstantRanges = nullptr; // Optional
		
		VkResult result = vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr,
					&PipelineLayout);
		if (result != VK_SUCCESS) {
		 	PrintVkError(result);
			throw std::runtime_error("failed to create pipeline layout!");
		}
		
		VkPipelineDepthStencilStateCreateInfo depthStencil{};
		depthStencil.sType = 
				VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
		depthStencil.depthTestEnable = VK_TRUE;
		depthStencil.depthWriteEnable = VK_TRUE;
		depthStencil.depthCompareOp = compareOp;
		depthStencil.depthBoundsTestEnable = VK_FALSE;		depthStencil.minDepthBounds = 0.0f; // Optional
		depthStencil.maxDepthBounds = 1.0f; // Optional
		depthStencil.stencilTestEnable = VK_FALSE;		depthStencil.front = {}; // Optional
		depthStencil.back = {}; // Optional

		VkGraphicsPipelineCreateInfo pipelineInfo{};
		pipelineInfo.sType =
				VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		pipelineInfo.stageCount = 2;
		pipelineInfo.pStages = shaderStages;
		pipelineInfo.pVertexInputState = &vertexInputInfo;		pipelineInfo.pInputAssemblyState = &inputAssembly;
		pipelineInfo.pViewportState = &viewportState;
		pipelineInfo.pRasterizationState = &rasterizer;
		pipelineInfo.pMultisampleState = &multisampling;
		pipelineInfo.pDepthStencilState = &depthStencil;
		pipelineInfo.pColorBlendState = &colorBlending;		pipelineInfo.pDynamicState = nullptr; // Optional
		pipelineInfo.layout = PipelineLayout;
		pipelineInfo.renderPass = renderPass;
		pipelineInfo.subpass = 0;
		pipelineInfo.basePipelineHandle = VK_NULL_HANDLE; // Optional		pipelineInfo.basePipelineIndex = -1; // Optional
		
		result = vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1,
				&pipelineInfo, nullptr, &Pipeline);
		if (result != VK_SUCCESS) {		 	PrintVkError(result);
			throw std::runtime_error("failed to create graphics pipeline!");
		}
		
		vkDestroyShaderModule(device, fragShaderModule, nullptr);
		vkDestroyShaderModule(device, vertShaderModule, nullptr);
	}

	static std::vector<char> readFile(const std::string& filename) {			std::ifstream file(filename, std::ios::ate | std::ios::binary);
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

	VkShaderModule createShaderModule(const std::vector<char>& code) {
		VkShaderModuleCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		createInfo.codeSize = code.size();		createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());
		
		VkShaderModule shaderModule;

		VkResult result = vkCreateShaderModule(device, &createInfo, nullptr,
						&shaderModule);
		if (result != VK_SUCCESS) {
		 	PrintVkError(result);
			throw std::runtime_error("failed to create shader module!");
		}
		
		return shaderModule;
	}

    void createFramebuffers() {		swapChainFramebuffers.resize(swapChainImageViews.size());
		for (size_t i = 0; i < swapChainImageViews.size(); i++) {
			std::array<VkImageView, 3> attachments = {
				colorImageView,
				depthImageView,
				swapChainImageViews[i]
			};			VkFramebufferCreateInfo framebufferInfo{};
			framebufferInfo.sType =
				VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
			framebufferInfo.renderPass = renderPass;
			framebufferInfo.attachmentCount =
							static_cast<uint32_t>(attachments.size());;			framebufferInfo.pAttachments = attachments.data();
			framebufferInfo.width = swapChainExtent.width; 
			framebufferInfo.height = swapChainExtent.height;
			framebufferInfo.layers = 1;
			
			VkResult result = vkCreateFramebuffer(device, &framebufferInfo, nullptr,						&swapChainFramebuffers[i]);
			if (result != VK_SUCCESS) {
			 	PrintVkError(result);
				throw std::runtime_error("failed to create framebuffer!");
			}
		}
	}

    void createCommandPool() {
    	QueueFamilyIndices queueFamilyIndices = 
    			findQueueFamilies(physicalDevice);
    			
		VkCommandPoolCreateInfo poolInfo{};
		poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;		poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();
		poolInfo.flags = 0; // Optional
		
		VkResult result = vkCreateCommandPool(device, &poolInfo, nullptr, &commandPool);
		if (result != VK_SUCCESS) {		 	PrintVkError(result);
			throw std::runtime_error("failed to create command pool!");
		}	}

	void createColorResources() {
		VkFormat colorFormat = swapChainImageFormat;		createImage(swapChainExtent.width, swapChainExtent.height, 1,
					msaaSamples, colorFormat, VK_IMAGE_TILING_OPTIMAL,
					VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT |
					VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
					VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
					colorImage, colorImageMemory);		colorImageView = createImageView(colorImage, colorFormat,
									VK_IMAGE_ASPECT_COLOR_BIT, 1,
									VK_IMAGE_VIEW_TYPE_2D, 1);
	}

	void createDepthResources() {
		VkFormat depthFormat = findDepthFormat();
		
		createImage(swapChainExtent.width, swapChainExtent.height, 1, 
					msaaSamples, depthFormat, VK_IMAGE_TILING_OPTIMAL,
					VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
					VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
					depthImage, depthImageMemory);
		depthImageView = createImageView(depthImage, depthFormat,
										 VK_IMAGE_ASPECT_DEPTH_BIT, 1,
										 VK_IMAGE_VIEW_TYPE_2D, 1);

		transitionImageLayout(depthImage, depthFormat, VK_IMAGE_LAYOUT_UNDEFINED,
							  VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, 1, 1);
	}

	VkFormat findDepthFormat() {
		return findSupportedFormat({VK_FORMAT_D32_SFLOAT,
									VK_FORMAT_D32_SFLOAT_S8_UINT,
									VK_FORMAT_D24_UNORM_S8_UINT},
									VK_IMAGE_TILING_OPTIMAL, 								VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT );
	}
	
	VkFormat findSupportedFormat(const std::vector<VkFormat> candidates,
						VkImageTiling tiling, VkFormatFeatureFlags features) {
		for (VkFormat format : candidates) {			VkFormatProperties props;
			
			vkGetPhysicalDeviceFormatProperties(physicalDevice, format, &props);
			if (tiling == VK_IMAGE_TILING_LINEAR &&
						(props.linearTilingFeatures & features) == features) {				return format;
			} else if (tiling == VK_IMAGE_TILING_OPTIMAL &&
						(props.optimalTilingFeatures & features) == features) {
				return format;
			}
		}
		
		throw std::runtime_error("failed to find supported format!");
	}
	
	bool hasStencilComponent(VkFormat format) {
		return format == VK_FORMAT_D32_SFLOAT_S8_UINT ||
			   format == VK_FORMAT_D24_UNORM_S8_UINT;
	}
		
	void createTextureImage(const char* FName, TextureData& TD) {
		int texWidth, texHeight, texChannels;		stbi_uc* pixels = stbi_load((TEXTURE_PATH + FName).c_str(), &texWidth, &texHeight,
							&texChannels, STBI_rgb_alpha);
		if (!pixels) {
			std::cout << (TEXTURE_PATH + FName).c_str() << "\n";
			throw std::runtime_error("failed to load texture image!");		}
		std::cout << FName << " -> size: " << texWidth
				  << "x" << texHeight << ", ch: " << texChannels <<"\n";

		VkDeviceSize imageSize = texWidth * texHeight * 4;
		TD.mipLevels = static_cast<uint32_t>(std::floor(
						std::log2(std::max(texWidth, texHeight)))) + 1;
		
		VkBuffer stagingBuffer;
		VkDeviceMemory stagingBufferMemory;
		 
		createBuffer(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		  						VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
		  						VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		  						stagingBuffer, stagingBufferMemory);
		void* data;
		vkMapMemory(device, stagingBufferMemory, 0, imageSize, 0, &data);
		memcpy(data, pixels, static_cast<size_t>(imageSize));
		vkUnmapMemory(device, stagingBufferMemory);
		
		stbi_image_free(pixels);
		
		createImage(texWidth, texHeight, TD.mipLevels,
					VK_SAMPLE_COUNT_1_BIT, VK_FORMAT_R8G8B8A8_SRGB,
					VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_SRC_BIT |
					VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
					VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, TD.textureImage,
					TD.textureImageMemory);
					
		transitionImageLayout(TD.textureImage, VK_FORMAT_R8G8B8A8_SRGB,
				VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, TD.mipLevels, 1);
		copyBufferToImage(stagingBuffer, TD.textureImage,				static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight), 1);

		generateMipmaps(TD.textureImage, VK_FORMAT_R8G8B8A8_SRGB,
						texWidth, texHeight, TD.mipLevels, 1);

		vkDestroyBuffer(device, stagingBuffer, nullptr);
		vkFreeMemory(device, stagingBufferMemory, nullptr);
	}
	
	
	void generateMipmaps(VkImage image, VkFormat imageFormat,
						 int32_t texWidth, int32_t texHeight,
						 uint32_t mipLevels, int layerCount) {
		VkFormatProperties formatProperties;
		vkGetPhysicalDeviceFormatProperties(physicalDevice, imageFormat,
							&formatProperties);

		if (!(formatProperties.optimalTilingFeatures &					VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT)) {
			throw std::runtime_error("texture image format does not support linear blitting!");
		}
		VkCommandBuffer commandBuffer = beginSingleTimeCommands();
		
		VkImageMemoryBarrier barrier{};
		barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		barrier.image = image;
		barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;		barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		barrier.subresourceRange.baseArrayLayer = 0;
		barrier.subresourceRange.layerCount = layerCount;
		barrier.subresourceRange.levelCount = 1;		int32_t mipWidth = texWidth;		int32_t mipHeight = texHeight;

		for (uint32_t i = 1; i < mipLevels; i++) { 
			barrier.subresourceRange.baseMipLevel = i - 1;			barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
			barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
			barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
			
			vkCmdPipelineBarrier(commandBuffer,								 VK_PIPELINE_STAGE_TRANSFER_BIT,
								 VK_PIPELINE_STAGE_TRANSFER_BIT, 0,
								 0, nullptr, 0, nullptr,
								 1, &barrier);

			VkImageBlit blit{};
			blit.srcOffsets[0] = { 0, 0, 0 };
			blit.srcOffsets[1] = { mipWidth, mipHeight, 1 };			blit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			blit.srcSubresource.mipLevel = i - 1;
			blit.srcSubresource.baseArrayLayer = 0;
			blit.srcSubresource.layerCount = layerCount;
			blit.dstOffsets[0] = { 0, 0, 0 };
			blit.dstOffsets[1] = { mipWidth > 1 ? mipWidth / 2 : 1,
								   mipHeight > 1 ? mipHeight/2:1, 1};
			blit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			blit.dstSubresource.mipLevel = i;
			blit.dstSubresource.baseArrayLayer = 0;
			blit.dstSubresource.layerCount = layerCount;
			
			vkCmdBlitImage(commandBuffer, image,
						   VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
						   image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1,
						   &blit, VK_FILTER_LINEAR);

			barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
			barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
			barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
			
			vkCmdPipelineBarrier(commandBuffer,
								 VK_PIPELINE_STAGE_TRANSFER_BIT,
								 VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,								 0, nullptr, 0, nullptr,
								 1, &barrier);
			if (mipWidth > 1) mipWidth /= 2;			if (mipHeight > 1) mipHeight /= 2;
		}		barrier.subresourceRange.baseMipLevel = mipLevels - 1;
		barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;		vkCmdPipelineBarrier(commandBuffer,
							 VK_PIPELINE_STAGE_TRANSFER_BIT,
							 VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
							 0, nullptr, 0, nullptr,
							 1, &barrier);
		endSingleTimeCommands(commandBuffer);
	}

							 
	void createImage(uint32_t width, uint32_t height, uint32_t mipLevels,
					 VkSampleCountFlagBits numSamples, VkFormat format, 
					 VkImageTiling tiling, VkImageUsageFlags usage,
				 	 VkMemoryPropertyFlags properties, VkImage& image,
				 	 VkDeviceMemory& imageMemory) {		
		VkImageCreateInfo imageInfo{};
		imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;		imageInfo.imageType = VK_IMAGE_TYPE_2D;
		imageInfo.extent.width = width;
		imageInfo.extent.height = height;
		imageInfo.extent.depth = 1;
		imageInfo.mipLevels = mipLevels;
		imageInfo.arrayLayers = 1;
		imageInfo.format = format;
		imageInfo.tiling = tiling;
		imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		imageInfo.usage = usage;
		imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		imageInfo.samples = numSamples;		imageInfo.flags = 0; // Optional
		
		VkResult result = vkCreateImage(device, &imageInfo, nullptr, &image);
		if (result != VK_SUCCESS) {		 	PrintVkError(result);
		 	throw std::runtime_error("failed to create image!");
		}
		
		VkMemoryRequirements memRequirements;
		vkGetImageMemoryRequirements(device, image, &memRequirements);		VkMemoryAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.allocationSize = memRequirements.size;
		allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits,
											properties);		if (vkAllocateMemory(device, &allocInfo, nullptr, &imageMemory) !=
								VK_SUCCESS) {
			throw std::runtime_error("failed to allocate image memory!");
		}		vkBindImageMemory(device, image, imageMemory, 0);
	}

	void transitionImageLayout(VkImage image, VkFormat format,
					VkImageLayout oldLayout, VkImageLayout newLayout,
					uint32_t mipLevels, int layersCount) {
		VkCommandBuffer commandBuffer = beginSingleTimeCommands();

		VkImageMemoryBarrier barrier{};		barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		barrier.oldLayout = oldLayout;
		barrier.newLayout = newLayout;
		barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.image = image;
		
		if (newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {			barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;

			if (hasStencilComponent(format)) {
				barrier.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;			}
		}else{
			barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		}
		barrier.subresourceRange.baseMipLevel = 0;
		barrier.subresourceRange.levelCount = mipLevels;
		barrier.subresourceRange.baseArrayLayer = 0;
		barrier.subresourceRange.layerCount = layersCount;

		VkPipelineStageFlags sourceStage;
		VkPipelineStageFlags destinationStage;

		if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED &&
					newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
			barrier.srcAccessMask = 0;			barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			
			sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
			destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		} else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL &&
				   newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {			barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
			sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
			destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		} else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && 
				   newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
			barrier.srcAccessMask = 0;
			barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT |
									VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
			sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
			destinationStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
		} else {			throw std::invalid_argument("unsupported layout transition!");
		}
		vkCmdPipelineBarrier(commandBuffer,
								sourceStage, destinationStage, 0,
								0, nullptr, 0, nullptr, 1, &barrier);

		endSingleTimeCommands(commandBuffer);
	}
	
	void copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t						   width, uint32_t height, int layerCount) {
		VkCommandBuffer commandBuffer = beginSingleTimeCommands();
		
		VkBufferImageCopy region{};
		region.bufferOffset = 0;
		region.bufferRowLength = 0;
		region.bufferImageHeight = 0;		region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		region.imageSubresource.mipLevel = 0;
		region.imageSubresource.baseArrayLayer = 0;
		region.imageSubresource.layerCount = layerCount;		region.imageOffset = {0, 0, 0};
		region.imageExtent = {width, height, 1};
		
		vkCmdCopyBufferToImage(commandBuffer, buffer, image,
				VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

		endSingleTimeCommands(commandBuffer);
	}
	
	void createTextureImageView(TextureData& TD) {
		TD.textureImageView = createImageView(TD.textureImage,
										   VK_FORMAT_R8G8B8A8_SRGB,
										   VK_IMAGE_ASPECT_COLOR_BIT,
										   TD.mipLevels,
										   VK_IMAGE_VIEW_TYPE_2D, 1);
	}
	
	void createTextureSampler(TextureData& TD) {
		VkSamplerCreateInfo samplerInfo{};		samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		samplerInfo.magFilter = VK_FILTER_LINEAR;
		samplerInfo.minFilter = VK_FILTER_LINEAR;
		samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerInfo.anisotropyEnable = VK_TRUE;
		samplerInfo.maxAnisotropy = 16;
		samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
		samplerInfo.unnormalizedCoordinates = VK_FALSE;
		samplerInfo.compareEnable = VK_FALSE;		samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
		samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
		samplerInfo.mipLodBias = 0.0f;
		samplerInfo.minLod = 0.0f;
		samplerInfo.maxLod = static_cast<float>(TD.mipLevels);
		
		VkResult result = vkCreateSampler(device, &samplerInfo, nullptr,
										  &TD.textureSampler);
		if (result != VK_SUCCESS) {		 	PrintVkError(result);
		 	throw std::runtime_error("failed to create texture sampler!");
		}
	}

	void loadModels() {
		Scene.resize(SceneToLoad.size());
		int i = 0;

		for (const auto& M : SceneToLoad) {
			loadModelWithTexture(M, i);
			i++;
		}
		
//throw std::runtime_error("Now We Stop Here!");			
	}
	
	void loadModelWithTexture(const Model& M, int i) {
		loadMesh(M.ObjFile, Scene[i].MD, phongAndSkyBoxVertices);
		createVertexBuffer(Scene[i].MD);
		createIndexBuffer(Scene[i].MD);
		
		createTextureImage(M.TextureFile, Scene[i].TD);
		createTextureImageView(Scene[i].TD);
		createTextureSampler(Scene[i].TD);
	}
		
	void loadMesh(const char* FName, ModelData& MD, VertexDescriptor &VD) {
		tinyobj::attrib_t attrib;
		std::vector<tinyobj::shape_t> shapes;		std::vector<tinyobj::material_t> materials;
		std::string warn, err;
		
		if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, 
						(MODEL_PATH + FName).c_str())) {
			throw std::runtime_error(warn + err);		}
		
		MD.vertDesc = &VD;

		std::cout << FName << "\n";

		std::vector<float> vertex{};
		vertex.resize(VD.size);

//		std::unordered_map<std::vector<float>, uint32_t> uniqueVertices{};
		for (const auto& shape : shapes) {
			for (const auto& index : shape.mesh.indices) {				
				vertex[VD.deltaPos + 0] = attrib.vertices[3 * index.vertex_index + 0];				vertex[VD.deltaPos + 1] = attrib.vertices[3 * index.vertex_index + 1];				vertex[VD.deltaPos + 2] = attrib.vertices[3 * index.vertex_index + 2];				
//				if (uniqueVertices.count(vertex) == 0) {
					int j = MD.vertices.size() / VD.size;//					uniqueVertices[vertex] =
//							static_cast<uint32_t>(j);
					int s = MD.vertices.size();
					MD.vertices.resize(s + VD.size);
					for(int k = 0; k < VD.size; k++) {
						MD.vertices[s+k] = vertex[k];
					}
/**/				MD.indices.push_back(j);
//				}
				
//				MD.indices.push_back(uniqueVertices[vertex]);
			}
		}
		
		std::cout << FName << " -> V: " << MD.vertices.size()
				  << ", I: " << MD.indices.size() << "\n";
	}

	void createVertexBuffer(ModelData& Md) {
		VkDeviceSize bufferSize = sizeof(Md.vertices[0]) * Md.vertices.size();
		
		VkBuffer stagingBuffer;
		VkDeviceMemory stagingBufferMemory;
		createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, 
							VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
							VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
							stagingBuffer, stagingBufferMemory);

		void* data;
		vkMapMemory(device, stagingBufferMemory, 0, bufferSize, 0, &data);
		memcpy(data, Md.vertices.data(), (size_t) bufferSize);
		vkUnmapMemory(device, stagingBufferMemory);	
		
		createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT |
								 VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
								VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
								Md.vertexBuffer, Md.vertexBufferMemory);
								
		copyBuffer(stagingBuffer, Md.vertexBuffer, bufferSize);
		
		vkDestroyBuffer(device, stagingBuffer, nullptr);
		vkFreeMemory(device, stagingBufferMemory, nullptr);
	}
	
	void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage,
					  VkMemoryPropertyFlags properties,
					  VkBuffer& buffer, VkDeviceMemory& bufferMemory) {
		VkBufferCreateInfo bufferInfo{};
		bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferInfo.size = size;
		bufferInfo.usage = usage;
		bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		
		VkResult result =
				vkCreateBuffer(device, &bufferInfo, nullptr, &buffer);		if (result != VK_SUCCESS) {		 	PrintVkError(result);
			throw std::runtime_error("failed to create vertex buffer!");
		}
		
		VkMemoryRequirements memRequirements;
		vkGetBufferMemoryRequirements(device, buffer, &memRequirements);
		
		VkMemoryAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.allocationSize = memRequirements.size;
		allocInfo.memoryTypeIndex =				findMemoryType(memRequirements.memoryTypeBits, properties);

		result = vkAllocateMemory(device, &allocInfo, nullptr,
				&bufferMemory);		if (result != VK_SUCCESS) {		 	PrintVkError(result);
			throw std::runtime_error("failed to allocate vertex buffer memory!");
		}
		
		vkBindBufferMemory(device, buffer, bufferMemory, 0);	
	}
	
	uint32_t findMemoryType(uint32_t typeFilter,
							VkMemoryPropertyFlags properties) {
		 VkPhysicalDeviceMemoryProperties memProperties;
		 vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);
		 
		 for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
		 	if ((typeFilter & (1 << i)) && 
		 		(memProperties.memoryTypes[i].propertyFlags & properties) ==
		 				properties) {
				return i;
			}
		}
		
		throw std::runtime_error("failed to find suitable memory type!");
	}
    
	void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size) {
		VkCommandBuffer commandBuffer = beginSingleTimeCommands();
		
		VkBufferCopy copyRegion{};
		copyRegion.srcOffset = 0; // Optional copyRegion.dstOffset = 0; // Optional		copyRegion.size = size;
		vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);
		
		endSingleTimeCommands(commandBuffer);
	}
	
	VkCommandBuffer beginSingleTimeCommands() { 
		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandPool = commandPool;
		allocInfo.commandBufferCount = 1;		
		VkCommandBuffer commandBuffer;
		vkAllocateCommandBuffers(device, &allocInfo, &commandBuffer);
		
		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;		beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
		
		vkBeginCommandBuffer(commandBuffer, &beginInfo);
		
		return commandBuffer;
	}
	
	void endSingleTimeCommands(VkCommandBuffer commandBuffer) {
		vkEndCommandBuffer(commandBuffer);
		
		VkSubmitInfo submitInfo{};		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &commandBuffer;
		vkQueueSubmit(graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
		vkQueueWaitIdle(graphicsQueue);
		
		vkFreeCommandBuffers(device, commandPool, 1, &commandBuffer);
	}
	
	void createIndexBuffer(ModelData& Md) {
		VkDeviceSize bufferSize = sizeof(Md.indices[0]) * Md.indices.size();
			VkBuffer stagingBuffer;
		VkDeviceMemory stagingBufferMemory;
		createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
								 VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
								 VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
								 stagingBuffer, stagingBufferMemory);		void* data;
		vkMapMemory(device, stagingBufferMemory, 0, bufferSize, 0, &data);
		memcpy(data, Md.indices.data(), (size_t) bufferSize);
		vkUnmapMemory(device, stagingBufferMemory);		createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT |
								 VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
								 VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
								 Md.indexBuffer, Md.indexBufferMemory);		copyBuffer(stagingBuffer, Md.indexBuffer, bufferSize);
		vkDestroyBuffer(device, stagingBuffer, nullptr);
		vkFreeMemory(device, stagingBufferMemory, nullptr);
	}

	void createUniformBuffers() {		VkDeviceSize bufferSize = sizeof(UniformBufferObject);

		uniformBuffers.resize(swapChainImages.size() * Scene.size());
		uniformBuffersMemory.resize(swapChainImages.size() * Scene.size());		for (size_t i = 0; i < swapChainImages.size() * Scene.size(); i++) {
			createBuffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
								 	 VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
								 	 VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
								 	 uniformBuffers[i], uniformBuffersMemory[i]);
		}		bufferSize = sizeof(GlobalUniformBufferObject);		globalUniformBuffers.resize(swapChainImages.size());
		globalUniformBuffersMemory.resize(swapChainImages.size());		for (size_t i = 0; i < swapChainImages.size(); i++) {
			createBuffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
								 	 VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
								 	 VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
								 	 globalUniformBuffers[i], globalUniformBuffersMemory[i]);
		}		bufferSize = sizeof(UniformBufferObject);

	}

	void createDescriptorPool() {
		std::array<VkDescriptorPoolSize, 3> poolSizes{};
		poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		poolSizes[0].descriptorCount = static_cast<uint32_t>(swapChainImages.size() * Scene.size());
		poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		poolSizes[1].descriptorCount = static_cast<uint32_t>(swapChainImages.size() * Scene.size());
		poolSizes[2].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		poolSizes[2].descriptorCount = static_cast<uint32_t>(swapChainImages.size() * Scene.size());

		VkDescriptorPoolCreateInfo poolInfo{};
		poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());		poolInfo.pPoolSizes = poolSizes.data();
		poolInfo.maxSets = static_cast<uint32_t>(swapChainImages.size() * (Scene.size() + 2));
		
		VkResult result = vkCreateDescriptorPool(device, &poolInfo, nullptr,
									&descriptorPool);
		if (result != VK_SUCCESS) {
		 	PrintVkError(result);
			throw std::runtime_error("failed to create descriptor pool!");
		}
	}
	
	void createDescriptorSets() {
		createPhongDescriptorSets();	}
		void createPhongDescriptorSets() {		std::vector<VkDescriptorSetLayout> layouts(swapChainImages.size() * Scene.size(),
												   PhongDescriptorSetLayout);
		VkDescriptorSetAllocateInfo allocInfo{};		allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocInfo.descriptorPool = descriptorPool;
		allocInfo.descriptorSetCount = static_cast<uint32_t>(swapChainImages.size() * Scene.size());
		allocInfo.pSetLayouts = layouts.data();

		PhongDescriptorSets.resize(swapChainImages.size() * Scene.size());
		
		VkResult result = vkAllocateDescriptorSets(device, &allocInfo,
											PhongDescriptorSets.data());
		if (result != VK_SUCCESS) {
			PrintVkError(result);
			throw std::runtime_error("failed to allocate descriptor sets!");		}
		
		for (size_t k = 0; k < swapChainImages.size(); k++) {
			for (size_t j = 0; j < Scene.size(); j++) {
				size_t i = j * swapChainImages.size() + k;
				
				VkDescriptorBufferInfo bufferInfo{};
				bufferInfo.buffer = uniformBuffers[i];
				bufferInfo.offset = 0;				bufferInfo.range = sizeof(UniformBufferObject);
				
				VkDescriptorImageInfo imageInfo{};				imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
				imageInfo.imageView = Scene[j].TD.textureImageView;
				imageInfo.sampler = Scene[j].TD.textureSampler;
				
				VkDescriptorBufferInfo globalBufferInfo{};
				globalBufferInfo.buffer = globalUniformBuffers[k];
				globalBufferInfo.offset = 0;				globalBufferInfo.range = sizeof(GlobalUniformBufferObject);
				
				std::array<VkWriteDescriptorSet, 3> descriptorWrites{};				descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				descriptorWrites[0].dstSet = PhongDescriptorSets[i];
				descriptorWrites[0].dstBinding = 0;
				descriptorWrites[0].dstArrayElement = 0;
				descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;				descriptorWrites[0].descriptorCount =  1;
				descriptorWrites[0].pBufferInfo = &bufferInfo;
	
				descriptorWrites[2].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				descriptorWrites[2].dstSet = PhongDescriptorSets[i];
				descriptorWrites[2].dstBinding = 2;				descriptorWrites[2].dstArrayElement = 0;
				descriptorWrites[2].descriptorType =
											VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
				descriptorWrites[2].descriptorCount = 1;
				descriptorWrites[2].pImageInfo = &imageInfo;
				
				descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				descriptorWrites[1].dstSet = PhongDescriptorSets[i];
				descriptorWrites[1].dstBinding = 1;
				descriptorWrites[1].dstArrayElement = 0;
				descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;				descriptorWrites[1].descriptorCount =  1;
				descriptorWrites[1].pBufferInfo = &globalBufferInfo;

				vkUpdateDescriptorSets(device,
							static_cast<uint32_t>(descriptorWrites.size()),
							descriptorWrites.data(), 0, nullptr);
			}		}	}


    void createCommandBuffers() {
    	commandBuffers.resize(swapChainFramebuffers.size());
    	
    	VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.commandPool = commandPool;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandBufferCount = (uint32_t) commandBuffers.size();
		
		VkResult result = vkAllocateCommandBuffers(device, &allocInfo,				commandBuffers.data());
		if (result != VK_SUCCESS) {
		 	PrintVkError(result);
			throw std::runtime_error("failed to allocate command buffers!");
		}
		
		for (size_t i = 0; i < commandBuffers.size(); i++) {
			VkCommandBufferBeginInfo beginInfo{};
			beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
			beginInfo.flags = 0; // Optional			beginInfo.pInheritanceInfo = nullptr; // Optional

			if (vkBeginCommandBuffer(commandBuffers[i], &beginInfo) !=
						VK_SUCCESS) {
				throw std::runtime_error("failed to begin recording command buffer!");			}
			
			VkRenderPassBeginInfo renderPassInfo{};
			renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;			renderPassInfo.renderPass = renderPass; 
			renderPassInfo.framebuffer = swapChainFramebuffers[i];
			renderPassInfo.renderArea.offset = {0, 0};
			renderPassInfo.renderArea.extent = swapChainExtent;

			std::array<VkClearValue, 2> clearValues{};
			clearValues[0].color = {0.0f, 0.0f, 0.0f, 1.0f};
			clearValues[1].depthStencil = {1.0f, 0};

			renderPassInfo.clearValueCount =
							static_cast<uint32_t>(clearValues.size());
			renderPassInfo.pClearValues = clearValues.data();
			
			vkCmdBeginRenderPass(commandBuffers[i], &renderPassInfo,
					VK_SUBPASS_CONTENTS_INLINE);

			vkCmdBindPipeline(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS,
					PhongPipeline);					
			for(int j = 0; j < Scene.size(); j++) {
				if(SceneToLoad[j].pt == Flat) {
					VkBuffer vertexBuffers[] = {Scene[j].MD.vertexBuffer};					VkDeviceSize offsets[] = {0};
					vkCmdBindVertexBuffers(commandBuffers[i], 0, 1, vertexBuffers, offsets);
					vkCmdBindIndexBuffer(commandBuffers[i], Scene[j].MD.indexBuffer, 0,
											VK_INDEX_TYPE_UINT32);
					vkCmdBindDescriptorSets(commandBuffers[i],
									VK_PIPELINE_BIND_POINT_GRAPHICS,
									PhongPipelineLayout, 0, 1,
									&PhongDescriptorSets[i + j * swapChainImages.size()],
									0, nullptr);
									
					vkCmdDrawIndexed(commandBuffers[i],
								static_cast<uint32_t>(Scene[j].MD.indices.size()), 1, 0, 0, 0);
				}
			}

			vkCmdBindPipeline(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS,
					PhongWirePipeline);					
			for(int j = 0; j < Scene.size(); j++) {
				if(SceneToLoad[j].pt == Wire) {
					VkBuffer vertexBuffers[] = {Scene[j].MD.vertexBuffer};					VkDeviceSize offsets[] = {0};
					vkCmdBindVertexBuffers(commandBuffers[i], 0, 1, vertexBuffers, offsets);
					vkCmdBindIndexBuffer(commandBuffers[i], Scene[j].MD.indexBuffer, 0,
											VK_INDEX_TYPE_UINT32);
					vkCmdBindDescriptorSets(commandBuffers[i],
									VK_PIPELINE_BIND_POINT_GRAPHICS,
									PhongPipelineLayout, 0, 1,
									&PhongDescriptorSets[i + j * swapChainImages.size()],
									0, nullptr);
									
					vkCmdDrawIndexed(commandBuffers[i],
								static_cast<uint32_t>(Scene[j].MD.indices.size()), 1, 0, 0, 0);
				}
			}
			
			vkCmdEndRenderPass(commandBuffers[i]);
			if (vkEndCommandBuffer(commandBuffers[i]) != VK_SUCCESS) {
				throw std::runtime_error("failed to record command buffer!");			}
		}
	}
    
    void createSyncObjects() {
    	imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
    	renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
    	inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);
    	imagesInFlight.resize(swapChainImages.size(), VK_NULL_HANDLE);
    	    	
    	VkSemaphoreCreateInfo semaphoreInfo{};		semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
		
		VkFenceCreateInfo fenceInfo{};
		fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
		
		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
			VkResult result1 = vkCreateSemaphore(device, &semaphoreInfo, nullptr,
								&imageAvailableSemaphores[i]);
			VkResult result2 = vkCreateSemaphore(device, &semaphoreInfo, nullptr,
								&renderFinishedSemaphores[i]);
			VkResult result3 = vkCreateFence(device, &fenceInfo, nullptr,
								&inFlightFences[i]);
			if (result1 != VK_SUCCESS ||				result2 != VK_SUCCESS ||
				result3 != VK_SUCCESS) {
			 	PrintVkError(result1);
			 	PrintVkError(result2);
			 	PrintVkError(result3);
				throw std::runtime_error("failed to create synchronization objects for a frame!!");
			}
		}
	}
    
    void mainLoop() {
        while (!glfwWindowShouldClose(window)) {
            glfwPollEvents();
            drawFrame();
            
            if(glfwGetKey(window, GLFW_KEY_ESCAPE)) {
            	break;
            }
        }
        
        vkDeviceWaitIdle(device);
    }
    
    void drawFrame() {
		vkWaitForFences(device, 1, &inFlightFences[currentFrame],
						VK_TRUE, UINT64_MAX);
		
		uint32_t imageIndex;
		
		VkResult result = vkAcquireNextImageKHR(device, swapChain, UINT64_MAX,
				imageAvailableSemaphores[currentFrame], VK_NULL_HANDLE, &imageIndex);

		if (result == VK_ERROR_OUT_OF_DATE_KHR) {
			recreateSwapChain();
			return;		} else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
			throw std::runtime_error("failed to acquire swap chain image!");
		}

		if (imagesInFlight[imageIndex] != VK_NULL_HANDLE) {			vkWaitForFences(device, 1, &imagesInFlight[imageIndex],
							VK_TRUE, UINT64_MAX);
		}
		imagesInFlight[imageIndex] = inFlightFences[currentFrame];
		
		updateUniformBuffer(imageIndex);
		
		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;		VkSemaphore waitSemaphores[] = {imageAvailableSemaphores[currentFrame]};
		VkPipelineStageFlags waitStages[] =
			{VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
		submitInfo.waitSemaphoreCount = 1;
		submitInfo.pWaitSemaphores = waitSemaphores;		submitInfo.pWaitDstStageMask = waitStages;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &commandBuffers[imageIndex];
		VkSemaphore signalSemaphores[] = {renderFinishedSemaphores[currentFrame]};
		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pSignalSemaphores = signalSemaphores;
		
		vkResetFences(device, 1, &inFlightFences[currentFrame]);

		if (vkQueueSubmit(graphicsQueue, 1, &submitInfo,
				inFlightFences[currentFrame]) != VK_SUCCESS) {			throw std::runtime_error("failed to submit draw command buffer!");
		}
		
		VkPresentInfoKHR presentInfo{};		presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		presentInfo.waitSemaphoreCount = 1;
		presentInfo.pWaitSemaphores = signalSemaphores;
		
		VkSwapchainKHR swapChains[] = {swapChain};
		presentInfo.swapchainCount = 1;
		presentInfo.pSwapchains = swapChains;
		presentInfo.pImageIndices = &imageIndex;
		presentInfo.pResults = nullptr; // Optional
		
		result = vkQueuePresentKHR(presentQueue, &presentInfo);

		if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR ||
			framebufferResized) {
            framebufferResized = false;
            recreateSwapChain();
        } else if (result != VK_SUCCESS) {
            throw std::runtime_error("failed to present swap chain image!");
        }
		
		currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
    }

	void updateUniformBuffer(uint32_t currentImage) {
		static auto startTime = std::chrono::high_resolution_clock::now();
		static float lastTime = 0.0f;
		
		auto currentTime = std::chrono::high_resolution_clock::now();
		float time = std::chrono::duration<float, std::chrono::seconds::period>
					(currentTime - startTime).count();
		float deltaT = time - lastTime;
		lastTime = time;
					
		const float ROT_SPEED = glm::radians(60.0f);
		const float MOVE_SPEED = 0.75f;

		static float debounce = time;
		
		if(glfwGetKey(window, GLFW_KEY_LEFT)) {
			CamDir = glm::mat3(glm::rotate(glm::mat4(1.0f),
							   deltaT * ROT_SPEED,
							   glm::vec3(CamDir[1])) * glm::mat4(CamDir));
		}
		if(glfwGetKey(window, GLFW_KEY_RIGHT)) {
			CamDir = glm::mat3(glm::rotate(glm::mat4(1.0f),
							   -deltaT * ROT_SPEED,
							   glm::vec3(CamDir[1])) * glm::mat4(CamDir));
		}
		if(glfwGetKey(window, GLFW_KEY_UP)) {
			CamDir = glm::mat3(glm::rotate(glm::mat4(1.0f),
							   deltaT * ROT_SPEED,
							   glm::vec3(CamDir[0])) * glm::mat4(CamDir));
		}
		if(glfwGetKey(window, GLFW_KEY_DOWN)) {
			CamDir = glm::mat3(glm::rotate(glm::mat4(1.0f),
							   -deltaT * ROT_SPEED,
							   glm::vec3(CamDir[0])) * glm::mat4(CamDir));
		}
		if(glfwGetKey(window, GLFW_KEY_Q)) {
			CamDir = glm::mat3(glm::rotate(glm::mat4(1.0f),
							   deltaT * ROT_SPEED,
							   glm::vec3(CamDir[2])) * glm::mat4(CamDir));
		}
		if(glfwGetKey(window, GLFW_KEY_E)) {
			CamDir = glm::mat3(glm::rotate(glm::mat4(1.0f),
							   -deltaT * ROT_SPEED,
							   glm::vec3(CamDir[2])) * glm::mat4(CamDir));
		}
		

		if(glfwGetKey(window, GLFW_KEY_A)) {
			CamPos -= MOVE_SPEED * glm::vec3(CamDir[0]) * deltaT;
		}
		if(glfwGetKey(window, GLFW_KEY_D)) {
			CamPos += MOVE_SPEED * glm::vec3(CamDir[0]) * deltaT;
		}
		if(glfwGetKey(window, GLFW_KEY_S)) {
			CamPos += MOVE_SPEED * glm::vec3(CamDir[2]) * deltaT;
		}
		if(glfwGetKey(window, GLFW_KEY_W)) {
			CamPos -= MOVE_SPEED * glm::vec3(CamDir[2]) * deltaT;
		}
		if(glfwGetKey(window, GLFW_KEY_F)) {
			CamPos -= MOVE_SPEED * glm::vec3(CamDir[1]) * deltaT;
		}
		if(glfwGetKey(window, GLFW_KEY_R)) {
			CamPos += MOVE_SPEED * glm::vec3(CamDir[1]) * deltaT;
		}

// std::cout << "Cam Pos: " << CamPos[0] << " " << CamPos[1] << " " << CamPos[2] << "\n";

		glm::mat4 CamMat = glm::translate(glm::transpose(glm::mat4(CamDir)), -CamPos);
					
		glm::mat4 Prj = glm::perspective(glm::radians(45.0f),						swapChainExtent.width / (float) swapChainExtent.height,
						0.1f, 50.0f);
		Prj[1][1] *= -1;

		// Updates unifoms for the objects
		for(int j = 0; j < Scene.size(); j++) {
			UniformBufferObject ubo{};
			glm::vec3 delta;
			
			float rotAng = 0.0f;

			ubo.worldMat = glm::mat4(1.0f);
			ubo.worldMat = glm::scale(ubo.worldMat, glm::vec3(SceneToLoad[j].scale));
			ubo.viewMat = CamMat;
			ubo.prjMat = Prj;


			int i = j * swapChainImages.size() + currentImage;
			
			void* data;
			vkMapMemory(device, uniformBuffersMemory[i], 0,
								sizeof(ubo), 0, &data);			memcpy(data, &ubo, sizeof(ubo));
			vkUnmapMemory(device, uniformBuffersMemory[i]);
		}
		
		// updates global uniforms
		GlobalUniformBufferObject gubo{};
		gubo.time = glm::fract(time);

		void* data;
		vkMapMemory(device, globalUniformBuffersMemory[currentImage], 0,
							sizeof(gubo), 0, &data);		memcpy(data, &gubo, sizeof(gubo));
		vkUnmapMemory(device, globalUniformBuffersMemory[currentImage]);

	}

    void recreateSwapChain() {
    	int width = 0, height = 0;		glfwGetFramebufferSize(window, &width, &height);
		
		while (width == 0 || height == 0) {
			glfwGetFramebufferSize(window, &width, &height);
			glfwWaitEvents();
		}

		vkDeviceWaitIdle(device);
    	
    	cleanupSwapChain();
		createSwapChain();
		createImageViews();
		createRenderPass();
		createPipelines();
		createColorResources();
		createDepthResources();
		createFramebuffers();
		createUniformBuffers();
		createDescriptorPool();
		createDescriptorSets();		createCommandBuffers();
	}

	void cleanupSwapChain() {
    	vkDestroyImageView(device, colorImageView, nullptr);
    	vkDestroyImage(device, colorImage, nullptr);
    	vkFreeMemory(device, colorImageMemory, nullptr);
    	
		vkDestroyImageView(device, depthImageView, nullptr);
		vkDestroyImage(device, depthImage, nullptr);		vkFreeMemory(device, depthImageMemory, nullptr);

		for (size_t i = 0; i < swapChainFramebuffers.size(); i++) {
			vkDestroyFramebuffer(device, swapChainFramebuffers[i], nullptr);
		}
		
		vkFreeCommandBuffers(device, commandPool,				static_cast<uint32_t>(commandBuffers.size()), commandBuffers.data());

		vkDestroyPipeline(device, PhongPipeline, nullptr);
		vkDestroyPipeline(device, PhongWirePipeline, nullptr);
		vkDestroyPipelineLayout(device, PhongPipelineLayout, nullptr);
		vkDestroyPipelineLayout(device, PhongWirePipelineLayout, nullptr);

		vkDestroyRenderPass(device, renderPass, nullptr);		for (size_t i = 0; i < swapChainImageViews.size(); i++){
			vkDestroyImageView(device, swapChainImageViews[i], nullptr);
		}
		
		vkDestroySwapchainKHR(device, swapChain, nullptr);
		
		for (size_t i = 0; i < swapChainImages.size() * Scene.size(); i++) {
			vkDestroyBuffer(device, uniformBuffers[i], nullptr);
			vkFreeMemory(device, uniformBuffersMemory[i], nullptr);
		}
		for (size_t i = 0; i < swapChainImages.size(); i++) {
			vkDestroyBuffer(device, globalUniformBuffers[i], nullptr);
			vkFreeMemory(device, globalUniformBuffersMemory[i], nullptr);
		}		
		
		vkDestroyDescriptorPool(device, descriptorPool, nullptr);
	}


    void cleanup() {
    	cleanupSwapChain();
    	
    	for (size_t i = 0; i < Scene.size(); i++) {
	    	vkDestroySampler(device, Scene[i].TD.textureSampler, nullptr);
	    	vkDestroyImageView(device, Scene[i].TD.textureImageView, nullptr);
			vkDestroyImage(device, Scene[i].TD.textureImage, nullptr);
			vkFreeMemory(device, Scene[i].TD.textureImageMemory, nullptr);
	    	
	    	vkDestroyBuffer(device, Scene[i].MD.indexBuffer, nullptr);
	    	vkFreeMemory(device, Scene[i].MD.indexBufferMemory, nullptr);
	
			vkDestroyBuffer(device, Scene[i].MD.vertexBuffer, nullptr);
	    	vkFreeMemory(device, Scene[i].MD.vertexBufferMemory, nullptr);
    	}
    	    	
    	vkDestroyDescriptorSetLayout(device, PhongDescriptorSetLayout, nullptr);
    	
    	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
			vkDestroySemaphore(device, renderFinishedSemaphores[i], nullptr);			vkDestroySemaphore(device, imageAvailableSemaphores[i], nullptr);
			vkDestroyFence(device, inFlightFences[i], nullptr);
    	}
    	
    	vkDestroyCommandPool(device, commandPool, nullptr);
    	
 		vkDestroyDevice(device, nullptr);
		
	   	if (enableValidationLayers) {
			DestroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);
		}
		
		vkDestroySurfaceKHR(instance, surface, nullptr);
    	vkDestroyInstance(instance, nullptr);

        glfwDestroyWindow(window);

        glfwTerminate();
    }







//// To put in the right place

};

int main() {
    Assignment11 app;

    try {
        app.run();
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}