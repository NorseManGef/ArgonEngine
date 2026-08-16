#include "Vulkan.h"
#include <cstdint>
#include <ranges>
#include <algorithm>
#include "ArgonEngine/ArgonInit.h"
#include "ArgonEngine/RenderSystemConstants.h"
#include "plog/Severity.h"
#include "vulkan/vulkan_core.h"
#ifdef USE_VULKAN
#include "VirtualResourceImage.h"

namespace Argon {

  ///////////////////////////////////////
 // INSTANCE ///////////////////////////
///////////////////////////////////////
void Vulkan::Instance::create_instance(const std::vector<const char*>& required_extensions) {
    if(_validation)
        check_validation_support();

    VkApplicationInfo appInfo{.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
                              .pApplicationName = "Argon Test",
                              .applicationVersion = VK_MAKE_VERSION(1,0,0),
                              .pEngineName = "Argon Engine",
                              .engineVersion = VK_MAKE_VERSION(1, 0, 0),
                              .apiVersion = VK_API_VERSION_1_0};

    VkInstanceCreateInfo createInfo{
        .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
        .pApplicationInfo = &appInfo
    };

    std::vector<const char*> extensions = required_extensions;

    #ifdef APPLE
        createInfo.flags = VkInstanceCreateFlagBits::VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
        extensions.push_back(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
    #endif

    if(_validation) {
        extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }

    createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
    createInfo.ppEnabledExtensionNames = extensions.data();

    VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
    if (_validation) {
        createInfo.enabledLayerCount = static_cast<uint32_t>(_validation_layers.size());
        createInfo.ppEnabledLayerNames = _validation_layers.data();

        // Set up debug messenger for instance creation/destruction messages
        create_debug_messenger(debugCreateInfo);
        createInfo.pNext = &debugCreateInfo;
    } else {
        createInfo.enabledLayerCount = 0;
        createInfo.pNext = nullptr;
    }

    VkResult result = vkCreateInstance(&createInfo, nullptr, &_instance);
    if(result != VK_SUCCESS) {
        PLOGF << "Failed to create vulkan instance";
        terminate_engine();
    }
}

void Vulkan::Instance::check_validation_support() {
    uint32_t layer_count;
    vkEnumerateInstanceLayerProperties(&layer_count, nullptr);
    std::vector<VkLayerProperties> layer_properties(layer_count);
    vkEnumerateInstanceLayerProperties(&layer_count, layer_properties.data());
    auto unsupported_layers = std::ranges::find_if(
        _validation_layers,
        [&layer_properties](auto const &validation_layer) {
            return std::ranges::none_of(
                layer_properties,
                [validation_layer](auto const &layer_property) {
                    return strcmp(layer_property.layerName, validation_layer) == 0;
                }
            );
        }
    );

    if(unsupported_layers != _validation_layers.end()) {
        PLOGF << "Validation layers were requested but not available";
        terminate_engine();
    }
}

void Vulkan::Instance::create_debug_messenger(VkDebugUtilsMessengerCreateInfoEXT& createInfo) {
    createInfo = {
        .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
        .messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
                           VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT |
                           VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                           VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT,
        .messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                       VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                       VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT,
        .pfnUserCallback = debugCallback,
        .pUserData = nullptr,
    };
    auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
        _instance, "vkCreateDebugUtilsMessengerEXT"
    );

    if(func != nullptr) {
        VkResult result = func(_instance, &createInfo, nullptr, &debug_messenger);
        if(result != VK_SUCCESS) {
            PLOGF << "Failed to set up debug messenger";
            terminate_engine();
        }
    } else {
        PLOGF << "Failed to load vkCreateDebugUtilsMessengerEXT function";
        terminate_engine();
    }
}

VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT severity,
                                             VkDebugUtilsMessageTypeFlagsEXT type,
                                             const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
                                             void* pUserData) {
    plog::Severity plogsev;
    switch(severity) {
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
            plogsev = plog::verbose;
            break;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
            plogsev = plog::info;
            break;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
            plogsev = plog::warning;
            break;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
            plogsev = plog::error;
            break;
        default:
            plogsev = plog::fatal;
            break;
    }

    std::string sType;
    switch(type) {
        case VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT:
            sType += "VULKAN GENERAL MESSAGE: ";
            break;
        case VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT:
            sType += "VULKAN VALIDATION MESSAGE: ";
            break;
        case VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT:
            sType += "VULKAN PERFORMANCE MESSAGE: ";
            break;
        default:
            sType += "VULKAN MESSAGE: ";
            break;
    }

    PLOG(plogsev) << sType << pCallbackData->pMessage;

    return VK_FALSE;
}

void Vulkan::Instance::clean() {
    if(debug_messenger != VK_NULL_HANDLE) {
        auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
            _instance, "vkDestroyDebugUtilsMessengerEXT"
        );
        if(func!=nullptr) {
            func(_instance, debug_messenger, nullptr);
        }
    }

    if(_instance != VK_NULL_HANDLE) {
        vkDestroyInstance(_instance, nullptr);
        _instance = VK_NULL_HANDLE;
    }
}

  ///////////////////////////////////////
 // DEVICE /////////////////////////////
///////////////////////////////////////
void Vulkan::Device::pick_physical_device(VkInstance instance, VkSurfaceKHR surface) {
    uint32_t device_count = 0;
    vkEnumeratePhysicalDevices(instance, &device_count, nullptr);

    if(device_count == 0) {
        PLOGF << "No available GPU(s) found";
        terminate_engine();
    }

    PLOGV << "Vulkan found: " << device_count << " GPU(s)";

    std::vector<VkPhysicalDevice> devices(device_count);
    vkEnumeratePhysicalDevices(instance, &device_count, devices.data());

    std::multimap<uint32_t, VkPhysicalDevice> candidates;

    for(const auto& device : devices) {
        uint32_t score = score_physical_device(device, surface);
        candidates.insert(std::make_pair(score, device));
    }

    if(candidates.rbegin()->first > 0) {
        _physical_device = candidates.rbegin()->second;
    } else {
        PLOGF << "Failed to find a suitable GPU; No devices met the minimum requirements";
        terminate_engine();
    }

}

void Vulkan::Device::create_logical_device(VkSurfaceKHR surface) {
    _queue_families = find_queue_families(_physical_device, surface);

    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
    std::set<uint32_t> unique_queue_families = _queue_families.unique_queue_families();

    float queue_priority = 1.0f; //TODO: determine if this should be settable by the user or developer

    for(uint32_t queue_family : unique_queue_families) {
        VkDeviceQueueCreateInfo queueCreateInfo{
            .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
            .queueFamilyIndex = queue_family,
            .queueCount = 1, //TODO: determine if this is the best best value
            .pQueuePriorities = &queue_priority
        };
        queueCreateInfos.push_back(queueCreateInfo);
    }

    VkPhysicalDeviceFeatures device_features{
        //TODO: create a way for devs to set required features for their applications and come up with a sane default
    };

    VkDeviceCreateInfo createInfo{
        .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
        .queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size()),
        .pQueueCreateInfos = queueCreateInfos.data(),
        .enabledExtensionCount = static_cast<uint32_t>(_extensions.size()),
        .ppEnabledExtensionNames = _extensions.data(),
        .pEnabledFeatures = &device_features,
    };

    VkResult result = vkCreateDevice(_physical_device, &createInfo, nullptr, &_device);
    if(result != VK_SUCCESS) {
        PLOGF << "Vulkan: Failed to create logical device!";
        terminate_engine();
    }
}

void Vulkan::Device::create_swap_chain(VkSurfaceKHR surface, uint32_t width, uint32_t height) {
    if(!_swapchain_support.queried)
        _swapchain_support = query_swapchain_details(_physical_device, surface);

    if(!_swapchain_support.is_adequate()) {
        PLOGF << "Vulkan: Swapchain support is inadequate for this device";
        terminate_engine();
    }

    VkSurfaceFormat2KHR surface_format = choose_surface_format(_swapchain_support.formats);
    VkPresentModeKHR present_mode = choose_present_mode(_swapchain_support.presentModes);
    VkExtent2D extent = choose_swap_extent(_swapchain_support.capabilities, width, height);
    uint32_t image_count = choose_image_count(_swapchain_support.capabilities);

    _image_format = surface_format.surfaceFormat.format;
    _extent = extent;

    VkSwapchainCreateInfoKHR createInfo{
        .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
        .surface = surface,
        .minImageCount = image_count,
        .imageColorSpace = surface_format.surfaceFormat.colorSpace,
        .imageExtent = extent,
        .imageArrayLayers = 1, //TODO: come up with a sane method of letting a user control this value
                               // as it is required for stereoscopic 3d applications
        .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
    };

    uint32_t queue_array[] = {
        _queue_families.graphics.value(),
        _queue_families.present.value(),
    };

    if(_queue_families.graphics != _queue_families.present) {
        createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        createInfo.queueFamilyIndexCount = 2;
        createInfo.pQueueFamilyIndices = queue_array;
    } else {
        createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    }

    createInfo.preTransform = _swapchain_support.capabilities.surfaceCapabilities.currentTransform; // No transform
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR; // Ignore alpha channel

    createInfo.presentMode = present_mode;
    createInfo.clipped = VK_TRUE;

    createInfo.oldSwapchain = VK_NULL_HANDLE;
    
    {
        VkResult result = vkCreateSwapchainKHR(_device, &createInfo, nullptr, &_swapchain);

        if(result != VK_SUCCESS) {
            PLOGF << "Vulkan: Failed to create swapchain";
            terminate_engine();
        }
    }
    uint32_t actual_image_count;
    {
        VkResult result = vkGetSwapchainImagesKHR(_device, _swapchain, &actual_image_count, nullptr);

        if(result != VK_SUCCESS) {
            PLOGF << "Vulkan: Failed to get swapchain image count";
            terminate_engine();
        }
    }
    _images.resize(actual_image_count);
    {
        VkResult result = vkGetSwapchainImagesKHR(_device, _swapchain, &actual_image_count, _images.data());    
    }

    create_image_views();
}

