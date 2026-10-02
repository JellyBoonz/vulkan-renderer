#include <iostream>
#include <string>
#include <glfw/glfw3.h>

#include "GLTFParser.h"

int main() {
    std::string gltfFilename;

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

    int width = 800;
    int height = 600;
    GLFWwindow* window = glfwCreateWindow(width, height, "window", nullptr, nullptr);
    if (window == nullptr) {
        std::cerr << "Could not create window.\n";
        glfwTerminate();
        return 1;
    }

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
    }

    glfwDestroyWindow(window);
    glfwTerminate();

    return parseGLTF(gltfFilename) ? 0 : 1;
}
