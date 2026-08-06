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
#include <optional>
#include <set>

#ifdef USE_VULKAN
namespace Argon {
class Vulkan:public RenderAPI {
    struct Instance {
        VkInstance _instance;
        VkDebugUtilsMessengerEXT debug_messenger;
        bool _validation;

        const std::vector<const char*> _validation_layers = {
            "VK_LAYER_KHRONOS_validation"
        };
        void create_instance(const std::vector<const char*>& required_extensions);
        void create_debug_messenger(VkDebugUtilsMessengerCreateInfoEXT& createInfo);
        void check_validation_support();

        static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT severity,
                                                            VkDebugUtilsMessageTypeFlagsEXT type,
                                                            const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
                                                            void* pUserData);
        void clean();

        Instance(bool validation = false):
            _validation(validation),
            _instance(nullptr),
            debug_messenger(nullptr)
        {
            std::vector<const char*> reqext;
            create_instance(reqext);
        };
        Instance(std::vector<const char*> reqext, bool validation = false):
            _validation(validation),
            _instance(nullptr),
            debug_messenger(nullptr)
        {
            create_instance(reqext);
        }
        ~Instance() { clean(); }
    };

    struct Device {
        VkPhysicalDevice _physical_device;
        VkDevice _device;

        VkQueue _graphicsQ;
        VkQueue _presentQ;
        VkQueue _computeQ;
        VkQueue _transferQ;

        VkPhysicalDeviceProperties2 _properties;
        VkPhysicalDeviceFeatures2 _features;
        VkPhysicalDeviceMemoryProperties2 _memory_properties;

        VkSwapchainKHR _swapchain;
        std::vector<VkImage> _images;
        std::vector<VkImageView> _image_views;
        VkFormat _image_format;
        VkExtent2D _extent;

        std::vector<const char*> _extensions = {
            VK_KHR_SWAPCHAIN_EXTENSION_NAME
        };

        struct Queue_family_indices {
            std::optional<uint32_t> graphics;
            std::optional<uint32_t> present;
            std::optional<uint32_t> compute;
            std::optional<uint32_t> transfer;

            bool is_complete() const {
                return graphics.has_value() && present.has_value();
            }

            std::set<uint32_t> unique_queue_families() const {
                std::set<uint32_t> unique_families;
                if(graphics.has_value()) unique_families.insert(graphics.value());
                if(present.has_value()) unique_families.insert(present.value());
                if(compute.has_value()) unique_families.insert(compute.value());
                if(transfer.has_value()) unique_families.insert(transfer.value());
                return unique_families;
            }
        };

        struct Swapchain_details {
            VkSurfaceCapabilities2KHR capabilities;
            std::vector<VkSurfaceFormat2KHR> formats;
            std::vector<VkPresentModeKHR> presentModes;

            bool queried = false;

            bool is_adequate() const {
                return !formats.empty() && !presentModes.empty();
            }
        };

        Queue_family_indices _queue_families;
        Swapchain_details _swapchain_support;

        void pick_physical_device(VkInstance instance, VkSurfaceKHR surface);
        void create_logical_device(VkSurfaceKHR surface);
        void create_swap_chain(VkSurfaceKHR surface, uint32_t width, uint32_t height);
        void create_image_views();

        void destroy_image_views();

        uint32_t score_physical_device(VkPhysicalDevice device, VkSurfaceKHR surface);
        Queue_family_indices find_queue_families(VkPhysicalDevice device, VkSurfaceKHR surface);
        bool check_device_extensions(VkPhysicalDevice device);
        Swapchain_details query_swapchain_details(VkPhysicalDevice device, VkSurfaceKHR surface) const;
        void retrieve_queue_handles();

        VkSurfaceFormat2KHR choose_surface_format(const std::vector<VkSurfaceFormat2KHR>& available_formats);
        VkPresentModeKHR choose_present_mode(const std::vector<VkPresentModeKHR>& available_present_modes);
        VkExtent2D choose_swap_extent(const VkSurfaceCapabilities2KHR& capabilities, uint32_t width, uint32_t height);
        uint32_t choose_image_count(const VkSurfaceCapabilities2KHR& capabilities);

        void query_device_info() {
            vkGetPhysicalDeviceProperties2(_physical_device, &_properties);
            vkGetPhysicalDeviceFeatures2(_physical_device, &_features);
            vkGetPhysicalDeviceMemoryProperties2(_physical_device, &_memory_properties);
        }

        void create(VkInstance instance, VkSurfaceKHR surface, uint32_t width, uint32_t height) {
            pick_physical_device(instance, surface);
            create_logical_device(surface);
            create_swap_chain(surface, width, height);
        }

        void recreate_swapchain(VkSurfaceKHR surface, uint32_t width, uint32_t height) {
            vkDeviceWaitIdle(_device);
            clean_swapchain();
            create_swap_chain(surface, width, height);
        } 

        void clean();

        void clean_swapchain();

        Device():
            _device(nullptr),
            _physical_device(nullptr),
            _graphicsQ(nullptr),
            _presentQ(nullptr),
            _computeQ(nullptr),
            _transferQ(nullptr),
            _swapchain(nullptr),
            _image_format(VK_FORMAT_UNDEFINED),
            _extent({0, 0})
        {}

        Device(VkInstance instance, VkSurfaceKHR surface, uint32_t width, uint32_t height):
            Device()
        {
            create(instance, surface, width, height);
        }

        ~Device() {
            clean();
        }
        
    };

    struct Pipeline {
        VkDevice _device;
        VkPipeline _pipeline;
        VkPipelineLayout _pipeline_layout;
        VkDescriptorSetLayout _desc_layout;
        VkViewport _viewport{};
        VkRect2D _scissor{};
        std::shared_ptr<VertexArray> _vertex_array;
        VkPipelineColorBlendAttachmentState _color_blend_attachment{};

        void create_graphics_pipeline(VkDevice device,
                                      VkRenderPass render_pass,
                                      const VirtualResource vertex_shader_path,
                                      const VirtualResource fragment_shader_path,
                                      VkExtent2D extent);

        bool is_valid() const {
            return _device != VK_NULL_HANDLE &&
                   _pipeline != VK_NULL_HANDLE &&
                   _pipeline_layout != VK_NULL_HANDLE &&
                   _desc_layout != VK_NULL_HANDLE;
        }

        VkShaderModule create_shader_module(const std::string shader_code);
        VkShaderModule load_shader(const VirtualResource shader_path);

        void create_desc_layout();
        void create_pipeline_layout();
        VkPipelineVertexInputStateCreateInfo create_vertex_input_info();
        VkPipelineInputAssemblyStateCreateInfo create_input_assembly_info();
        VkPipelineViewportStateCreateInfo create_viewport_info(VkExtent2D extent);
        VkPipelineRasterizationStateCreateInfo create_rasterization_info();
        VkPipelineMultisampleStateCreateInfo create_multisample_info();
        VkPipelineColorBlendStateCreateInfo create_color_blend_info();
        VkPipelineDepthStencilStateCreateInfo create_depth_stencil_info();

        void clean();
    };

    VkBuffer vertex_buffer;


    void init_vulkan();
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
