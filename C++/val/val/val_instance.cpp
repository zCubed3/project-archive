#include "val_instance.hpp"

#include <stdexcept>

#include <val/pipelines/val_render_pass_builder.hpp>

#ifdef SDL_SUPPORT
#include <SDL.h>
#include <SDL_vulkan.h>
#endif

// TODO: Verbosity

namespace VAL {
    ValInstance::CreationError ValInstance::last_error = ValInstance::ERR_NONE;

    std::vector<ValExtension> ValInstance::validate_instance_extensions(const ValInstanceCreateInfo& create_info) {
        std::vector<VkExtensionProperties> found_extensions;
        uint32_t found_extension_count;

        vkEnumerateInstanceExtensionProperties(nullptr, &found_extension_count, nullptr);
        found_extensions.resize(found_extension_count);
        vkEnumerateInstanceExtensionProperties(nullptr, &found_extension_count, found_extensions.data());

        std::vector<ValExtension> missing_extensions;
        for (const ValExtension &extension: create_info.instance_extensions) {
            bool has_extension = false;
            for (VkExtensionProperties properties: found_extensions) {
                if (strcmp(extension.name, properties.extensionName) == 0) {
                    has_extension = true;
                    break;
                }
            }

            if (!has_extension) {
                missing_extensions.push_back(extension);
            }
        }

        return missing_extensions;
    }

    std::vector<ValExtension> ValInstance::validate_device_extensions(VkPhysicalDevice vk_gpu, const ValInstanceCreateInfo& create_info) {
        std::vector<VkExtensionProperties> found_extensions;
        uint32_t found_extension_count;

        vkEnumerateDeviceExtensionProperties(vk_gpu, nullptr, &found_extension_count, nullptr);
        found_extensions.resize(found_extension_count);
        vkEnumerateDeviceExtensionProperties(vk_gpu, nullptr, &found_extension_count, found_extensions.data());

        std::vector<ValExtension> missing_extensions;
        for (const ValExtension &extension: create_info.instance_extensions) {
            bool has_extension = false;
            for (VkExtensionProperties properties: found_extensions) {
                if (strcmp(extension.name, properties.extensionName) == 0) {
                    has_extension = true;
                    break;
                }
            }

            if (!has_extension) {
                missing_extensions.push_back(extension);
            }
        }

        return missing_extensions;
    }

    std::vector<ValLayer> ValInstance::validate_layers(const ValInstanceCreateInfo& create_info) {
        std::vector<VkLayerProperties> found_layers;
        uint32_t found_layer_count;

        vkEnumerateInstanceLayerProperties(&found_layer_count, nullptr);
        found_layers.resize(found_layer_count);
        vkEnumerateInstanceLayerProperties(&found_layer_count, found_layers.data());

        std::vector<ValLayer> missing_layers;
        for (const ValLayer &layer: create_info.validation_layers) {
            bool has_layer = false;
            for (VkLayerProperties properties: found_layers) {
                if (strcmp(layer.name, properties.layerName) == 0) {
                    has_layer = true;
                    break;
                }
            }

            if (!has_layer) {
                missing_layers.push_back(layer);
            }
        }

        return missing_layers;
    }

