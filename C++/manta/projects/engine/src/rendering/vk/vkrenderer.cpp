#include "vkrenderer.hpp"

#include <stdexcept>
#include <vector>
#include <iostream>

#include <SDL2/SDL_vulkan.h>

#include "assets/vkmeshbuffer.hpp"

#include <imgui.h>
#include <backends/imgui_impl_sdl.h>
#include <backends/imgui_impl_vulkan.h>

//
// Thanks to https://vulkan-tutorial.com for most of the code used for initialization!
//

namespace Manta::Rendering::Vulkan {
    void VkRenderer::Initialize(const std::string& window_name) {
        sdl_window = SDL_CreateWindow(
                window_name.data(),
                SDL_WINDOWPOS_UNDEFINED,
                SDL_WINDOWPOS_UNDEFINED,
                DEFAULT_WIDTH,
                DEFAULT_HEIGHT,
                SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE
        );

        //
        // Vulkan creation process, this is quite lengthy!
        //
        CreateVkInstance();
        CreateSurface();
        FindVkPhysicalDevice();
        CreateVkDevice();
        CreateSwapchain();
        CreateImageViews();
        CreateRenderPass();
        CreateFramebuffers();
        CreateCommandPool();
        CreateCommandBuffer();
        CreateSyncObjects();

        VmaAllocatorCreateInfo alloc_create_info{};
        alloc_create_info.physicalDevice = vk_physical_device;
        alloc_create_info.device = vk_device;
        alloc_create_info.instance = vk_instance;
        alloc_create_info.vulkanApiVersion = VK_API_VERSION_1_0;

        if (vmaCreateAllocator(&alloc_create_info, &vma_allocator) != VK_SUCCESS)
            throw std::runtime_error("Failed to create VmaAllocator!");

        // TODO: Vulkan cleanup code!
    }