void Vulkan::Device::create_image_views() {
    _image_views.resize(_images.size());

    for(size_t i = 0; i < _images.size(); ++i) {
        VkImageViewCreateInfo createInfo{
            .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
            .image = _images[i],
            .viewType = VK_IMAGE_VIEW_TYPE_2D, //TODO: determine if the needs to be settable by the user
            .format = _image_format,
            .components.r = VK_COMPONENT_SWIZZLE_IDENTITY,
            .components.g = VK_COMPONENT_SWIZZLE_IDENTITY,
            .components.b = VK_COMPONENT_SWIZZLE_IDENTITY,
            .components.a = VK_COMPONENT_SWIZZLE_IDENTITY,
            .subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT, // color data
            .subresourceRange.baseMipLevel = 0,
            .subresourceRange.levelCount = 1,
            .subresourceRange.baseArrayLayer = 0,
            .subresourceRange.layerCount = 1,
        };

        VkResult result = vkCreateImageView(_device, &createInfo, nullptr, &_image_views[i]);

        if(result != VK_SUCCESS) {
            PLOGE << "Vulkan: Failed to create image view " + std::to_string(i);
        }
    }
}

void Vulkan::Device::destroy_image_views() {
    for(auto image_view : _image_views) {
        if(image_view != VK_NULL_HANDLE) {
            vkDestroyImageView(_device, image_view, nullptr);
        }
    }
    _image_views.clear();
}

uint32_t Vulkan::Device::score_physical_device(VkPhysicalDevice device, VkSurfaceKHR surface) {
    Queue_family_indices indices = find_queue_families(device, surface);
    if(!_swapchain_support.queried)
        _swapchain_support = query_swapchain_details(device, surface);

    if(!(indices.is_complete() && check_device_extensions(device) && _swapchain_support.is_adequate())) {
        return 0;
    }

    VkPhysicalDeviceProperties2 props;
    VkPhysicalDeviceFeatures2 feats;
    vkGetPhysicalDeviceProperties2(device, &props);
    vkGetPhysicalDeviceFeatures2(device, &feats);

    uint32_t score = 0;

    if(props.properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
        score += 1000;
    } else  if(props.properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU) {
        score += 500;
    }

    score += props.properties.limits.maxImageDimension2D / 1000;

    if(feats.features.geometryShader) score += 100;

    if(feats.features.tessellationShader) score += 50;

    if(feats.features.samplerAnisotropy) score += 25;

    VkPhysicalDeviceMemoryProperties2 memprops;
    vkGetPhysicalDeviceMemoryProperties2(device, &memprops);

    uint64_t total_memory = 0;
    for(int i = 0; i < memprops.memoryProperties.memoryHeapCount; ++i) {
        if(memprops.memoryProperties.memoryHeaps[i].flags & VK_MEMORY_HEAP_DEVICE_LOCAL_BIT) {
            total_memory += memprops.memoryProperties.memoryHeaps[i].size;
        }
    }

    uint32_t memGB = static_cast<uint32_t>(total_memory / (1024*1024*1024));
    score += memGB*10;

    return score;
}

Vulkan::Device::Queue_family_indices Vulkan::Device::find_queue_families(VkPhysicalDevice device, VkSurfaceKHR surface) {
    Queue_family_indices indices;

    uint32_t count;
    vkGetPhysicalDeviceQueueFamilyProperties2(device, &count, nullptr);

    std::vector<VkQueueFamilyProperties2> queue_families(count);
    vkGetPhysicalDeviceQueueFamilyProperties2(device, &count, queue_families.data());

    for(int i = 0; i < queue_families.size(); ++i) {
        const auto family = queue_families[i];

        if(family.queueFamilyProperties.queueFlags & VK_QUEUE_GRAPHICS_BIT) indices.graphics = i;
        if(family.queueFamilyProperties.queueFlags & VK_QUEUE_COMPUTE_BIT) indices.compute = i;
        if(family.queueFamilyProperties.queueFlags & VK_QUEUE_TRANSFER_BIT) indices.transfer = i;

        VkBool32 present_support = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &present_support);
        if(present_support) indices.present = i;

        if(indices.is_complete()) break; //TODO: Evaluate if compute or transfer queues could ever be needed
                                         // and might require either the removal of this line or a way to specify the
                                         // queues needed for a game
    }

    return indices;
}

bool Vulkan::Device::check_device_extensions(VkPhysicalDevice device) {
    uint32_t count;
    vkEnumerateDeviceExtensionProperties(device, nullptr, &count, nullptr);

    std::vector<VkExtensionProperties> available_extensions(count);
    vkEnumerateDeviceExtensionProperties(device, nullptr, &count, available_extensions.data());

    std::set<std::string> required_extensions(_extensions.begin(), _extensions.end());

    for(const auto& extension : available_extensions) {
        required_extensions.erase(extension.extensionName);
    }

    return required_extensions.empty();
}

Vulkan::Device::Swapchain_details Vulkan::Device::query_swapchain_details(VkPhysicalDevice device, VkSurfaceKHR surface) const {
    Swapchain_details details;

    VkPhysicalDeviceSurfaceInfo2KHR surface2 {
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SURFACE_INFO_2_KHR,
        .pNext = nullptr,
        .surface = surface
    };

    vkGetPhysicalDeviceSurfaceCapabilities2KHR(device, &surface2, &details.capabilities);

    uint32_t format_count;
    vkGetPhysicalDeviceSurfaceFormats2KHR(device, &surface2, &format_count, details.formats.data());

    if(format_count != 0) {
        details.formats.resize(format_count);
        vkGetPhysicalDeviceSurfaceFormats2KHR(device, &surface2, &format_count, details.formats.data());
    }

    uint32_t present_mode_count;
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &present_mode_count, nullptr);

    if(present_mode_count != 0) {
        details.presentModes.resize(present_mode_count);
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &present_mode_count, details.presentModes.data());
    }

    details.queried = true;

    return details;
}

void Vulkan::Device::retrieve_queue_handles() {
    vkGetDeviceQueue(_device, _queue_families.graphics.value(), 0, &_graphicsQ);
    vkGetDeviceQueue(_device, _queue_families.present.value(), 0, &_presentQ);
    vkGetDeviceQueue(_device, _queue_families.compute.value(), 0, &_computeQ);
    vkGetDeviceQueue(_device, _queue_families.transfer.value(), 0, &_transferQ);
}