    VkInstance ValInstance::create_vk_instance(const ValInstanceCreateInfo& create_info) {
        VkInstance vk_instance = nullptr;

        uint32_t application_version = VK_MAKE_VERSION(
                create_info.app_major_version,
                create_info.app_minor_version,
                create_info.app_patch_version);

        uint32_t engine_version = VK_MAKE_VERSION(
                create_info.engine_major_version,
                create_info.engine_minor_version,
                create_info.engine_patch_version);

        uint32_t vulkan_version;

        switch (create_info.version) {
            default:
                vulkan_version = VK_API_VERSION_1_0;
                break;

            case ValInstanceCreateInfo::VulkanAPIVersion::Version1_1:
                vulkan_version = VK_API_VERSION_1_1;
                break;

            case ValInstanceCreateInfo::VulkanAPIVersion::Version1_2:
                vulkan_version = VK_API_VERSION_1_2;
                break;

            case ValInstanceCreateInfo::VulkanAPIVersion::Version1_3:
                vulkan_version = VK_API_VERSION_1_3;
                break;
        }

        VkApplicationInfo app_info{};
        app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        app_info.pApplicationName = create_info.app_name.c_str();
        app_info.applicationVersion = application_version;
        app_info.pEngineName = create_info.engine_name.c_str();
        app_info.engineVersion = engine_version;
        app_info.apiVersion = vulkan_version;

        VkInstanceCreateInfo instance_create_info{};
        instance_create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        instance_create_info.pApplicationInfo = &app_info;

        //
        // Extensions
        //
        std::vector<const char *> enabled_extensions{};
        std::vector<ValExtension> missing_extensions = validate_instance_extensions(create_info);

        for (const ValExtension &extension: create_info.instance_extensions) {
            bool missing_extension = false;
            bool mandatory_extension = false;
            for (const ValExtension &missing: missing_extensions) {
                if (strcmp(extension.name, missing.name) == 0) {
                    missing_extension = true;

                    if (extension.flags & ValExtension::EXTENSION_FLAG_OPTIONAL) {
                        continue;
                    }

                    mandatory_extension = true;
                }
            }

            if (!missing_extension) {
                enabled_extensions.push_back(extension.name);
            } else {
                if (mandatory_extension) {
                    last_error = ERR_VK_MISSING_INSTANCE_EXTENSION;
                    return nullptr;
                }
            }
        }

        instance_create_info.ppEnabledExtensionNames = enabled_extensions.data();
        instance_create_info.enabledExtensionCount = static_cast<uint32_t>(enabled_extensions.size());

        //
        // Layers
        //
        std::vector<const char *> enabled_layers{};
        std::vector<ValLayer> missing_layers = validate_layers(create_info);

        for (const ValLayer &layer: create_info.validation_layers) {
            bool missing_layer = false;
            bool mandatory_layer = false;
            for (const ValLayer &missing: missing_layers) {
                if (strcmp(layer.name, missing.name) == 0) {
                    missing_layer = true;

                    if (layer.flags & ValLayer::LAYER_FLAG_OPTIONAL) {
                        continue;
                    }

                    mandatory_layer = true;
                }
            }

            if (!missing_layer) {
                enabled_layers.push_back(layer.name);
            } else {
                if (mandatory_layer) {
                    last_error = ERR_VK_MISSING_INSTANCE_EXTENSION;
                    return nullptr;
                }
            }
        }

        instance_create_info.ppEnabledLayerNames = enabled_layers.data();
        instance_create_info.enabledLayerCount = static_cast<uint32_t>(enabled_layers.size());


        if (vkCreateInstance(&instance_create_info, nullptr, &vk_instance) != VK_SUCCESS) {
            return nullptr;
        }

        return vk_instance;
    }

