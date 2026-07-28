#include "Vulkan.h"
#include <ranges>
#include <algorithm>
#include "ArgonEngine/ArgonInit.h"
#include "plog/Severity.h"
#include "vulkan/vulkan_core.h"
#ifdef USE_VULKAN
#include "VirtualResourceImage.h"

namespace Argon {

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

    VkResult result = vkCreateInstance(&createInfo, nullptr, &instance);
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
        instance, "vkCreateDebugUtilsMessengerEXT"
    );

    if(func != nullptr) {
        VkResult result = func(instance, &createInfo, nullptr, &debug_messenger);
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
            instance, "vkDestroyDebugUtilsMessengerEXT"
        );
        if(func!=nullptr) {
            func(instance, debug_messenger, nullptr);
        }
    }

    if(instance != VK_NULL_HANDLE) {
        vkDestroyInstance(instance, nullptr);
        instance = VK_NULL_HANDLE;
    }
}

}

#endif
