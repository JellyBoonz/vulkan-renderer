#pragma once

#include <string>
#include <tiny_gltf.h>

bool loadGLTF(const std::string& filename, tinygltf::Model& model);