    ValInstance::ChosenGPU ValInstance::pick_gpu(VkInstance vk_instance, VkSurfaceKHR vk_surface, const ValInstanceCreateInfo& create_info) {
        uint32_t gpu_count;
        std::vector<VkPhysicalDevice> gpus;

        vkEnumeratePhysicalDevices(vk_instance, &gpu_count, nullptr);
        gpus.resize(gpu_count);
        vkEnumeratePhysicalDevices(vk_instance, &gpu_count, gpus.data());


        for (VkPhysicalDevice gpu: gpus) {
            uint32_t enumeration_count;

            VkPhysicalDeviceProperties properties;
            VkPhysicalDeviceFeatures features;
            std::vector<VkQueueFamilyProperties> queue_families;

            vkGetPhysicalDeviceProperties(gpu, &properties);
            vkGetPhysicalDeviceFeatures(gpu, &features);

            vkGetPhysicalDeviceQueueFamilyProperties(gpu, &enumeration_count, nullptr);
            queue_families.resize(enumeration_count);
            vkGetPhysicalDeviceQueueFamilyProperties(gpu, &enumeration_count, queue_families.data());

            enumeration_count = 0;

            uint32_t queue_index = 0;
            std::vector<ValQueue> queues;

            // TODO: Better queue exclusivity?
            for (VkQueueFamilyProperties queue_family: queue_families) {
                for (ValQueue::QueueType type: create_info.requested_queues) {
                    // Ensure we haven't already found a suitable queue
                    bool already_found = false;

                    for (ValQueue &queue: queues) {
                        if (queue.type == type) {
                            already_found = true;
                        }
                    }

                    if (already_found) {
                        continue;
                    }

                    ValQueue queue{};
                    queue.type = type;
                    queue.family = queue_index;

                    bool unique = false;

                    switch (type) {
                        case ValQueue::QUEUE_TYPE_GRAPHICS: {
                            if (queue_family.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
                                queues.push_back(queue);
                                unique = true;
                            }

                            break;
                        }

                        case ValQueue::QUEUE_TYPE_TRANSFER: {
                            if (queue_family.queueFlags & VK_QUEUE_TRANSFER_BIT) {
                                queues.push_back(queue);
                                unique = true;
                            }

                            break;
                        }

#if SDL_SUPPORT
                        case ValQueue::QUEUE_TYPE_PRESENT: {
                            VkBool32 surface_support = false;
                            vkGetPhysicalDeviceSurfaceSupportKHR(gpu, queue_index, vk_surface, &surface_support);

                            if (surface_support) {
                                queues.push_back(queue);
                                unique = true;
                            }

                            break;
                        }
#endif
                    }

                    if (unique) {
                        break;
                    }
                }

                queue_index++;
            }

            // TODO: Optional required features
            // TODO: GPU ranking / picking
            // TODO: Config option to manually decide GPU?
            // TODO: Go through ALL GPUs and don't pick the first supported GPU as the default always!

            if (queues.size() != create_info.requested_queues.size()) {
                continue;
            }

            // TODO: This will break if Vulkan stops using VkBool32 for features
            VkPhysicalDeviceFeatures enabled_features{};
            size_t num_features = sizeof(VkPhysicalDeviceFeatures) / sizeof(VkBool32);

            for (int f = 0; f < num_features; f++) {
                VkBool32 has = (&features.robustBufferAccess)[f];
                VkBool32 requested = (&create_info.vk_enabled_features.robustBufferAccess)[f];

                (&enabled_features.robustBufferAccess)[f] = has && requested;
            }

            return ValInstance::ChosenGPU{gpu, features, queues};
        }

        return ValInstance::ChosenGPU{};
    }

    VkDevice ValInstance::create_vk_device(ChosenGPU &vk_gpu, std::vector<ValQueue> &val_queues, const ValInstanceCreateInfo& create_info) {
        VkDevice vk_device = nullptr;

        std::vector<uint32_t> device_queues;
        std::vector<VkDeviceQueueCreateInfo> device_queue_infos;

        float queue_priority = 1;

        for (const ValQueue &queue: val_queues) {
            device_queues.push_back(queue.family);

            VkDeviceQueueCreateInfo queue_info{};
            queue_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            queue_info.queueFamilyIndex = queue.family;
            queue_info.queueCount = 1;
            queue_info.pQueuePriorities = &queue_priority;

            device_queue_infos.push_back(queue_info);
        }

        VkDeviceCreateInfo device_create_info{};
        device_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

        device_create_info.pQueueCreateInfos = device_queue_infos.data();
        device_create_info.queueCreateInfoCount = static_cast<uint32_t>(device_queue_infos.size());

        device_create_info.pEnabledFeatures = &vk_gpu.vk_features;

        //
        // Extensions
        //
        std::vector<const char *> enabled_extensions{};
        std::vector<ValExtension> missing_extensions = validate_device_extensions(vk_gpu.vk_device, create_info);

        for (const ValExtension &extension: create_info.device_extensions) {
            bool missing_extension = false;
            bool mandatory_extension = false;
            for (const ValExtension &missing: missing_extensions) {
                if (strcmp(extension.name, missing.name) == 0) {
                    missing_extension = true;

                    if (extension.flags & ValExtension::EXTENSION_FLAG_OPTIONAL) {
                        continue;
                    }

                    mandatory_extension = true;
                }
            }

            if (!missing_extension) {
                enabled_extensions.push_back(extension.name);
            } else {
                if (mandatory_extension) {
                    last_error = ERR_VK_MISSING_INSTANCE_EXTENSION;
                    return nullptr;
                }
            }
        }

        device_create_info.ppEnabledExtensionNames = enabled_extensions.data();
        device_create_info.enabledExtensionCount = static_cast<uint32_t>(enabled_extensions.size());

        device_create_info.enabledLayerCount = 0;

        if (vkCreateDevice(vk_gpu.vk_device, &device_create_info, nullptr, &vk_device) != VK_SUCCESS) {
            last_error = ERR_VK_DEVICE_FAILURE;
            return nullptr;
        }

        for (ValQueue &queue: val_queues) {
            vkGetDeviceQueue(vk_device, queue.family, 0, &queue.vk_queue);
        }

        return vk_device;
    }

