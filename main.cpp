#include <cstdint>
#include <iostream>
#include <string>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "GLTFParser.h"

int main() {
    std::string gltfFilename{};

    std::cout << "Enter the name or path of a glTF file: " << std::flush;
    if (!std::getline(std::cin, gltfFilename) || gltfFilename.empty()) {
        std::cerr << "No glTF file name was provided.\n";
        return 1;
    }

    std::cout << "Selected glTF file: " << gltfFilename << '\n' << std::flush;

    glfwInitHint(GLFW_PLATFORM_COCOA, GLFW_TRUE);
    glfwInitHint(GLFW_COCOA_MENUBAR, GLFW_TRUE);
    if (!glfwInit())
    {
        std::cerr << "Could not initialize GLFW.\n";
        return 1;
    }

    // Vulkan will render here later
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    int width{800};
    int height{600};
    GLFWwindow* window{glfwCreateWindow(width, height, "window", nullptr, nullptr)};
    if (window == nullptr) {
        std::cerr << "Could not create window.\n";
        glfwTerminate();
        return 1;
    }

    VkApplicationInfo applicationInfo{};
    applicationInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    applicationInfo.pApplicationName = "Vulkan Renderer";
    applicationInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    applicationInfo.pEngineName = "No Engine";
    applicationInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    applicationInfo.apiVersion = VK_API_VERSION_1_4;

    std::uint32_t requiredExtensionCount{0};
    const char** requiredExtensions{glfwGetRequiredInstanceExtensions(&requiredExtensionCount)};
    if (requiredExtensions == nullptr) {
        std::cerr << "GLFW could not determine the required Vulkan extensions.\n";
        glfwDestroyWindow(window);
        glfwTerminate();
        return 1;
    }

    VkInstanceCreateInfo instanceCreateInfo{};
    instanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instanceCreateInfo.pApplicationInfo = &applicationInfo;
    instanceCreateInfo.enabledExtensionCount = requiredExtensionCount;
    instanceCreateInfo.ppEnabledExtensionNames = requiredExtensions;

    VkInstance instance{VK_NULL_HANDLE};
    if (vkCreateInstance(&instanceCreateInfo, nullptr, &instance) != VK_SUCCESS) {
        std::cerr << "Could not create Vulkan instance.\n";
        glfwDestroyWindow(window);
        glfwTerminate();
        return 1;
    }

    VkSurfaceKHR surface{VK_NULL_HANDLE};
    if (glfwCreateWindowSurface(instance, window, nullptr, &surface) != VK_SUCCESS) {
        std::cerr << "Could not create Vulkan window surface.\n";
        vkDestroyInstance(instance, nullptr);
        glfwDestroyWindow(window);
        glfwTerminate();
        return 1;
    }

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
    }

    vkDestroySurfaceKHR(instance, surface, nullptr);
    vkDestroyInstance(instance, nullptr);
    glfwDestroyWindow(window);
    glfwTerminate();

    return parseGLTF(gltfFilename) ? 0 : 1;
}
