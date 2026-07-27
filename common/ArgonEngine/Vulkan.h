/*
 * @brief This file contains the vulkan backend for the renderAPI class
 * @file Vulkan.h
 * @author Garrett Rosende
 **/

#include "RenderAPI.h"
#include "RenderSystem.h"
#include <vulkan/vulkan.h>
#include <plog/Log.h>

#ifdef USE_VULKAN
namespace Argon {
class Vulkan:public RenderAPI {
    VkBuffer vertex_buffer = nullptr;
    VkInstance instance = nullptr;
    VkPhysicalDevice physical_device = nullptr;
    VkDevice device = nullptr;

    bool _validation;

    const std::vector<const char*> _validation_layers = {
        "VK_LAYER_KHRONOS_validation"
    };

    void init_vulkan();
    void create_instance(const std::vector<const char*>& required_extensions);
    void create_debug_messenger(VkDebugUtilsMessengerCreateInfoEXT& createInfo);
    void create_surface();
    void pick_physical_device();
    void create_logical_device();
    void create_swap_chain();
    void create_image_views();
    void create_graphics_pipeline();
    void create_command_pool();
    void create_vertex_buffer();
    void create_command_buffers();
    void create_sync_objects();

    void check_validation_support();

    static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT severity,
                                                        VkDebugUtilsMessageTypeFlagsEXT type,
                                                        const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
                                                        void * pUserData);

    void clean();

public:
    Vulkan() {} 
    ~Vulkan() {
        clean();
    }
    void draw_vertex_array(std::shared_ptr<VertexArray> array, int end_vert, int draw_mode);
    void update_resources();
    void set_blend(unsigned int blend);
    void set_cull_face(int face);
    void set_depth_range(float near, float far);
    void set_render_flags(unsigned int render_flags);
    void set_clear_color(Argon::Vector4f v);
    void set_viewport(int x, int y, int w, int h);
    void set_uniforms(Uniforms** uniforms, int size);
    void set_shader(Renderable*, VirtualResource& x, Uniforms** uniforms, int size);
    void bind_render_framebuffer();
    void bind_default_framebuffer();
    void bind_depth_texture(VirtualResource t);
    void bind_color_texture(VirtualResource t, int level, int buffer);
    void clear(bool color, bool depth, bool stencil);
    void cache_texture(VirtualResource tex);
    void cache_array(std::shared_ptr<VertexArray> array);
    void cache_material(Material& state, const VirtualResource& shader);
};
}
#endif