    VmaAllocator ValInstance::create_vma_allocator(VkInstance vk_instance, VkDevice vk_device, VkPhysicalDevice vk_gpu) {
        VmaAllocatorCreateInfo allocatorInfo = {};
        allocatorInfo.physicalDevice = vk_gpu;
        allocatorInfo.device = vk_device;
        allocatorInfo.instance = vk_instance;

        VmaAllocator vma_allocator = nullptr;

        if (vmaCreateAllocator(&allocatorInfo, &vma_allocator) != VK_SUCCESS) {
            return nullptr;
        }

        return vma_allocator;
    }

    // TODO: User sizeable pools (in create info)
    // TODO: Abstract descriptor pools
    VkDescriptorPool ValInstance::create_vk_descriptor_pool(VkDevice vk_device) {
        VkDescriptorPoolSize pool_sizes[] =
                {
                        {VK_DESCRIPTOR_TYPE_SAMPLER, 1000},
                        {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000},
                        {VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000},
                        {VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000},
                        {VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000},
                        {VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000},
                        {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000},
                        {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000},
                        {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000},
                        {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000},
                        {VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000}};

        VkDescriptorPoolCreateInfo create_info{};
        create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        create_info.poolSizeCount = 11;
        create_info.pPoolSizes = pool_sizes;
        create_info.maxSets = 1000;
        create_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;

        VkDescriptorPool vk_pool = nullptr;
        if (vkCreateDescriptorPool(vk_device, &create_info, nullptr, &vk_pool) != VK_SUCCESS) {
            return nullptr;
        }

        return vk_pool;
    }

    VkSemaphore ValInstance::create_vk_semaphore(VkDevice vk_device) {
        VkSemaphoreCreateInfo semaphore_create_info{};
        semaphore_create_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

        VkSemaphore vk_semaphore;
        if (vkCreateSemaphore(vk_device, &semaphore_create_info, nullptr, &vk_semaphore) != VK_SUCCESS) {
            return nullptr;
        }

        return vk_semaphore;
    }

    VkFence ValInstance::create_vk_fence(VkDevice vk_device) {
        VkFenceCreateInfo fence_create_info{};
        fence_create_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fence_create_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;

        VkFence vk_fence;
        if (vkCreateFence(vk_device, &fence_create_info, nullptr, &vk_fence) != VK_SUCCESS) {
            return nullptr;
        }

        return vk_fence;
    }

    bool ValInstance::cache_surface_info(VkInstance vk_instance, VkSurfaceKHR vk_surface, VkPhysicalDevice vk_gpu) {
        if (vk_surface == nullptr) {
            throw std::runtime_error("vk_surface was nullptr!");
        }

        uint32_t enumeration_count;

        vkGetPhysicalDeviceSurfaceFormatsKHR(vk_gpu, vk_surface, &enumeration_count, nullptr);
        vk_supported_surface_formats.resize(enumeration_count);
        vkGetPhysicalDeviceSurfaceFormatsKHR(vk_gpu, vk_surface, &enumeration_count, vk_supported_surface_formats.data());

        if (enumeration_count == 0) {
            throw std::runtime_error("No supported surface formats found!");
        }

        enumeration_count = 0;

        vkGetPhysicalDeviceSurfacePresentModesKHR(vk_gpu, vk_surface, &enumeration_count, nullptr);
        vk_supported_present_modes.resize(enumeration_count);
        vkGetPhysicalDeviceSurfacePresentModesKHR(vk_gpu, vk_surface, &enumeration_count, vk_supported_present_modes.data());

        if (enumeration_count == 0) {
            // TODO: Error no present modes available!
            return false;
        }

        return true;
    }

