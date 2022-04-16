//
// Created by kafka on 2/3/2022.
//

#include <window/win_window_generic.h>

namespace xgk::win
{
    template<bool IsMainWindow, bool IsValidationEnabled>
    GenericWindow<IsMainWindow, IsValidationEnabled>::GenericWindow(u32 width, u32 height, std::string_view title)
            : m_title{ title }
            , m_width{ width }
            , m_height{ height }
            , m_window{ nullptr }
            , m_isShown{ false }
    {
        init();
    }

    template<bool IsMainWindow, bool IsValidationEnabled>
    GenericWindow<IsMainWindow, IsValidationEnabled>::~GenericWindow() noexcept
    {
        glfwDestroyWindow(m_window);
        glfwTerminate();
    }

    template<bool IsMainWindow, bool IsValidationEnabled>
    void
    GenericWindow<IsMainWindow, IsValidationEnabled>::close()
    {
        if(m_isShown)
        {
            if constexpr(IsValidationEnabled)
            {
                log::info("Closing window");
            }

            glfwWindowShouldClose(m_window);

            m_isShown = false;
        }
    }

    template<bool IsMainWindow, bool IsValidationEnabled>
    void
    GenericWindow<IsMainWindow, IsValidationEnabled>::init()
    {
        if constexpr(IsValidationEnabled)
        {
            log::info("Creating window with Width {} and Height {}", m_width, m_height);
        }

        glfwInit();
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

        m_window = glfwCreateWindow(static_cast<int>(m_width), static_cast<int>(m_height), m_title.c_str(), nullptr, nullptr);

        glfwSetWindowUserPointer(m_window, this);

        //callback functions
        glfwSetFramebufferSizeCallback(m_window, frameBufferResizeCallback);
        glfwSetKeyCallback(m_window, keyHandlerCallBack);
        glfwSetWindowCloseCallback(m_window, windowCloseCallback);
    }

    template<bool IsMainWindow, bool IsValidationEnabled>
    void
    GenericWindow<IsMainWindow, IsValidationEnabled>::frameBufferResizeCallback(WindowEngine* window, int width, int height)
    {
        auto _this = static_cast<GenericWindow*>(glfwGetWindowUserPointer(window));

        _this->resizedEvent(static_cast<u32>(width), static_cast<u32>(height));
    }

    template<bool IsMainWindow, bool IsValidationEnabled>
    void
    GenericWindow<IsMainWindow, IsValidationEnabled>::keyHandlerCallBack(WindowEngine* window, int key, int scancode, int action, int mods)
    {
        auto _this = static_cast<GenericWindow*>(glfwGetWindowUserPointer(window));

        _this->keyPressedEvent(static_cast<Key>(key), scancode, action, mods);
    }

    template<bool IsMainWindow, bool IsValidationEnabled>
    void
    GenericWindow<IsMainWindow, IsValidationEnabled>::windowCloseCallback(WindowEngine* window)
    {
        auto _this = static_cast<GenericWindow*>(glfwGetWindowUserPointer(window));

        _this->closeEvent();
    }

    template<bool IsMainWindow, bool IsValidationEnabled>
    void
    GenericWindow<IsMainWindow, IsValidationEnabled>::resize(u32 width, u32 height)
    {
        //TODO check if this is not more then size of desktop ...
        m_width = width;
        m_height = height;

        glfwSetWindowSize(m_window, static_cast<int>(width), static_cast<int>(height));
    }

    template<bool IsMainWindow, bool IsValidationEnabled>
    void
    GenericWindow<IsMainWindow, IsValidationEnabled>::show()
    {
        if(!m_isShown)
        {
            if constexpr(IsValidationEnabled)
            {
                log::info("Popping up window");
            }

            glfwShowWindow(m_window);

            m_isShown = true;
        }
    }

    template<bool IsMainWindow, bool IsValidationEnabled>
    void

    GenericWindow<IsMainWindow, IsValidationEnabled>::showFullWindow()
    {

    }

    template<bool IsMainWindow, bool IsValidationEnabled>
    VkSurfaceKHR
    GenericWindow<IsMainWindow, IsValidationEnabled>::createVulkanWindowSurface(const VkInstance& vulkanInstance) const
    {
        if constexpr(IsValidationEnabled)
        {
            log::info("Creating vulkan window surface");
        }

        VkSurfaceKHR surface;

        if(glfwCreateWindowSurface(vulkanInstance, m_window, nullptr, &surface) != VK_SUCCESS)
        {
            throw Exception::VulkanSurfaceError("Could not create Vulkan surface");
        }

        return surface;
    }

    template<bool IsMainWindow, bool IsValidationEnabled>
    std::vector<const char*>
    GenericWindow<IsMainWindow, IsValidationEnabled>::requiredWindowExtensions() const
    {
        if constexpr(IsValidationEnabled)
        {
            log::info("Obtaining required window extensions");
        }

        u32 extensionCount = 0;

        const char** extensions = glfwGetRequiredInstanceExtensions(&extensionCount);

        std::vector<const char*> returnValue(extensions, extensions + extensionCount);

        if constexpr(IsValidationEnabled)
        {
            returnValue.emplace_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

            log::info("Extensions:");

            for(const auto& name : returnValue)
            {
                log::info("{}", name);
            }
        }

        if(extensions == nullptr || returnValue.empty())
        {
            throw Exception::WindowError("Could not obtain extensions");
        }

        return returnValue;
    }
    template<bool IsMainWindow, bool IsValidationEnabled>
    void
    GenericWindow<IsMainWindow, IsValidationEnabled>::update()
    {
        glfwPollEvents();
    }

    template class GenericWindow<true, true>;
    template class GenericWindow<true, false>;
    template class GenericWindow<false, true>;
    template class GenericWindow<false, false>;
}