#include <algorithm>
#include <cstdint>
#include <cstring>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <vector>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

namespace {
void vkCheck(VkResult result, const char* call) {
    if (result != VK_SUCCESS) throw std::runtime_error(std::string(call) + " failed: " + std::to_string(result));
}
std::vector<char> readFile(const char* path) {
    std::ifstream file(path, std::ios::ate | std::ios::binary);
    if (!file) throw std::runtime_error(std::string("Could not read ") + path);
    std::vector<char> data(static_cast<std::size_t>(file.tellg()));
    file.seekg(0);
        file.read(data.data(), static_cast<std::streamsize>(data.size()));
    return data;
}
VkShaderModule shader(VkDevice device, const char* path) {
    const auto data = readFile(path);
    VkShaderModuleCreateInfo info{VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO};
    info.codeSize = data.size();
        info.pCode = reinterpret_cast<const std::uint32_t*>(data.data());
    VkShaderModule result{};
    vkCheck(vkCreateShaderModule(device, &info, nullptr, &result), "vkCreateShaderModule");
    return result;
}
}

int main() {
    GLFWwindow* window{};
    VkInstance instance{};
        VkSurfaceKHR surface{};
        VkDevice device{};
        VkSwapchainKHR swapchain{};
    VkPipelineLayout layout{};
        VkPipeline pipeline{};
        VkCommandPool commandPool{};
    VkCommandBuffer commandBuffer{};
        VkSemaphore imageAvailable{}, renderFinished{};
        VkFence inFlight{};
    std::vector<VkImageView> imageViews;
    try {
        if (!glfwInit()) throw std::runtime_error("Could not initialize GLFW");
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        window = glfwCreateWindow(800, 600, "Vulkan Triangle", nullptr, nullptr);
        if (!window) throw std::runtime_error("Could not create GLFW window");

        std::uint32_t count{};
        const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&count);
        std::vector<const char*> extensions(glfwExtensions, glfwExtensions + count);
#ifdef __APPLE__
        extensions.push_back(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
#endif
        VkApplicationInfo app{VK_STRUCTURE_TYPE_APPLICATION_INFO};
        app.pApplicationName = "Vulkan Triangle";
        app.apiVersion = VK_API_VERSION_1_3;
        VkInstanceCreateInfo instanceInfo{VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO};
        instanceInfo.pApplicationInfo = &app;
        instanceInfo.enabledExtensionCount = static_cast<std::uint32_t>(extensions.size());
        instanceInfo.ppEnabledExtensionNames = extensions.data();
#ifdef __APPLE__
        instanceInfo.flags = VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
#endif
        vkCheck(vkCreateInstance(&instanceInfo, nullptr, &instance), "vkCreateInstance");
        vkCheck(glfwCreateWindowSurface(instance, window, nullptr, &surface), "glfwCreateWindowSurface");

        vkCheck(vkEnumeratePhysicalDevices(instance, &count, nullptr), "vkEnumeratePhysicalDevices");
        std::vector<VkPhysicalDevice> physicalDevices(count);
        vkCheck(vkEnumeratePhysicalDevices(instance, &count, physicalDevices.data()), "vkEnumeratePhysicalDevices");
        VkPhysicalDevice physicalDevice{};
        std::uint32_t queueFamily{};
        for (auto candidate : physicalDevices) {
            std::uint32_t familyCount{};
        vkGetPhysicalDeviceQueueFamilyProperties(candidate, &familyCount, nullptr);
            std::vector<VkQueueFamilyProperties> families(familyCount);
            vkGetPhysicalDeviceQueueFamilyProperties(candidate, &familyCount, families.data());
            for (std::uint32_t i{};
        i < familyCount; ++i) {
                VkBool32 present{};
        vkCheck(vkGetPhysicalDeviceSurfaceSupportKHR(candidate, i, surface, &present), "vkGetPhysicalDeviceSurfaceSupportKHR");
                if ((families[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) && present) { physicalDevice = candidate;
        queueFamily = i;
        break; }
            }
            if (physicalDevice) break;
        }
        if (!physicalDevice) throw std::runtime_error("No graphics and presentation queue family found");

        const float priority{1.0F};
        VkDeviceQueueCreateInfo queueInfo{VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO};
        queueInfo.queueFamilyIndex = queueFamily;
        queueInfo.queueCount = 1;
        queueInfo.pQueuePriorities = &priority;
        VkPhysicalDeviceDynamicRenderingFeatures dynamic{VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DYNAMIC_RENDERING_FEATURES};
        dynamic.dynamicRendering = VK_TRUE;
        const char* deviceExtensions[]{VK_KHR_SWAPCHAIN_EXTENSION_NAME, "VK_KHR_portability_subset"};
        VkDeviceCreateInfo deviceInfo{VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO};
        deviceInfo.pNext = &dynamic;
        deviceInfo.queueCreateInfoCount = 1;
        deviceInfo.pQueueCreateInfos = &queueInfo;
        deviceInfo.enabledExtensionCount = 2;
        deviceInfo.ppEnabledExtensionNames = deviceExtensions;
        vkCheck(vkCreateDevice(physicalDevice, &deviceInfo, nullptr, &device), "vkCreateDevice");
        VkQueue queue{};
        vkGetDeviceQueue(device, queueFamily, 0, &queue);

        // swapchain setup
        VkSurfaceCapabilitiesKHR caps{};
        vkCheck(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, &caps), "vkGetPhysicalDeviceSurfaceCapabilitiesKHR");
        vkCheck(vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &count, nullptr), "vkGetPhysicalDeviceSurfaceFormatsKHR");
        std::vector<VkSurfaceFormatKHR> formats(count);
        vkCheck(vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &count, formats.data()), "vkGetPhysicalDeviceSurfaceFormatsKHR");
        const VkSurfaceFormatKHR format = formats.front();
        int width{}, height{};
        glfwGetFramebufferSize(window, &width, &height);
        VkExtent2D extent{static_cast<std::uint32_t>(width), static_cast<std::uint32_t>(height)};
        if (caps.currentExtent.width != UINT32_MAX) extent = caps.currentExtent;
        VkSwapchainCreateInfoKHR swapchainInfo{VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR};
        swapchainInfo.surface = surface;
        swapchainInfo.minImageCount = caps.minImageCount;
        swapchainInfo.imageFormat = format.format;
        swapchainInfo.imageColorSpace = format.colorSpace;
        swapchainInfo.imageExtent = extent;
        swapchainInfo.imageArrayLayers = 1;
        swapchainInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
        swapchainInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        swapchainInfo.preTransform = caps.currentTransform;
        swapchainInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
        swapchainInfo.presentMode = VK_PRESENT_MODE_FIFO_KHR;
        swapchainInfo.clipped = VK_TRUE;
        vkCheck(vkCreateSwapchainKHR(device, &swapchainInfo, nullptr, &swapchain), "vkCreateSwapchainKHR");
        vkCheck(vkGetSwapchainImagesKHR(device, swapchain, &count, nullptr), "vkGetSwapchainImagesKHR");
        std::vector<VkImage> images(count);
        vkCheck(vkGetSwapchainImagesKHR(device, swapchain, &count, images.data()), "vkGetSwapchainImagesKHR");
        for (auto image : images) {
            VkImageViewCreateInfo info{VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO};
            info.image = image;
        info.viewType = VK_IMAGE_VIEW_TYPE_2D;
        info.format = format.format;
            info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        info.subresourceRange.levelCount = 1;
        info.subresourceRange.layerCount = 1;
            VkImageView view{};
        vkCheck(vkCreateImageView(device, &info, nullptr, &view), "vkCreateImageView");
        imageViews.push_back(view);
        }

        // pipeline setup
        const std::string shaderDir{TRIANGLE_SHADER_DIR};
        VkShaderModule vert = shader(device, (shaderDir + "/triangle.vert.spv").c_str());
        VkShaderModule frag = shader(device, (shaderDir + "/triangle.frag.spv").c_str());
        VkPipelineShaderStageCreateInfo stages[2]{{VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO}, {VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO}};
        stages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
        stages[0].module = vert;
        stages[0].pName = "main";
        stages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
        stages[1].module = frag;
        stages[1].pName = "main";
        VkPipelineVertexInputStateCreateInfo vertexInput{VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO};
        VkPipelineInputAssemblyStateCreateInfo assembly{VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO};
        assembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        VkPipelineViewportStateCreateInfo viewport{VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO};
        viewport.viewportCount = 1;
        viewport.scissorCount = 1;
        VkPipelineRasterizationStateCreateInfo raster{VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO};
        raster.polygonMode = VK_POLYGON_MODE_FILL;
        raster.cullMode = VK_CULL_MODE_NONE;
        raster.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
        raster.lineWidth = 1.0F;
        VkPipelineMultisampleStateCreateInfo multisample{VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO};
        multisample.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
        VkPipelineColorBlendAttachmentState blendAttachment{};
        blendAttachment.colorWriteMask = 0xF;
        VkPipelineColorBlendStateCreateInfo blend{VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO};
        blend.attachmentCount = 1;
        blend.pAttachments = &blendAttachment;
        VkDynamicState states[]{VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};
        VkPipelineDynamicStateCreateInfo dynamicState{VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO};
        dynamicState.dynamicStateCount = 2;
        dynamicState.pDynamicStates = states;
        VkPipelineLayoutCreateInfo layoutInfo{VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO};
        vkCheck(vkCreatePipelineLayout(device, &layoutInfo, nullptr, &layout), "vkCreatePipelineLayout");
        VkPipelineRenderingCreateInfo rendering{VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO};
        rendering.colorAttachmentCount = 1;
        rendering.pColorAttachmentFormats = &format.format;
        VkGraphicsPipelineCreateInfo pipelineInfo{VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO};
        pipelineInfo.pNext = &rendering;
        pipelineInfo.stageCount = 2;
        pipelineInfo.pStages = stages;
        pipelineInfo.pVertexInputState = &vertexInput;
        pipelineInfo.pInputAssemblyState = &assembly;
        pipelineInfo.pViewportState = &viewport;
        pipelineInfo.pRasterizationState = &raster;
        pipelineInfo.pMultisampleState = &multisample;
        pipelineInfo.pColorBlendState = &blend;
        pipelineInfo.pDynamicState = &dynamicState;
        pipelineInfo.layout = layout;
        vkCheck(vkCreateGraphicsPipelines(device, {}, 1, &pipelineInfo, nullptr, &pipeline), "vkCreateGraphicsPipelines");
        vkDestroyShaderModule(device, vert, nullptr);
        vkDestroyShaderModule(device, frag, nullptr);

        // command setup
        VkCommandPoolCreateInfo poolInfo{VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO};
        poolInfo.queueFamilyIndex = queueFamily;
        poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        vkCheck(vkCreateCommandPool(device, &poolInfo, nullptr, &commandPool), "vkCreateCommandPool");
        VkCommandBufferAllocateInfo allocate{VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO};
        allocate.commandPool = commandPool;
        allocate.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocate.commandBufferCount = 1;
        vkCheck(vkAllocateCommandBuffers(device, &allocate, &commandBuffer), "vkAllocateCommandBuffers");
        VkSemaphoreCreateInfo semaphore{VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO};
        VkFenceCreateInfo fence{VK_STRUCTURE_TYPE_FENCE_CREATE_INFO};
        fence.flags = VK_FENCE_CREATE_SIGNALED_BIT;
        vkCheck(vkCreateSemaphore(device, &semaphore, nullptr, &imageAvailable), "vkCreateSemaphore");
        vkCheck(vkCreateSemaphore(device, &semaphore, nullptr, &renderFinished), "vkCreateSemaphore");
        vkCheck(vkCreateFence(device, &fence, nullptr, &inFlight), "vkCreateFence");

        while (!glfwWindowShouldClose(window)) {
            glfwPollEvents();
        vkCheck(vkWaitForFences(device, 1, &inFlight, VK_TRUE, UINT64_MAX), "vkWaitForFences");
            std::uint32_t index{};
        vkCheck(vkAcquireNextImageKHR(device, swapchain, UINT64_MAX, imageAvailable, {}, &index), "vkAcquireNextImageKHR");
        vkCheck(vkResetFences(device, 1, &inFlight), "vkResetFences");
        vkCheck(vkResetCommandBuffer(commandBuffer, 0), "vkResetCommandBuffer");
            VkCommandBufferBeginInfo begin{VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO};
        vkCheck(vkBeginCommandBuffer(commandBuffer, &begin), "vkBeginCommandBuffer");
            VkImageMemoryBarrier barrier{VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER};
        barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        barrier.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        barrier.image = images[index];
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        barrier.subresourceRange.levelCount = 1;
        barrier.subresourceRange.layerCount = 1;
            vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier);
            VkClearValue clear{};
        clear.color = {{0.02F, 0.02F, 0.04F, 1.0F}};
        VkRenderingAttachmentInfo color{VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO};
        color.imageView = imageViews[index];
        color.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        color.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        color.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        color.clearValue = clear;
            VkRenderingInfo render{VK_STRUCTURE_TYPE_RENDERING_INFO};
        render.renderArea.extent = extent;
        render.layerCount = 1;
        render.colorAttachmentCount = 1;
        render.pColorAttachments = &color;
        vkCmdBeginRendering(commandBuffer, &render);
            VkViewport vp{0, 0, static_cast<float>(extent.width), static_cast<float>(extent.height), 0, 1};
        VkRect2D scissor{{0, 0}, extent};
        vkCmdSetViewport(commandBuffer, 0, 1, &vp);
        vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
        vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
        vkCmdDraw(commandBuffer, 3, 1, 0, 0);
        vkCmdEndRendering(commandBuffer);
            barrier.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        barrier.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
        barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        barrier.dstAccessMask = 0;
        vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier);
        vkCheck(vkEndCommandBuffer(commandBuffer), "vkEndCommandBuffer");
            VkPipelineStageFlags stage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        VkSubmitInfo submit{VK_STRUCTURE_TYPE_SUBMIT_INFO};
        submit.waitSemaphoreCount = 1;
        submit.pWaitSemaphores = &imageAvailable;
        submit.pWaitDstStageMask = &stage;
        submit.commandBufferCount = 1;
        submit.pCommandBuffers = &commandBuffer;
        submit.signalSemaphoreCount = 1;
        submit.pSignalSemaphores = &renderFinished;
        vkCheck(vkQueueSubmit(queue, 1, &submit, inFlight), "vkQueueSubmit");
            VkPresentInfoKHR present{VK_STRUCTURE_TYPE_PRESENT_INFO_KHR};
        present.waitSemaphoreCount = 1;
        present.pWaitSemaphores = &renderFinished;
        present.swapchainCount = 1;
        present.pSwapchains = &swapchain;
        present.pImageIndices = &index;
        vkCheck(vkQueuePresentKHR(queue, &present), "vkQueuePresentKHR");
        }
        vkDeviceWaitIdle(device);
    } catch (const std::exception& e) { std::cerr << e.what() << '\n'; }
    if (device) { vkDestroyFence(device, inFlight, nullptr);
        vkDestroySemaphore(device, renderFinished, nullptr);
        vkDestroySemaphore(device, imageAvailable, nullptr);
        vkDestroyCommandPool(device, commandPool, nullptr);
        vkDestroyPipeline(device, pipeline, nullptr);
        vkDestroyPipelineLayout(device, layout, nullptr);
        for (auto view : imageViews) vkDestroyImageView(device, view, nullptr);
        vkDestroySwapchainKHR(device, swapchain, nullptr);
        vkDestroyDevice(device, nullptr); }
    if (surface) vkDestroySurfaceKHR(instance, surface, nullptr);
        if (instance) vkDestroyInstance(instance, nullptr);
        if (window) glfwDestroyWindow(window);
        glfwTerminate();
    return 0;
}
