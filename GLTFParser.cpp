#include <fstream>
#include <iostream>
#include <string>

#include <nlohmann/json.hpp>

using json = nlohmann::json;

bool parseGLTF(const std::string& gltfFilename) {
    std::ifstream file(gltfFilename, std::ios::binary);
    if (!file) {
        std::cerr << "Could not open glTF file: " << gltfFilename << '\n';
        return false;
    }

    if (file.peek() == std::ifstream::traits_type::eof()) {
        std::cerr << "The glTF file is empty: " << gltfFilename << '\n';
        return false;
    }

    try {
        json gltf = json::parse(file);

        std::size_t bufferSize = gltf["buffers"][0]["byteLength"].get<std::size_t>();
        int positionAccessor = gltf["meshes"][0]["primitives"][0]["attributes"]["POSITION"];
        auto& accessor = gltf["accessors"][positionAccessor];

        int bufferViewIndex = accessor["bufferView"];
        int componentType   = accessor["componentType"];
        std::size_t count   = accessor["count"];

        std::cout << bufferViewIndex << " " << componentType << " " << count << std::endl;
        return true;
    } catch (const json::exception& error) {
        std::cerr << "Could not read glTF JSON: " << error.what() << '\n';
        return false;
    }
}