    // Based on: https://vkguide.dev/docs/extra-chapter/implementing_imgui/
    void VkRenderer::InitializeImGui() {
        ImGui::CreateContext();

        // TODO: Should this be smaller?
        VkDescriptorPoolSize pool_sizes[] =
        {
                { VK_DESCRIPTOR_TYPE_SAMPLER, 1000 },
                { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
                { VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000 },
                { VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000 },
                { VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000 },
                { VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000 },
                { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 },
                { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 },
                { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },
                { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 },
                { VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000 }
        };

        VkDescriptorPoolCreateInfo pool_info = {};
        pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
        pool_info.maxSets = 1000;
        pool_info.poolSizeCount = std::size(pool_sizes);
        pool_info.pPoolSizes = pool_sizes;

        VkDescriptorPool pool_imgui;
        if (vkCreateDescriptorPool(vk_device, &pool_info, nullptr, &pool_imgui) != VK_SUCCESS)
            throw std::runtime_error("Failed to create VkDescriptorPool for ImGui!");

        ImGui_ImplSDL2_InitForVulkan(sdl_window);

        ImGui_ImplVulkan_InitInfo init_info = {};
        init_info.Instance = vk_instance;
        init_info.PhysicalDevice = vk_physical_device;
        init_info.Device = vk_device;
        init_info.Queue = vk_queues.graphics_queue;
        init_info.DescriptorPool = pool_imgui;
        init_info.MinImageCount = 3;
        init_info.ImageCount = 3;
        init_info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;

        ImGui_ImplVulkan_Init(&init_info, vk_renderpass);

        // TODO: Setup the rest
    }

    void VkRenderer::Present() {
        vkWaitForFences(vk_device, 1, &vk_fence_flight, VK_TRUE, UINT64_MAX);
        vkResetFences(vk_device, 1, &vk_fence_flight);

        uint32_t image_index;
        vkAcquireNextImageKHR(vk_device, vk_swapchain, UINT64_MAX, vk_semaphore_image_available, VK_NULL_HANDLE, &image_index);

        vkResetCommandBuffer(vk_commandbuffer, 0);
        RecordCmdBuffer(vk_commandbuffer, image_index);

        VkSubmitInfo submit_info{};
        submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

        VkSemaphore wait_semaphores[] = {vk_semaphore_image_available};
        VkPipelineStageFlags wait_stages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
        submit_info.waitSemaphoreCount = 1;
        submit_info.pWaitSemaphores = wait_semaphores;
        submit_info.pWaitDstStageMask = wait_stages;

        submit_info.commandBufferCount = 1;
        submit_info.pCommandBuffers = &vk_commandbuffer;

        VkSemaphore signal_semaphores[] = {vk_semaphore_render_finished};
        submit_info.signalSemaphoreCount = 1;
        submit_info.pSignalSemaphores = signal_semaphores;

        if (vkQueueSubmit(vk_queues.graphics_queue, 1, &submit_info, vk_fence_flight) != VK_SUCCESS)
            throw std::runtime_error("Failed to submit queue!");

        VkPresentInfoKHR present_info{};
        present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

        present_info.waitSemaphoreCount = 1;
        present_info.pWaitSemaphores = signal_semaphores;

        VkSwapchainKHR swapchains[] = {vk_swapchain};
        present_info.swapchainCount = 1;
        present_info.pSwapchains = swapchains;

        present_info.pImageIndices = &image_index;

        vkQueuePresentKHR(vk_queues.present_queue, &present_info);
    }

    //
    // Assets
    //
    ShaderBuffer *VkRenderer::CreateShaderBuffer() {
        return nullptr;
    }

    MeshBuffer *VkRenderer::CreateMeshBuffer() {
        return new VkMeshBuffer();
    }

    //
    // Vulkan init
    //
    void VkRenderer::CreateVkInstance() {
        VkApplicationInfo app_info {};
        app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        app_info.pApplicationName = "Manta"; // TODO: App names
        app_info.applicationVersion = VK_MAKE_VERSION(1, 0, 0); // TODO: App versioning
        app_info.pEngineName = "Manta";
        app_info.engineVersion = VK_MAKE_VERSION(1, 0, 0); // TODO: Engine versioning
        app_info.apiVersion = VK_API_VERSION_1_0;

        VkInstanceCreateInfo create_info{};
        create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        create_info.pApplicationInfo = &app_info;

        // Used example from https://wiki.libsdl.org/SDL_Vulkan_GetInstanceExtensions

        //
        // Instance extension setup
        //
        uint32_t sdl_extension_count;

        if (!SDL_Vulkan_GetInstanceExtensions(sdl_window, &sdl_extension_count, nullptr))
            throw std::runtime_error("SDL_Vulkan_GetInstanceExtensions failed!");

        // TODO: Make this generic somehow?
        std::vector<const char*> vk_extensions = {

        };

        size_t ext_offset = vk_extensions.size();
        vk_extensions.resize(ext_offset + sdl_extension_count);

        if (!SDL_Vulkan_GetInstanceExtensions(sdl_window, &sdl_extension_count, vk_extensions.data() + ext_offset))
            throw std::runtime_error("SDL_Vulkan_GetInstanceExtensions failed!");

        uint32_t ext_count = 0;
        vkEnumerateInstanceExtensionProperties(nullptr, &ext_count, nullptr);

        std::vector<VkExtensionProperties> extensions(ext_count);
        vkEnumerateInstanceExtensionProperties(nullptr, &ext_count, extensions.data());

#ifdef DEBUG
        std::cout << "Vulkan: Listing supported instance extensions..." << std::endl;

        for (auto ext : extensions)
            std::cout << "  " << ext.extensionName << std::endl;
#endif

        // TODO: Check for missing extensions we want / need

        create_info.enabledExtensionCount = vk_extensions.size();
        create_info.ppEnabledExtensionNames = vk_extensions.data();

        //
        // Instance layer setup
        //
        std::vector<const char*> vk_layers = {
#ifdef DEBUG
                "VK_LAYER_KHRONOS_validation",
#endif
        };

        std::vector<bool> has_layers = {

        };

        uint32_t inst_layer_count = 0;
        vkEnumerateInstanceLayerProperties(&inst_layer_count, nullptr);

        std::vector<VkLayerProperties> layers(inst_layer_count);
        vkEnumerateInstanceLayerProperties(&inst_layer_count, layers.data());

#ifdef DEBUG
        std::cout << "Vulkan: Listing instance layers..." << std::endl;

        for (auto layer : layers)
            std::cout << "  " << layer.layerName << std::endl;
#endif

        for (auto layer : vk_layers) {
            has_layers.emplace_back(CheckForLayer(layer, layers));
        }

        for (int l = vk_layers.size() - 1; l >= 0; l--) {
            if (!has_layers[l]) {
#ifdef DEBUG
                std::cout << "Vulkan: Missing layer '" << vk_layers[l] << "'" << std::endl;
#endif

                vk_layers.erase(vk_layers.begin() + l);
            } else
                enabled_layers.emplace_back(vk_layers[l]);
        }

        create_info.enabledLayerCount = vk_layers.size();
        create_info.ppEnabledLayerNames = vk_layers.data();

        if (vkCreateInstance(&create_info, nullptr, &vk_instance) != VK_SUCCESS)
            throw std::runtime_error("vkCreateInstance failed!");
    }

    void VkRenderer::FindVkPhysicalDevice() {
        uint32_t device_count = 0;
        vkEnumeratePhysicalDevices(vk_instance, &device_count, nullptr);

        std::vector<VkPhysicalDevice> devices(device_count);
        vkEnumeratePhysicalDevices(vk_instance, &device_count, devices.data());

#ifdef DEBUG
        std::cout << "Vulkan: Listing and ranking physical devices!" << std::endl;
        uint32_t device_no = 0;
#endif

        VkPhysicalDevice best_device = nullptr;
        int highest_score = 0;

        for (auto device : devices) {
#ifdef DEBUG
            std::cout << "  Device " << device_no++ << ":" << std::endl;
#endif
            int score = RatePhysicalDevice(device);

            if (best_device == nullptr || score > highest_score) {
                highest_score = score;
                best_device = device;
            }
        }

        if (!best_device)
            throw std::runtime_error("No vulkan supported GPUs were found!");

        vk_physical_device = best_device;
    }

    void VkRenderer::CreateVkDevice() {
        uint32_t queue_family_count = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(vk_physical_device, &queue_family_count, nullptr);

        std::vector<VkQueueFamilyProperties> queue_families(queue_family_count);
        vkGetPhysicalDeviceQueueFamilyProperties(vk_physical_device, &queue_family_count, queue_families.data());

        uint32_t i = 0;
        for (auto family : queue_families) {
            if (family.queueFlags & VK_QUEUE_GRAPHICS_BIT)
                vk_queues.graphics_family = i;

            VkBool32 present_support = false;
            vkGetPhysicalDeviceSurfaceSupportKHR(vk_physical_device, i, vk_surface, &present_support);

            if (present_support) {
                bool is_unique = true;

                if (vk_queues.graphics_family.has_value())
                    if (vk_queues.graphics_family.value() == i)
                        is_unique = false;

                if (is_unique)
                    vk_queues.present_family = i;
            }

            if (vk_queues.IsComplete())
                break;

            i++;
        }

        std::vector<VkDeviceQueueCreateInfo> queue_create_infos;
        std::vector<uint32_t> queues = { vk_queues.graphics_family.value(), vk_queues.present_family.value() };

        float queue_priority = 1.0f;
        for (uint32_t queue : queues) {
            VkDeviceQueueCreateInfo queue_create_info{};

            queue_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            queue_create_info.queueFamilyIndex = queue;
            queue_create_info.queueCount = 1;
            queue_create_info.pQueuePriorities = &queue_priority;

            queue_create_infos.emplace_back(queue_create_info);
        }

        VkDeviceCreateInfo create_info{};
        create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

        create_info.pQueueCreateInfos = queue_create_infos.data();
        create_info.queueCreateInfoCount = queue_create_infos.size();

        // TODO: Actually toggle features we might want to use!
        VkPhysicalDeviceFeatures device_features{};

        create_info.pEnabledFeatures = &device_features;

        create_info.enabledLayerCount = enabled_layers.size();
        create_info.ppEnabledLayerNames = enabled_layers.data();

        // TODO: Check if an extension is supported first!
        std::vector<const char*> extensions = {
            VK_KHR_SWAPCHAIN_EXTENSION_NAME
        };

        create_info.enabledExtensionCount = extensions.size();
        create_info.ppEnabledExtensionNames = extensions.data();

        if (vkCreateDevice(vk_physical_device, &create_info, nullptr, &vk_device) != VK_SUCCESS)
            throw std::runtime_error("Failed to create VkDevice!");

        vkGetDeviceQueue(vk_device, vk_queues.graphics_family.value(), 0, &vk_queues.graphics_queue);
        vkGetDeviceQueue(vk_device, vk_queues.present_family.value(), 0, &vk_queues.present_queue);
    }

    // TODO: Does this need to do more?
    void VkRenderer::CreateSurface() {
        if (!SDL_Vulkan_CreateSurface(sdl_window, vk_instance, &vk_surface))
            throw std::runtime_error("Failed to create window surface!");
    }

    void VkRenderer::CreateSwapchain() {
        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(vk_physical_device, vk_surface, &vk_swapchain_details.capabilities);

        uint32_t format_count;
        vkGetPhysicalDeviceSurfaceFormatsKHR(vk_physical_device, vk_surface, &format_count, nullptr);

        vk_swapchain_details.formats.resize(format_count);
        vkGetPhysicalDeviceSurfaceFormatsKHR(vk_physical_device, vk_surface, &format_count, vk_swapchain_details.formats.data());

        uint32_t present_mode_count;
        vkGetPhysicalDeviceSurfacePresentModesKHR(vk_physical_device, vk_surface, &present_mode_count, nullptr);

        vk_swapchain_details.present_modes.resize(present_mode_count);
        vkGetPhysicalDeviceSurfacePresentModesKHR(vk_physical_device, vk_surface, &present_mode_count, vk_swapchain_details.present_modes.data());

#ifdef DEBUG
        std::cout << "Vulkan: Listing supported surface formats!" << std::endl << std::hex;

        for (auto format : vk_swapchain_details.formats) {
            std::cout << "  0x" << format.format << std::endl;
        }

        std::cout << "Vulkan: Listing supported present modes!" << std::endl;

        for (auto modes : vk_swapchain_details.present_modes) {
            std::cout << "  0x" << modes << std::endl;
        }

        std::cout << std::dec << std::flush;
#endif

        // TODO: Other swapchain formats?
        bool found_format = false, found_mode = false;
        for (auto format : vk_swapchain_details.formats) {
            if (format.format == VK_FORMAT_B8G8R8A8_UNORM && format.colorSpace == VK_COLORSPACE_SRGB_NONLINEAR_KHR) {
                vk_swapchain_settings.format = format;
                found_format = true;
            }
        }

        for (auto mode : vk_swapchain_details.present_modes) {
            if (mode == VK_PRESENT_MODE_MAILBOX_KHR) {
                vk_swapchain_settings.present_mode = mode;
                found_mode = true;
            }
        }

        if (!found_format)
            throw std::runtime_error("Failed to find a suitable swapchain format!");

        if (!found_mode)
            throw std::runtime_error("Failed to find a suitable present mode!");

        // TODO: HiDPI?
        VkExtent2D swap_extent {};

        swap_extent.width = vk_swapchain_details.capabilities.currentExtent.width;
        swap_extent.height = vk_swapchain_details.capabilities.currentExtent.height;

        uint32_t image_count = vk_swapchain_details.capabilities.minImageCount + 1;

        if (image_count > vk_swapchain_details.capabilities.maxImageCount && vk_swapchain_details.capabilities.maxImageCount > 0)
            image_count = vk_swapchain_details.capabilities.maxImageCount;

        VkSwapchainCreateInfoKHR create_info{};
        create_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        create_info.surface = vk_surface;
        create_info.minImageCount = image_count;
        create_info.imageFormat = vk_swapchain_settings.format.format;
        create_info.imageColorSpace = vk_swapchain_settings.format.colorSpace;
        create_info.imageExtent = swap_extent;
        create_info.imageArrayLayers = 1;
        create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

        uint32_t queue_families[] = { vk_queues.graphics_family.value(), vk_queues.present_family.value() };
        if (vk_queues.graphics_family != vk_queues.present_family) {
            create_info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
            create_info.queueFamilyIndexCount = 2;
            create_info.pQueueFamilyIndices = queue_families;
        } else {
            create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
            create_info.queueFamilyIndexCount = 0;
            create_info.pQueueFamilyIndices = nullptr;
        }

        create_info.preTransform = vk_swapchain_details.capabilities.currentTransform;
        create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
        create_info.presentMode = vk_swapchain_settings.present_mode;
        create_info.clipped = VK_TRUE;
        create_info.oldSwapchain = nullptr;

        if (vkCreateSwapchainKHR(vk_device, &create_info, nullptr, &vk_swapchain) != VK_SUCCESS)
            throw std::runtime_error("Failed to create VkSwapchain!");

        vkGetSwapchainImagesKHR(vk_device, vk_swapchain, &image_count, nullptr);
        vk_swapchain_images.resize(image_count);
        vkGetSwapchainImagesKHR(vk_device, vk_swapchain, &image_count, vk_swapchain_images.data());

        vk_swapchain_extent = swap_extent;

#ifdef DEBUG
        std::cout << "Vulkan: Swapchain has " << image_count << " images!" << std::endl;
#endif
    }

    void VkRenderer::CreateImageViews() {
        vk_swapchain_image_views.resize(vk_swapchain_images.size());

        for (auto i = 0; i < vk_swapchain_images.size(); i++) {
            VkImageViewCreateInfo create_info{};
            create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            create_info.image = vk_swapchain_images[i];

            create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
            create_info.format = vk_swapchain_settings.format.format;

            create_info.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
            create_info.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
            create_info.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
            create_info.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

            create_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            create_info.subresourceRange.baseMipLevel = 0;
            create_info.subresourceRange.levelCount = 1;
            create_info.subresourceRange.baseArrayLayer = 0;
            create_info.subresourceRange.layerCount = 1;

            if (vkCreateImageView(vk_device, &create_info, nullptr, &vk_swapchain_image_views[i]) != VK_SUCCESS) {
                throw std::runtime_error("Failed to create VkImageView!");
            }

        }
    }

    void VkRenderer::CreateRenderPass() {
        VkAttachmentDescription color_attachment{};
        color_attachment.format = vk_swapchain_settings.format.format;
        color_attachment.samples = VK_SAMPLE_COUNT_1_BIT;

        color_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        color_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;

        color_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        color_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

        color_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        color_attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

        VkAttachmentReference color_attachment_ref{};

        color_attachment_ref.attachment = 0;
        color_attachment_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        VkSubpassDescription subpass {};
        subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;

        subpass.colorAttachmentCount = 1;
        subpass.pColorAttachments = &color_attachment_ref;

        VkRenderPassCreateInfo renderpass_info{};
        renderpass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        renderpass_info.attachmentCount = 1;
        renderpass_info.pAttachments = &color_attachment;
        renderpass_info.subpassCount = 1;
        renderpass_info.pSubpasses = &subpass;

        if (vkCreateRenderPass(vk_device, &renderpass_info, nullptr, &vk_renderpass) != VK_SUCCESS)
            throw std::runtime_error("failed to create render pass!");
    }

    void VkRenderer::CreateFramebuffers() {
        vk_swapchain_frame_bufs.resize(vk_swapchain_images.size());

        for (auto i = 0; i < vk_swapchain_images.size(); i++) {
            VkImageView attachments[] = {
                    vk_swapchain_image_views[i]
            };

            VkFramebufferCreateInfo framebuffer_info{};
            framebuffer_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
            framebuffer_info.renderPass = vk_renderpass;
            framebuffer_info.attachmentCount = 1;
            framebuffer_info.pAttachments = attachments;
            framebuffer_info.width = vk_swapchain_extent.width;
            framebuffer_info.height = vk_swapchain_extent.height;
            framebuffer_info.layers = 1;

            if (vkCreateFramebuffer(vk_device, &framebuffer_info, nullptr, &vk_swapchain_frame_bufs[i]) != VK_SUCCESS) {
                throw std::runtime_error("Failed to create VkFramebuffer!");
            }
        }
    }

    void VkRenderer::CreateCommandPool() {
        VkCommandPoolCreateInfo pool_info{};
        pool_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        pool_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        pool_info.queueFamilyIndex = vk_queues.graphics_family.value();

        if (vkCreateCommandPool(vk_device, &pool_info, nullptr, &vk_commandpool) != VK_SUCCESS)
            throw std::runtime_error("Failed to create VkCommandPool!");
    }

    void VkRenderer::CreateCommandBuffer() {
        VkCommandBufferAllocateInfo alloc_info{};
        alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        alloc_info.commandPool = vk_commandpool;
        alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        alloc_info.commandBufferCount = 1;

        if (vkAllocateCommandBuffers(vk_device, &alloc_info, &vk_commandbuffer) != VK_SUCCESS)
            throw std::runtime_error("Failed to allocate CommandBuffer!");
    }

    void VkRenderer::CreateSyncObjects() {
        VkSemaphoreCreateInfo semaphore_info{};
        semaphore_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

        VkFenceCreateInfo fence_info{};
        fence_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fence_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;

        auto result =
                vkCreateSemaphore(vk_device, &semaphore_info, nullptr, &vk_semaphore_image_available) ||
                vkCreateSemaphore(vk_device, &semaphore_info, nullptr, &vk_semaphore_render_finished) ||
                vkCreateFence(vk_device, &fence_info, nullptr, &vk_fence_flight);
    }

    //
    // Recording
    //

    void VkRenderer::RecordCmdBuffer(VkCommandBuffer buf, uint32_t i) {
        VkCommandBufferBeginInfo begin_info{};
        begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

        if (vkBeginCommandBuffer(vk_commandbuffer, &begin_info) != VK_SUCCESS)
            throw std::runtime_error("Failed to begin recording into command buffer!");

        VkRenderPassBeginInfo renderpass_info{};
        renderpass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;

        renderpass_info.renderPass = vk_renderpass;
        renderpass_info.framebuffer = vk_swapchain_frame_bufs[i];

        renderpass_info.renderArea.offset = {0, 0};
        renderpass_info.renderArea.extent = vk_swapchain_extent;

        VkClearValue clear_color = {{{0.1f, 0.1f, 0.1f, 1.0f}}};
        renderpass_info.clearValueCount = 1;
        renderpass_info.pClearValues = &clear_color;

        vkCmdBeginRenderPass(vk_commandbuffer, &renderpass_info, VK_SUBPASS_CONTENTS_INLINE);

        vkCmdEndRenderPass(vk_commandbuffer);

        if (vkEndCommandBuffer(vk_commandbuffer) != VK_SUCCESS)
            throw std::runtime_error("failed to record command buffer!");
    }

    //
    // Helpers
    //
    // TODO: Check if we need certain features?
    int VkRenderer::RatePhysicalDevice(VkPhysicalDevice device) {
        VkPhysicalDeviceProperties device_props;
        vkGetPhysicalDeviceProperties(device, &device_props);

        VkPhysicalDeviceFeatures device_features;
        vkGetPhysicalDeviceFeatures(device, &device_features);

        int score = 0;

        // Score immediately by manufacturer
        const int AMD_ID = 0x1002;
        const int NVIDIA_ID = 0x10DE;
        // TODO: Mobile GPUs?

        // TODO: The day when Intel releases a dedicated GPU, add them here :)
        if (device_props.vendorID == AMD_ID || device_props.vendorID == NVIDIA_ID)
            score += 100;

        if (device_props.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
            score += 100;


#ifdef DEBUG
        std::cout << "    Name: " << device_props.deviceName << std::endl;

        if (device_props.vendorID == AMD_ID)
            std::cout << "    Vendor: AMD" << std::endl;
        else if (device_props.vendorID == AMD_ID)
            std::cout << "    Vendor: Nvidia" << std::endl;
        else
            std::cout << "    Vendor: Unknown" << std::endl;

        std::cout << "\n    Score: " << score << std::endl;
#endif

        return 0;
    }

    bool VkRenderer::CheckForLayer(const std::string &layer, const std::vector<VkLayerProperties> &vk_layers) {
        for (auto vk_layer : vk_layers) {
            if (strcmp(layer.c_str(), vk_layer.layerName) == 0) {
                return true;
            }
        }

        return false;
    }

    // TODO:
    bool VkRenderer::CheckForExtension(const std::string &ext, const std::vector<VkExtensionProperties> &vk_exts) {
        return false;
    }

    //
    // Render Queues
    //
    bool VkRenderer::RenderQueues::IsComplete() {
        return graphics_family.has_value() && present_family.has_value();
    }
}