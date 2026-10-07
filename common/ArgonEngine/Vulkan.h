/*
 * @brief This file contains the vulkan backend for the renderAPI class
 * @file Vulkan.h
 * @author Garrett Rosende
 **/

#pragma once

#include "ArgonEngine/ArgonInit.h"
#include "ArgonEngine/Utility.h"
#include "RenderAPI.h"
#include "RenderSystem.h"
#include "Hardware.h"
#include "vulkan/vulkan_core.h"
#include <type_traits>
#include <vulkan/vulkan.h>
#include <plog/Log.h>
#include <SPIRV-Reflect/spirv_reflect.h>
#include <shaderc/shaderc.h>
#include <spirv-tools/libspirv.h>
#include <optional>
#include <set>
#include <SDL3/SDL_vulkan.h>
#include <unordered_map>

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
        void populate_debug_messenger_info(VkDebugUtilsMessengerCreateInfoEXT& createInfo);
        void create_debug_messenger(VkDebugUtilsMessengerCreateInfoEXT& createInfo);
        void check_validation_support();

        static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT severity,
                                                            VkDebugUtilsMessageTypeFlagsEXT type,
                                                            const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
                                                            void* pUserData);
        void clean();

        Instance():
            _validation(false),
            _instance(VK_NULL_HANDLE),
            debug_messenger(VK_NULL_HANDLE)
        {};

        Instance(bool validation):
            _validation(validation),
            _instance(nullptr),
            debug_messenger(nullptr)
        {};
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
        VkSurfaceKHR _surface;

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
            VkSurfaceCapabilitiesKHR capabilities;
            std::vector<VkSurfaceFormatKHR> formats;
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

        VkSurfaceFormatKHR choose_surface_format(const std::vector<VkSurfaceFormatKHR>& available_formats);
        VkPresentModeKHR choose_present_mode(const std::vector<VkPresentModeKHR>& available_present_modes);
        VkExtent2D choose_swap_extent(const VkSurfaceCapabilitiesKHR& capabilities, uint32_t width, uint32_t height);
        uint32_t choose_image_count(const VkSurfaceCapabilitiesKHR& capabilities);

        void query_device_info() {
            vkGetPhysicalDeviceProperties2(_physical_device, &_properties);
            vkGetPhysicalDeviceFeatures2(_physical_device, &_features);
            vkGetPhysicalDeviceMemoryProperties2(_physical_device, &_memory_properties);
        }

        void create(VkInstance instance, VkSurfaceKHR surface, uint32_t width, uint32_t height) {
            _surface = surface;
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

    struct Buffer;

    struct Required_Pipeline_State {
        unsigned int _current_blend = kBlendReplace;
        bool _current_blend_enabled = true;
        VkPrimitiveTopology _current_topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        VkCullModeFlags _current_cull_mode = VK_CULL_MODE_BACK_BIT;
        VkFrontFace _current_front_face = VK_FRONT_FACE_COUNTER_CLOCKWISE;
        float _current_min_depth = 0.0f;
        float _current_max_depth = 1.0f;
        unsigned int _current_render_flags = kRenderDefault;
        VkClearValue _current_clear_value;
        VkViewport _current_viewport;

        VkDescriptorSetLayout _current_desc_layout = VK_NULL_HANDLE;
        
        VkShaderModule *_current_vertex_shader = VK_NULL_HANDLE;
        VkShaderModule *_current_fragment_shader = VK_NULL_HANDLE;

        VkRenderPass _current_render_pass;

        VkExtent2D _extent;
        VkDescriptorPool _desc_pool; // SHOULD ALWAYS BE THE DESCRIPTOR POOL CREATED IN INITIALIZAITON
        VkDevice _device; // SHOULD ALWAYS BE THE DEVICE IN USE

        bool operator==(const Required_Pipeline_State& b)const{
            return
            _current_blend == b._current_blend &&
            _current_blend_enabled == b._current_blend_enabled &&
            _current_topology == b._current_topology &&
            _current_cull_mode == b._current_cull_mode &&
            _current_front_face == b._current_front_face &&
            _current_min_depth == b._current_min_depth &&
            _current_max_depth == b._current_max_depth &&
            _current_render_flags == b._current_render_flags &&

            // TODO: compare the appropriate VkClearValue members

            _current_viewport.x == b._current_viewport.x &&
            _current_viewport.y == b._current_viewport.y &&
            _current_viewport.width == b._current_viewport.width &&
            _current_viewport.height == b._current_viewport.height &&
            _current_viewport.minDepth == b._current_viewport.minDepth &&
            _current_viewport.maxDepth == b._current_viewport.maxDepth &&

            _current_desc_layout == b._current_desc_layout &&
            _desc_pool == b._desc_pool &&

            _current_vertex_shader == b._current_vertex_shader &&
            _current_fragment_shader == b._current_fragment_shader &&
            _current_render_pass == b._current_render_pass &&

            _extent.width == b._extent.width &&
            _extent.height == b._extent.height &&

            _device == b._device;
        }

    };

    struct Required_Pipeline_State_Hash {
        std::size_t operator()(const Required_Pipeline_State& s) const {
            std::size_t seed = 0;

            auto hash_combine = [&seed](std::size_t value) {
                seed ^= value + 0x9e3779b9 + (seed << 6) + (seed >> 2);
            };

            hash_combine(std::hash<unsigned int>{}(s._current_blend));
            hash_combine(std::hash<bool>{}(s._current_blend_enabled));

            hash_combine(std::hash<std::underlying_type_t<VkPrimitiveTopology>>{}(
                static_cast<std::underlying_type_t<VkPrimitiveTopology>>(
                    s._current_topology)));

            hash_combine(std::hash<VkCullModeFlags>{}(s._current_cull_mode));
            hash_combine(std::hash<std::underlying_type_t<VkFrontFace>>{}(
                static_cast<std::underlying_type_t<VkFrontFace>>(
                    s._current_front_face)));

            hash_combine(std::hash<float>{}(s._current_min_depth));
            hash_combine(std::hash<float>{}(s._current_max_depth));

            hash_combine(std::hash<unsigned int>{}(s._current_render_flags));

            hash_combine(hash_bytes(s._current_clear_value));

            hash_combine(hash_bytes(s._current_viewport));

            hash_combine(std::hash<VkDescriptorSetLayout>{}(s._current_desc_layout));

            hash_combine(std::hash<VkShaderModule*>{}(s._current_vertex_shader));
            hash_combine(std::hash<VkShaderModule*>{}(s._current_fragment_shader));

            hash_combine(std::hash<VkRenderPass>{}(s._current_render_pass));

            hash_combine(std::hash<uint32_t>{}(s._extent.width));
            hash_combine(std::hash<uint32_t>{}(s._extent.height));

            hash_combine(std::hash<VkDescriptorPool>{}(s._desc_pool));
            hash_combine(std::hash<VkDevice>{}(s._device));

            return seed;
        }

        private:
        template<typename T>
        static std::size_t hash_bytes(const T& value) {
            const auto* bytes =
                reinterpret_cast<const unsigned char*>(&value);

            std::size_t hash = 14695981039346656037ull;

            for (std::size_t i = 0; i < sizeof(T); ++i) {
                hash ^= bytes[i];
                hash *= 1099511628211ull;
            }

            return hash;
        }
    };

    struct Pipeline {
        VkDevice _device;
        VkPipeline _pipeline;
        VkPipelineLayout _pipeline_layout;
        VkDescriptorSetLayout _desc_layout;
        VkDescriptorSet _desc_set;
        VkDescriptorPool _desc_pool;
        VkViewport _viewport{};
        VkRect2D _scissor{};
        VkExtent2D _extent{};
        std::shared_ptr<VertexArray> _vertex_array;
        VkPipelineColorBlendAttachmentState _color_blend_attachment{};

        VkVertexInputBindingDescription _binding_desc{};
        std::vector<VkVertexInputAttributeDescription> _attribs;

        unsigned int _current_blend = kBlendReplace;
        bool _current_blend_enabled = true;
        VkPrimitiveTopology _current_topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        VkCullModeFlags _current_cull_mode = VK_CULL_MODE_BACK_BIT;
        VkFrontFace _current_front_face = VK_FRONT_FACE_COUNTER_CLOCKWISE;
        float _current_min_depth = 0.0f;
        float _current_max_depth = 1.0f;
        unsigned int _current_render_flags = kRenderDefault;
        VkClearValue _current_clear_value;
        
        VkShaderModule *_current_vertex_shader = VK_NULL_HANDLE;
        VkShaderModule *_current_fragment_shader = VK_NULL_HANDLE;

        VkRenderPass _current_render_pass;

        uint32_t _last_frame = 0;

        void create_graphics_pipeline();

        void create_graphics_pipeline(Required_Pipeline_State state);

        bool is_valid() const {
            return _device != VK_NULL_HANDLE &&
                   _pipeline != VK_NULL_HANDLE &&
                   _pipeline_layout != VK_NULL_HANDLE &&
                   _desc_layout != VK_NULL_HANDLE;
        }

        VkDescriptorSetLayout create_desc_sets();
        void create_pipeline_layout();
        VkPipelineVertexInputStateCreateInfo create_vertex_input_info();
        VkPipelineInputAssemblyStateCreateInfo create_input_assembly_info();
        VkPipelineViewportStateCreateInfo create_viewport_info();
        VkPipelineRasterizationStateCreateInfo create_rasterization_info();
        VkPipelineMultisampleStateCreateInfo create_multisample_info();
        VkPipelineColorBlendStateCreateInfo create_color_blend_info();
        VkPipelineDepthStencilStateCreateInfo create_depth_stencil_info();
        VkPipelineDynamicStateCreateInfo create_dynamic_state_info();

        VkBlendFactor blend_converter(unsigned int b);

        void clean();

        Pipeline(): 
            _device(VK_NULL_HANDLE),
            _pipeline(VK_NULL_HANDLE),
            _pipeline_layout(VK_NULL_HANDLE),
            _desc_layout(VK_NULL_HANDLE),
            _vertex_array(nullptr)
        {}

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
        static void pipeline_barrier(VkCommandBuffer buffer, VkDependencyFlags dep_flags = 0,
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
        void set_primitive_topology(VkCommandBuffer buffer, VkPrimitiveTopology topology);
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

    struct Uniform;

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


        VkCommandBuffer begin_single_time_commands(VkDevice device, VkCommandPool command_pool) const;
        void end_single_time_commands(VkDevice device, VkCommandPool command_pool,
                                      VkCommandBuffer buffer, VkQueue queue) const;

        Buffer create_vertex_buffer(VkDevice device, VkPhysicalDevice physical_device, VkCommandPool command_pool,
                                    VkQueue graphics_queue, std::shared_ptr<VertexArray> vertex_array);
        Buffer create_index_buffer(VkDevice device, VkPhysicalDevice physical_device, VkCommandPool command_pool,
                                   VkQueue graphics_queue, std::shared_ptr<VertexArray> vertex_array);
        Buffer create_uniform_buffer(VkDevice device, VkPhysicalDevice physical_device, VkDeviceSize size);
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

        uint32_t _dynamic_alignment = UINT32_MAX; // UINT32_MAX implies that the buffer is either not initialized or not a uniform buffer
        std::map<uint64_t, VkDeviceSize> _current_uniform_offsets;
        VkDeviceSize _current_uniform_buffer = 0;

        static uint64_t uniform_binding_key(uint32_t set, uint32_t binding) {
            return (static_cast<uint64_t>(set) << 32) | binding;
        }

        static Buffer create_dynamic_uniform_buffer(VkDevice device, VkPhysicalDevice physical_device, VkDeviceSize size);
        static void update_dynamic_uniform_buffer(Buffer& dynamic_ubuffer, VkDeviceSize block_offset,
                                                  VkDeviceSize uniform_offset, const void* data, VkDeviceSize data_size);

        static VkMemoryRequirements get_mem_requirements(VkDevice device, VkDeviceSize size,
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

    struct RealTexFormat {
        int argon_format_flag;
        VkFormat actual_format;

        void set_format(int flag);
    };

    struct TexturePrim {
        VkDevice _device;
        VkPhysicalDevice _physical_device;

        VkImage _image;
        VkDeviceMemory _memory;
        VkImageView _view;

        VkSampler _sampler;

        VkFormat _format;
        uint32_t _width, _height;

        VkImageLayout _layout;

        uint64_t _update_id;
        uint64_t _last_frame;

        void create_image(VkDevice device, VkPhysicalDevice physical_device,
                          uint32_t width, uint32_t height, RealTexFormat format);

        void create_image_view();

        void upload_data(VkCommandBuffer buffer,
                         const void* data,
                         VkDeviceSize size,
                         VkDeviceSize offset);

        void create_sampler(VkPhysicalDevice physical_device);

        void clean();

        TexturePrim():
            _device(VK_NULL_HANDLE),
            _physical_device(VK_NULL_HANDLE),
            _image(VK_NULL_HANDLE),
            _memory(VK_NULL_HANDLE),
            _view(VK_NULL_HANDLE),
            _width(0),
            _height(0),
            _format(VK_FORMAT_UNDEFINED),
            _layout(VK_IMAGE_LAYOUT_UNDEFINED),
            _update_id(0),
            _last_frame(0)
        {}

        ~TexturePrim() {
            clean();
        }

        TexturePrim(const TexturePrim&) = delete;
        TexturePrim& operator=(const TexturePrim&) = delete;

        TexturePrim(TexturePrim&& other) noexcept;
        TexturePrim& operator=(TexturePrim&& other) noexcept;
    };

    struct vert_data {
        size_t buff_size = 0;
        Buffer vert_buffer;

        size_t index_size = 0;
        Buffer index_buffer;

        size_t update_id = 0;
        size_t vertices = 0;

        size_t _last_frame = 0;
    };

    struct Uniform {
        uint32_t set = 0;
        uint32_t binding = 0;
        size_t block_size = 0;

        UniformType type = kUniformFloat;        

        bool isTexture = false;

        size_t offset = 0;
        size_t size = 0;

        size_t frame = 0;
    };

    struct shader_stage {
        VkShaderModule module = VK_NULL_HANDLE;
        VkShaderStageFlagBits stage = VK_SHADER_STAGE_FLAG_BITS_MAX_ENUM;
    };

    struct shader_data {
        std::vector<shader_stage> stages;

        std::map<StringIntern, Uniform> uniforms;
        std::map<StringIntern, uint32_t> attribs;

        size_t _last_frame = 0;
    };

    enum class CmdState {
        Idle,
        Recording,
    };

    enum class RenderState {
        Rendering,
        NotRendering
    };

    Instance* _instance;
    Device* _device;
    std::unordered_map<Required_Pipeline_State, Pipeline, Required_Pipeline_State_Hash> _pipelines;
    CommandPool* _command_pool;
    VkCommandBuffer _cmd_buffers[MAX_FRAMES_IN_FLIGHT];
    CmdState cmdstate = CmdState::Idle;
    RenderState renderstate = RenderState::NotRendering;
    RenderPass* _render_pass;
    Synchronization* _sync;
    size_t _current_frame = 0; // 0..MAX_FRAMES_IN_FLIGHT-1
    uint32_t _image_index = 0; // 0.._device->swapchain_image_views.size()

    Buffer uniform_buffers[MAX_FRAMES_IN_FLIGHT];

    Required_Pipeline_State state;

    shader_data * current_shader;

    std::map<std::shared_ptr<VertexArray>, vert_data> vertex_arrays;
    std::map<VirtualResource, TexturePrim> textures;
    std::map<VirtualResource, shader_data> shaders;

    void ensure_recording();
    void ensure_idle();

    uint32_t acquire_next_image();

    static uint32_t find_mem_type(VkPhysicalDevice physical_device, uint32_t type_filter,
                           VkMemoryPropertyFlags properties);

    template<typename map_strintern_T>
    inline bool upload_uniform_data_piece(const map_strintern_T& uniform_piece,
                                          Uniform& uniform, StringIntern name) {
        auto it = uniform_piece.find(name);

        if(it == uniform_piece.end() || it->second.empty())return false;

        auto& buffer = uniform_buffers[_current_frame];

        uint64_t key = Buffer::uniform_binding_key(uniform.set, uniform.binding);

        auto offset_it = buffer._current_uniform_offsets.find(key);

        if(offset_it == buffer._current_uniform_offsets.end()) {
            VkDeviceSize block_size = uniform.block_size;
            VkDeviceSize block_stride = (block_size + buffer._dynamic_alignment - 1) & ~(buffer._dynamic_alignment -1);

            VkDeviceSize block_offset = buffer._current_uniform_buffer;
            buffer._current_uniform_buffer += block_stride;

            offset_it = buffer._current_uniform_offsets.emplace(key,block_offset).first;
        }

        VkDeviceSize block_offset = offset_it->second;

        Buffer::update_dynamic_uniform_buffer(buffer, block_offset, uniform.offset,
                                              &it->second[0], it->second.size());

        return true;
    }
    
    /* DOESN'T WORK WITH MATRIX TYPES
    template<typename T>
    void append_ints(const T& value, std::vector<int32_t>& out) {
        if constexpr (std::is_arithmetic_v<T>) {
            out.push_back(static_cast<int32_t>(value));
        } else {
            for (const auto& x : value) {
                append_ints(x, out);
            }
        }
    }

    template<typename map_strintern_T>
    inline void upload_uniform_data_piece_int(const map_strintern_T& uniform_piece, 
                                              uint32_t offset, StringIntern str, int x) {
        auto it = uniform_piece.find(str);
        if(it != uniform_piece.end() && !it->second.empty()) {
            std::vector<int32_t> vec;
            vec.reserve(it->second.size());
            for(auto i : it->second) {
                append_ints(it->second, vec);
            }

            auto& val = vec[0];

            Buffer::update_dynamic_uniform_buffer(
                uniform_buffer,
                _device->_physical_device,
                x,
                offset,
                &val,
                it->second.size()
            );
        }
    }
    */

    struct ShaderBinary {
        uint32_t* data = nullptr;
        size_t word_count = 0;

        ~ShaderBinary() {
            if(data!=nullptr) {
                free(data);
            }

            data = nullptr;
            word_count = 0;
        }
    };

    struct shader_stage_info {
        std::string_view define;
        shaderc_shader_kind shaderc_kind;
        VkShaderStageFlagBits vk_stage;
    };

    static const std::map<std::string_view, shader_stage_info> shader_stage_map;

    void make_shader(VirtualResource& shader_path);
    ShaderBinary compile_shader(const std::string& source, shaderc_shader_kind kind,
                                const std::string& define, const std::string& filename);
    bool validate_shader(const ShaderBinary& shader_code);
    VkShaderModule create_shader_module(const ShaderBinary& shader_code);
    std::vector<shader_stage_info> parse_shader_stages(std::string& source);
    void reflect_shader(ShaderBinary& shader_code, VkShaderStageFlagBits stage, shader_data& data);
    void reflect_uniform_block(const SpvReflectTypeDescription* type_description, 
                               const SpvReflectDescriptorBinding* binding,
                               shader_data& data, size_t size, size_t offset, const char* name);
    void unwrap_reflected_arrays(const SpvReflectTypeDescription* type_description,
                                 const SpvReflectDescriptorBinding* binding,
                                 shader_data& data,
                                 uint32_t dimension,
                                 uint32_t size,
                                 uint32_t offset,
                                 const std::string& name);

    VkDescriptorSetLayout create_desc_layout(shader_data* data, uint32_t set);

    

public:
    Vulkan() {} 
    ~Vulkan() {
        clean();
    }

    void init_vulkan(std::vector<const char*>& required_extensions, SDL_Window* window,
                     uint32_t width, uint32_t height, VkSampleCountFlagBits msaa_samples = VK_SAMPLE_COUNT_1_BIT,
                     uint32_t max_frames_in_flight = MAX_FRAMES_IN_FLIGHT,
                     bool allow_command_pool_reset = true, bool command_pool_transient = false);

    void begin_frame();
    void end_frame();
    void clean();

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
    void pre_draw();
    void post_draw();
    void clear(bool color, bool depth, bool stencil);
    void cache_texture(VirtualResource tex);
    void cache_array(std::shared_ptr<VertexArray> array);
    void cache_material(Material& state, const VirtualResource& shader);
};
}
#endif
