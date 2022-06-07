#define VK_USE_PLATFORM_WIN32_KHR
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>

#include <iostream>
#include <stdexcept>
#include <cstdlib>
#include <vector>
#include <optional>
#include <set>

// Window size
const uint32_t WIDTH = 640;
const uint32_t HEIGHT = 480;

// Queue family struct
struct QueueFamilyIndices {
    std::optional<uint32_t> graphicsFamily;
    std::optional<uint32_t> presentFamily;

    bool isComplete() {
        return graphicsFamily.has_value() && presentFamily.has_value();
    }
};

class Assignment13 {
public:
    void run() {
        initWindow();
        initVulkan();
        mainLoop();
        cleanup();
    }

private:
    // A12
    GLFWwindow* window; // GLFW window handle
    VkInstance instance; // Vulkan instance handle

    // A13
    VkSurfaceKHR surface; // Presentation surface handle 
    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE; // Physical device handle (destroyed with instance)
    VkDevice device; // Logical device handle
    VkQueue graphicsQueue; // Graphics queue handle
    VkQueue presentQueue;  // Present queue handle
    VkCommandPool commandPool; // Command pool - memory manager for command buffers
    VkCommandBuffer commandBuffer; // Command buffer object (destroyed with command pool)

    /* A12: Initialize GLFW and create a window */
    void initWindow(){
        glfwInit(); // init GLFW library

        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API); // do not create an OpenGL context
        glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE); // disable window resizing

        window = glfwCreateWindow(WIDTH, HEIGHT, "Assignment 13", nullptr, nullptr); // create window
    }

    /* Init the vulkan objects*/
    void initVulkan(){
        createInstance(); // A12: connection between application and Vulkan library
        createSurface(); // A13: Create Vulkan surface
        pickPhysicalDevice(); // A13: look for and select a graphic card
        createLogicalDevice(); // A13: set up logical device
        createCommandPool(); // A13: Create command pool
        createCommandBuffer(); // A13: Create command buffer
    }

    /* A12: Create Vulkan instance */
    void createInstance() {

        // Struct with info about application
        VkApplicationInfo appInfo{};
        appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        appInfo.pApplicationName = "Assignment 13";
        appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.pEngineName = "No Engine";
        appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.apiVersion = VK_API_VERSION_1_0;

        // Struct with info for creating an instance
        VkInstanceCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        createInfo.pApplicationInfo = &appInfo;

        // Find needed extension(s) to interface with the window system
        uint32_t glfwExtensionCount = 0;
        const char** glfwExtensions;
        glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
        
        // Pass it to the struct
        createInfo.enabledExtensionCount = glfwExtensionCount;
        createInfo.ppEnabledExtensionNames = glfwExtensions;
        
        // Enable global validation layers (later)
        createInfo.enabledLayerCount = 0;

        // Create instance
        VkResult result = vkCreateInstance(&createInfo, nullptr, &instance);
        
        // Check if instance creation was successful
        if (vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS) {
            throw std::runtime_error("failed to create instance!");
        }
    }

    /* A13: Create a presentation surface*/
    void createSurface(){
        if (glfwCreateWindowSurface(instance, window, nullptr, &surface) != VK_SUCCESS) {
            throw std::runtime_error("failed to create window surface!");
        }
    }

    /* A13: Pick physical device (graphic card)*/
    void pickPhysicalDevice(){
        // Find number of devices
        uint32_t deviceCount = 0;
        vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);
        
        // If no devices found
        if (deviceCount == 0) {
            throw std::runtime_error("failed to find GPUs with Vulkan support!");
        }

        // Allocate array to hold the physical device handlers
        std::vector<VkPhysicalDevice> devices(deviceCount);
        vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());
        
        // Check if devices are suitable
        for (const auto& device : devices) {
            if (isDeviceSuitable(device)) {
                physicalDevice = device;
                break;
            }
        }

        if (physicalDevice == VK_NULL_HANDLE) {
            throw std::runtime_error("failed to find a suitable GPU!");
        }
    }

    /* A13: Help function to check if device is suitable*/
    bool isDeviceSuitable(VkPhysicalDevice device){
        QueueFamilyIndices indices = findQueueFamilies(device);

        return indices.isComplete();
    }

    /* A13: Find availiable queue families with support for needed commands*/
    QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device) {
        QueueFamilyIndices indices;

        // Retrieve the number of queue families
        uint32_t queueFamilyCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);
        // Allocate vector to store queue families
        std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

        // See if one of the queue families supports VK_QUEUE_GRAPHICS_BIT
        int i = 0;
        for (const auto& queueFamily : queueFamilies) {
            if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
                indices.graphicsFamily = i;
            }
            // Look for queue family that can present to our window surface
            VkBool32 presentSupport = false;
            vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);
            // Check if supported and store presentation family queue index
            if (presentSupport) {
                indices.presentFamily = i;
            }

            if (indices.isComplete()) {
                break;
            }
            i++;
        }

        return indices;
    }

    /* A13: Set up a logical device to interface with physical device*/
    void createLogicalDevice(){
        // Find queue families
        QueueFamilyIndices indices = findQueueFamilies(physicalDevice);

        std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
        std::set<uint32_t> uniqueQueueFamilies = {indices.graphicsFamily.value(), indices.presentFamily.value()};

        // Struct for creating queues for all unique families
        float queuePriority = 1.0f;
        for (uint32_t queueFamily : uniqueQueueFamilies) {
            VkDeviceQueueCreateInfo queueCreateInfo{};
            queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            queueCreateInfo.queueFamilyIndex = queueFamily;
            queueCreateInfo.queueCount = 1;
            queueCreateInfo.pQueuePriorities = &queuePriority;
            queueCreateInfos.push_back(queueCreateInfo);
        }

        // Set of device features
        VkPhysicalDeviceFeatures deviceFeatures{};

        // Device info structure
        VkDeviceCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

        createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
        createInfo.pQueueCreateInfos = queueCreateInfos.data();

        createInfo.pEnabledFeatures = &deviceFeatures;

        // Instantiate logical device
        if (vkCreateDevice(physicalDevice, &createInfo, nullptr, &device) != VK_SUCCESS) {
            throw std::runtime_error("failed to create logical device!");
        }
        // Retrieve queue handles for each queue family
        vkGetDeviceQueue(device, indices.graphicsFamily.value(), 0, &graphicsQueue);
        vkGetDeviceQueue(device, indices.presentFamily.value(), 0, &presentQueue);
    }

    /* A13: Create a command pool*/
    void createCommandPool() {
        QueueFamilyIndices queueFamilyIndices = findQueueFamilies(physicalDevice);

        VkCommandPoolCreateInfo poolInfo{};
        poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();
        
        if (vkCreateCommandPool(device, &poolInfo, nullptr, &commandPool) != VK_SUCCESS) {
            throw std::runtime_error("failed to create command pool!");
        }
    }
    
    /* A13: Create a command buffer*/
    void createCommandBuffer(){
        // Struct to specify command pool and number of buffers to allocate
        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.commandPool = commandPool;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandBufferCount = 1;

        if (vkAllocateCommandBuffers(device, &allocInfo, &commandBuffer) != VK_SUCCESS) {
            throw std::runtime_error("failed to allocate command buffers!");
        }
    }

    /* A12: Run until window is closed*/
    void mainLoop(){
        // Until error or window closed
        while (!glfwWindowShouldClose(window)){
            glfwPollEvents();
        }
    }

    /* Deallocate resources*/
    void cleanup(){
        vkDestroyCommandPool(device, commandPool, nullptr); // A13: destroy command pool
        vkDestroyDevice(device, nullptr); // A13: destroy logical device
        vkDestroySurfaceKHR(instance, surface, nullptr); // A13: destroy presentation surface
        vkDestroyInstance(instance, nullptr); // A12: destroy instance
        glfwDestroyWindow(window); // A12: destroy window
        glfwTerminate(); // A12: terminate GLFW
    }
};

int main() {
    Assignment13 app;

    try {
        app.run();
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}