    VkFormat ValInstance::find_supported_surface_format(const std::vector<VkFormat> &vk_formats) {
        for (VkFormat vk_format: vk_formats) {
            for (VkSurfaceFormatKHR vk_surface_format: vk_supported_surface_formats) {
                if (vk_surface_format.format == vk_format && vk_surface_format.colorSpace == VK_COLORSPACE_SRGB_NONLINEAR_KHR) {
                    return vk_format;
                }
            }
        }

        return VK_FORMAT_UNDEFINED;
    }

    VkFormat ValInstance::find_supported_format(const std::vector<VkFormat> &vk_formats, VkImageTiling vk_tiling, VkFormatFeatureFlags vk_feature_flags) {
        for (VkFormat vk_attempt_format: vk_formats) {
            VkFormatProperties vk_format_properties;
            vkGetPhysicalDeviceFormatProperties(vk_physical_device, vk_attempt_format, &vk_format_properties);

            if (vk_tiling == VK_IMAGE_TILING_LINEAR && (vk_format_properties.linearTilingFeatures & vk_feature_flags) == vk_feature_flags) {
                return vk_attempt_format;
            }

            if (vk_tiling == VK_IMAGE_TILING_OPTIMAL && (vk_format_properties.optimalTilingFeatures & vk_feature_flags) == vk_feature_flags) {
                return vk_attempt_format;
            }
        }

        return VK_FORMAT_UNDEFINED;
    }

    VkPresentModeKHR ValInstance::find_supported_present_mode(const std::vector<VkPresentModeKHR> &vk_present_modes) {
        for (VkPresentModeKHR vk_present_mode: vk_present_modes) {
            for (VkPresentModeKHR vk_supported_present_mode: vk_supported_present_modes) {
                if (vk_present_mode == vk_supported_present_mode) {
                    return vk_present_mode;
                }
            }
        }

        return VK_PRESENT_MODE_FIFO_KHR;
    }

