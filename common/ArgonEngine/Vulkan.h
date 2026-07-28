/*
 * @brief This file contains the vulkan backend for the renderAPI class
 * @file Vulkan.h
 * @author Garrett Rosende
 **/

#include "RenderAPI.h"
#include "RenderSystem.h"
#include "vulkan/vulkan_core.h"
#include <vulkan/vulkan.h>
#include <plog/Log.h>

#ifdef USE_VULKAN
namespace Argon {
class Vulkan:public RenderAPI {
    struct Instance {
        VkInstance instance;
        VkDebugUtilsMessengerEXT debug_messenger;
        VkPhysicalDevice physical_device;
        VkDevice device;
        bool _validation;

        const std::vector<const char*> _validation_layers = {
            "VK_LAYER_KHRONOS_validation"
        };
        void create_instance(const std::vector<const char*>& required_extensions);
        void create_debug_messenger(VkDebugUtilsMessengerCreateInfoEXT& createInfo);
        void pick_physical_device();
        void create_logical_device();
        void check_validation_support();

        static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT severity,
                                                            VkDebugUtilsMessageTypeFlagsEXT type,
                                                            const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
                                                            void* pUserData);
        void clean();

        Instance(bool validation = false):
            _validation(validation),
            instance(nullptr),
            debug_messenger(nullptr),
            physical_device(nullptr),
            device(nullptr)
        {
            std::vector<const char*> reqext;
            create_instance(reqext);
        };
        Instance(std::vector<const char*> reqext, bool validation = false):
            _validation(validation),
            instance(nullptr),
            debug_messenger(nullptr),
            physical_device(nullptr),
            device(nullptr)
        {
            create_instance(reqext);
        }
        ~Instance() { clean(); }
    };
    VkBuffer vertex_buffer;

    void init_vulkan();
    void create_surface();
    void create_swap_chain();
    void create_image_views();
    void create_graphics_pipeline();
    void create_command_pool();
    void create_vertex_buffer();
    void create_command_buffers();
    void create_sync_objects();

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
