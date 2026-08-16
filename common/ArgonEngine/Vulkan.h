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
#define MAX_FRAMES_IN_FLIGHT 2
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

        Pipeline(): 
            _device(VK_NULL_HANDLE),
            _pipeline(VK_NULL_HANDLE),
            _pipeline_layout(VK_NULL_HANDLE),
            _desc_layout(VK_NULL_HANDLE),
            _vertex_array(nullptr)
        {}

        Pipeline(VkDevice device, VkRenderPass render_pass,
                 const VirtualResource vertex_shader_path,
                 const VirtualResource fragment_shader_path,
                 VkExtent2D extent):
            _device(device),
            _pipeline(VK_NULL_HANDLE),
            _pipeline_layout(VK_NULL_HANDLE),
            _desc_layout(VK_NULL_HANDLE),
            _vertex_array(nullptr)
        {
            create_graphics_pipeline(device, render_pass, vertex_shader_path, fragment_shader_path, extent);
        }

        ~Pipeline() {
            clean();
        }
    };

    struct CommandPool {
        VkDevice _device;
        std::vector<VkCommandBuffer> _allocated_buffers;
        VkCommandPool _command_pool;
        uint32_t _queue_family_index;
        bool _allow_reset;
        bool _transient;

        void create_command_pool(VkDevice device, uint32_t queue_family_index,
                                 bool allow_reset = true, bool transient = false);

        std::vector<VkCommandBuffer> allocate_buffers(uint32_t count, 
                                                      VkCommandBufferLevel level = VK_COMMAND_BUFFER_LEVEL_PRIMARY);
        VkCommandBuffer allocate_buffer(VkCommandBufferLevel level = VK_COMMAND_BUFFER_LEVEL_PRIMARY);
        void free_buffers(const std::vector<VkCommandBuffer>& buffers);
        void free_buffer(VkCommandBuffer buffer);
        void reset(bool release_resources = false);

        void begin_buffer(VkCommandBuffer buffer, VkCommandBufferUsageFlags usage = 0,
                          const VkCommandBufferInheritanceInfo* inheritance_info = nullptr);
        void end_buffer(VkCommandBuffer buffer);

        VkCommandBuffer begin_single_time_commands();
        void end_single_time_commands(VkCommandBuffer buffer, VkQueue queue);

        void begin_render_pass(VkCommandBuffer buffer, VkRenderPass render_pass,
                               VkFramebuffer framebuffer, VkRect2D render_area,
                               const std::vector<VkClearValue>& clear_values);
        void end_render_pass(VkCommandBuffer buffer);

        void bind_pipeline(VkCommandBuffer buffer, VkPipeline pipeline,
                           VkPipelineBindPoint bindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS);
        void bind_vertex_buffers(VkCommandBuffer buffer, uint32_t first_binding,
                                 const std::vector<VkBuffer>& buffers,
                                 const std::vector<VkDeviceSize>& offsets);
        void bind_index_buffer(VkCommandBuffer buffer, VkBuffer index_buffer,
                               VkDeviceSize offset = 0, VkIndexType index_type = VK_INDEX_TYPE_UINT32);
        void bind_desc_sets(VkCommandBuffer buffer, VkPipelineLayout pipeline_layout,
                            uint32_t first_set, const std::vector<VkDescriptorSet>& desc_sets,
                            const std::vector<uint32_t>& dynamic_offsets = {});

        void draw(VkCommandBuffer buffer, uint32_t vertex_count, uint32_t instance_count = 1,
                  uint32_t first_vertex = 0, uint32_t first_instance = 0);
        void draw_indexed(VkCommandBuffer buffer, uint32_t index_count, uint32_t instance_count = 1,
                          uint32_t first_index = 0, int32_t vertex_offset = 0, uint32_t first_instance = 0);
        void set_viewport(VkCommandBuffer buffer, float x, float y, float width, float height, 
                          float min_depth = 0.0f, float max_depth = 1.0f);
        void set_scissor(VkCommandBuffer buffer, int32_t x, int32_t y, uint32_t width, uint32_t height);

        void copy_buffer(VkCommandBuffer buffer, VkBuffer src_buffer, VkBuffer dst_buffer,
                         VkDeviceSize size, VkDeviceSize src_offset = 0, VkDeviceSize dst_offset = 0);
        void pipeline_barrier(VkCommandBuffer buffer, VkDependencyFlags dep_flags = 0,
                              const std::vector<VkMemoryBarrier2>& memory_barriers = {},
                              const std::vector<VkBufferMemoryBarrier2>& buffer_barriers = {},
                              const std::vector<VkImageMemoryBarrier2>& image_barriers = {});
        void push_constants(VkCommandBuffer buffer, VkPipelineLayout pipeline_layout,
                            VkShaderStageFlags stage_flags, uint32_t offset, uint32_t size, const void* data);
        void record_frame_commands(VkCommandBuffer buffer,
                                   VkRenderPass render_pass, VkFramebuffer framebuffer,
                                   VkPipeline pipeline, VkPipelineLayout pipeline_layout,
                                   VkRect2D render_area, const std::vector<VkClearValue>& clear_values,
                                   const std::vector<VkBuffer>& vertex_buffers,
                                   const std::vector<VkDeviceSize>& vertex_offsets,
                                   VkBuffer index_buffer = VK_NULL_HANDLE, VkDeviceSize index_offsets = 0,
                                   const std::vector<VkDescriptorSet>& desc_sets = {},
                                   uint32_t vertex_count = 0, uint32_t index_count = 0,
                                   uint32_t instance_count = 1);
        void clean();

        CommandPool():
            _device(nullptr),
            _allocated_buffers(),
            _command_pool(nullptr)
        {}

        CommandPool(VkDevice device, uint32_t queue_family_index,
                    bool allow_reset = true, bool transient = false):
            _device(device),
            _queue_family_index(queue_family_index),
            _allow_reset(allow_reset),
            _transient(transient)
        {
            create_command_pool(device, queue_family_index);
        }

        ~CommandPool() {
            clean();
        }
    };

    struct Buffer {
        VkDevice _device;
        VkBuffer _buffer;
        VkDeviceMemory _memory;
        VkDeviceSize _size;
        VkBufferUsageFlags _usage;
        VkMemoryPropertyFlags _memory_properties;

        void* _mapped_memory;
        bool _is_coherent;

        void create_buffer(VkDevice device, VkPhysicalDevice physical_device, VkDeviceSize size,
                           VkBufferUsageFlags usage, VkMemoryPropertyFlags memory_properties);

        void create_buffer_with_data(VkDevice device, VkPhysicalDevice physical_device, 
                                     const void* data, VkDeviceSize size, VkBufferUsageFlags usage, 
                                     VkMemoryPropertyFlags memory_properties);

        void* map(VkDeviceSize offset = 0, VkDeviceSize size = VK_WHOLE_SIZE);
        void unmap();

        void upload_data(const void* data, VkDeviceSize size, VkDeviceSize offset = 0);
        void copy_to(VkDevice device, VkCommandPool command_pool, VkQueue graphics_queue,
                     Buffer& dst_buffer, VkDeviceSize size, VkDeviceSize src_offset = 0,
                     VkDeviceSize dst_offset = 0);
        void flush(VkDeviceSize offset = 0, VkDeviceSize size = VK_WHOLE_SIZE);
        void invalidate(VkDeviceSize offset = 0, VkDeviceSize size = VK_WHOLE_SIZE);

        uint32_t find_mem_type(VkPhysicalDevice physical_device, uint32_t type_filter,
                               VkMemoryPropertyFlags properties) const;

        VkCommandBuffer begin_single_time_commands(VkDevice device, VkCommandPool command_pool) const;
        void end_single_time_commands(VkDevice device, VkCommandPool command_pool,
                                      VkCommandBuffer buffer, VkQueue queue) const;

        Buffer create_vertex_buffer(VkDevice device, VkPhysicalDevice physical_device, VkCommandPool command_pool,
                                    VkQueue graphics_queue, std::shared_ptr<VertexArray> vertex_array);
        Buffer create_index_buffer(VkDevice device, VkPhysicalDevice physical_device, VkCommandPool command_pool,
                                   VkQueue graphics_queue, std::shared_ptr<VertexArray> vertex_array);
        Buffer create_uniform_buffer(VkDevice device, VkPhysicalDevice physical_device);
        Buffer create_staging_buffer(VkDevice device, VkPhysicalDevice physical_device, VkDeviceSize size);

        std::vector<Buffer> create_attrib_buffers(VkDevice device, VkPhysicalDevice physical_device,
                                                  VkCommandPool command_pool, VkQueue graphicsQueue,
                                                  std::shared_ptr<VertexArray> vertex_array);
        void update_vertex_buffer(Buffer& buffer, VkDevice device, VkPhysicalDevice physical_device,
                                  VkCommandPool command_pool, VkQueue graphics_queue,
                                  std::shared_ptr<VertexArray> vertex_array,
                                  VkDeviceSize offset = 0);

        std::vector<Buffer> create_uniform_buffers_in_flight(VkDevice device, VkPhysicalDevice physical_device,
                                                             uint32_t frames_in_flight);
        Buffer create_dynamic_uniform_buffer(VkDevice device, VkPhysicalDevice physical_device, uint32_t object_count);
        void update_dynamic_uniform_buffer(Buffer& dynamic_ubuffer, VkPhysicalDevice physical_device,
                                           uint32_t object_index, Uniforms uniform);

        static VkMemoryRequirements2 get_mem_requirements(VkDevice device, VkDeviceSize size,
                                                          VkBufferUsageFlags usage);

        void clean();

        Buffer():
            _device(nullptr),
            _buffer(nullptr),
            _memory(nullptr),
            _size(0),
            _mapped_memory(nullptr)
        {}

        Buffer(VkDevice device, VkPhysicalDevice physical_device, VkDeviceSize size,
               VkBufferUsageFlags usage, VkMemoryPropertyFlags memory_properties):
            _device(device),
            _size(size),
            _usage(usage),
            _memory_properties(memory_properties),
            _mapped_memory(nullptr),
            _buffer(nullptr)
        {
            create_buffer(device, physical_device, size, usage, memory_properties);
        }
        
        ~Buffer() {
            clean();
        }
    };

    struct RenderPass {
        VkDevice _device;
        VkRenderPass _render_pass;
        std::vector<VkFramebuffer> framebuffers;
        VkExtent2D _extent{};

        void create_render_pass(VkDevice device, VkFormat color_format, VkFormat depth_format,
                                VkSampleCountFlagBits msaa_samples = VK_SAMPLE_COUNT_1_BIT);

        void create_framebuffers(const std::vector<VkImageView>& swapchain_image_views,
                                 VkImageView depth_image_view, VkExtent2D extent);

        void recreate_framebuffers(const std::vector<VkImageView>& swapchain_image_views,
                                   VkImageView depth_image_view, VkExtent2D extent);

        void destroy_framebuffers();

        void clean();

        RenderPass():
            _device(nullptr),
            _render_pass(nullptr)
        {}

        RenderPass(VkDevice device, VkFormat color_format, VkFormat depth_format,
                   VkSampleCountFlagBits msaa_samples = VK_SAMPLE_COUNT_1_BIT):
            _device(device),
            _render_pass(nullptr)
        {
            create_render_pass(device, color_format, depth_format, msaa_samples);
        }

        ~RenderPass() {
            clean();
        }

        RenderPass(const RenderPass&) = delete;
        RenderPass& operator=(const RenderPass&) = delete;
    };

    struct Synchronization {
        struct FrameSyncObjects {
            VkSemaphore image_available_semaphore;
            VkSemaphore render_finished_semaphore;
            VkFence in_flight_fence;

            FrameSyncObjects():
                image_available_semaphore(VK_NULL_HANDLE),
                render_finished_semaphore(VK_NULL_HANDLE),
                in_flight_fence(VK_NULL_HANDLE)
            {}
        };

        VkDevice _device;
        std::vector<FrameSyncObjects> _frame_sync_objects;
        uint32_t _max_frames_in_flight;
        std::vector<VkSemaphore> _semaphores;
        std::vector<VkFence> _fences;

        void create_sync(VkDevice device, uint32_t max_frames_in_flight = MAX_FRAMES_IN_FLIGHT);

        bool wait_for_frame(uint32_t frame_index, uint64_t timeout = UINT64_MAX);
        void reset_frame_fence(uint32_t frame_index);
        bool wait_for_all_frames(uint64_t timeout = UINT64_MAX);
        VkResult wait_for_fences(VkDevice device, const std::vector<VkFence>& fences,
                                 bool wait_all = true, uint64_t timeout = UINT64_MAX);

        void reset_fences(VkDevice device, const std::vector<VkFence>& fences);

        VkSemaphore create_semaphore(VkDevice device);
        VkFence create_fence(VkDevice device, bool signaled = true);
        void destroy_semaphore(VkDevice device, VkSemaphore semaphore);
        void destroy_fence(VkDevice device, VkFence fence);

        VkSubmitInfo create_submit_info(VkCommandBuffer buffer,
                                        VkSemaphore wait_semaphore,
                                        VkPipelineStageFlags wait_stage,
                                        VkSemaphore signal_semaphore);

        VkPresentInfoKHR create_present_info(VkSwapchainKHR swapchain, uint32_t image_index,
                                             VkSemaphore wait_semaphore);

        void submit_command_buffers(VkQueue queue,
                                    const std::vector<VkCommandBuffer>& command_buffers,
                                    const std::vector<VkSemaphore>& wait_semaphores = {},
                                    const std::vector<VkPipelineStageFlags>& wait_stages = {},
                                    const std::vector<VkSemaphore>& signal_semaphores = {},
                                    VkFence fence = VK_NULL_HANDLE);

        VkResult present_image(VkQueue present_queue, VkSwapchainKHR swapchain,
                               uint32_t image_index, const std::vector<VkSemaphore>& wait_semaphores = {});

        VkResult acquire_next_image(VkDevice device, VkSwapchainKHR swapchain,
                                    uint64_t timeout, VkSemaphore semaphore,
                                    VkFence fence, uint32_t* image_index);

        void clean();

        Synchronization():
            _device(nullptr)
        {}

        Synchronization(VkDevice device, uint32_t max_frames_in_flight = MAX_FRAMES_IN_FLIGHT):
            _device(device)
        {
            create_sync(device, max_frames_in_flight);
        }
    };

    void init_vulkan();

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
