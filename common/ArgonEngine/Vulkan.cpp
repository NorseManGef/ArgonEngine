#include "Vulkan.h"
#include <ranges>
#include <algorithm>
#include "ArgonEngine/ArgonInit.h"
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

}

#endif
