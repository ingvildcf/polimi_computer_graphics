#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <iostream>
#include <stdexcept>
#include <cstdlib>
#include <vector>

class Example07 {
public:
    void run() {

		uint32_t extensionCount = 0;
		vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);		 
		std::vector<VkExtensionProperties> extensions(extensionCount);
		vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, 
		 										extensions.data());
		 
		std::cout << "Available global extensions:\n";		for (const auto& extension : extensions) {
			std::cout << '\t' << extension.extensionName << '\n';
		}

		glfwInit();
		uint32_t glfwExtensionCount = 0;
		const char** glfwExtensions;		glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

		std::cout << "\nExtensions required by GLFW:\n";
		for(int i = 0; i < glfwExtensionCount; i++) {
			std::cout << '\t' << glfwExtensions[i] << '\n';
		}
		
		std::cout << "\n";
    }
};

int main() {
    Example07 app;

    try {
        app.run();
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}