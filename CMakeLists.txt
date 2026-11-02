cmake_minimum_required(VERSION 4.2)
project(vulkan_renderer)

set(CMAKE_CXX_STANDARD 20)

# Homebrew installs CMake package files outside CLion's default search paths.
include(FetchContent)
set(TINYGLTF_BUILD_LOADER_EXAMPLE OFF CACHE BOOL "" FORCE)
set(TINYGLTF_INSTALL OFF CACHE BOOL "" FORCE)
FetchContent_Declare(tinygltf
        GIT_REPOSITORY https://github.com/syoyo/tinygltf.git
        GIT_TAG 488a70a3df62a4df1a736e9e56fb8836580c4888)
FetchContent_MakeAvailable(tinygltf)
find_package(Vulkan REQUIRED)
find_package(glm CONFIG REQUIRED)
find_package(glfw3 CONFIG REQUIRED
        PATHS
        /opt/homebrew/opt/glfw
        /usr/local/opt/glfw)

set(SHADER_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/shaders")
set(SHADER_BINARY_DIR "${CMAKE_CURRENT_BINARY_DIR}/shaders")

add_custom_command(
        OUTPUT "${SHADER_BINARY_DIR}/cube.vert.spv"
        COMMAND ${CMAKE_COMMAND} -E make_directory "${SHADER_BINARY_DIR}"
        COMMAND Vulkan::glslc "${SHADER_SOURCE_DIR}/cube.vert"
                -o "${SHADER_BINARY_DIR}/cube.vert.spv"
        DEPENDS "${SHADER_SOURCE_DIR}/cube.vert"
        VERBATIM)
add_custom_command(
        OUTPUT "${SHADER_BINARY_DIR}/cube.frag.spv"
        COMMAND ${CMAKE_COMMAND} -E make_directory "${SHADER_BINARY_DIR}"
        COMMAND Vulkan::glslc "${SHADER_SOURCE_DIR}/cube.frag"
                -o "${SHADER_BINARY_DIR}/cube.frag.spv"
        DEPENDS "${SHADER_SOURCE_DIR}/cube.frag"
        VERBATIM)
add_custom_target(cube_shaders DEPENDS
        "${SHADER_BINARY_DIR}/cube.vert.spv"
        "${SHADER_BINARY_DIR}/cube.frag.spv")

add_executable(vulkan_renderer main.cpp
        GLTFParser.cpp)
add_dependencies(vulkan_renderer cube_shaders)
target_compile_definitions(vulkan_renderer PRIVATE
        CUBE_SHADER_DIR="${SHADER_BINARY_DIR}")

target_link_libraries(vulkan_renderer PRIVATE
        tinygltf
        Vulkan::Vulkan
        glm::glm
        glfw)
