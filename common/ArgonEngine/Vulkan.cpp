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
        .front = {}, //TODO: make this user-settable,
        .back = {}, //TODO: make this user-settable
        .minDepthBounds = 0.0f,
        .maxDepthBounds = 1.0f,
    };

    return depth_stencil_info;
}

}

#endif
