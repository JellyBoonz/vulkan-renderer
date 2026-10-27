#include <cassert>
#include <cstdint>
#include <iostream>
#include <string>
#include <vector>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "GLTFParser.h"

int main(int argc, char* argv[]) {
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

    std::vector<const char*> extensions(
        requiredExtensions,
        requiredExtensions + requiredExtensionCount
    );
    extensions.push_back(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);

    VkInstanceCreateInfo instanceCreateInfo{};
    instanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instanceCreateInfo.pApplicationInfo = &applicationInfo;
    instanceCreateInfo.flags =
    VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
    instanceCreateInfo.enabledExtensionCount =
        static_cast<std::uint32_t>(extensions.size());
    instanceCreateInfo.ppEnabledExtensionNames = extensions.data();

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

    std::uint32_t physicalDeviceCount{0};
    if (vkEnumeratePhysicalDevices(instance, &physicalDeviceCount, nullptr) != VK_SUCCESS || physicalDeviceCount == 0) {
        std::cerr << "No Vulkan physical devices are available.\n";
        vkDestroySurfaceKHR(instance, surface, nullptr);
        vkDestroyInstance(instance, nullptr);
        glfwDestroyWindow(window);
        glfwTerminate();
        return 1;
    }

    std::vector<VkPhysicalDevice> physicalDevices{};
    physicalDevices.resize(physicalDeviceCount);
    if (vkEnumeratePhysicalDevices(instance, &physicalDeviceCount, physicalDevices.data()) != VK_SUCCESS) {
        std::cerr << "Could not enumerate Vulkan physical devices.\n";
        vkDestroySurfaceKHR(instance, surface, nullptr);
        vkDestroyInstance(instance, nullptr);
        glfwDestroyWindow(window);
        glfwTerminate();
        return 1;
    }

    for (std::uint32_t index{0}; index < physicalDeviceCount; ++index) {
        VkPhysicalDeviceProperties properties{};
        vkGetPhysicalDeviceProperties(physicalDevices[index], &properties);
        std::cout << '[' << index << "] " << properties.deviceName << '\n';
    }

    std::uint32_t deviceIndex{0};
    if (argc > 1) {
        deviceIndex = static_cast<std::uint32_t>(std::stoi(argv[1]));
    }
    assert(deviceIndex < physicalDeviceCount);

    VkPhysicalDevice physicalDevice{physicalDevices[deviceIndex]};
    VkPhysicalDeviceProperties selectedProperties{};
    vkGetPhysicalDeviceProperties(physicalDevice, &selectedProperties);
    std::cout << "Selected device [" << deviceIndex << "]: " << selectedProperties.deviceName << '\n';



    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
    }

    vkDestroySurfaceKHR(instance, surface, nullptr);
    vkDestroyInstance(instance, nullptr);
    glfwDestroyWindow(window);
    glfwTerminate();

    return parseGLTF(gltfFilename) ? 0 : 1;
}
