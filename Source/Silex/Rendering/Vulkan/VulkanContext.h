
#pragma once

#include "Rendering/RenderingContext.h"
#include <vulkan/vulkan.h>


namespace Silex
{
#define VK_LAYER_KHRONOS_VALIDATION_NAME "VK_LAYER_KHRONOS_validation"

#define GET_VULKAN_INSTANCE_PROC(instance, func) (PFN_##func)vkGetInstanceProcAddr(instance, #func);
#define GET_VULKAN_DEVICE_PROC(device, func)     (PFN_##func)vkGetDeviceProcAddr(device, #func);

#define SL_CHECK_VKRESULT(result, retVal)                                                         \
    if (VkResult SL_COMBINE(vkres, __LINE__) = result; SL_COMBINE(vkres, __LINE__) != VK_SUCCESS) \
    {                                                                                             \
        SL_LOG_LOCATION_ERROR(VkResultToString(SL_COMBINE(vkres, __LINE__)));                     \
        return retVal;                                                                            \
    }


    //=============================================
    // Vulkan ユーティリティ 関数
    //=============================================
    const char* VkResultToString(VkResult result);

    //=============================================
    // Vulkan 構造体
    //=============================================
    struct VulkanSurface : public Surface
    {
        VkSurfaceKHR surface   = nullptr;
        uint32       width     = 0;
        uint32       height    = 0;
        VSyncMode    vsyncMode = VSYNC_MODE_DISABLED;
    };

    //=============================================
    // Vulkan コンテキスト
    //=============================================
    class VulkanContext : public RenderingContext
    {
        SL_CLASS(VulkanContext, RenderingContext)

    public:

        VulkanContext();
        ~VulkanContext();

        // 各プラットフォームの Vulkanサーフェース 拡張機能名
        virtual const char* GetPlatformSurfaceExtensionName() = 0;

    public:

        // 初期化
        bool Initialize(bool enableValidation) override;

        // API実装
        RenderingAPI* CreateRendringAPI() override;
        void DestroyRendringAPI(RenderingAPI* api) override;

        // デバイス情報
        const DeviceInfo& GetDeviceInfo() const override;

    public:

        // プレゼント命令のサポート
        bool QueueHasPresent(Surface* surface, uint32 queueIndex) const;

        // 各種プロパティ取得
        const std::vector<VkQueueFamilyProperties>& GetQueueFamilyProperties() const;
        const std::vector<const char*>& GetEnabledInstanceExtensions() const;
        const std::vector<const char*>& GetEnabledDeviceExtensions() const;

        // 物理デバイス
        VkPhysicalDevice GetPhysicalDevice() const;

        // インスタンス
        VkInstance GetInstance() const;

    public:

        struct ExtensionFunctions
        {
            PFN_vkGetPhysicalDeviceSurfaceSupportKHR      GetPhysicalDeviceSurfaceSupportKHR      = nullptr;
            PFN_vkGetPhysicalDeviceSurfaceCapabilitiesKHR GetPhysicalDeviceSurfaceCapabilitiesKHR = nullptr;
            PFN_vkGetPhysicalDeviceSurfaceFormatsKHR      GetPhysicalDeviceSurfaceFormatsKHR      = nullptr;
            PFN_vkGetPhysicalDeviceSurfacePresentModesKHR GetPhysicalDeviceSurfacePresentModesKHR = nullptr;
        } extensionFunctions;

        const ExtensionFunctions& GetExtensionFunctions() const;

    protected:

        // デバイス情報
        DeviceInfo deviceInfo;

        // インスタンス
        VkInstance               instance       = nullptr;
        VkDebugUtilsMessengerEXT debugMessenger = nullptr;

        // 物理デバイス
        VkPhysicalDevice                     physicalDevice        = nullptr;
        std::vector<VkQueueFamilyProperties> queueFamilyProperties = {};

        // デバッグメッセンジャー関数ポインタ
        PFN_vkCreateDebugUtilsMessengerEXT  CreateDebugUtilsMessengerEXT  = nullptr;
        PFN_vkDestroyDebugUtilsMessengerEXT DestroyDebugUtilsMessengerEXT = nullptr;

        // レイヤー
        std::unordered_set<std::string> requestLayers;
        std::vector<const char*>        enableLayers;

        // インスタンス拡張
        std::unordered_set<std::string> requestInstanceExtensions;
        std::vector<const char*>        enableInstanceExtensions;

        // デバイス拡張
        std::unordered_set<std::string> requestDeviceExtensions;
        std::vector<const char*>        enableDeviceExtensions;
    };
}
