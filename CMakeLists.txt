cmake_minimum_required(VERSION 4.2)
project(vulkan_renderer)

set(CMAKE_CXX_STANDARD 20)

# Homebrew installs CMake package files outside CLion's default search paths.
find_package(nlohmann_json CONFIG REQUIRED
        PATHS
        /opt/homebrew/opt/nlohmann-json
        /usr/local/opt/nlohmann-json)
find_package(OpenSSL REQUIRED)
find_package(Vulkan REQUIRED)
find_package(glfw3 CONFIG REQUIRED
        PATHS
        /opt/homebrew/opt/glfw
        /usr/local/opt/glfw)

set(SHADER_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/shaders")
set(SHADER_BINARY_DIR "${CMAKE_CURRENT_BINARY_DIR}/shaders")

add_custom_command(
        OUTPUT "${SHADER_BINARY_DIR}/triangle.vert.spv"
        COMMAND ${CMAKE_COMMAND} -E make_directory "${SHADER_BINARY_DIR}"
        COMMAND Vulkan::glslc "${SHADER_SOURCE_DIR}/triangle.vert"
                -o "${SHADER_BINARY_DIR}/triangle.vert.spv"
        DEPENDS "${SHADER_SOURCE_DIR}/triangle.vert"
        VERBATIM)
add_custom_command(
        OUTPUT "${SHADER_BINARY_DIR}/triangle.frag.spv"
        COMMAND ${CMAKE_COMMAND} -E make_directory "${SHADER_BINARY_DIR}"
        COMMAND Vulkan::glslc "${SHADER_SOURCE_DIR}/triangle.frag"
                -o "${SHADER_BINARY_DIR}/triangle.frag.spv"
        DEPENDS "${SHADER_SOURCE_DIR}/triangle.frag"
        VERBATIM)
add_custom_target(triangle_shaders DEPENDS
        "${SHADER_BINARY_DIR}/triangle.vert.spv"
        "${SHADER_BINARY_DIR}/triangle.frag.spv")

add_executable(vulkan_renderer main.cpp
        GLTFParser.cpp)
add_dependencies(vulkan_renderer triangle_shaders)
target_compile_definitions(vulkan_renderer PRIVATE
        TRIANGLE_SHADER_DIR="${SHADER_BINARY_DIR}")

target_link_libraries(vulkan_renderer PRIVATE
        nlohmann_json::nlohmann_json
        OpenSSL::Crypto
        Vulkan::Vulkan
        glfw)
