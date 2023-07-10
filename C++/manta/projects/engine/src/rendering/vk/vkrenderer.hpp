#ifndef MANTA_VKRENDERER_HPP
#define MANTA_VKRENDERER_HPP

#include <vector>
#include <optional>

#include <rendering/newrenderer.hpp>

#include <vulkan/vulkan.hpp>
#include <vk_mem_alloc.h>

namespace Manta::Rendering::Vulkan {
    // A renderer that supports Vulkan context
    // Vulkan is quite versatile and can target most modern devices!
    class VkRenderer : public NewRenderer {
    public:
        //
        // Functions
        //
        void Initialize(const std::string& window_name) override;

        void InitializeImGui() override;

        void Present() override;

        //
        // Assets
        //
        ShaderBuffer* CreateShaderBuffer() override;
        MeshBuffer* CreateMeshBuffer() override;

    protected:
        //
        // Vulkan stuff
        //
        void CreateVkInstance();
        void FindVkPhysicalDevice();
        void CreateVkDevice();
        void CreateSurface();
        void CreateSwapchain();
        void CreateImageViews();
        void CreateRenderPass();
        void CreateFramebuffers();
        void CreateCommandPool();
        void CreateCommandBuffer();
        void CreateSyncObjects();
        // TODO: Generate pipelines

        // TODO: Generic recording loop
        void RecordCmdBuffer(VkCommandBuffer buf, uint32_t i);

        int RatePhysicalDevice(VkPhysicalDevice device);
        bool CheckForLayer(const std::string& layer, const std::vector<VkLayerProperties>& vk_layers);
        bool CheckForExtension(const std::string& ext, const std::vector<VkExtensionProperties>& vk_exts);


        std::vector<const char*> enabled_layers;

        VkInstance vk_instance;
        VkPhysicalDevice vk_physical_device;
        VkDevice vk_device;

        struct RenderQueues {
            std::optional<uint32_t> graphics_family, present_family;
            VkQueue graphics_queue, present_queue;

            bool IsComplete();
        };

        RenderQueues vk_queues;

        struct SwapchainDetails {
            VkSurfaceCapabilitiesKHR capabilities;
            std::vector<VkSurfaceFormatKHR> formats;
            std::vector<VkPresentModeKHR> present_modes;
        };

        struct SwapchainSettings {
            VkSurfaceFormatKHR format;
            VkPresentModeKHR present_mode;
        };

        VkSurfaceKHR vk_surface;

        SwapchainDetails vk_swapchain_details;
        SwapchainSettings vk_swapchain_settings;
        std::vector<VkImage> vk_swapchain_images;
        std::vector<VkImageView> vk_swapchain_image_views;
        std::vector<VkFramebuffer> vk_swapchain_frame_bufs;
        VkExtent2D vk_swapchain_extent;
        VkSwapchainKHR vk_swapchain;

        // TODO: Do Viewports need their own passes?
        VkRenderPass vk_renderpass;
        VkCommandPool vk_commandpool;
        VkCommandBuffer vk_commandbuffer;

        VkSemaphore vk_semaphore_image_available;
        VkSemaphore vk_semaphore_render_finished;
        VkFence vk_fence_flight;

    public:
        VmaAllocator vma_allocator;
    };
}


#endif
