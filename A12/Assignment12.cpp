// Following the Vulkan Tutorial until the
// Drawing a triangle -> Setup -> Instance
// section (currently at https://vulkan-tutorial.com/Drawing_a_triangle/Setup/Instance)
// create a 640 x 480 window, with title "Assignment 12", and list the global
// extensions supported by your O.S. 

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <iostream>
#include <stdexcept>
#include <cstdlib>
#include <vector>

// Window size
const uint32_t WIDTH = 640;
const uint32_t HEIGHT = 480;

class Assignment12 {
public:
    void run() {
        initWindow();
        initVulkan();
        mainLoop();
        cleanup();
    }

private:
    // GLFW window object
    GLFWwindow* window;
    // Vulkan instance object
    VkInstance instance;

    /* Checking for and printing supported extensions*/
    void printExtensions(){
        // Find number of extensions
        uint32_t extensionCount = 0;
        vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
        
        // Allocate array to hold extension details (name + version)
        std::vector<VkExtensionProperties> extensions(extensionCount);
        
        // Get extension details
        vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions.data());
        std::cout << "available extensions:\n";

        // Print extensions
        for (const auto& extension : extensions) {
            std::cout << '\t' << extension.extensionName << '\n';
        }
    }

    /* Print extensions and create Vulkan instance */
    void createInstance() {

        // Function for printing extensions
        printExtensions();

        // Struct with info about application
        VkApplicationInfo appInfo{};
        appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        appInfo.pApplicationName = "Assignment 12";
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

    /* Initialize GLFW and create a window */
    void initWindow(){
        glfwInit(); // init GLFW library

        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API); // do not create an OpenGL context
        glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE); // disable window resizing

        window = glfwCreateWindow(WIDTH, HEIGHT, "Assignment 12", nullptr, nullptr); // create window
    }

    /* Init the vulkan objects*/
    void initVulkan(){
        createInstance(); // connection between application and Vulkan library
    }

    /* Run until window is closed*/
    void mainLoop(){
        // Until error or window closed
        while (!glfwWindowShouldClose(window)){
            glfwPollEvents();
        }
    }

    /* Deallocate resources*/
    void cleanup(){

        vkDestroyInstance(instance, nullptr); // destroy instance

        glfwDestroyWindow(window); // destroy window

        glfwTerminate(); // terminate GLFW
    }
};

int main() {
    Assignment12 app;

    try {
        app.run();
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}