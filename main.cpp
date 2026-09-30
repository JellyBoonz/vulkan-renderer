#include <iostream>
#include <string>

#include "GLTFParser.h"

int main() {
    std::string gltfFilename;

    std::cout << "Enter the name or path of a glTF file: " << std::flush;
    if (!std::getline(std::cin, gltfFilename) || gltfFilename.empty()) {
        std::cerr << "No glTF file name was provided.\n";
        return 1;
    }

    std::cout << "Selected glTF file: " << gltfFilename << '\n' << std::flush;

    return parseGLTF(gltfFilename) ? 0 : 1;
}