    bool ValInstance::determine_present_info(const ValInstancePresentPreferences &present_preferences) {
        // Try to guess a fitting present format
        std::vector<VkFormat> vk_formats;
        std::vector<VkFormat> vk_depth_formats;

        if (present_preferences.sRGB) {
            vk_formats.push_back(VK_FORMAT_B8G8R8A8_SRGB);
        } else {
            vk_formats.push_back(VK_FORMAT_B8G8R8A8_UNORM);
        }

        if (present_preferences.depth32Bit) {
            vk_formats.push_back(VK_FORMAT_D32_SFLOAT_S8_UINT);
        }

        // We always push the D24 and D16 formats regardless
        vk_depth_formats.push_back(VK_FORMAT_D24_UNORM_S8_UINT);
        vk_depth_formats.push_back(VK_FORMAT_D16_UNORM_S8_UINT);

        // Try to guess a fitting present mode
        std::vector<VkPresentModeKHR> vk_present_modes;

        if (present_preferences.vsync) {
            vk_present_modes.push_back(VK_PRESENT_MODE_FIFO_KHR);
            vk_present_modes.push_back(VK_PRESENT_MODE_FIFO_RELAXED_KHR);
        }

        // We always want mailbox or immediate mode
        vk_present_modes.push_back(VK_PRESENT_MODE_MAILBOX_KHR);
        vk_present_modes.push_back(VK_PRESENT_MODE_IMMEDIATE_KHR);

        // Then try to find them
        VkFormat vk_color_format = find_supported_surface_format(vk_formats);
        VkFormat vk_depth_format = find_supported_format(vk_depth_formats, VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
        VkPresentModeKHR vk_present_mode = find_supported_present_mode(vk_present_modes);

        present_info = new ValPresentInfo();

        present_info->vk_color_format = vk_color_format;
        present_info->vk_depth_format = vk_depth_format;
        present_info->vk_color_space = VK_COLORSPACE_SRGB_NONLINEAR_KHR;
        present_info->vk_present_mode = vk_present_mode;

        return true;
    }

    ValInstance::ValInstance(const ValInstanceCreateInfo& create_info) {
        vk_instance = create_vk_instance(create_info);

#ifdef SDL_SUPPORT
        // TODO: Replace this stupid way of creating render targets
        ValRenderTargetCreateInfo window_create_info{};
        window_create_info.p_window = create_info.p_sdl_window;
        window_create_info.initialize_swapchain = false;
        window_create_info.type = ValRenderTargetCreateInfo::RENDER_TARGET_TYPE_WINDOW;

        ValWindowRenderTarget *main_window = static_cast<ValWindowRenderTarget *>(ValRenderTarget::create_render_target(&window_create_info, this));
#endif

        // TODO: Proper multiple window support (thanks windows for making presenting harder :| )
        ChosenGPU gpu = pick_gpu(vk_instance, main_window->vk_surface, create_info);
        vk_device = create_vk_device(gpu, gpu.queues, create_info);

        vk_physical_device = gpu.vk_device;
        val_queues = gpu.queues;

        for (ValQueue &queue: val_queues) {
            queue.create_pool(this);
        }

        vma_allocator = create_vma_allocator(vk_instance, vk_device, gpu.vk_device);

        vk_image_available_semaphore = create_vk_semaphore(vk_device);
        vk_render_finished_semaphore = create_vk_semaphore(vk_device);
        vk_render_fence = create_vk_fence(vk_device);

        vk_descriptor_pool = create_vk_descriptor_pool(vk_device);

#ifdef SDL_SUPPORT
        // We need to cache all the possible present modes and formats first
        cache_surface_info(vk_instance, main_window->vk_surface, gpu.vk_device);

        // Then we need to find an adequate that the user prefers
        // We have two ways of determining this
        // The user can use a "ValPresentPreferenceInfo" to automatically decide
        // Or the user can provide a list of formats and present modes they prefer
        if (!create_info.present_preferences) {
            // User provided formats
            VkFormat vk_color_format = find_supported_surface_format(create_info.vk_color_formats);
            VkFormat vk_depth_format = find_supported_format(create_info.vk_depth_formats,
                                                                       VK_IMAGE_TILING_OPTIMAL,
                                                                       VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
            VkPresentModeKHR vk_present_mode = find_supported_present_mode(create_info.vk_present_modes);

            present_info = new ValPresentInfo();

            present_info->vk_color_format = vk_color_format;
            present_info->vk_depth_format = vk_depth_format;
            present_info->vk_color_space = VK_COLORSPACE_SRGB_NONLINEAR_KHR;
            present_info->vk_present_mode = vk_present_mode;
        } else {
            determine_present_info(create_info.present_preferences.value());
        }

        //ValWindowRenderTarget::PresentInfo* present_info = main_window->get_present_info(gpu.vk_device);
        //instance->present_info = present_info;

        // We create a default render pass that has 1 color and 1 depth input
        // The user has access to the main render target we've created, it is up to them to initialize it
        val_main_window = main_window;
#endif
    }

    ValQueue ValInstance::get_queue(ValQueue::QueueType type) {
        for (ValQueue &queue: val_queues) {
            if (queue.type == type) {
                return queue;
            }
        }

        return {};
    }

    void ValInstance::await_frame() {
        if (vk_device != nullptr && vk_render_fence != nullptr) {
            vkWaitForFences(vk_device, 1, &vk_render_fence, VK_TRUE, UINT64_MAX);
        }
    }

    ValInstance::~ValInstance() {
        await_frame();

        if (vk_instance != nullptr) {
            for (ValQueue &queue: val_queues) {
                queue.release(this);
            }

            //vkDestroyRenderPass(vk_device, vk_render_pass, nullptr);

            vkDestroyFence(vk_device, vk_render_fence, nullptr);
            vkDestroySemaphore(vk_device, vk_image_available_semaphore, nullptr);
            vkDestroySemaphore(vk_device, vk_render_finished_semaphore, nullptr);

            vmaDestroyAllocator(vma_allocator);

            vkDestroyDescriptorPool(vk_device, vk_descriptor_pool, nullptr);

            // The main window is usually released on its own!
            //val_main_window->release(this);
            //delete val_main_window;

            vkDestroyDevice(vk_device, nullptr);
            vkDestroyInstance(vk_instance, nullptr);
        }
    }
}