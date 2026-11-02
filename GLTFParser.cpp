#include "GLTFParser.h"

#include <algorithm>
#include <cctype>
#include <filesystem>
#include <iostream>

bool loadGLTF(const std::string& filename, tinygltf::Model& model) {
    std::string extension{std::filesystem::path(filename).extension().string()};
    std::transform(extension.begin(), extension.end(), extension.begin(),
                   [](unsigned char character) { return static_cast<char>(std::tolower(character)); });
    if (extension != ".gltf" && extension != ".glb") {
        std::cerr << "Expected a .gltf or .glb file: " << filename << '\n';
        return false;
    }

    tinygltf::TinyGLTF loader{};
    tinygltf::Model loadedModel{};
    std::string error{};
    std::string warning{};
    const bool loaded{extension == ".glb"
        ? loader.LoadBinaryFromFile(&loadedModel, &error, &warning, filename)
        : loader.LoadASCIIFromFile(&loadedModel, &error, &warning, filename)};
    if (!warning.empty()) std::cerr << "glTF warning: " << warning << '\n';
    if (!error.empty()) std::cerr << "glTF error: " << error << '\n';
    if (!loaded) {
        std::cerr << "Could not load glTF file: " << filename << '\n';
        return false;
    }
    model = std::move(loadedModel);
    return true;
}
