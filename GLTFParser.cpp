#include <cstdint>
#include <fstream>
#include <iostream>
#include <string>
#include <string_view>
#include <vector>

#include <nlohmann/json.hpp>
#include <openssl/evp.h>

using json = nlohmann::json;

namespace {

bool decodeBase64DataUri(const std::string& uri, std::vector<std::uint8_t>& decodedBytes) {
    constexpr std::string_view dataPrefix{"data:"};
    constexpr std::string_view base64Marker{";base64"};

    if (!uri.starts_with(dataPrefix)) {
        std::cerr << "Buffer URI is not an embedded data URI.\n";
        return false;
    }

    const std::size_t comma{uri.find(',')};
    if (comma == std::string::npos || uri.substr(0, comma).find(base64Marker) == std::string::npos) {
        std::cerr << "Buffer data URI is not Base64 encoded.\n";
        return false;
    }

    const std::string_view encoded{uri.data() + comma + 1, uri.size() - comma - 1};
    if (encoded.empty() || encoded.size() % 4 != 0) {
        std::cerr << "Buffer contains invalid Base64 data.\n";
        return false;
    }

    decodedBytes.resize(encoded.size() / 4 * 3);
    const int decodedLength{EVP_DecodeBlock(decodedBytes.data(),
                                             reinterpret_cast<const unsigned char*>(encoded.data()),
                                             static_cast<int>(encoded.size()))};
    if (decodedLength < 0) {
        std::cerr << "Buffer contains invalid Base64 data.\n";
        return false;
    }

    const std::size_t padding{encoded.ends_with("==") ? std::size_t{2} :
                                                        (encoded.ends_with("=") ? std::size_t{1} : std::size_t{0})};
    decodedBytes.resize(static_cast<std::size_t>(decodedLength) - padding);
    return true;
}

} // namespace

bool parseGLTF(const std::string& gltfFilename) {
    std::ifstream file{gltfFilename, std::ios::binary};
    if (!file) {
        std::cerr << "Could not open glTF file: " << gltfFilename << '\n';
        return false;
    }

    if (file.peek() == std::ifstream::traits_type::eof()) {
        std::cerr << "The glTF file is empty: " << gltfFilename << '\n';
        return false;
    }

    try {
        const json gltf = json::parse(file);

        const std::size_t positionAccessorIndex{
            gltf["meshes"][std::size_t{0}]
                ["primitives"][std::size_t{0}]
                ["attributes"]["POSITION"]
        };
        const auto& positionAccessor{gltf["accessors"][positionAccessorIndex]};
        const std::size_t bufferViewIndex{positionAccessor["bufferView"]};
        const auto& bufferView{gltf["bufferViews"][bufferViewIndex]};
        const std::size_t bufferIndex{bufferView["buffer"]};
        const auto& buffer{gltf["buffers"][bufferIndex]};

        const std::size_t bufferSize{buffer["byteLength"].get<std::size_t>()};
        const std::string& bufferUri{buffer["uri"]};
        std::vector<std::uint8_t> bufferBytes{};
        if (!decodeBase64DataUri(bufferUri, bufferBytes)) {
            return false;
        }
        if (bufferBytes.size() != bufferSize) {
            std::cerr << "Decoded buffer size does not match the glTF byteLength.\n";
            return false;
        }

        const int componentType{positionAccessor["componentType"]};
        const std::size_t count{positionAccessor["count"]};

        std::cout << "Decoded " << bufferBytes.size() << " bytes from buffer " << bufferIndex << ".\n";
        std::cout << bufferViewIndex << " " << componentType << " " << count << std::endl;

        const auto startByte{bufferView["byteOffset"].get<std::size_t>()};

        for (std::size_t i{startByte}; i < startByte + count * 12; i += 12) {
            float x{};
            float y{};
            float z{};

            std::memcpy(&x, bufferBytes.data() + i,     sizeof(float));
            std::memcpy(&y, bufferBytes.data() + i + 4, sizeof(float));
            std::memcpy(&z, bufferBytes.data() + i + 8, sizeof(float));

            std::cout << x << ", " << y << ", " << z << '\n';
        }

        return true;
    } catch (const json::exception& error) {
        std::cerr << "Could not read glTF JSON: " << error.what() << '\n';
        return false;
    }
}