VkSurfaceFormat2KHR choose_surface_format(const std::vector<VkSurfaceFormat2KHR>& available_formats) {
    //TODO: HDR support with sane user-settable flag
    for(const auto& a : available_formats) {
        if(a.surfaceFormat.format == VK_FORMAT_B8G8R8A8_SRGB &&
           a.surfaceFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
                return a;
    }

    for(const auto& a : available_formats) {
        if(a.surfaceFormat.format == VK_FORMAT_R8G8B8_SRGB &&
           a.surfaceFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
                return a;
    }

    PLOGW << "Vulkan: surface format unknown: " << available_formats[0].surfaceFormat.format
          << " with color space: " << available_formats[0].surfaceFormat.colorSpace;
    return available_formats[0];
}

VkPresentModeKHR Vulkan::Device::choose_present_mode(const std::vector<VkPresentModeKHR>& available_present_modes) {
    //TODO: Allow this to be user-settable

    //Triple buffering
    for(const auto& a : available_present_modes) {
        if(a == VK_PRESENT_MODE_MAILBOX_KHR) return a;
    }

    //Vsync off
    for(const auto& a : available_present_modes) {
        if(a == VK_PRESENT_MODE_IMMEDIATE_KHR) return a;
    }

    // Fow now we use regular vsync as a fallback as it is guaranteed to be supported.
    return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D Vulkan::Device::choose_swap_extent(const VkSurfaceCapabilities2KHR& capabilities, uint32_t width, uint32_t height) {
    if(capabilities.surfaceCapabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
        return capabilities.surfaceCapabilities.currentExtent;
    VkExtent2D actual_extent = {width, height};
    actual_extent.width = std::clamp(actual_extent.width,
                                     capabilities.surfaceCapabilities.minImageExtent.width,
                                     capabilities.surfaceCapabilities.maxImageExtent.width);
    actual_extent.height = std::clamp(actual_extent.height,
                                      capabilities.surfaceCapabilities.minImageExtent.height,
                                      capabilities.surfaceCapabilities.maxImageExtent.height);
    return actual_extent;
}

uint32_t Vulkan::Device::choose_image_count(const VkSurfaceCapabilities2KHR& capabilities) {
    uint32_t image_count = capabilities.surfaceCapabilities.minImageCount + 1;
    uint32_t max_count = capabilities.surfaceCapabilities.maxImageCount;

    // Make sure image_count doesn't exceed the maximum (0 means no limit)
    if(max_count > 0 && image_count > max_count)
        image_count = max_count;

    return image_count;
}

void Vulkan::Device::clean() {
    clean_swapchain();

    if(_device != VK_NULL_HANDLE) {
        vkDeviceWaitIdle(_device);
        vkDestroyDevice(_device, nullptr);
        _device = nullptr;
        _graphicsQ = _presentQ = _computeQ = _transferQ = nullptr;
    }

    _physical_device = nullptr; //doesn't need to be destroyed
}

void Vulkan::Device::clean_swapchain() {
    if(_device != VK_NULL_HANDLE) {
        destroy_image_views();

        if(_swapchain != VK_NULL_HANDLE) {
            vkDestroySwapchainKHR(_device, _swapchain, nullptr);
            _swapchain = VK_NULL_HANDLE;
        }
    }

    _images.clear();

    _image_format = VK_FORMAT_UNDEFINED;
    _extent = {0,0};
}

  ///////////////////////////////////////
 // PIPELINE ///////////////////////////
///////////////////////////////////////
void Vulkan::Pipeline::create_graphics_pipeline(VkDevice device, VkRenderPass render_pass,
                                                VirtualResource vertex_shader_path, 
                                                VirtualResource fragment_shader_path,
                                                VkExtent2D extent) {
    if(device == VK_NULL_HANDLE) {
        PLOGF << "Vulkan: Invalid device handle provided to graphics pipeline";
        terminate_engine();
    }
    if(render_pass == VK_NULL_HANDLE) {
        PLOGF << "Vulkan: Invalid render pass handle provided to graphics pipeline";
        terminate_engine();
    }
    if(extent.width == 0 || extent.height == 0) {
        PLOGF << "Vulkan: Invalid extent provided to graphics pipeline";
    }

    _device = device;

    VkShaderModule vertex_shader = load_shader(vertex_shader_path);
    VkShaderModule fragment_shader = load_shader(fragment_shader_path);

    VkPipelineShaderStageCreateInfo vertex_shader_stageInfo{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
        .stage = VK_SHADER_STAGE_VERTEX_BIT,
        .module = vertex_shader,
        .pName = "main", // entry point into the shader
    };

    VkPipelineShaderStageCreateInfo fragment_shader_stageInfo{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
        .stage = VK_SHADER_STAGE_FRAGMENT_BIT,
        .module = fragment_shader,
        .pName = "main",
    };

    VkPipelineShaderStageCreateInfo shader_stages[] = {vertex_shader_stageInfo, fragment_shader_stageInfo};

    create_desc_layout();
    create_pipeline_layout();

    auto vertex_input_info = create_vertex_input_info();
    auto input_assembly_info = create_input_assembly_info();
    auto viewport_info = create_viewport_info(extent);
    auto rasterization_info = create_rasterization_info();
    auto multisample_info = create_multisample_info();
    auto depth_stencil_info = create_depth_stencil_info();
    auto color_blend_info = create_color_blend_info();

    VkGraphicsPipelineCreateInfo pipeline_createInfo{
        .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
        .stageCount = 2, //vertex+fragment stages. TODO: Determine if other stages might be needed and allow users to set those stages if so.
        .pStages = shader_stages,
        .pVertexInputState = &vertex_input_info,
        .pInputAssemblyState = &input_assembly_info,
        .pViewportState = &viewport_info,
        .pRasterizationState = &rasterization_info,
        .pMultisampleState = &multisample_info,
        .pDepthStencilState = &depth_stencil_info,
        .pColorBlendState = &color_blend_info,
        .pDynamicState = nullptr, //TODO: Determine if dynamic state is needed
        .layout = _pipeline_layout,
        .renderPass = render_pass,
        .subpass = 0, //TODO: Determine if this should be user-settable
        .basePipelineHandle = VK_NULL_HANDLE, //TODO: Implement base pipelines if feasible for better performance
        .basePipelineIndex = -1,
    };

    VkResult result = vkCreateGraphicsPipelines(device,
                                                VK_NULL_HANDLE,
                                                1,
                                                &pipeline_createInfo,
                                                VK_NULL_HANDLE,
                                                &_pipeline);

    if(result != VK_SUCCESS) {
        PLOGF << "Vulkan: Failed to create graphics pipeline";
        terminate_engine();
    }

    vkDestroyShaderModule(device, vertex_shader, nullptr);
    vkDestroyShaderModule(device, fragment_shader, nullptr);
}

VkShaderModule Vulkan::Pipeline::create_shader_module(const std::string shader_code) {
    VkShaderModuleCreateInfo createInfo{
        .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
        .codeSize = shader_code.size(),
        .pCode = reinterpret_cast<const uint32_t*>(shader_code.c_str()),
    };

    VkShaderModule module;
    VkResult result = vkCreateShaderModule(_device, &createInfo, nullptr, &module);
    if(result != VK_SUCCESS) {
        PLOGF << "Vulkan: Failed to create shader module";
        terminate_engine();
    }

    return module;
}

VkShaderModule Vulkan::Pipeline::load_shader(VirtualResource shader_path) {
    std::string code = shader_path.get_data_as_string();

    return create_shader_module(code);
}

void Vulkan::Pipeline::create_desc_layout() {
    VkDescriptorSetLayoutBinding ubo_layout_binding{
        .binding = 0,
        .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
        .descriptorCount = 1,
        .stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
        .pImmutableSamplers = nullptr, //TODO: Determine if we have texture sampling and need to use this value
    };

    VkDescriptorSetLayoutCreateInfo layout_info{
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
        .bindingCount = 1,
        .pBindings = &ubo_layout_binding,
    };

    VkResult result = vkCreateDescriptorSetLayout(_device,
                                                  &layout_info,
                                                  nullptr,
                                                  &_desc_layout);

    if(result != VK_SUCCESS) {
        PLOGF << "Vulkan: Failed to create descriptor set layout";
        terminate_engine();
    }
}

void Vulkan::Pipeline::create_pipeline_layout() {
    VkPipelineLayoutCreateInfo pipeline_layout_info{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
        .setLayoutCount = 1,
        .pSetLayouts = &_desc_layout,
        .pushConstantRangeCount = 0, //TODO: determine if this is optimal
        .pPushConstantRanges = nullptr,
    };

    VkResult result = vkCreatePipelineLayout(_device,
                                             &pipeline_layout_info,
                                             nullptr,
                                             &_pipeline_layout);

    if(result != VK_SUCCESS) {
        PLOGF << "Vulkan: Failed to create pipeline layout";
        terminate_engine();
    }
}

VkPipelineVertexInputStateCreateInfo Vulkan::Pipeline::create_vertex_input_info() {
    VkVertexInputBindingDescription binding_desc{
        .binding = 0,
        .stride = static_cast<uint32_t>(_vertex_array->stride),
        .inputRate = VK_VERTEX_INPUT_RATE_VERTEX,
    };

    std::vector<VkVertexInputAttributeDescription> attribs;
    attribs.reserve(_vertex_array->attributes.size());

    auto attrib_location = [&](VertexAttribPair pair)-> uint32_t {
        for(size_t x = 0; x<_vertex_array->attributes.size(); x++) {
            if(_vertex_array->attributes[x].attribute == pair.attribute) {
                return x;
            }
        }
        return UINT32_MAX;
    };

    auto render_format = [](int type, int components)->VkFormat {
        switch(type) {
            case kRenderTypeFloat:
                switch(components) {
                    case 1: return VK_FORMAT_R32_SFLOAT;
                    case 2: return VK_FORMAT_R32G32_SFLOAT;
                    case 3: return VK_FORMAT_R32G32B32_SFLOAT;
                    case 4: return VK_FORMAT_R32G32B32A32_SFLOAT;
                }
            break;
            case kRenderTypeByte:
                switch (components) {
                    case 1: return VK_FORMAT_R8_SINT;
                    case 2: return VK_FORMAT_R8G8_SINT;
                    case 4: return VK_FORMAT_R8G8B8A8_SINT;
                }
            break;
            case kRenderTypeUByte:
                switch (components) {
                    case 1: return VK_FORMAT_R8_UINT;
                    case 2: return VK_FORMAT_R8G8_UINT;
                    case 4: return VK_FORMAT_R8G8B8A8_UINT;
                }
            break;
            case kRenderTypeShort:
                switch (components) {
                    case 1: return VK_FORMAT_R16_SINT;
                    case 2: return VK_FORMAT_R16G16_SINT;
                    case 4: return VK_FORMAT_R16G16B16A16_SINT;
                }
            break;
        }
        PLOGE << "Vulkan: Unsupported vertex attribute format";
        return VK_FORMAT_UNDEFINED;
    };

    for(const auto& a : _vertex_array->attributes) {
        VkVertexInputAttributeDescription attrib{
            .location = attrib_location(a),
            .binding = 0,
            .format = render_format(a.type, a.components),
            .offset = static_cast<uint32_t>(a.offset),
        };
        attribs.push_back(attrib);
    }

    VkPipelineVertexInputStateCreateInfo vertex_input_info{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
        .vertexBindingDescriptionCount = 1,
        .pVertexBindingDescriptions = &binding_desc,
        .vertexAttributeDescriptionCount = static_cast<uint32_t>(attribs.size()),
        .pVertexAttributeDescriptions = attribs.data(),
    };

    return vertex_input_info;
}

VkPipelineInputAssemblyStateCreateInfo Vulkan::Pipeline::create_input_assembly_info() {
    VkPipelineInputAssemblyStateCreateInfo input_assembly_info{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
        .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
        .primitiveRestartEnable = VK_FALSE,
    };

    return input_assembly_info;
}

VkPipelineViewportStateCreateInfo Vulkan::Pipeline::create_viewport_info(VkExtent2D extent) {
    _viewport = {
        .x = 0.0f,
        .y = 0.0f,
        .width = static_cast<float>(extent.width),
        .height = static_cast<float>(extent.height),
        .minDepth = 0.0f,
        .maxDepth = 1.0f,
    };
    
    _scissor.offset = {0,0};
    _scissor.extent = extent;

    VkPipelineViewportStateCreateInfo viewport_info {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
        .viewportCount = 1,
        .pViewports = &_viewport,
        .scissorCount = 1,
        .pScissors = &_scissor,
    };

    return viewport_info;
}

VkPipelineRasterizationStateCreateInfo Vulkan::Pipeline::create_rasterization_info() {
    VkPipelineRasterizationStateCreateInfo rasterization_info {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
        .depthClampEnable = VK_FALSE,
        .rasterizerDiscardEnable = VK_FALSE,
        .polygonMode = VK_POLYGON_MODE_FILL,
        .cullMode = VK_CULL_MODE_BACK_BIT,
        .frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE,
        .depthBiasEnable = VK_FALSE, //TODO: detemine if we need depth bias
        .depthBiasConstantFactor = 0.0f,
        .depthBiasClamp = 0.0f,
        .depthBiasSlopeFactor = 0.0f,
        .lineWidth = 1.0f,
    };

    return rasterization_info;
}

VkPipelineMultisampleStateCreateInfo Vulkan::Pipeline::create_multisample_info() {
    VkPipelineMultisampleStateCreateInfo multisample_info{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
        //TODO: make this user-settable
        .rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
        .sampleShadingEnable = VK_FALSE,
        .minSampleShading = 1.0f,
        .pSampleMask = VK_NULL_HANDLE,
        .alphaToCoverageEnable = VK_FALSE,
        .alphaToOneEnable = VK_FALSE,
    };
    return multisample_info;
}

VkPipelineColorBlendStateCreateInfo Vulkan::Pipeline::create_color_blend_info() {
    _color_blend_attachment = {
        //TODO: make this user-settable
        .blendEnable = VK_FALSE,
        .srcColorBlendFactor = VK_BLEND_FACTOR_ONE,
        .dstColorBlendFactor = VK_BLEND_FACTOR_ZERO,
        .colorBlendOp = VK_BLEND_OP_ADD,
        .srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE,
        .dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO,
        .alphaBlendOp = VK_BLEND_OP_ADD,
        .colorWriteMask = VK_COLOR_COMPONENT_R_BIT |
                          VK_COLOR_COMPONENT_G_BIT |
                          VK_COLOR_COMPONENT_B_BIT |
                          VK_COLOR_COMPONENT_A_BIT,
    };

    VkPipelineColorBlendStateCreateInfo color_blend_info{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
        .logicOpEnable = VK_FALSE, //TODO: determine if this should be enabled
        .logicOp = VK_LOGIC_OP_COPY,
        .attachmentCount = 1,
        .pAttachments = &_color_blend_attachment,
        .blendConstants = {0.0f, 0.0f, 0.0f, 0.0f},
    };
    return color_blend_info;
}

VkPipelineDepthStencilStateCreateInfo Vulkan::Pipeline::create_depth_stencil_info() {
    VkPipelineDepthStencilStateCreateInfo depth_stencil_info{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
        .depthTestEnable = VK_TRUE,
        .depthWriteEnable = VK_TRUE, //needed for depthTest
        .depthCompareOp = VK_COMPARE_OP_LESS,
        .depthBoundsTestEnable = VK_TRUE,
        .stencilTestEnable = VK_TRUE, //TODO: determine if this should be implemented
        .front = {}, //TODO: make this user-settable
        .back = {}, //TODO: make this user-settable
        .minDepthBounds = 0.0f,
        .maxDepthBounds = 1.0f,
    };

    return depth_stencil_info;
}

  ///////////////////////////////////////
 // COMMAND POOL ///////////////////////
///////////////////////////////////////
void Vulkan::CommandPool::create_command_pool(VkDevice device, uint32_t queue_family_index,
                                              bool allow_reset, bool transient) {
    _device = device;
    _queue_family_index = queue_family_index;
    _allow_reset = allow_reset;
    _transient = transient;

    VkCommandPoolCreateFlags flags = 0;

    if(allow_reset)
        flags |= VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

    if(transient)
        flags |= VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;

    VkCommandPoolCreateInfo poolInfo{
        .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        .flags = flags,
        .queueFamilyIndex = _queue_family_index,
    };

    VkResult result = vkCreateCommandPool(device, &poolInfo, nullptr, &_command_pool);

    if(result != VK_SUCCESS) {
        PLOGF << "Vulkan: Failed to create command pool";
        terminate_engine();
    }
}

std::vector<VkCommandBuffer> Vulkan::CommandPool::allocate_buffers(uint32_t count, VkCommandBufferLevel level) {
    if(_command_pool == VK_NULL_HANDLE) {
        PLOGF << "Vulkan: Cannot allocate buffers: command pool is null";
        terminate_engine();
    }

    VkCommandBufferAllocateInfo allocInfo{
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .commandPool = _command_pool,
        .level = level,
        .commandBufferCount = count,
    };

    std::vector<VkCommandBuffer> command_buffers(count);
    VkResult result = vkAllocateCommandBuffers(_device, &allocInfo, command_buffers.data());

    if(result != VK_SUCCESS) {
        PLOGF << "Vulkan: Failed to allocate command buffers";
        terminate_engine();
    }

    _allocated_buffers.insert(_allocated_buffers.end(), command_buffers.begin(), command_buffers.end());

    return command_buffers;
}

VkCommandBuffer Vulkan::CommandPool::allocate_buffer(VkCommandBufferLevel level) {
    auto buffers = allocate_buffers(1, level);
    return buffers[0];
}

void Vulkan::CommandPool::free_buffers(const std::vector<VkCommandBuffer>& buffers) {
    if(_command_pool == VK_NULL_HANDLE || buffers.empty()) return;

    vkFreeCommandBuffers(_device, _command_pool, static_cast<uint32_t>(buffers.size()), buffers.data());

    for(const auto& buffer : buffers) {
        auto it = std::find(_allocated_buffers.begin(), _allocated_buffers.end(), buffer);
        if(it!=_allocated_buffers.end())
            _allocated_buffers.erase(it);
    }
}

void Vulkan::CommandPool::free_buffer(VkCommandBuffer buffer) {
    free_buffers({buffer});
}

void Vulkan::CommandPool::reset(bool release_resources) {
    if(_command_pool == VK_NULL_HANDLE) return;

    VkCommandPoolResetFlags flags = 0;
    if(release_resources)
        flags |= VK_COMMAND_POOL_RESET_RELEASE_RESOURCES_BIT;

    VkResult result = vkResetCommandPool(_device, _command_pool, flags);

    if(result != VK_SUCCESS) {
        PLOGF << "Vulkan: Failed to reset command pool";
        terminate_engine();
    }
}

void Vulkan::CommandPool::begin_buffer(VkCommandBuffer buffer, VkCommandBufferUsageFlags usage,
                                       const VkCommandBufferInheritanceInfo* inheritance_info) {
    VkCommandBufferBeginInfo beginInfo{
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .flags = usage,
        .pInheritanceInfo = inheritance_info, // Only used for secondary command buffers
    };
    
    VkResult result = vkBeginCommandBuffer(buffer, & beginInfo);

    if(result != VK_SUCCESS) {
        PLOGF << "Vulkan: Failed to begin recording command buffer";
        terminate_engine();
    }
}

void Vulkan::CommandPool::end_buffer(VkCommandBuffer buffer) {
    VkResult result = vkEndCommandBuffer(buffer);

    if(result != VK_SUCCESS) {
        PLOGF << "Vulkan: Failed to end recording command buffer";
        terminate_engine();
    }
}

VkCommandBuffer Vulkan::CommandPool::begin_single_time_commands() {
    VkCommandBuffer buffer = allocate_buffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY);
    begin_buffer(buffer, VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
    return buffer;
}

void Vulkan::CommandPool::end_single_time_commands(VkCommandBuffer buffer, VkQueue queue) {
    end_buffer(buffer);

    VkCommandBufferSubmitInfo buffer_submitInfo{
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO,
        .commandBuffer = buffer,
    };

    VkSubmitInfo2 submitInfo{
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2,
        .commandBufferInfoCount = 1,
        .pCommandBufferInfos = & buffer_submitInfo,
    };

    {
        VkResult result = vkQueueSubmit2(queue, 1, &submitInfo, VK_NULL_HANDLE);
        
        if(result != VK_SUCCESS) {
            PLOGF << "Vulkan: Failed to submit single-time command buffer";
            terminate_engine();
        }
    }
    {
        VkResult result = vkQueueWaitIdle(queue);

        if(result != VK_SUCCESS) {
            PLOGF << "Vulkan: Failed to wait for queue idle after single-time command";
            terminate_engine();
        }
    }

    free_buffer(buffer);
}
void Vulkan::CommandPool::begin_render_pass(VkCommandBuffer buffer, VkRenderPass render_pass,
                                            VkFramebuffer framebuffer, VkRect2D render_area,
                                            const std::vector<VkClearValue>& clear_values) {
    VkRenderPassBeginInfo render_passInfo{
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
        .renderPass = render_pass,
        .framebuffer = framebuffer,
        .renderArea = render_area,
        .clearValueCount = static_cast<uint32_t>(clear_values.size()),
        .pClearValues = clear_values.data(),
    };

    VkSubpassBeginInfo subpassInfo{
        .sType = VK_STRUCTURE_TYPE_SUBPASS_BEGIN_INFO,
        .contents = VK_SUBPASS_CONTENTS_INLINE,
    };

    vkCmdBeginRenderPass2(buffer, &render_passInfo, &subpassInfo);
}

void Vulkan::CommandPool::end_render_pass(VkCommandBuffer buffer) {
    vkCmdEndRenderPass2(buffer, nullptr);
}

void Vulkan::CommandPool::bind_pipeline(VkCommandBuffer buffer, VkPipeline pipeline, VkPipelineBindPoint bind_point) {
    vkCmdBindPipeline(buffer, bind_point, pipeline);
}

void Vulkan::CommandPool::bind_vertex_buffers(VkCommandBuffer buffer, uint32_t first_binding,
                                              const std::vector<VkBuffer>& buffers,
                                              const std::vector<VkDeviceSize>& offsets) {
    if(buffers.size() != offsets.size()) {
        PLOGF << "Vulkan: Failed to bind vertex buffers: number of buffers must match number of offsets";
        terminate_engine();
    }

    vkCmdBindVertexBuffers(buffer, first_binding, static_cast<uint32_t>(buffers.size()), buffers.data(), offsets.data());
}

void Vulkan::CommandPool::bind_index_buffer(VkCommandBuffer buffer, VkBuffer index_buffer,
                                            VkDeviceSize offset, VkIndexType index_type) {
    vkCmdBindIndexBuffer(buffer, index_buffer, offset, index_type);
}

void Vulkan::CommandPool::bind_desc_sets(VkCommandBuffer buffer, VkPipelineLayout pipeline_layout,
                                         uint32_t first_set, const std::vector<VkDescriptorSet>& desc_sets,
                                         const std::vector<uint32_t>& dynamic_offsets) {
    vkCmdBindDescriptorSets(buffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                            pipeline_layout, first_set,
                            static_cast<uint32_t>(desc_sets.size()),
                            desc_sets.data(),
                            static_cast<uint32_t>(dynamic_offsets.size()),
                            dynamic_offsets.data());
}

void Vulkan::CommandPool::draw(VkCommandBuffer buffer, uint32_t vertex_count, uint32_t instance_count,
                               uint32_t first_vertex, uint32_t first_instance) {
    vkCmdDraw(buffer, vertex_count, instance_count, first_vertex, first_instance);
}

void Vulkan::CommandPool::draw_indexed(VkCommandBuffer buffer, uint32_t index_count, uint32_t instance_count,
                                       uint32_t first_index, int32_t vertex_offset, uint32_t first_instance) {
    vkCmdDrawIndexed(buffer, index_count, instance_count, first_index, vertex_offset, first_instance);
}

void Vulkan::CommandPool::set_viewport(VkCommandBuffer buffer, float x, float y, float width, float height,
                                       float min_depth, float max_depth) {
    VkViewport viewport{
        .x = x,
        .y = y,
        .width = width,
        .height = height,
        .minDepth = min_depth,
        .maxDepth = max_depth,
    };
    
    vkCmdSetViewport(buffer, 0, 1, &viewport);
}

void Vulkan::CommandPool::set_scissor(VkCommandBuffer buffer, int32_t x, int32_t y, uint32_t width, uint32_t height) {
    VkRect2D scissor{
        .offset = {x, y},
        .extent = {width, height},
    };

    vkCmdSetScissor(buffer, 0, 1, &scissor);
}

void Vulkan::CommandPool::copy_buffer(VkCommandBuffer buffer, VkBuffer src_buffer, VkBuffer dst_buffer,
                                      VkDeviceSize size, VkDeviceSize src_offset, VkDeviceSize dst_offset) {
    VkBufferCopy2 copy {
        .sType = VK_STRUCTURE_TYPE_BUFFER_COPY_2,
        .pNext = nullptr,
        .srcOffset = src_offset,
        .dstOffset = dst_offset,
        .size = size,
    };

    VkCopyBufferInfo2 copyInfo {
        .sType = VK_STRUCTURE_TYPE_COPY_BUFFER_INFO_2,
        .pNext = nullptr,
        .srcBuffer = src_buffer,
        .dstBuffer = dst_buffer,
        .regionCount = 1,
        .pRegions = &copy,
    };

    vkCmdCopyBuffer2(buffer, &copyInfo);
}

void Vulkan::CommandPool::pipeline_barrier(VkCommandBuffer buffer, VkDependencyFlags dep_flags,
                                           const std::vector<VkMemoryBarrier2>& memory_barriers,
                                           const std::vector<VkBufferMemoryBarrier2>& buffer_barriers,
                                           const std::vector<VkImageMemoryBarrier2>& image_barriers) {
    VkDependencyInfo depInfo{
        .sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
        .pNext = nullptr,
        .dependencyFlags = dep_flags,
        .memoryBarrierCount = static_cast<uint32_t>(memory_barriers.size()),
        .pMemoryBarriers = memory_barriers.data(),
        .bufferMemoryBarrierCount = static_cast<uint32_t>(buffer_barriers.size()),
        .pBufferMemoryBarriers = buffer_barriers.data(),
        .imageMemoryBarrierCount = static_cast<uint32_t>(image_barriers.size()),
        .pImageMemoryBarriers = image_barriers.data(),
    };
    vkCmdPipelineBarrier2(buffer, &depInfo);
}

void Vulkan::CommandPool::push_constants(VkCommandBuffer buffer, VkPipelineLayout pipeline_layout,
                                         VkShaderStageFlags stage_flags, uint32_t offset, 
                                         uint32_t size, const void* data) {
    vkCmdPushConstants(buffer, pipeline_layout, stage_flags, offset, size, data);
}

void Vulkan::CommandPool::record_frame_commands(VkCommandBuffer buffer, VkRenderPass render_pass, 
                                                VkFramebuffer framebuffer, VkPipeline pipeline,
                                                VkPipelineLayout pipeline_layout, VkRect2D render_area,
                                                const std::vector<VkClearValue>& clear_values,
                                                const std::vector<VkBuffer>& vertex_buffers,
                                                const std::vector<VkDeviceSize>& vertex_offsets,
                                                VkBuffer index_buffer, VkDeviceSize index_offsets,
                                                const std::vector<VkDescriptorSet>& desc_sets,
                                                uint32_t vertex_count, uint32_t index_count,
                                                uint32_t instance_count) {
    begin_render_pass(buffer, render_pass, framebuffer, render_area, clear_values);

    bind_pipeline(buffer, pipeline, VK_PIPELINE_BIND_POINT_GRAPHICS);

    set_viewport(buffer,
                static_cast<float>(render_area.offset.x),
                static_cast<float>(render_area.offset.y),
                static_cast<float>(render_area.extent.width),
                static_cast<float>(render_area.extent.height));

    set_scissor(buffer, render_area.offset.x, render_area.offset.y,
                render_area.extent.width, render_area.extent.height);

    if(!vertex_buffers.empty())
        bind_vertex_buffers(buffer, 0, vertex_buffers, vertex_offsets);

    if(index_buffer != VK_NULL_HANDLE)
        bind_index_buffer(buffer, index_buffer);

    if(!desc_sets.empty())
        bind_desc_sets(buffer, pipeline_layout, 0, desc_sets);

    if(index_buffer != VK_NULL_HANDLE && index_count > 0)
        draw_indexed(buffer, index_count, instance_count);
    else if(vertex_count > 0)
        draw(buffer, vertex_count, instance_count);

    end_render_pass(buffer);
}

void Vulkan::CommandPool::clean() {
    if(_device != VK_NULL_HANDLE) {
        if(!_allocated_buffers.empty()) {
            vkFreeCommandBuffers(_device, _command_pool,
                                 static_cast<uint32_t>(_allocated_buffers.size()),
                                 _allocated_buffers.data());

            _allocated_buffers.clear();
        }

        if(_command_pool != VK_NULL_HANDLE) {
            vkDestroyCommandPool(_device, _command_pool, nullptr);
            _command_pool = VK_NULL_HANDLE;
        }

        _device = VK_NULL_HANDLE;
    }
}

  ///////////////////////////////////////
 // MEMORY BUFFERS /////////////////////
///////////////////////////////////////
void Vulkan::Buffer::create_buffer(VkDevice device, VkPhysicalDevice physical_device, VkDeviceSize size,
                                   VkBufferUsageFlags usage, VkMemoryPropertyFlags memory_properties) {
    _device = device;
    _size = size;
    _usage = usage;
    _memory_properties = memory_properties;

    VkBufferCreateInfo bufferInfo{
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .size = size,
        .usage = usage,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE, //TODO: Make this user settable
    };

    {
        VkResult result = vkCreateBuffer(device, &bufferInfo, nullptr, &_buffer);

        if(result != VK_SUCCESS) {
            PLOGF << "Vulkan: Failed to create buffer";
            terminate_engine();
        }
    }

    VkMemoryRequirements2 mem_reqs;
    VkBufferMemoryRequirementsInfo2 mem_reqInfo{
        .sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_REQUIREMENTS_INFO_2,
        .pNext = nullptr,
        .buffer = _buffer,
    };
    vkGetBufferMemoryRequirements2(device, &mem_reqInfo, &mem_reqs);

    uint32_t mem_index = find_mem_type(physical_device, mem_reqs.memoryRequirements.memoryTypeBits, memory_properties);

    VkPhysicalDeviceMemoryProperties2 physical_mem_props;
    vkGetPhysicalDeviceMemoryProperties2(physical_device, &physical_mem_props);
    _is_coherent = (physical_mem_props.memoryProperties.memoryTypes[mem_index].propertyFlags & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT) != 0;

    VkMemoryAllocateInfo allocInfo{
        .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
        .allocationSize = mem_reqs.memoryRequirements.size,
        .memoryTypeIndex = mem_index,
    };
    {
        VkResult result = vkAllocateMemory(device, &allocInfo, nullptr, &_memory);

        if(result != VK_SUCCESS) {
            PLOGF << "Vulkan: Failed to allocate buffer memory";
            terminate_engine();
        }
    }

    VkBindBufferMemoryInfo bind_bufferInfo{
        .sType = VK_STRUCTURE_TYPE_BIND_BUFFER_MEMORY_INFO,
        .buffer = _buffer,
        .memory = _memory,
        .memoryOffset = 0,
    };
    {
        VkResult result = vkBindBufferMemory2(device, 1, &bind_bufferInfo);

        if(result != VK_SUCCESS) {
            PLOGF << "Vulkan: Failed to bind buffer memory";
            terminate_engine();
        }
    }
}

void Vulkan::Buffer::create_buffer_with_data(VkDevice device, VkPhysicalDevice physical_device,
                                             const void * data, VkDeviceSize size, VkBufferUsageFlags usage,
                                             VkMemoryPropertyFlags memory_properties) {
    create_buffer(device, physical_device, size, usage, memory_properties);

    if(data != nullptr && size > 0) {
        upload_data(data, size);
    }
}

void* Vulkan::Buffer::map(VkDeviceSize offset, VkDeviceSize size) {
    if(_mapped_memory != nullptr) {
        PLOGW << "Vulkan: Buffer is already mapped";
        return _mapped_memory;
    }

    if(!(_memory_properties & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)) {
        PLOGF << "Vulkan: Buffer cannot map non-host-visible memory";
        terminate_engine();
    }

    VkResult result = vkMapMemory(_device, _memory, offset, size, _memory_properties, &_mapped_memory);

    if(result != VK_SUCCESS) {
        PLOGF << "Vulkan: Failed to map buffer memory";
        terminate_engine();
    }

    return _mapped_memory;
}
    
void Vulkan::Buffer::unmap() {
    if(_mapped_memory != nullptr) {
        vkUnmapMemory(_device, _memory);
        _mapped_memory = nullptr;
    }
}

void Vulkan::Buffer::upload_data(const void* data, VkDeviceSize size, VkDeviceSize offset) {
    if(data == nullptr || size == 0) {
        return;
    }

    void* mapped_data = map(offset, size);
    std::memcpy(mapped_data, data, static_cast<size_t>(size));

    if(!_is_coherent) {
        flush(offset, size);
    }

    unmap();
}

void Vulkan::Buffer::copy_to(VkDevice device, VkCommandPool command_pool, VkQueue graphics_queue,
                             Buffer& dst_buffer, VkDeviceSize size, VkDeviceSize src_offset, 
                             VkDeviceSize dst_offset) {
    VkCommandBuffer buffer = begin_single_time_commands(device, command_pool);

    VkBufferCopy2 copy{
        .sType = VK_STRUCTURE_TYPE_BUFFER_COPY_2,
        .srcOffset = src_offset,
        .dstOffset = dst_offset,
        .size = size,
    };

    VkCopyBufferInfo2 copyInfo{
        .sType = VK_STRUCTURE_TYPE_COPY_BUFFER_INFO_2,
        .pNext = nullptr,
        .srcBuffer = _buffer,
        .dstBuffer = dst_buffer._buffer,
        .regionCount = 1,
        .pRegions = &copy,
    };

    vkCmdCopyBuffer2(buffer, &copyInfo);
}

void Vulkan::Buffer::flush(VkDeviceSize offset, VkDeviceSize size) {
    if(_is_coherent) {
        return;
    }

    VkMappedMemoryRange mapped_range{
        .sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE,
        .memory = _memory,
        .offset = offset,
        .size = size,
    };

    VkResult result = vkFlushMappedMemoryRanges(_device, 1, &mapped_range);

    if(result != VK_SUCCESS) {
        PLOGF << "Vulkan: Failed to flush mapped memory range";
        terminate_engine();
    }
}

void Vulkan::Buffer::invalidate(VkDeviceSize offset, VkDeviceSize size) {
    if(_is_coherent) {
        return;
    }

    VkMappedMemoryRange memory_range{
        .sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE,
        .memory = _memory,
        .offset = offset,
        .size = size,
    };

    VkResult result = vkInvalidateMappedMemoryRanges(_device, 1, &memory_range);
    if(result != VK_SUCCESS) {
        PLOGF << "Vulkan: Failed to invalidate mapped memory range";
        terminate_engine();
    }
}

uint32_t find_mem_type(VkPhysicalDevice physical_device, uint32_t type_filter,
                       VkMemoryPropertyFlags properties) {
    VkPhysicalDeviceMemoryProperties2 mem_props;
    vkGetPhysicalDeviceMemoryProperties2(physical_device, &mem_props);

    for(uint32_t i = 0; i < mem_props.memoryProperties.memoryTypeCount; ++i) {
        bool type_supported = (type_filter & (1 << i)) != 0;

        bool has_required_props = (mem_props.memoryProperties.memoryTypes[i].propertyFlags & properties) == 
                                   properties;

        if(type_supported && has_required_props) {
            return i;
        }
    }

    PLOGF << "Vulkan: Failed to find suitable memory type for buffer";
    terminate_engine();

    return -1; //For lsp
}

VkCommandBuffer Vulkan::Buffer::begin_single_time_commands(VkDevice device, VkCommandPool command_pool) const {
    VkCommandBufferAllocateInfo allocInfo{
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .commandPool = command_pool,
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = 1,
    };

    VkCommandBuffer buffer;
    {
        VkResult result = vkAllocateCommandBuffers(device, &allocInfo, &buffer);

        if(result != VK_SUCCESS) {
            PLOGF << "Vulkan: Failed to allocate single-time command buffer";
            terminate_engine();
        }
    }
    VkCommandBufferBeginInfo beginInfo{
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
    };
    VkResult result = vkBeginCommandBuffer(buffer, &beginInfo);

    if(result != VK_SUCCESS) {
        PLOGF << "Vulkan: Failed to begin recording single-time command buffer";
        terminate_engine();
    }
    return buffer;
}

void Vulkan::Buffer::end_single_time_commands(VkDevice device, VkCommandPool command_pool,
                                              VkCommandBuffer buffer, VkQueue queue) const {
    {
        VkResult result = vkEndCommandBuffer(buffer);

        if(result != VK_SUCCESS) {
            PLOGF << "Vulkan: Failed to end recording single-time command buffer";
            terminate_engine();
        }
    }

    VkSubmitInfo submitInfo{
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .commandBufferCount = 1,
        .pCommandBuffers = &buffer,
    };

    {
        VkResult result = vkQueueSubmit(queue, 1, &submitInfo, VK_NULL_HANDLE);

        if(result != VK_SUCCESS) {
            PLOGF << "Vulkan: Failed to submit single-time command buffer";
            terminate_engine();
        }
    }
    {
        VkResult result = vkQueueWaitIdle(queue);

        if(result != VK_SUCCESS) {
            PLOGF << "Vulkan: Failed to wait for idle queue after single-time command";
            terminate_engine();
        }
    }

    vkFreeCommandBuffers(device, command_pool, 1, &buffer);
}

Vulkan::Buffer Vulkan::Buffer::create_vertex_buffer(VkDevice device, VkPhysicalDevice physical_device,
                                                    VkCommandPool command_pool, VkQueue graphics_queue,
                                                    std::shared_ptr<VertexArray> vertex_array) {
    VkDeviceSize buffer_size = vertex_array->data.size();
    Buffer staging_buffer;
    staging_buffer.create_buffer_with_data(device, physical_device, &vertex_array->data, buffer_size,
                                           VK_BUFFER_USAGE_TRANSFER_SRC_BIT, 
                                           VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | 
                                           VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    Buffer vertex_buffer;
    vertex_buffer.create_buffer(device, physical_device, buffer_size,
                         VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                         VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    staging_buffer.copy_to(device, command_pool, graphics_queue, vertex_buffer, buffer_size);

    staging_buffer.clean();

    return vertex_buffer;
}

Vulkan::Buffer Vulkan::Buffer::create_index_buffer(VkDevice device, VkPhysicalDevice physical_device,
                                                   VkCommandPool command_pool, VkQueue graphics_queue, 
                                                   std::shared_ptr<VertexArray> vertex_array) {
    VkDeviceSize buffer_size = vertex_array->index_data.size();

    Buffer staging_buffer;
    staging_buffer.create_buffer_with_data(device, physical_device, &vertex_array->index_data, buffer_size, 
                                           VK_BUFFER_USAGE_TRANSFER_SRC_BIT, 
                                           VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                                           VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    Buffer index_buffer;
    index_buffer.create_buffer(device, physical_device, buffer_size, 
                               VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                               VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    staging_buffer.copy_to(device, command_pool, graphics_queue, index_buffer, buffer_size);
    staging_buffer.clean();

    return index_buffer;
}

Vulkan::Buffer Vulkan::Buffer::create_uniform_buffer(VkDevice device, VkPhysicalDevice physical_device) {
    Buffer uniform_buffer;
    uniform_buffer.create_buffer(device, physical_device, sizeof(Uniforms), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                                 VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    return uniform_buffer;
}

Vulkan::Buffer Vulkan::Buffer::create_staging_buffer(VkDevice device, VkPhysicalDevice physical_device, 
                                                     VkDeviceSize size) {
    Buffer staging_buffer;
    staging_buffer.create_buffer(device, physical_device, size, 
                                 VK_BUFFER_USAGE_TRANSFER_SRC_BIT, 
                                 VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                                 VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    return staging_buffer;
}

std::vector<Vulkan::Buffer> Vulkan::Buffer::create_attrib_buffers(VkDevice device, VkPhysicalDevice physical_device,
                                                                  VkCommandPool command_pool, VkQueue graphics_queue,
                                                                  std::shared_ptr<VertexArray> vertex_array) {
    std::vector<Buffer> attrib_buffers;
    attrib_buffers.reserve(vertex_array->attributes.size());

    for(auto attrib : vertex_array->attributes) {
        VkDeviceSize attrib_buffer_size = attrib.stride * vertex_array->data.size();

        // TODO: This function creates horribly inefficient buffers that store a bunch of extra data.
        // performing a calculation to extract only the data relevant to attrib would be more memory efficient.
        auto attrib_data = vertex_array->data_start() + attrib.offset;
        
        Buffer staging_buffer;
        staging_buffer.create_buffer_with_data(device, physical_device, &attrib_data,
                                               attrib_buffer_size, 
                                               VK_BUFFER_USAGE_TRANSFER_SRC_BIT, 
                                               VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                                               VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

        Buffer vertex_buffer;
        vertex_buffer.create_buffer(device, physical_device, 
                                    attrib_buffer_size,
                                    VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                                    VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

        staging_buffer.copy_to(device, command_pool, graphics_queue, vertex_buffer, attrib_buffer_size);
        staging_buffer.clean();

        attrib_buffers.push_back(std::move(vertex_buffer));
    }

    return attrib_buffers;
}

void Vulkan::Buffer::update_vertex_buffer(Buffer& buffer, VkDevice device, VkPhysicalDevice physical_device,
                                          VkCommandPool command_pool, VkQueue graphics_queue,
                                          std::shared_ptr<VertexArray> vertex_array, VkDeviceSize offset) {
    VkDeviceSize size = vertex_array->stride * vertex_array->data.size();

    if(offset + size > buffer._size) {
        PLOGF << "Vulkan: Vertex buffer update exceeds buffer size";
        terminate_engine();
    }

    if(buffer._memory_properties == VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT) {
        Buffer staging_buffer;
        staging_buffer.create_buffer_with_data(device, physical_device, &vertex_array->data, size,
                                               VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                                               VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                                               VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

        staging_buffer.copy_to(device, command_pool, graphics_queue, buffer, size, 0, offset);
        staging_buffer.clean();
    } else {
        buffer.upload_data(&vertex_array->data, size);
    }
}

std::vector<Vulkan::Buffer> Vulkan::Buffer::create_uniform_buffers_in_flight(VkDevice device,
                                                                     VkPhysicalDevice physical_device,
                                                                     uint32_t frames_in_flight) {
    std::vector<Buffer> uniform_buffers;
    uniform_buffers.reserve(frames_in_flight);

    for(uint32_t i = 0; i < frames_in_flight; ++i) {
        Buffer uniform_buffer;
        uniform_buffer.create_buffer(device, physical_device, sizeof(Uniforms),
                                     VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                                     VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | 
                                     VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

        uniform_buffers.push_back(uniform_buffer);
    }

    return uniform_buffers;
}

Vulkan::Buffer Vulkan::Buffer::create_dynamic_uniform_buffer(VkDevice device, VkPhysicalDevice physical_device,
                                                             uint32_t object_count) {
    VkPhysicalDeviceProperties2 properties;
    vkGetPhysicalDeviceProperties2(physical_device, &properties);

    size_t min_uniform_alignment = properties.properties.limits.minUniformBufferOffsetAlignment;
    size_t dynamic_alignment = sizeof(Uniforms);

    if(min_uniform_alignment > 0) {
        dynamic_alignment = (dynamic_alignment + min_uniform_alignment - 1) & ~(min_uniform_alignment - 1);
    }

    VkDeviceSize buffer_size = object_count * dynamic_alignment;

    Buffer dynamic_buffer;
    dynamic_buffer.create_buffer(device, physical_device, buffer_size,
                                 VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                                 VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | 
                                 VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    return dynamic_buffer;
}

void Vulkan::Buffer::update_dynamic_uniform_buffer(Buffer& dynamic_buffer, VkPhysicalDevice physical_device,
                                                   uint32_t object_index, Uniforms uniform) {
    VkPhysicalDeviceProperties2 props;
    vkGetPhysicalDeviceProperties2(physical_device, &props);

    size_t min_uniform_alignment = props.properties.limits.minUniformBufferOffsetAlignment;
    size_t dynamic_alignment = sizeof(uniform);

    if(min_uniform_alignment) {
         dynamic_alignment = (dynamic_alignment + min_uniform_alignment -1) & ~(min_uniform_alignment - 1);
    }

    VkDeviceSize offset = object_index + dynamic_alignment;

    dynamic_buffer.upload_data(&uniform, sizeof(uniform), offset);
}

VkMemoryRequirements2 Vulkan::Buffer::get_mem_requirements(VkDevice device, VkDeviceSize size, 
                                                           VkBufferUsageFlags usage) {
    VkBufferCreateInfo bufferInfo{
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .size = size,
        .usage = usage,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
    };

    VkBuffer temp_buffer;

    VkResult result = vkCreateBuffer(device, &bufferInfo, nullptr, &temp_buffer);

    if(result != VK_SUCCESS) {
        PLOGF << "Vulkan: Failed to create temporary buffer for memery requirements query";
        terminate_engine();
    }

    VkBufferMemoryRequirementsInfo2 mem_reqInfo{
        .sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_REQUIREMENTS_INFO_2,
        .buffer = temp_buffer,
    };
    
    VkMemoryRequirements2 mem_reqs;
    vkGetBufferMemoryRequirements2(device, &mem_reqInfo, &mem_reqs);

    vkDestroyBuffer(device, temp_buffer, nullptr);

    return mem_reqs;
}

void Vulkan::Buffer::clean() {
    if(_device != VK_NULL_HANDLE) {
        unmap();
        
        if(_buffer != VK_NULL_HANDLE) {
            vkDestroyBuffer(_device, _buffer, nullptr);
            _buffer = VK_NULL_HANDLE;
        }

        if(_memory != VK_NULL_HANDLE) {
            vkFreeMemory(_device, _memory, nullptr);
            _memory = VK_NULL_HANDLE;
        }

        _device = VK_NULL_HANDLE; // the device is managed by the device struct and does not need to be destroyed
    }
}

  ///////////////////////////////////////
 // RENDER PASS ////////////////////////
///////////////////////////////////////
void Vulkan::RenderPass::create_render_pass(VkDevice device, VkFormat color_format, VkFormat depth_format,
                                            VkSampleCountFlagBits msaa_samples) {
    _device = device;

    VkAttachmentDescription2 color_attachment{
        .sType = VK_STRUCTURE_TYPE_ATTACHMENT_DESCRIPTION_2,
        .format = color_format,
        .samples = msaa_samples,
        .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
        .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
        .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE, // TODO: potentially implement in future
        .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
        .finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
    };

    VkAttachmentDescription2 depth_attachment{
        .sType = VK_STRUCTURE_TYPE_ATTACHMENT_DESCRIPTION_2,
        .format = depth_format,
        .samples = msaa_samples,
        .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
        .storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
        .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
        .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
        .finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
    };

    VkAttachmentReference2 color_attachment_ref{
        .sType = VK_STRUCTURE_TYPE_ATTACHMENT_REFERENCE_2,
        .attachment = 0,
        .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
    };

    VkAttachmentReference2 depth_attachment_ref{
        .sType = VK_STRUCTURE_TYPE_ATTACHMENT_REFERENCE_2,
        .attachment = 1,
        .layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
    };

    VkSubpassDescription2 subpass{
        .sType = VK_STRUCTURE_TYPE_SUBPASS_DESCRIPTION_2,
        .colorAttachmentCount = 1,
        .pColorAttachments = &color_attachment_ref,
        .pDepthStencilAttachment = &depth_attachment_ref,
    };

    VkSubpassDependency2 subpass_dependency{
        .sType = VK_STRUCTURE_TYPE_SUBPASS_DEPENDENCY_2,
        .srcSubpass = VK_SUBPASS_EXTERNAL,
        .srcSubpass = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | 
                      VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
        .dstSubpass = 0,
        .dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT |
                        VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
        .srcAccessMask = 0,
        .dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT |
                         VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
    };

    std::array<VkAttachmentDescription2, 2> attachments = {color_attachment, depth_attachment};

    VkRenderPassCreateInfo2 render_passInfo{
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO_2,
        .attachmentCount = static_cast<uint32_t>(attachments.size()),
        .pAttachments = attachments.data(),
        .subpassCount = 1,
        .pSubpasses = &subpass,
        .dependencyCount = 1,
        .pDependencies = &subpass_dependency,
    };

    VkResult result = vkCreateRenderPass2(device, &render_passInfo, nullptr, &_render_pass);
    if(result != VK_SUCCESS) {
        PLOGF << "Vulkan: Failed to create render pass";
        terminate_engine();
    }
}

void Vulkan::RenderPass::create_framebuffers(const std::vector<VkImageView>& swapchain_image_views,
                                             VkImageView depth_image_view, VkExtent2D extent) {
    _extent = extent;

    framebuffers.resize(swapchain_image_views.size());

    for(size_t i = 0; i < swapchain_image_views.size(); ++i) {
        std::array<VkImageView, 2> attachments = {
            swapchain_image_views[i],
            depth_image_view,
        };

        VkFramebufferCreateInfo framebufferInfo{
            .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
            .renderPass = _render_pass,
            .attachmentCount = static_cast<uint32_t>(attachments.size()),
            .pAttachments = attachments.data(),
            .width = extent.width,
            .height = extent.height,
            .layers = 1,
        };

        VkResult result = vkCreateFramebuffer(_device, &framebufferInfo, nullptr, &framebuffers[i]);
        if(result != VK_SUCCESS) {
            PLOGF << "Vulkan: Failed to create framebuffer " << std::to_string(i);
            terminate_engine();
        }
    }
}

void Vulkan::RenderPass::recreate_framebuffers(const std::vector<VkImageView>& swapchain_image_views,
                                               VkImageView depth_image_view, VkExtent2D extent) {
    vkDeviceWaitIdle(_device);
    destroy_framebuffers();
    create_framebuffers(swapchain_image_views, depth_image_view, extent);
}

void Vulkan::RenderPass::destroy_framebuffers() {
    for(VkFramebuffer framebuffer : framebuffers) {
        if(framebuffer != VK_NULL_HANDLE) {
            vkDestroyFramebuffer(_device, framebuffer, nullptr);
        }
    }
    framebuffers.clear();
}

void Vulkan::RenderPass::clean() {
    if(_device != VK_NULL_HANDLE) {
        destroy_framebuffers();

        if(_render_pass != VK_NULL_HANDLE) {
            vkDestroyRenderPass(_device, _render_pass, nullptr);
            _render_pass = VK_NULL_HANDLE;
        }

        _device = VK_NULL_HANDLE;
    }
}

  ///////////////////////////////////////
 // SYNCRONIZAITON /////////////////////
///////////////////////////////////////
void Vulkan::Synchronization::create_sync(VkDevice device, uint32_t max_frames_in_flight) {
    _device = device;
    _max_frames_in_flight = max_frames_in_flight;

    _frame_sync_objects.resize(max_frames_in_flight);

    for(uint32_t i = 0; i < max_frames_in_flight; ++i) {
        _frame_sync_objects[i].image_available_semaphore = 
            create_semaphore(device);

        _frame_sync_objects[i].render_finished_semaphore = 
            create_semaphore(device);

        _frame_sync_objects[i].in_flight_fence =
            create_fence(device, true);
    }
}

bool Vulkan::Synchronization::wait_for_frame(uint32_t frame_index, uint64_t timeout) {
    if(frame_index >= _frame_sync_objects.size()) {
        PLOGF << "Vulkan: Frame index out of range";
        terminate_engine();
    }

    VkFence fence = _frame_sync_objects[frame_index].in_flight_fence;
    VkResult result = vkWaitForFences(_device, 1, &fence, VK_TRUE, timeout);

    if(result == VK_SUCCESS) {
        return true;
    } else if (result == VK_TIMEOUT) {
        PLOGW << "Vulkan: Timeout waiting for frame " << frame_index;
        return false;
    } else {
        PLOGF << "Vulkan: Failed to wait for frame fence";
        terminate_engine();
        return false;
    }
}

void Vulkan::Synchronization::reset_frame_fence(uint32_t frame_index) {
    if (frame_index >= _frame_sync_objects.size()) {
        PLOGF << "Vulkan: Frame index out of range";
        terminate_engine();
    }

    VkFence fence = _frame_sync_objects[frame_index].in_flight_fence;
    VkResult result = vkResetFences(_device, 1, &fence);

    if(result != VK_SUCCESS) {
        PLOGF << "Vulkan: Failed to reset fences";
        terminate_engine();
    }
}

bool Vulkan::Synchronization::wait_for_all_frames(uint64_t timeout) {
    if(_frame_sync_objects.empty()) {
        return true;
    }

    std::vector<VkFence> all_fences;
    all_fences.reserve(_frame_sync_objects.size());
    for (const auto& frame_sync : _frame_sync_objects) {
        all_fences.push_back(frame_sync.in_flight_fence);
    }

    VkResult result = vkWaitForFences(_device, static_cast<uint32_t>(all_fences.size()), 
                                      all_fences.data(), VK_TRUE, timeout);

    if(result == VK_SUCCESS) {
        return true;
    } else if (result == VK_TIMEOUT) {
        PLOGW << "Vulkan: Timed out waiting for frames";
        return false;
    } else {
        PLOGF << "Vulkan: Failed to wait for frames";
        terminate_engine();
        return false;
    }
}

VkSemaphore Vulkan::Synchronization::create_semaphore(VkDevice device) {
    VkSemaphoreCreateInfo semaphoreInfo{
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
    };

    VkSemaphore semaphore;
    VkResult result = vkCreateSemaphore(device, &semaphoreInfo, nullptr, &semaphore);
    if(result != VK_SUCCESS) {
        PLOGF << "Vulkan: Failed to create semaphore";
        terminate_engine();
    }

    return semaphore;
}

VkFence Vulkan::Synchronization::create_fence(VkDevice device, bool signaled) {
    VkFenceCreateInfo fenceInfo{
        .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
    };

    if(signaled)
        fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    VkFence fence;
    VkResult result = vkCreateFence(device, &fenceInfo, nullptr, &fence);
    if(result != VK_SUCCESS) {
        PLOGF << "Vulkan: Failed to create fence";
        terminate_engine();
    }

    return fence;
}

void Vulkan::Synchronization::destroy_semaphore(VkDevice device, VkSemaphore semaphore) {
    if (semaphore != VK_NULL_HANDLE) {
        vkDestroySemaphore(device, semaphore, nullptr);

        auto it = std::find(_semaphores.begin(), _semaphores.end(), semaphore);
        if(it != _semaphores.end()) {
            _semaphores.erase(it);
        }
    }
}

void Vulkan::Synchronization::destroy_fence(VkDevice device, VkFence fence) {
    if (fence != VK_NULL_HANDLE) {
        vkDestroyFence(device, fence, nullptr);

        auto it = std::find(_fences.begin(), _fences.end(), fence);
        if(it != _fences.end()) {
            _fences.erase(it);
        }
    }
}

void Vulkan::Synchronization::submit_command_buffers(VkQueue queue,
                                                     const std::vector<VkCommandBuffer>& buffers,
                                                     const std::vector<VkSemaphore>& wait_semaphores,
                                                     const std::vector<VkPipelineStageFlags>& wait_stages,
                                                     const std::vector<VkSemaphore>& signal_semaphores,
                                                     VkFence fence) {
    if (wait_semaphores.size() != wait_stages.size()) {
        PLOGF << "Vulkan: Number of semaphores must match number of wait stages";
        terminate_engine();
    }

    VkSubmitInfo submitInfo{
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2,
        .waitSemaphoreCount = static_cast<uint32_t>(wait_semaphores.size()),
        .pWaitSemaphores = wait_semaphores.empty() ? nullptr : wait_semaphores.data(),
        .pWaitDstStageMask = wait_stages.empty() ? nullptr : wait_stages.data(),
        .commandBufferCount = static_cast<uint32_t>(buffers.size()),
        .pCommandBuffers = buffers.data(),
        .signalSemaphoreCount = static_cast<uint32_t>(signal_semaphores.size()),
        .pSignalSemaphores = signal_semaphores.empty() ? nullptr : signal_semaphores.data(),
    };

    VkResult result = vkQueueSubmit(queue, 1, &submitInfo, fence);
    if(result != VK_SUCCESS) {
        PLOGF << "Vulkan: Failed to submit buffers to queue";
        terminate_engine();
    }
}

VkResult Vulkan::Synchronization::present_image(VkQueue present_queue, VkSwapchainKHR swapchain,
                                                uint32_t image_index, const std::vector<VkSemaphore>& wait_semaphores) {
    VkPresentInfoKHR presentInfo{
        .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
        .waitSemaphoreCount = static_cast<uint32_t>(wait_semaphores.size()),
        .pWaitSemaphores = wait_semaphores.empty() ? nullptr : wait_semaphores.data(),
        .swapchainCount = 1,
        .pSwapchains = &swapchain,
        .pImageIndices = &image_index,
        .pResults = nullptr,
    };

    return vkQueuePresentKHR(present_queue, &presentInfo);
}

VkResult Vulkan::Synchronization::acquire_next_image(VkDevice device, VkSwapchainKHR swapchain,
                                                     uint64_t timeout, VkSemaphore semaphore,
                                                     VkFence fence, uint32_t* image_index) {
    VkAcquireNextImageInfoKHR next_imageInfo{
        .sType = VK_STRUCTURE_TYPE_ACQUIRE_NEXT_IMAGE_INFO_KHR,
        .swapchain = swapchain,
        .timeout = timeout,
        .semaphore = semaphore,
        .fence = fence,
        .deviceMask = 1, // TODO: don't hard code this
    };
    return vkAcquireNextImage2KHR(device, &next_imageInfo, image_index);
}

VkSubmitInfo Vulkan::Synchronization::create_submit_info(VkCommandBuffer buffer,
                                                         VkSemaphore wait_semaphore,
                                                         VkPipelineStageFlags wait_stage,
                                                         VkSemaphore signal_semaphores) {
    VkSubmitInfo submitInfo{
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .commandBufferCount = 1,
        .pCommandBuffers = &buffer,
    };

    if(wait_semaphore != VK_NULL_HANDLE) {
        submitInfo.waitSemaphoreCount = 1;
        submitInfo.pWaitSemaphores = &wait_semaphore;
        submitInfo.pWaitDstStageMask = &wait_stage;
    }

    if(signal_semaphores != VK_NULL_HANDLE) {
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores = &signal_semaphores;
    }

    return submitInfo;
}

VkPresentInfoKHR Vulkan::Synchronization::create_present_info(VkSwapchainKHR swapchain, uint32_t image_index,
                                                              VkSemaphore wait_semaphore) {
    VkPresentInfoKHR presentInfo{
        .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
    };

    if(wait_semaphore != VK_NULL_HANDLE) {
        presentInfo.waitSemaphoreCount = 1;
        presentInfo.pWaitSemaphores = &wait_semaphore;
    }

    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = &swapchain;
    presentInfo.pImageIndices = &image_index;

    return presentInfo;
}

VkResult wait_for_fences(VkDevice device, const std::vector<VkFence>& fences,
                         bool wait_all, uint64_t timeout) {
    if(fences.empty()) {
        return VK_SUCCESS;
    }

    return vkWaitForFences(device, static_cast<uint32_t>(fences.size()), fences.data(), wait_all, timeout);
}

void reset_fences(VkDevice device, const std::vector<VkFence>& fences) {
    if(!fences.empty()) {
        VkResult result = vkResetFences(device, static_cast<uint32_t>(fences.size()), fences.data());

        if(result != VK_SUCCESS) {
            PLOGF << "Vulkan: Failed to reset fences";
            terminate_engine();
        }
    }
}

void Vulkan::Synchronization::clean() {
    if(_device != VK_NULL_HANDLE) {
        wait_for_all_frames();

        for(auto& fs : _frame_sync_objects) {
            if(fs.image_available_semaphore != VK_NULL_HANDLE) {
                vkDestroySemaphore(_device, fs.image_available_semaphore, nullptr);
                fs.image_available_semaphore = VK_NULL_HANDLE;
            }

            if(fs.render_finished_semaphore != VK_NULL_HANDLE) {
                vkDestroySemaphore(_device, fs.render_finished_semaphore, nullptr);
                fs.render_finished_semaphore = VK_NULL_HANDLE;
            }

            if(fs.in_flight_fence != VK_NULL_HANDLE) {
                vkDestroyFence(_device, fs.in_flight_fence, nullptr);
                fs.in_flight_fence = VK_NULL_HANDLE;
            }
        }

        if(!_frame_sync_objects.empty()) {
            _frame_sync_objects.clear();
        }

        for(VkSemaphore sp : _semaphores) {
            if(sp != VK_NULL_HANDLE) {
                vkDestroySemaphore(_device, sp, nullptr);
            }
        }
        if(!_semaphores.empty()) {
            _semaphores.clear();
        }

        for(VkFence f : _fences) {
            if(f != VK_NULL_HANDLE) {
                vkDestroyFence(_device, f, nullptr);
            }
        }
        if(!_fences.empty()) {
            _fences.clear();
        }

        _device = VK_NULL_HANDLE;
        _max_frames_in_flight = 0;
    }
}

}

#endif
