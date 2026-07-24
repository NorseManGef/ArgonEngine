#pragma once
/**
 * @brief Agnostic graphics functions.
 */
#include <sstream>
#include <string>
#include <unordered_map>

namespace Argon {
/**
 * @brief Selection of renderer to use.
 */
enum class Renderer { OGL, VULKAN };

inline std::string renderer_name(Renderer renderer) {
    switch (renderer) {
    case Renderer::OGL:
        return "ogl";
    case Renderer::VULKAN:
        return "vulkan";
    default:
        return "Unknown"; // this should never happen
    }
}

inline std::unordered_map<std::string, Renderer> get_renderers() {
    std::unordered_map<std::string, Renderer> renderers;
    for (Renderer renderer = Renderer::OGL; renderer <= Renderer::VULKAN;
         renderer = (Renderer)((int)renderer + 1)) {
        renderers.insert(std::make_pair(renderer_name(renderer), renderer));
    }
    return renderers;
}

inline std::string get_renderer_help() {
    std::stringstream str;
    str << "Which graphics API to use." << std::endl;
    str << "Options:";
    auto renderers = get_renderers();
    for (auto pair : renderers) {
        auto name = pair.first;
        str << std::endl << name;
    }
    return str.str();
}
} // namespace Argon
